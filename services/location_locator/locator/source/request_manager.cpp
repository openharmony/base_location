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

#include "request_manager.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "fusion_controller.h"
#include "location_log.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#include "locator_event_manager.h"
#include "subability_common.h"
#include "privacy_kit.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace Location {
std::mutex RequestManager::requestMutex;
RequestManager::RequestManager()
{
    DelayedSingleton<LocatorDftManager>::GetInstance()->Init();
}

RequestManager::~RequestManager() {
}

bool RequestManager::InitSystemListeners()
{
    LBSLOGI(REQUEST_MANAGER, "Register app state observer.");
    return RegisterAppStateObserver();
}

void RequestManager::HandleStartLocating(std::shared_ptr<Request> request)
{
    std::lock_guard lock(requestMutex);
    // restore request to all request list
    bool isNewRequest = RestorRequest(request);
    // update request map
    if (isNewRequest) {
        UpdateRequestRecord(request, true);
        DelayedSingleton<LocatorDftManager>::GetInstance()->LocationSessionStart(request);
    }
    // process location request
    HandleRequest();
}

bool RequestManager::RestorRequest(std::shared_ptr<Request> newRequest)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return false;
    }
    auto receivers = locatorAbility->GetReceivers();
    if (receivers == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "receivers is empty");
        return false;
    }
    if (newRequest == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "newRequest is empty");
        return false;
    }
    newRequest->SetRequesting(true);
    sptr<IRemoteObject> newCallback = newRequest->GetLocatorCallBack()->AsObject();

    LBSLOGI(REQUEST_MANAGER, "add request:%{public}s", newRequest->ToString().c_str());
    // if callback and request config type is same, take new request configuration over the old one in request list
    // otherwise, add restore the new request in the list.
    auto iterator = receivers->find(newCallback);
    if (iterator == receivers->end()) {
        std::list<std::shared_ptr<Request>> requestList;
        requestList.push_back(newRequest);
        receivers->insert(make_pair(newCallback, requestList));
        LBSLOGD(REQUEST_MANAGER, "add new receiver with new callback");
        return true;
    }

    sptr<RequestConfig> newConfig = newRequest->GetRequestConfig();
    std::list<std::shared_ptr<Request>> requestWithSameCallback = iterator->second;
    for (auto iter = requestWithSameCallback.begin(); iter != requestWithSameCallback.end(); ++iter) {
        auto request = *iter;
        if (request == nullptr) {
            continue;
        }
        auto requestConfig = request->GetRequestConfig();
        if (requestConfig == nullptr || newConfig == nullptr) {
            continue;
        }
        if (newConfig->IsSame(*requestConfig)) {
            request->SetRequestConfig(*newConfig);
            LBSLOGI(REQUEST_MANAGER, "find same type request, update request configuration");
            return false;
        }
    }
    requestWithSameCallback.push_back(newRequest);
    LBSLOGD(REQUEST_MANAGER, "add new receiver with old callback");
    return true;
}

void RequestManager::UpdateRequestRecord(std::shared_ptr<Request> request, bool shouldInsert)
{
    std::shared_ptr<std::list<std::string>> proxys = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxys);
    if (proxys->empty()) {
        LBSLOGE(REQUEST_MANAGER, "can not get proxy name according to request configuration");
        return;
    }

    for (std::list<std::string>::iterator iter = proxys->begin(); iter != proxys->end(); ++iter) {
        std::string abilityName = *iter;
        UpdateRequestRecord(request, abilityName, shouldInsert);
    }
}

void RequestManager::UpdateRequestRecord(std::shared_ptr<Request> request, std::string abilityName, bool shouldInsert)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    auto mapIter = requests->find(abilityName);
    if (mapIter == requests->end()) {
        LBSLOGE(REQUEST_MANAGER, "can not find %{public}s ability request list.", abilityName.c_str());
        return;
    }

    auto list = &(mapIter->second);
    LBSLOGD(REQUEST_MANAGER, "%{public}s ability current request size %{public}s",
        abilityName.c_str(), std::to_string(list->size()).c_str());
    if (shouldInsert) {
        list->push_back(request);
        runningUids_.push_back(request->GetUid());
    } else {
        for (auto iter = list->begin(); iter != list->end();) {
            auto findRequest = *iter;
            if (request == findRequest) {
                iter = list->erase(iter);
                runningUids_.remove(findRequest->GetUid());
                LBSLOGD(REQUEST_MANAGER, "find request");
            } else {
                ++iter;
            }
        }
    }
    LBSLOGD(REQUEST_MANAGER, "%{public}s ability request size %{public}s",
        abilityName.c_str(), std::to_string(list->size()).c_str());
}

