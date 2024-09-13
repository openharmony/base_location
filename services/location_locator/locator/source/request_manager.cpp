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
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability_proxy.h"
#endif
#include "fusion_controller.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#include "locator_event_manager.h"
#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability_proxy.h"
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
#include "passive_ability_proxy.h"
#endif
#include "request_config.h"
#include "location_log_event_ids.h"
#include "common_hisysevent.h"
#include "hook_utils.h"
#include "permission_manager.h"
#ifdef DEVICE_STANDBY_ENABLE
#include "standby_service_client.h"
#endif

#ifdef RES_SCHED_SUPPROT
#include "res_type.h"
#include "res_sched_client.h"
#endif

#include "location_data_rdb_manager.h"

namespace OHOS {
namespace Location {
ffrt::mutex RequestManager::requestMutex_;

RequestManager* RequestManager::GetInstance()
{
    static RequestManager data;
    return &data;
}

RequestManager::RequestManager()
{
    isDeviceIdleMode_.store(false);
    isDeviceStillState_.store(false);
    auto locatorDftManager = LocatorDftManager::GetInstance();
    if (locatorDftManager != nullptr) {
        locatorDftManager->Init();
    }
}

RequestManager::~RequestManager()
{
}

bool RequestManager::InitSystemListeners()
{
    LBSLOGI(REQUEST_MANAGER, "Init system listeners.");
    return true;
}

void RequestManager::UpdateUsingPermission(std::shared_ptr<Request> request, const bool isStart)
{
    std::unique_lock<ffrt::mutex> lock(permissionRecordMutex_, std::defer_lock);
    lock.lock();
    if (request == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "request is null");
        lock.unlock();
        return;
    }
    UpdateUsingApproximatelyPermission(request, isStart);
    lock.unlock();
}

void RequestManager::UpdateUsingApproximatelyPermission(std::shared_ptr<Request> request, const bool isStart)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    uint32_t callingTokenId = request->GetTokenId();
    if (isStart && !request->GetApproximatelyPermState()) {
        PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION);
        locatorAbility->UpdatePermissionUsedRecord(request->GetTokenId(),
            ACCESS_APPROXIMATELY_LOCATION, request->GetPermUsedType(), 1, 0);
        request->SetApproximatelyPermState(true);
    } else if (!isStart && request->GetApproximatelyPermState()) {
        PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION);
        request->SetApproximatelyPermState(false);
    }
}

void RequestManager::HandleStartLocating(std::shared_ptr<Request> request)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    auto locatorDftManager = LocatorDftManager::GetInstance();
    // restore request to all request list
    bool isNewRequest = RestorRequest(request);
    // update request map
    if (isNewRequest) {
        locatorAbility->RegisterPermissionCallback(request->GetTokenId(),
            {ACCESS_APPROXIMATELY_LOCATION, ACCESS_LOCATION, ACCESS_BACKGROUND_LOCATION});
        UpdateRequestRecord(request, true);
        locatorDftManager->LocationSessionStart(request);
    }
    // process location request
    HandleRequest();
}

bool RequestManager::RestorRequest(std::shared_ptr<Request> newRequest)
{
    std::unique_lock lock(requestMutex_);

    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_lock lock(requestMutex_);
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
        HandleChrEvent(*list);
        UpdateRunningUids(request, abilityName, true);
    } else {
        for (auto iter = list->begin(); iter != list->end();) {
            auto findRequest = *iter;
            if (request == findRequest) {
                iter = list->erase(iter);
                UpdateRunningUids(findRequest, abilityName, false);
                LBSLOGD(REQUEST_MANAGER, "find request");
            } else {
                ++iter;
            }
        }
    }
    LBSLOGD(REQUEST_MANAGER, "%{public}s ability request size %{public}s",
        abilityName.c_str(), std::to_string(list->size()).c_str());
}

void RequestManager::HandleChrEvent(std::list<std::shared_ptr<Request>> requests)
{
    if (requests.size() > LBS_REQUEST_MAX_SIZE) {
        std::vector<std::string> names;
        std::vector<std::string> values;
        int index = 0;
        for (auto it = requests.begin(); it != requests.end(); ++it, ++index) {
            auto request = *it;
            if (request == nullptr) {
                continue;
            }
            names.push_back(std::to_string(index));
            std::string packageName = request->GetPackageName();
            values.push_back(packageName);
        }
        WriteLocationInnerEvent(LBS_REQUEST_TOO_MUCH, names, values);
    }
}

