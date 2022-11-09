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

#include "privacy_kit.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "gnss_ability_proxy.h"
#include "fusion_controller.h"
#include "location_log.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#include "locator_event_manager.h"
#include "network_ability_proxy.h"
#include "passive_ability_proxy.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
std::mutex RequestManager::requestMutex_;
RequestManager::RequestManager()
{
    DelayedSingleton<LocatorDftManager>::GetInstance()->Init();
}

RequestManager::~RequestManager()
{
}

bool RequestManager::InitSystemListeners()
{
    LBSLOGI(REQUEST_MANAGER, "Init system listeners.");
    return true;
}

void RequestManager::UpdateUsingPermission(std::shared_ptr<Request> request)
{
    if (request == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "request is null");
        return;
    }
    LBSLOGI(REQUEST_MANAGER, "UpdateUsingPermission : tokenId = %{public}d, firstTokenId = %{public}d",
        request->GetTokenId(), request->GetFirstTokenId());
    UpdateUsingLocationPermission(request);
    UpdateUsingApproximatelyPermission(request);
    UpdateUsingBackgroundPermission(request);
}

void RequestManager::UpdateUsingLocationPermission(std::shared_ptr<Request> request)
{
    uint32_t callingTokenId = request->GetTokenId();
    uint32_t callingFirstTokenid = request->GetFirstTokenId();
    int32_t uid = request->GetUid();
    if (IsUidInProcessing(uid) &&
        CommonUtils::CheckLocationPermission(callingTokenId, callingFirstTokenid)) {
        if (!request->GetLocationPermState()) {
            PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_LOCATION);
            request->SetLocationPermState(true);
        }
    } else {
        if (request->GetLocationPermState()) {
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_LOCATION);
            request->SetLocationPermState(false);
        }
    }
}

void RequestManager::UpdateUsingApproximatelyPermission(std::shared_ptr<Request> request)
{
    uint32_t callingTokenId = request->GetTokenId();
    uint32_t callingFirstTokenid = request->GetFirstTokenId();
    int32_t uid = request->GetUid();
    if (IsUidInProcessing(uid) &&
        CommonUtils::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        if (!request->GetApproximatelyPermState()) {
            PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION);
            request->SetApproximatelyPermState(true);
        }
    } else {
        if (request->GetApproximatelyPermState()) {
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION);
            request->SetApproximatelyPermState(false);
        }
    }
}

void RequestManager::UpdateUsingBackgroundPermission(std::shared_ptr<Request> request)
{
    uint32_t callingTokenId = request->GetTokenId();
    uint32_t callingFirstTokenid = request->GetFirstTokenId();
    int32_t uid = request->GetUid();
    std::string bundleName;
    if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
        LBSLOGE(REQUEST_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
        return;
    }
    if (DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get()->IsAppBackground(bundleName) &&
        IsUidInProcessing(uid) && CommonUtils::CheckBackgroundPermission(callingTokenId, callingFirstTokenid)) {
        if (!request->GetBackgroundPermState()) {
            PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_BACKGROUND_LOCATION);
            request->SetBackgroundPermState(true);
        }
    } else {
        if (request->GetBackgroundPermState()) {
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_BACKGROUND_LOCATION);
            request->SetBackgroundPermState(false);
        }
    }
}

void RequestManager::HandleStartLocating(std::shared_ptr<Request> request)
{
    // restore request to all request list
    bool isNewRequest = RestorRequest(request);
    // update request map
    if (isNewRequest) {
        DelayedSingleton<LocatorAbility>::GetInstance()->RegisterPermissionCallback(request->GetTokenId(),
            {ACCESS_APPROXIMATELY_LOCATION, ACCESS_LOCATION, ACCESS_BACKGROUND_LOCATION});
        UpdateRequestRecord(request, true);
        UpdateUsingPermission(request);
        DelayedSingleton<LocatorDftManager>::GetInstance()->LocationSessionStart(request);
    }
    // process location request
    HandleRequest();
}

bool RequestManager::RestorRequest(std::shared_ptr<Request> newRequest)
{
    std::lock_guard lock(requestMutex_);

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
    std::lock_guard lock(requestMutex_);
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
    if (callback == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "stop locating but callback is null");
        return;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    std::unique_lock<std::mutex> lock(requestMutex_, std::defer_lock);
    lock.lock();
    auto receivers = locatorAbility->GetReceivers();
    if (receivers == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "receivers map is empty");
        lock.unlock();
        return;
    }
    sptr<IRemoteObject> deadCallback = callback->AsObject();
    // get dead request list
    LBSLOGD(REQUEST_MANAGER, "stop callback %{public}p", &deadCallback);
    auto iterator = receivers->find(deadCallback);
    if (iterator == receivers->end()) {
        LBSLOGD(REQUEST_MANAGER, "this callback has no record in receiver map");
        lock.unlock();
        return;
    }

    auto requests = iterator->second;
    auto deadRequests = std::make_shared<std::list<std::shared_ptr<Request>>>();
    for (auto iter = requests.begin(); iter != requests.end(); ++iter) {
        auto request = *iter;
        locatorAbility->UnregisterPermissionCallback(request->GetTokenId());
        deadRequests->push_back(request);
        LBSLOGI(REQUEST_MANAGER, "remove request:%{public}s", request->ToString().c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "get %{public}s dead request", std::to_string(deadRequests->size()).c_str());
    // update request map
    if (deadRequests->size() == 0) {
        lock.unlock();
        return;
    }
    iterator->second.clear();
    receivers->erase(iterator);
    lock.unlock();
    DeleteRequestRecord(deadRequests);
    deadRequests->clear();
    // process location request
    HandleRequest();
}

