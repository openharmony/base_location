/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "locator_background_proxy.h"

#include <thread>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location_config_manager.h"
#include "location_log.h"
#include "locator_ability.h"
#include "request_manager.h"
#include "permission_manager.h"
#include "location_data_rdb_manager.h"

#include "accesstoken_kit.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"

#ifdef BGTASKMGR_SUPPORT
#include "background_mode.h"
#include "background_task_mgr_helper.h"
#endif

#ifdef FMSKIT_NATIVE_SUPPORT
#include "form_mgr.h"
#endif

namespace OHOS {
namespace Location {
std::mutex LocatorBackgroundProxy::requestListMutex_;
std::mutex LocatorBackgroundProxy::locatorMutex_;
const uint32_t EVENT_STARTLOCATING = 0x0100;
const uint32_t EVENT_STOPLOCATING = 0x0200;

LocatorBackgroundProxy* LocatorBackgroundProxy::GetInstance()
{
    static LocatorBackgroundProxy data;
    return &data;
}

LocatorBackgroundProxy::LocatorBackgroundProxy()
{
    InitArgsFromProp();
    locatorBackgroundHandler_ = std::make_shared<LocatorBackgroundHandler>(AppExecFwk::EventRunner::Create(true,
        AppExecFwk::ThreadMode::FFRT));
    if (!featureSwitch_) {
        return;
    }
    requestsMap_ = std::make_shared<std::map<int32_t, std::shared_ptr<std::list<std::shared_ptr<Request>>>>>();
    requestsList_ = std::make_shared<std::list<std::shared_ptr<Request>>>();
    CommonUtils::GetCurrentUserId(curUserId_);
    requestsMap_->insert(make_pair(curUserId_, requestsList_));

    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetTimeInterval(timeInterval_);
    callback_ = sptr<mLocatorCallback>(new (std::nothrow) LocatorBackgroundProxy::mLocatorCallback());
    if (callback_ == nullptr) {
        return;
    }
    request_ = std::make_shared<Request>();
    if (request_ == nullptr) {
        return;
    }
    request_->SetUid(SYSTEM_UID);
    request_->SetPid(getpid());
    request_->SetPackageName(PROC_NAME);
    request_->SetRequestConfig(*requestConfig);
    request_->SetLocatorCallBack(callback_);
    request_->SetUuid(PROC_NAME);
    request_->SetTokenId(IPCSkeleton::GetCallingTokenID());
    request_->SetTokenIdEx(IPCSkeleton::GetCallingFullTokenID());
    SubscribeSaStatusChangeListerner();
    isUserSwitchSubscribed_ = LocatorBackgroundProxy::UserSwitchSubscriber::Subscribe();
    proxySwtich_ = (LocationDataRdbManager::QuerySwitchState() == ENABLED);
    RegisterAppStateObserver();
}

LocatorBackgroundProxy::~LocatorBackgroundProxy()
{
    UnregisterAppStateObserver();
}

// modify the parameters, in order to make the test easier
void LocatorBackgroundProxy::InitArgsFromProp()
{
    featureSwitch_ = 1;
    timeInterval_ = DEFAULT_TIME_INTERVAL;
}

void LocatorBackgroundProxy::SubscribeSaStatusChangeListerner()
{
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    if (subscriber_ == nullptr) {
        subscriber_ = std::make_shared<UserSwitchSubscriber>(subscriberInfo);
    }
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    statusChangeListener_ = new (std::nothrow) SystemAbilityStatusChangeListener(subscriber_);
    if (samgrProxy == nullptr || statusChangeListener_ == nullptr) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY,
            "SubscribeSaStatusChangeListerner samgrProxy or statusChangeListener_ is nullptr");
        return;
    }
    int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY,
        "SubscribeSaStatusChangeListerner SubscribeSystemAbility COMMON_EVENT_SERVICE_ID result:%{public}d", ret);
}