void RequestManager::HandleStopLocating(sptr<ILocatorCallback> callback)
{
    if (callback == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "stop locating but callback is null");
        return;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_lock<ffrt::mutex> lock(requestMutex_, std::defer_lock);
    lock.lock();
    auto receivers = locatorAbility->GetReceivers();
    if (receivers == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "receivers map is empty");
        lock.unlock();
        return;
    }
    sptr<IRemoteObject> deadCallback = callback->AsObject();
    // get dead request list
    LBSLOGD(REQUEST_MANAGER, "stop callback");
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
        HookUtils::ExecuteHookWhenStopLocation(request);
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
        UpdateUsingPermission(request, false);
        if (request->GetLocatorCallBack() != nullptr && request->GetLocatorCallbackRecipient() != nullptr) {
            request->GetLocatorCallBack()->AsObject()->RemoveDeathRecipient(request->GetLocatorCallbackRecipient());
        }
        auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
        locatorBackgroundProxy->OnDeleteRequestRecord(request);
    }
}

void RequestManager::HandleRequest()
{
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_lock<ffrt::mutex> lock(requestMutex_, std::defer_lock);
    lock.lock();
    auto requests = locatorAbility->GetRequests();
    lock.unlock();
    if (requests == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    std::map<std::string, std::list<std::shared_ptr<Request>>>::iterator iter;
    for (iter = requests->begin(); iter != requests->end(); ++iter) {
        std::string abilityName = iter->first;
        std::list<std::shared_ptr<Request>> requestList = iter->second;
        HandleRequest(abilityName, requestList);
    }
}

void RequestManager::HandleRequest(std::string abilityName, std::list<std::shared_ptr<Request>> list)
{
    // generate work record, and calculate interval
    std::shared_ptr<WorkRecord> workRecord = std::make_shared<WorkRecord>();
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        auto request = *iter;
        if (!AddRequestToWorkRecord(abilityName, request, workRecord)) {
            WriteLocationInnerEvent(REMOVE_REQUEST, {"PackageName", request->GetPackageName(),
                    "abilityName", abilityName, "requestAddress", request->GetUuid()});
            UpdateUsingPermission(request, false);
            continue;
        }
        UpdateUsingPermission(request, true);
        if (!ActiveLocatingStrategies(request)) {
            continue;
        }
        LBSLOGD(REQUEST_MANAGER, "add pid:%{public}d uid:%{public}d %{public}s", request->GetPid(), request->GetUid(),
            request->GetPackageName().c_str());
    }
    LBSLOGD(REQUEST_MANAGER, "detect %{public}s ability requests(size:%{public}s) work record:%{public}s",
        abilityName.c_str(), std::to_string(list.size()).c_str(), workRecord->ToString().c_str());

    ProxySendLocationRequest(abilityName, *workRecord);
}

bool RequestManager::ActiveLocatingStrategies(const std::shared_ptr<Request>& request)
{
    if (request == nullptr) {
        return false;
    }
    auto requestConfig = request->GetRequestConfig();
    if (requestConfig == nullptr) {
        return false;
    }
    int requestType = requestConfig->GetScenario();
    if (requestType == SCENE_UNSET) {
        requestType = requestConfig->GetPriority();
    }
    auto fusionController = FusionController::GetInstance();
    if (fusionController != nullptr) {
        fusionController->ActiveFusionStrategies(requestType);
    }
    return true;
}

/**
 * determine whether the request is valid.
 */
bool RequestManager::IsRequestAvailable(std::shared_ptr<Request>& request)
{
    if (!request->GetIsRequesting()) {
        return false;
    }
    // for frozen app, do not add to workRecord
    if (LocatorAbility::GetInstance()->IsProxyPid(request->GetPid())) {
        return false;
    }
    AppIdentity identity;
    identity.SetUid(request->GetUid());
    identity.SetTokenId(request->GetTokenId());
    if (!CommonUtils::IsAppBelongCurrentAccount(identity)) {
        LBSLOGD(REPORT_MANAGER, "AddRequestToWorkRecord uid: %{public}d ,CheckAppIsCurrentUser fail",
            request->GetUid());
        return false;
    }
    // for once_request app, if it has timed out, do not add to workRecord
    int64_t curTime = CommonUtils::GetCurrentTime();
    if (request->GetRequestConfig()->GetFixNumber() == 1 &&
        fabs(curTime - request->GetRequestConfig()->GetTimeStamp()) >
        (request->GetRequestConfig()->GetTimeOut() / MILLI_PER_SEC)) {
        LBSLOGE(LOCATOR, "%{public}d has timed out.", request->GetPid());
        return false;
    }
    return true;
}

