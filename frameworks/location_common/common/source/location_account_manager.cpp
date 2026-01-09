/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "location_account_manager.h"
#include <vector>
#include <algorithm>
#include <mutex>
#include "common_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"


namespace OHOS {
namespace Location {

LocationAccountManager* LocationAccountManager::GetInstance()
{
    static LocationAccountManager manager;
    return &manager;
}

LocationAccountManager::LocationAccountManager()
{
    SubscribeSaStatusChangeListerner();
    isUserSwitchSubscribed_ = LocationAccountManager::UserSwitchSubscriber::Subscribe();
}

LocationAccountManager::~LocationAccountManager()
{
}

std::vector<int> LocationAccountManager::getActiveUserIds()
{
    return activeIds_;
}

void LocationAccountManager::OnUserSwitch(int32_t userId)
{
    bool containsActiveId = std::find(activeIds_.begin(), activeIds_.end(), userId) != activeIds_.end();
    if (!containsActiveId) {
        activeIds_.push_back(userId);
    }
}

void LocationAccountManager::OnUserRemove(int32_t userId)
{
    auto iter = std::find(activeIds_.begin(), activeIds_.end(), userId);
    if (iter != activeIds_.end()) {
        activeIds_.erase(iter);
    }
}

void LocationAccountManager::SubscribeSaStatusChangeListerner()
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
        LBSLOGE(ACCOUNT_MANAGER,
            "SubscribeSaStatusChangeListerner samgrProxy or statusChangeListener_ is nullptr");
        return;
    }
    int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    LBSLOGI(ACCOUNT_MANAGER,
        "SubscribeSaStatusChangeListerner SubscribeSystemAbility COMMON_EVENT_SERVICE_ID result:%{public}d", ret);
}

LocationAccountManager::UserSwitchSubscriber::UserSwitchSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info)
{
    LBSLOGD(ACCOUNT_MANAGER, "create UserSwitchEventSubscriber");
}

void LocationAccountManager::UserSwitchSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    int32_t userId = event.GetCode();
    const auto action = event.GetWant().GetAction();
    auto accountManager = LocationAccountManager::GetInstance();
    LBSLOGD(ACCOUNT_MANAGER, "action = %{public}s, userId = %{public}d", action.c_str(), userId);
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        accountManager->OnUserSwitch(userId);
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        accountManager->OnUserRemove(userId);
    }
}

bool LocationAccountManager::UserSwitchSubscriber::Subscribe()
{
    LBSLOGD(ACCOUNT_MANAGER, "subscribe common event");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<UserSwitchSubscriber> subscriber = std::make_shared<UserSwitchSubscriber>(subscriberInfo);
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    if (result) {
    } else {
        LBSLOGE(ACCOUNT_MANAGER, "Subscribe service event error.");
    }
    return result;
}

LocationAccountManager::SystemAbilityStatusChangeListener::SystemAbilityStatusChangeListener(
    std::shared_ptr<UserSwitchSubscriber> &subscriber) : subscriber_(subscriber)
{}

void LocationAccountManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(ACCOUNT_MANAGER, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (subscriber_ == nullptr) {
        LBSLOGE(ACCOUNT_MANAGER, "OnAddSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    LBSLOGI(ACCOUNT_MANAGER, "SubscribeCommonEvent subscriber_ result = %{public}d", result);
}

void LocationAccountManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(ACCOUNT_MANAGER, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (subscriber_ == nullptr) {
        LBSLOGE(ACCOUNT_MANAGER, "OnRemoveSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    LBSLOGE(ACCOUNT_MANAGER, "UnSubscribeCommonEvent subscriber_ result = %{public}d", result);
}

bool LocationAccountManager::IsAppBackground(std::string bundleName)
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

bool LocationAccountManager::IsAppBackground(int uid, std::string bundleName)
{
    std::unique_lock lock(foregroundAppMutex_);
    auto iter = foregroundAppMap_.find(uid);
    if (iter == foregroundAppMap_.end()) {
        return IsAppBackground(bundleName);
    }
    return false;
}

void LocationAccountManager::UpdateBackgroundAppStatues(int32_t uid, int32_t status)
{
    std::unique_lock lock(foregroundAppMutex_);
    if (status == FOREGROUPAPP_STATUS) {
        foregroundAppMap_[uid] = status;
    } else {
        auto iter = foregroundAppMap_.find(uid);
        if (iter != foregroundAppMap_.end()) {
            foregroundAppMap_.erase(iter);
        }
    }
    LBSLOGD(REQUEST_MANAGER, "UpdateBackgroundApp uid = %{public}d, state = %{public}d", uid, status);
}

bool LocationAccountManager::IsAppInLocationContinuousTasks(pid_t uid, pid_t pid)
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

bool LocationAccountManager::IsAppHasFormVisible(uint32_t tokenId, uint64_t tokenIdEx)
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

} // namespace Location
} // namespace OHOS