void LocatorBackgroundProxy::StartLocatorThread()
{
    auto requestManager = RequestManager::GetInstance();
    std::unique_lock<std::mutex> lock(locatorMutex_, std::defer_lock);
    lock.lock();
    if (isLocating_ || !proxySwtich_) {
        LBSLOGD(LOCATOR_BACKGROUND_PROXY, "cancel locating");
        lock.unlock();
        return;
    }
    isLocating_ = true;
    lock.unlock();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "real start locating");
    requestManager->HandleStartLocating(request_);
}

void LocatorBackgroundProxy::StopLocatorThread()
{
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_lock<std::mutex> lock(locatorMutex_, std::defer_lock);
    lock.lock();
    if (!isLocating_) {
        lock.unlock();
        return;
    }
    isLocating_ = false;
    lock.unlock();
    locatorAbility->StopLocating(callback_);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "end locating");
}

void LocatorBackgroundProxy::StopLocator()
{
    locatorBackgroundHandler_->SendHighPriorityEvent(EVENT_STOPLOCATING, 0, 0);
}

void LocatorBackgroundProxy::StartLocator()
{
    locatorBackgroundHandler_->SendHighPriorityEvent(EVENT_STARTLOCATING, 0, 0);
    locatorBackgroundHandler_->SendHighPriorityEvent(EVENT_STOPLOCATING, 0, DEFAULT_TIMEOUT_5S);
}

void LocatorBackgroundProxy::UpdateListOnRequestChange(const std::shared_ptr<Request>& request)
{
}

// called when the app freezes or wakes up
// When the app enters frozen state, start proxy
// when the app wakes up, stop proxy
void LocatorBackgroundProxy::OnSuspend(const std::shared_ptr<Request>& request, bool active)
{
}

// called when SA switch on or switch off
// when switch on, start proxy
// when switch off, stop proxy
void LocatorBackgroundProxy::OnSaStateChange(bool enable)
{
}

// called when deleteRequest called from locator ability (e.g. app stop locating)
void LocatorBackgroundProxy::OnDeleteRequestRecord(const std::shared_ptr<Request>& request)
{
}

bool LocatorBackgroundProxy::CheckPermission(const std::shared_ptr<Request>& request) const
{
    uint32_t tokenId = request->GetTokenId();
    uint32_t firstTokenId = request->GetFirstTokenId();
    return ((PermissionManager::CheckLocationPermission(tokenId, firstTokenId) ||
            PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) &&
            PermissionManager::CheckBackgroundPermission(tokenId, firstTokenId));
}

void LocatorBackgroundProxy::UpdateListOnSuspend(const std::shared_ptr<Request>& request, bool active)
{
}

void LocatorBackgroundProxy::UpdateListOnUserSwitch(int32_t userId)
{
    std::unique_lock lock(requestListMutex_);
    auto iter = requestsMap_->find(userId);
    if (iter == requestsMap_->end()) {
        auto mRequestsList = std::make_shared<std::list<std::shared_ptr<Request>>>();
        requestsMap_->insert(make_pair(userId, mRequestsList));
        LBSLOGD(LOCATOR_BACKGROUND_PROXY, "add requsetlist on user:%{public}d", userId);
    }
    // if change to another user, proxy requestList should change
    requestsList_ = (*requestsMap_)[userId];
    curUserId_ = userId;
}


const std::list<std::shared_ptr<Request>>& LocatorBackgroundProxy::GetRequestsInProxy() const
{
    return *requestsList_;
}

// called in LocatorCallbackProxy::OnLocationReport
// check if callback is from proxy
bool LocatorBackgroundProxy::IsCallbackInProxy(const sptr<ILocatorCallback>& callback) const
{
    if (!featureSwitch_) {
        return false;
    }
    std::unique_lock lock(requestListMutex_);
    for (auto request : *requestsList_) {
        if (request->GetLocatorCallBack() == callback) {
            return true;
        }
    }
    return false;
}

int32_t LocatorBackgroundProxy::GetUserId(int32_t uid) const
{
    int userId = 0;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    return userId;
}

void LocatorBackgroundProxy::OnUserSwitch(int32_t userId)
{
    UpdateListOnUserSwitch(userId);
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->ApplyRequests(0);
    }
}