void RequestManager::IsStandby()
{
#ifdef DEVICE_STANDBY_ENABLE
    LBSLOGI(LOCATOR, "%{public}s called", __func__);
    bool isStandby = false;
    DevStandbyMgr::StandbyServiceClient& standbyServiceClient = DevStandbyMgr::StandbyServiceClient::GetInstance();
    ErrCode code = standbyServiceClient.IsDeviceInStandby(isStandby);
    if (code == ERR_OK && isStandby) {
        isDeviceIdleMode_.store(true);
        LBSLOGI(LOCATOR, "isStandby = true");
        return;
    }
#endif
    isDeviceIdleMode_.store(false);
    LBSLOGI(LOCATOR, "isStandby = false");
}

bool RequestManager::AddRequestToWorkRecord(std::string abilityName, std::shared_ptr<Request>& request,
    std::shared_ptr<WorkRecord>& workRecord)
{
    if (request == nullptr) {
        return false;
    }
    if (!IsRequestAvailable(request)) {
        return false;
    }
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED) {
        RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_LOCATION_SWITCH_OFF, request);
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d the location switch is off", __func__, __LINE__);
        return false;
    }
    uint32_t tokenId = request->GetTokenId();
    uint32_t firstTokenId = request->GetFirstTokenId();
    // if location access permission granted, add request info to work record
    if (!PermissionManager::CheckLocationPermission(tokenId, firstTokenId) &&
        !PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_LOCATION_PERMISSION_DENIED, request);
        LBSLOGI(LOCATOR, "CheckLocationPermission return false, tokenId=%{public}d", tokenId);
        return false;
    }
    std::string bundleName = "";
    pid_t uid = request->GetUid();
    pid_t pid = request->GetPid();
    if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
        LBSLOGD(REPORT_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
    }
    auto reportManager = ReportManager::GetInstance();
    if (reportManager != nullptr) {
        if (reportManager->IsAppBackground(bundleName, tokenId,
            request->GetTokenIdEx(), uid, pid)&&
            !PermissionManager::CheckBackgroundPermission(tokenId, firstTokenId)) {
            RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_BACKGROUND_PERMISSION_DENIED, request);
            LBSLOGE(REPORT_MANAGER, "CheckBackgroundPermission return false, tokenId=%{public}d", tokenId);
            return false;
        }
    }
    auto requestConfig = request->GetRequestConfig();
    if (requestConfig == nullptr) {
        return false;
    }

    if (HookUtils::ExecuteHookWhenAddWorkRecord(isDeviceStillState_.load(), isDeviceIdleMode_.load(),
        abilityName, bundleName)) {
        LBSLOGI(REQUEST_MANAGER, "Enter idle and still status, not add request");
        return false;
    }
    // add request info to work record
    if (workRecord != nullptr) {
        request->SetNlpRequestType();
        workRecord->Add(request);
    }
    return true;
}

void RequestManager::ProxySendLocationRequest(std::string abilityName, WorkRecord& workRecord)
{
    int systemAbilityId = CommonUtils::AbilityConvertToId(abilityName);
    if (!SaLoadWithStatistic::InitLocationSa(systemAbilityId)) {
        return ;
    }
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(systemAbilityId, CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: remote obj is nullptr", __func__);
        return;
    }
    LBSLOGI(LOCATOR, "%{public}s: %{public}s workRecord uid_ size %{public}d",
        __func__, abilityName.c_str(), workRecord.Size());
    workRecord.SetDeviceId(CommonUtils::InitDeviceId());
    if (abilityName == GNSS_ABILITY) {
#ifdef FEATURE_GNSS_SUPPORT
        std::unique_ptr<GnssAbilityProxy> gnssProxy = std::make_unique<GnssAbilityProxy>(remoteObject);
        gnssProxy->SendLocationRequest(workRecord);
#endif
    } else if (abilityName == NETWORK_ABILITY) {
#ifdef FEATURE_NETWORK_SUPPORT
        std::unique_ptr<NetworkAbilityProxy> networkProxy = std::make_unique<NetworkAbilityProxy>(remoteObject);
        networkProxy->SendLocationRequest(workRecord);
#endif
    } else if (abilityName == PASSIVE_ABILITY) {
#ifdef FEATURE_PASSIVE_SUPPORT
        std::unique_ptr<PassiveAbilityProxy> passiveProxy = std::make_unique<PassiveAbilityProxy>(remoteObject);
        passiveProxy->SendLocationRequest(workRecord);
#endif
    }
}

