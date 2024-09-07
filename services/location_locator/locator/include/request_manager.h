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

#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include <list>
#include <map>
#include <mutex>
#include <singleton.h>
#include <string>

#include "ffrt.h"
#include "iremote_object.h"

#include "i_locator_callback.h"
#include "request.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class LocationErrRequest {
public:
    LocationErrRequest();
    ~LocationErrRequest();
    pid_t GetUid();
    void SetUid(pid_t uid);
    pid_t GetPid();
    void SetPid(pid_t pid);
    int32_t GetLastReportErrcode();
    void SetLastReportErrcode(int32_t lastReportErrcode);
    void SetLocatorErrCallbackRecipient(const sptr<IRemoteObject::DeathRecipient>& recipient);
    sptr<IRemoteObject::DeathRecipient> GetLocatorErrCallbackRecipient();
private:
    void GetProxyNameByPriority(std::shared_ptr<std::list<std::string>> proxys);

    pid_t uid_;
    pid_t pid_;
    int32_t lastReportErrcode_;
    sptr<IRemoteObject::DeathRecipient> locatorErrCallbackRecipient_;
};

class RequestManager {
public:
    RequestManager();
    ~RequestManager();
    bool InitSystemListeners();
    void HandleStartLocating(std::shared_ptr<Request> request);
    void HandleStopLocating(sptr<ILocatorCallback> callback);
    void HandlePowerSuspendChanged(int32_t pid, int32_t uid, int32_t flag);
    void UpdateRequestRecord(std::shared_ptr<Request> request, bool shouldInsert);
    void HandleRequest();
    void UpdateUsingPermission(std::shared_ptr<Request> request, const bool isStart);
    void HandlePermissionChanged(uint32_t tokenId);
    void RegisterLocationErrorCallback(sptr<ILocatorCallback> callback, AppIdentity appIdentity);
    void UnRegisterLocationErrorCallback(sptr<ILocatorCallback> callback);
    void ReportLocationError(const int errorCode, std::shared_ptr<Request> request);
    void SyncStillMovementState(bool state);
    void SyncIdleState(bool state);
    static RequestManager* GetInstance();
    void IsStandby();
    void UpdateLocationError(std::shared_ptr<Request> request);

private:
    bool RestorRequest(std::shared_ptr<Request> request);
    void HandleChrEvent(std::list<std::shared_ptr<Request>> requests);
    void UpdateRequestRecord(std::shared_ptr<Request> request, std::string abilityName, bool shouldInsert);
    void DeleteRequestRecord(std::shared_ptr<std::list<std::shared_ptr<Request>>> requests);
    void HandleRequest(std::string abilityName, std::list<std::shared_ptr<Request>> list);
    void ProxySendLocationRequest(std::string abilityName, WorkRecord& workRecord);
    sptr<IRemoteObject> GetRemoteObject(std::string abilityName);
    bool IsUidInProcessing(int32_t uid);
    void UpdateUsingApproximatelyPermission(std::shared_ptr<Request> request, const bool isStart);
    bool ActiveLocatingStrategies(const std::shared_ptr<Request>& request);
    bool AddRequestToWorkRecord(std::string abilityName, std::shared_ptr<Request>& request,
        std::shared_ptr<WorkRecord>& workRecord);
    bool IsRequestAvailable(std::shared_ptr<Request>& request);
    void UpdateRunningUids(const std::shared_ptr<Request>& request, std::string abilityName, bool isAdd);
    void ReportDataToResSched(std::string state, const pid_t uid);
    std::map<int32_t, int32_t> runningUidMap_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<LocationErrRequest>> locationErrorCallbackMap_;
    static ffrt::mutex requestMutex_;
    ffrt::mutex runningUidsMutex_;
    ffrt::mutex locationErrorCallbackMutex_;
    ffrt::mutex permissionRecordMutex_;
    std::atomic_bool isDeviceIdleMode_;
    std::atomic_bool isDeviceStillState_;
};

class LocatorErrCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    LocatorErrCallbackDeathRecipient(int32_t tokenId);
    ~LocatorErrCallbackDeathRecipient() override;
private:
    int32_t tokenId_;
};
} // namespace Location
} // namespace OHOS
#endif // REQUEST_MANAGER_H