void RequestManager::DeleteRequestRecord(std::shared_ptr<std::list<std::shared_ptr<Request>>> requests)
{
    for (auto iter = requests->begin(); iter != requests->end(); ++iter) {
        auto request = *iter;
        UpdateRequestRecord(request, false);
        UpdateUsingPermission(request);
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
    std::unique_lock<std::mutex> lock(requestMutex_, std::defer_lock);
    lock.lock();
    auto requests = DelayedSingleton<LocatorAbility>::GetInstance()->GetRequests();
    if (requests == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        lock.unlock();
        return;
    }
    auto mapIter = requests->find(abilityName);
    if (mapIter == requests->end()) {
        LBSLOGE(REQUEST_MANAGER, "can not find %{public}s ability request list.", abilityName.c_str());
        lock.unlock();
        return;
    }
    auto list = mapIter->second;
    // generate work record, and calculate interval
    std::shared_ptr<WorkRecord> workRecord = std::make_shared<WorkRecord>();
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        auto request = *iter;
        if (request == nullptr) {
            continue;
        }
        UpdateUsingPermission(request);
        if (!request->GetIsRequesting()) {
            continue;
        }
        // if location access permission granted, add request info to work record
        if (!CommonUtils::CheckLocationPermission(request->GetTokenId(), request->GetFirstTokenId()) &&
            !CommonUtils::CheckApproximatelyPermission(request->GetTokenId(), request->GetFirstTokenId())) {
            LBSLOGI(LOCATOR, "CheckLocationPermission return false, tokenId=%{public}d", request->GetTokenId());
            continue;
        }
        auto requestConfig = request->GetRequestConfig();
        if (requestConfig == nullptr) {
            continue;
        }
        // add request info to work record
        workRecord->Add(request->GetUid(), request->GetPid(), request->GetPackageName(),
            requestConfig->GetTimeInterval(), request->GetUUid());
        int requestType = requestConfig->GetScenario();
        if (requestType == SCENE_UNSET) {
            requestType = requestConfig->GetPriority();
        }
        DelayedSingleton<FusionController>::GetInstance()->ActiveFusionStrategies(requestType);
        LBSLOGD(REQUEST_MANAGER, "add pid:%{public}d uid:%{public}d %{public}s", request->GetPid(), request->GetUid(),
            request->GetPackageName().c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "detect %{public}s ability requests(size:%{public}s) work record:%{public}s",
        abilityName.c_str(), std::to_string(list.size()).c_str(), workRecord->ToString().c_str());
    lock.unlock();

    ProxySendLocationRequest(abilityName, *workRecord);
}

void RequestManager::ProxySendLocationRequest(std::string abilityName, WorkRecord& workRecord)
{
    sptr<IRemoteObject> remoteObject = GetRemoteObject(abilityName);
    if (remoteObject == nullptr) {
        return;
    }
    workRecord.SetDeviceId(CommonUtils::InitDeviceId());
    if (abilityName == GNSS_ABILITY) {
        std::unique_ptr<GnssAbilityProxy> gnssProxy = std::make_unique<GnssAbilityProxy>(remoteObject);
        gnssProxy->SendLocationRequest(workRecord);
    } else if (abilityName == NETWORK_ABILITY) {
        std::unique_ptr<NetworkAbilityProxy> networkProxy = std::make_unique<NetworkAbilityProxy>(remoteObject);
        networkProxy->SendLocationRequest(workRecord);
    } else if (abilityName == PASSIVE_ABILITY) {
        std::unique_ptr<PassiveAbilityProxy> passiveProxy = std::make_unique<PassiveAbilityProxy>(remoteObject);
        passiveProxy->SendLocationRequest(workRecord);
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
    bool isActive = (state == static_cast<int>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND));
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

void RequestManager::HandlePermissionChanged(uint32_t tokenId)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "HandlePermissionChanged locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr || requests->empty()) {
        LBSLOGE(REQUEST_MANAGER, "HandlePermissionChanged requests map is empty");
        return;
    }
    for (auto mapIter = requests->begin(); mapIter != requests->end(); mapIter++) {
        auto list = mapIter->second;
        for (auto request : list) {
            if (request == nullptr || tokenId != request->GetTokenId()) {
                continue;
            }
            DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get()->UpdateListOnRequestChange(
                request);
        }
    }
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
} // namespace Location
} // namespace OHOS