sptr<IRemoteObject> RequestManager::GetRemoteObject(std::string abilityName)
{
    sptr<IRemoteObject> remoteObject = nullptr;
    auto locatorAbility = LocatorAbility::GetInstance();
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
        LBSLOGD(REQUEST_MANAGER, "Current uid : %{public}d is not locating.", uid);
        return;
    }
    LocatorAbility::GetInstance()->ApplyRequests(1);
}

void RequestManager::HandlePermissionChanged(uint32_t tokenId)
{
    auto locatorAbility = LocatorAbility::GetInstance();
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
            auto backgroundProxy = LocatorBackgroundProxy::GetInstance();
            if (backgroundProxy != nullptr) {
                backgroundProxy->UpdateListOnRequestChange(request);
            }
        }
    }
}

bool RequestManager::IsUidInProcessing(int32_t uid)
{
    std::unique_lock<ffrt::mutex> lock(runningUidsMutex_);
    auto iter = runningUidMap_.find(uid);
    if (iter == runningUidMap_.end()) {
        return false;
    }
    return true;
}

void RequestManager::UpdateRunningUids(const std::shared_ptr<Request>& request, std::string abilityName, bool isAdd)
{
    std::unique_lock<ffrt::mutex> lock(runningUidsMutex_);
    auto uid = request->GetUid();
    auto pid = request->GetPid();
    int32_t uidCount = 0;
    auto iter = runningUidMap_.find(uid);
    if (iter != runningUidMap_.end()) {
        uidCount = iter->second;
        runningUidMap_.erase(uid);
    }
    if (isAdd) {
        auto requestConfig = request->GetRequestConfig();
        WriteLocationInnerEvent(ADD_REQUEST, {
            "PackageName", request->GetPackageName(),
            "abilityName", abilityName,
            "requestAddress", request->GetUuid(),
            "scenario", std::to_string(requestConfig->GetScenario()),
            "priority", std::to_string(requestConfig->GetPriority()),
            "timeInterval", std::to_string(requestConfig->GetTimeInterval()),
            "maxAccuracy", std::to_string(requestConfig->GetMaxAccuracy())});
        uidCount += 1;
        if (uidCount == 1) {
            WriteAppLocatingStateEvent("start", pid, uid);
            ReportDataToResSched("start", pid, uid);
        }
    } else {
        WriteLocationInnerEvent(REMOVE_REQUEST, {"PackageName", request->GetPackageName(),
                    "abilityName", abilityName, "requestAddress", request->GetUuid()});
        uidCount -= 1;
        if (uidCount == 0) {
            WriteAppLocatingStateEvent("stop", pid, uid);
            ReportDataToResSched("stop", pid, uid);
        }
    }
    if (uidCount > 0) {
        runningUidMap_.insert(std::make_pair(uid, uidCount));
    }
}

void RequestManager::ReportDataToResSched(std::string state, const pid_t pid, const pid_t uid)
{
#ifdef RES_SCHED_SUPPROT
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(pid);
    payload["uid"] = std::to_string(uid);
    payload["state"] = state;
    uint32_t type = ResourceSchedule::ResType::RES_TYPE_LOCATION_STATUS_CHANGE;
    int64_t value =  ResourceSchedule::ResType::LocationStatus::APP_LOCATION_STATUE_CHANGE;
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, value, payload);
#endif
}

void RequestManager::RegisterLocationErrorCallback(
    sptr<ILocatorCallback> callback, AppIdentity identity)
{
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow)
        LocatorErrCallbackDeathRecipient(identity.GetTokenId()));
    callback->AsObject()->AddDeathRecipient(death);
    std::shared_ptr<LocationErrRequest> locatorErrRequest = std::make_shared<LocationErrRequest>();
    locatorErrRequest->SetUid(identity.GetUid());
    locatorErrRequest->SetPid(identity.GetPid());
    locatorErrRequest->SetLocatorErrCallbackRecipient(death);
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    locationErrorCallbackMap_[callback->AsObject()] = locatorErrRequest;
    LBSLOGD(LOCATOR, "after RegisterLocationErrorCallback, callback size:%{public}s",
        std::to_string(locationErrorCallbackMap_.size()).c_str());
}

void RequestManager::UnRegisterLocationErrorCallback(
    sptr<ILocatorCallback> callback)
{
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    auto iter = locationErrorCallbackMap_.find(callback->AsObject());
    if (iter != locationErrorCallbackMap_.end()) {
        auto locatorErrorCallback = iter->first;
        auto locatorErrRequest = iter->second;
        locatorErrorCallback->RemoveDeathRecipient(locatorErrRequest->GetLocatorErrCallbackRecipient());
        locationErrorCallbackMap_.erase(iter);
    }
    LBSLOGD(LOCATOR, "after UnRegisterLocationErrorCallback, callback size:%{public}s",
        std::to_string(locationErrorCallbackMap_.size()).c_str());
}