void RequestManager::HandleStopLocating(sptr<ILocatorCallback> callback)
{
    std::lock_guard lock(requestMutex);
    if (callback == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "stop locating but callback is null");
        return;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto receivers = locatorAbility->GetReceivers();
    if (receivers == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "receivers map is empty");
        return;
    }
    sptr<IRemoteObject> deadCallback = callback->AsObject();
    // get dead request list
    LBSLOGD(REQUEST_MANAGER, "stop callback %{public}p", &deadCallback);
    auto iterator = receivers->find(deadCallback);
    if (iterator == receivers->end()) {
        LBSLOGD(REQUEST_MANAGER, "this callback has no record in receiver map");
        return;
    }

    auto requests = iterator->second;
    auto deadRequests = std::make_shared<std::list<std::shared_ptr<Request>>>();
    for (auto iter = requests.begin(); iter != requests.end(); ++iter) {
        auto request = *iter;
        deadRequests->push_back(request);
        LBSLOGI(REQUEST_MANAGER, "remove request:%{public}s", request->ToString().c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "get %{public}s dead request", std::to_string(deadRequests->size()).c_str());

    // update request map
    if (deadRequests->size() == 0) {
        return;
    }
    DeleteRequestRecord(deadRequests);
    deadRequests->clear();
    iterator->second.clear();
    receivers->erase(iterator);

    // process location request
    HandleRequest();
}

void RequestManager::DeleteRequestRecord(std::shared_ptr<std::list<std::shared_ptr<Request>>> requests)
{
    for (auto iter = requests->begin(); iter != requests->end(); ++iter) {
        auto request = *iter;
        UpdateRequestRecord(request, false);
        DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get()->OnDeleteRequestRecord(request);
    }
}

void RequestManager::HandleRequest()
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto proxyMap = locatorAbility->GetProxyMap();
    if (proxyMap->empty()) {
        LBSLOGE(REQUEST_MANAGER, "proxy map is empty");
        return;
    }
    std::map<std::string, sptr<IRemoteObject>>::iterator iter;
    for (iter = proxyMap->begin(); iter != proxyMap->end(); ++iter) {
        std::string abilityName = iter->first;
        HandleRequest(abilityName);
    }
}

void RequestManager::HandleRequest(std::string abilityName)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    auto mapIter = requests->find(abilityName);
    if (mapIter == requests->end()) {
        LBSLOGE(REQUEST_MANAGER, "can not find %{public}s ability request list.", abilityName.c_str());
        return;
    }
    auto list = mapIter->second;

    // generate work record, and calculate interval
    std::shared_ptr<WorkRecord> workRecord = std::make_shared<WorkRecord>();
    int timeInterval = 0;
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        auto request = *iter;
        if (request == nullptr || !request->GetIsRequesting()) {
            continue;
        }
        pid_t uid = request->GetUid();
        pid_t pid = request->GetPid();
        uint32_t tokenId = request->GetTokenId();
        uint32_t firstTokenId = request->GetFirstTokenId();
        std::string packageName = request->GetPackageName();
        // if location access permission granted, add request info to work record
        if (!CommonUtils::CheckLocationPermission(tokenId, firstTokenId) &&
            !CommonUtils::CheckApproximatelyPermission(tokenId, firstTokenId)) {
            LBSLOGI(LOCATOR, "CheckLocationPermission return false, tokenId=%{public}d", tokenId);
            continue;
        }
        // add request info to work record
        workRecord->Add(uid, pid, packageName);
        auto requestConfig = request->GetRequestConfig();
        if (requestConfig == nullptr) {
            continue;
        }
        timeInterval = requestConfig->GetTimeInterval();
        int requestType = requestConfig->GetScenario();
        if (requestType == SCENE_UNSET) {
            requestType = requestConfig->GetPriority();
        }
        DelayedSingleton<FusionController>::GetInstance()->ActiveFusionStrategies(requestType);
        LBSLOGD(REQUEST_MANAGER, "add pid:%{public}d uid:%{public}d %{public}s", pid, uid, packageName.c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "detect %{public}s ability requests(size:%{public}s) work record:%{public}s",
        abilityName.c_str(), std::to_string(list.size()).c_str(), workRecord->ToString().c_str());
    ProxySendLocationRequest(abilityName, *workRecord, timeInterval);
}

void RequestManager::ProxySendLocationRequest(std::string abilityName, WorkRecord& workRecord, int timeInterval)
{
    sptr<IRemoteObject> remoteObject = GetRemoteObject(abilityName);
    if (remoteObject == nullptr) {
        return;
    }
    workRecord.SetDeviceId(CommonUtils::InitDeviceId());
    if (abilityName == GNSS_ABILITY) {
        std::unique_ptr<GnssAbilityProxy> gnssProxy = std::make_unique<GnssAbilityProxy>(remoteObject);
        gnssProxy->SendLocationRequest(timeInterval, workRecord);
    } else if (abilityName == NETWORK_ABILITY) {
        std::unique_ptr<NetworkAbilityProxy> networkProxy = std::make_unique<NetworkAbilityProxy>(remoteObject);
        networkProxy->SendLocationRequest(timeInterval, workRecord);
    } else if (abilityName == PASSIVE_ABILITY) {
        std::unique_ptr<PassiveAbilityProxy> passiveProxy = std::make_unique<PassiveAbilityProxy>(remoteObject);
        passiveProxy->SendLocationRequest(timeInterval, workRecord);
    }
    DelayedSingleton<FusionController>::GetInstance()->Process(abilityName);
}