void LocatorBackgroundProxy::OnUserRemove(int32_t userId)
{
    // if user is removed, remove the requestList from the user in requestsMap
    std::unique_lock lock(requestListMutex_);
    auto iter = requestsMap_->find(userId);
    if (iter != requestsMap_->end()) {
        requestsMap_->erase(iter);
        LBSLOGD(LOCATOR_BACKGROUND_PROXY, "erase requsetlist on user:%{public}d", userId);
    }
}

// limit the number of requests per app
bool LocatorBackgroundProxy::CheckMaxRequestNum(pid_t uid, const std::string& packageName) const
{
    int32_t num = 0;
    auto iter = requestsMap_->find(GetUserId(uid));
    if (iter == requestsMap_->end()) {
        return false;
    }
    for (auto request : *(iter->second)) {
        if (request->GetUid() == uid && packageName.compare(request->GetPackageName()) == 0) {
            if (++num >= REQUESTS_NUM_MAX) {
                return false;
            }
        }
    }
    return true;
}

void LocatorBackgroundProxy::mLocatorCallback::OnLocationReport(const std::unique_ptr<Location>& location)
{
    LBSLOGD(LOCATOR_BACKGROUND_PROXY, "locator background OnLocationReport");
    LocatorBackgroundProxy::GetInstance()->StopLocator();
}

void LocatorBackgroundProxy::mLocatorCallback::OnLocatingStatusChange(const int status)
{
}

void LocatorBackgroundProxy::mLocatorCallback::OnErrorReport(const int errorCode)
{
}

LocatorBackgroundProxy::UserSwitchSubscriber::UserSwitchSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info)
{
    LBSLOGD(LOCATOR_BACKGROUND_PROXY, "create UserSwitchEventSubscriber");
}

void LocatorBackgroundProxy::UserSwitchSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    int32_t userId = event.GetCode();
    const auto action = event.GetWant().GetAction();
    auto locatorProxy = LocatorBackgroundProxy::GetInstance();
    LBSLOGD(LOCATOR_BACKGROUND_PROXY, "action = %{public}s, userId = %{public}d", action.c_str(), userId);
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        locatorProxy->OnUserSwitch(userId);
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        locatorProxy->OnUserRemove(userId);
    }
}

bool LocatorBackgroundProxy::UserSwitchSubscriber::Subscribe()
{
    LBSLOGD(LOCATOR_BACKGROUND_PROXY, "subscribe common event");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<UserSwitchSubscriber> subscriber = std::make_shared<UserSwitchSubscriber>(subscriberInfo);
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    if (result) {
    } else {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "Subscribe service event error.");
    }
    return result;
}

LocatorBackgroundProxy::SystemAbilityStatusChangeListener::SystemAbilityStatusChangeListener(
    std::shared_ptr<UserSwitchSubscriber> &subscriber) : subscriber_(subscriber)
{}

void LocatorBackgroundProxy::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (subscriber_ == nullptr) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "OnAddSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "SubscribeCommonEvent subscriber_ result = %{public}d", result);
}

void LocatorBackgroundProxy::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (subscriber_ == nullptr) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "OnRemoveSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    LBSLOGE(LOCATOR_BACKGROUND_PROXY, "UnSubscribeCommonEvent subscriber_ result = %{public}d", result);
}

bool LocatorBackgroundProxy::IsAppBackground(std::string bundleName)
{
    sptr<ISystemAbilityManager> samgrClient = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrClient == nullptr) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "Get system ability manager failed.");
        return false;
    }
    sptr<AppExecFwk::IAppMgr> iAppManager =
        iface_cast<AppExecFwk::IAppMgr>(samgrClient->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (iAppManager == nullptr) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "Failed to get ability manager service.");
        return false;
    }
    std::vector<AppExecFwk::AppStateData> foregroundAppList;
    iAppManager->GetForegroundApplications(foregroundAppList);
    auto it = std::find_if(foregroundAppList.begin(), foregroundAppList.end(), [bundleName] (auto foregroundApp) {
        return bundleName.compare(foregroundApp.bundleName) == 0;
    });
    if (it != foregroundAppList.end()) {
        LBSLOGD(LOCATOR_BACKGROUND_PROXY, "app : %{public}s is foreground.", bundleName.c_str());
        return false;
    }
    return true;
}