void RequestManager::ReportLocationError(const int errorCode, std::shared_ptr<Request> request)
{
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    for (auto iter : locationErrorCallbackMap_) {
        auto locatorErrRequest = iter.second;
        if (locatorErrRequest == nullptr) {
            continue;
        }
        if (LocatorAbility::GetInstance()->IsProxyPid(locatorErrRequest->GetPid()) ||
            (request->GetUid() != 0 && (request->GetUid() != locatorErrRequest->GetUid()))) {
            continue;
        }
        if (locatorErrRequest->GetLastReportErrcode() != LOCATING_FAILED_DEFAULT &&
            locatorErrRequest->GetLastReportErrcode() == errorCode) {
            continue;
        }
        sptr<ILocatorCallback> locatorErrorCallback = iface_cast<ILocatorCallback>(iter.first);
        if (locatorErrorCallback == nullptr) {
            continue;
        }
        locatorErrorCallback->OnErrorReport(errorCode);
        locatorErrRequest->SetLastReportErrcode(errorCode);
    }
}

void RequestManager::UpdateLocationError(std::shared_ptr<Request> request)
{
    std::unique_lock<ffrt::mutex> lock(locationErrorCallbackMutex_);
    for (auto iter : locationErrorCallbackMap_) {
        auto locatorErrRequest = iter.second;
        if (request->GetUid() != 0 && (request->GetUid() == locatorErrRequest->GetUid())) {
            locatorErrRequest->SetLastReportErrcode(LOCATING_FAILED_DEFAULT);
        }
    }
}

void RequestManager::SyncStillMovementState(bool state)
{
    bool newDeviceState = false;
    bool oldDeviceState = false;
    oldDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    isDeviceStillState_.store(state);
    LBSLOGI(REQUEST_MANAGER, "device movement state change, isDeviceStillState_ %{public}d",
        isDeviceStillState_.load());
    newDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    if (newDeviceState != oldDeviceState) {
        HandleRequest();
    }
}

void RequestManager::SyncIdleState(bool state)
{
    bool newDeviceState = false;
    bool oldDeviceState = false;
    oldDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    isDeviceIdleMode_.store(state);
    LBSLOGI(REQUEST_MANAGER, "device idle mode change, isDeviceIdleMode_ %{public}d",
        isDeviceIdleMode_.load());
    newDeviceState = isDeviceStillState_.load() && isDeviceIdleMode_.load();
    if (newDeviceState != oldDeviceState) {
        HandleRequest();
    }
}

LocatorErrCallbackDeathRecipient::LocatorErrCallbackDeathRecipient(int32_t tokenId)
{
    tokenId_ = tokenId;
}

LocatorErrCallbackDeathRecipient::~LocatorErrCallbackDeathRecipient()
{
}

void LocatorErrCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remote.promote());
    auto requestManager = RequestManager::GetInstance();
    if (requestManager != nullptr) {
        requestManager->UnRegisterLocationErrorCallback(callback);
        LBSLOGI(REQUEST_MANAGER, "locatorerr callback OnRemoteDied tokenId = %{public}d", tokenId_);
    }
}

LocationErrRequest::LocationErrRequest()
{
    uid_ = 0;
    pid_ = 0;
    lastReportErrcode_ = LOCATING_FAILED_DEFAULT;
    locatorErrCallbackRecipient_ = nullptr;
}

LocationErrRequest::~LocationErrRequest() {}

pid_t LocationErrRequest::GetUid()
{
    return uid_;
}

void LocationErrRequest::SetUid(pid_t uid)
{
    uid_ = uid;
}

pid_t LocationErrRequest::GetPid()
{
    return pid_;
}

void LocationErrRequest::SetPid(pid_t pid)
{
    pid_ = pid;
}

int32_t LocationErrRequest::GetLastReportErrcode()
{
    return lastReportErrcode_;
}

void LocationErrRequest::SetLastReportErrcode(int32_t lastReportErrcode)
{
    lastReportErrcode_ = lastReportErrcode;
}

void LocationErrRequest::SetLocatorErrCallbackRecipient(const sptr<IRemoteObject::DeathRecipient>& recipient)
{
    locatorErrCallbackRecipient_ = recipient;
}

sptr<IRemoteObject::DeathRecipient> LocationErrRequest::GetLocatorErrCallbackRecipient()
{
    return locatorErrCallbackRecipient_;
}
} // namespace Location
} // namespace OHOS