sptr<IRemoteObject> RequestManager::GetRemoteObject(std::string abilityName)
{
    sptr<IRemoteObject> remoteObject = nullptr;
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return remoteObject;
    }
    auto remoteManagerMap = locatorAbility->GetProxyMap();
    if (remoteManagerMap == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "proxy map is empty");
        return remoteObject;
    }
    auto remoteObjectIter = remoteManagerMap->find(abilityName);
    if (remoteObjectIter == remoteManagerMap->end()) {
        LBSLOGE(REQUEST_MANAGER, "sa init fail!");
        return remoteObject;
    }
    remoteObject = remoteObjectIter->second;
    return remoteObject;
}

void RequestManager::HandlePowerSuspendChanged(int32_t pid, int32_t uid, int32_t state)
{
    if (!IsUidInProcessing(uid)) {
        LBSLOGE(REQUEST_MANAGER, "Current uid : %{public}d is not locating.", uid);
        return;
    }
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t callingFirstTokenid = IPCSkeleton::GetFirstTokenID();
    bool isActive = false;
    if (state == FOREGROUND) {
        if (CommonUtils::CheckBackgroundPermission(callingTokenId, callingFirstTokenid)) {
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_BACKGROUND_LOCATION);
        }
        isActive = true;
    } else if (state == BACKGROUND) {
        if (CommonUtils::CheckBackgroundPermission(callingTokenId, callingFirstTokenid)) {
            PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_BACKGROUND_LOCATION);
        }
    } else {
        return;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr || requests->empty()) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    for (auto mapIter = requests->begin(); mapIter != requests->end(); mapIter++) {
        auto list = mapIter->second;
        for (auto request : list) {
            std::string uid1 = std::to_string(request->GetUid());
            std::string uid2 = std::to_string(uid);
            std::string pid1 = std::to_string(request->GetPid());
            std::string pid2 = std::to_string(pid);
            if ((uid1.compare(uid2) != 0) || (pid1.compare(pid2) != 0)) {
                continue;
            }
            request->SetRequesting(isActive);
            DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get()->OnSuspend(request, isActive);
        }
    }
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ApplyRequests();
}

bool RequestManager::IsUidInProcessing(int32_t uid)
{
    if (runningUids_.size() == 0) {
        return false;
    }

    bool isFound = std::any_of(runningUids_.begin(), runningUids_.end(), [uid](int32_t i) {
        return i == uid;
    });
    return isFound;
}

bool RequestManager::RegisterAppStateObserver()
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

bool RequestManager::UnregisterAppStateObserver()
{
    if (iAppMgr_ != nullptr && appStateObserver_ != nullptr) {
        iAppMgr_->UnregisterApplicationStateObserver(appStateObserver_);
    }
    iAppMgr_ = nullptr;
    appStateObserver_ = nullptr;
    return true;
}

bool RequestManager::IsAppBackground(const std::string& bundleName)
{
    if (iAppMgr_ == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "Failed get the app manager proxy.");
        return false;
    }

    std::vector<AppExecFwk::AppStateData> foregroundAppList;
    iAppMgr_->GetForegroundApplications(foregroundAppList);

    for (const auto& foregroundApp : foregroundAppList) {
        if (foregroundApp.bundleName == bundleName) {
            LBSLOGE(REQUEST_MANAGER, "The app is foreground now.");
            return false;
        }
    }
    return true;
}

void RequestManager::SetBundleName(std::string bundleName)
{
    bundleName_ = bundleName;
}

std::string RequestManager::GetBundleName()
{
    return bundleName_;
}

AppStateChangeCallback::AppStateChangeCallback()
{
}

AppStateChangeCallback::~AppStateChangeCallback()
{
}

void AppStateChangeCallback::OnForegroundApplicationChanged(const AppExecFwk::AppStateData& appStateData)
{
    int32_t uid = appStateData.uid;
    int32_t pid = appStateData.pid;
    int32_t state = appStateData.state;
    LBSLOGI(REQUEST_MANAGER, "The state of App changed, uid = %{public}d, pid = %{public}d, state = %{public}d", uid, pid, state);
    DelayedSingleton<RequestManager>::GetInstance()->HandlePowerSuspendChanged(pid, uid, state);
}
} // namespace Location
} // namespace OHOS