bool LocatorBackgroundProxy::RegisterAppStateObserver()
{
    if (appStateObserver_ != nullptr) {
        LBSLOGI(REQUEST_MANAGER, "app state observer exist.");
        return true;
    }
    appStateObserver_ = sptr<AppStateChangeCallback>(new (std::nothrow) AppStateChangeCallback());
    sptr<ISystemAbilityManager> samgrClient = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrClient == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "Get system ability manager failed.");
        appStateObserver_ = nullptr;
        return false;
    }
    iAppMgr_ = iface_cast<AppExecFwk::IAppMgr>(samgrClient->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (iAppMgr_ == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "Failed to get ability manager service.");
        appStateObserver_ = nullptr;
        return false;
    }
    int32_t result = iAppMgr_->RegisterApplicationStateObserver(appStateObserver_);
    if (result != 0) {
        LBSLOGE(REQUEST_MANAGER, "Failed to Register app state observer.");
        iAppMgr_ = nullptr;
        appStateObserver_ = nullptr;
        return false;
    }
    return true;
}

bool LocatorBackgroundProxy::UnregisterAppStateObserver()
{
    if (iAppMgr_ != nullptr && appStateObserver_ != nullptr) {
        iAppMgr_->UnregisterApplicationStateObserver(appStateObserver_);
    }
    iAppMgr_ = nullptr;
    appStateObserver_ = nullptr;
    return true;
}

bool LocatorBackgroundProxy::IsAppInLocationContinuousTasks(pid_t uid, pid_t pid)
{
#ifdef BGTASKMGR_SUPPORT
    std::vector<std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>> continuousTasks;
    ErrCode result = BackgroundTaskMgr::BackgroundTaskMgrHelper::GetContinuousTaskApps(continuousTasks);
    if (result != ERR_OK) {
        return false;
    }
    for (auto iter = continuousTasks.begin(); iter != continuousTasks.end(); iter++) {
        auto continuousTask = *iter;
        if (continuousTask == nullptr) {
            continue;
        }
        if (continuousTask->GetCreatorUid() != uid || continuousTask->GetCreatorPid() != pid) {
            continue;
        }
        auto typeIds = continuousTask->GetTypeIds();
        for (auto typeId : typeIds) {
            if (typeId == BackgroundTaskMgr::BackgroundMode::Type::LOCATION) {
                return true;
            }
        }
    }
#endif
    return false;
}

bool LocatorBackgroundProxy::IsAppHasFormVisible(uint32_t tokenId, uint64_t tokenIdEx)
{
    bool ret = false;
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return ret;
    }
#ifdef FMSKIT_NATIVE_SUPPORT
    ret = OHOS::AppExecFwk::FormMgr::GetInstance().HasFormVisible(tokenId);
#endif
    return ret;
}

AppStateChangeCallback::AppStateChangeCallback()
{
}

AppStateChangeCallback::~AppStateChangeCallback()
{
}

void AppStateChangeCallback::OnForegroundApplicationChanged(const AppExecFwk::AppStateData& appStateData)
{
    auto requestManager = RequestManager::GetInstance();
    int32_t pid = appStateData.pid;
    int32_t uid = appStateData.uid;
    int32_t state = appStateData.state;
    LBSLOGD(REQUEST_MANAGER,
        "The state of App changed, uid = %{public}d, pid = %{public}d, state = %{public}d", uid, pid, state);
    requestManager->HandlePowerSuspendChanged(pid, uid, state);
}

LocatorBackgroundHandler::LocatorBackgroundHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

LocatorBackgroundHandler::~LocatorBackgroundHandler() {}

void LocatorBackgroundHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case EVENT_STARTLOCATING: {
            locatorBackgroundProxy->StartLocatorThread();
            break;
        }
        case EVENT_STOPLOCATING: {
            locatorBackgroundProxy->StopLocatorThread();
            break;
        }
        default:
            break;
    }
}
} // namespace OHOS
} // namespace Location
