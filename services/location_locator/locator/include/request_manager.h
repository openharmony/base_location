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
#include <mutex>
#include <singleton.h>
#include <string>

#include "iremote_object.h"

#include "i_locator_callback.h"
#include "request.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class RequestManager : public DelayedSingleton<RequestManager> {
public:
    RequestManager();
    ~RequestManager();
    bool InitSystemListeners();
    void HandleStartLocating(std::shared_ptr<Request> request);
    void HandleStopLocating(sptr<ILocatorCallback> callback);
    void HandlePowerSuspendChanged(int32_t pid, int32_t uid, int32_t flag);
    void UpdateRequestRecord(std::shared_ptr<Request> request, bool shouldInsert);
    void HandleRequest();
    void UpdateUsingPermission(std::shared_ptr<Request> request);
    void HandlePermissionChanged(uint32_t tokenId);
private:
    bool RestorRequest(std::shared_ptr<Request> request);
    void HandleChrEvent(std::list<std::shared_ptr<Request>> requests);
    void UpdateRequestRecord(std::shared_ptr<Request> request, std::string abilityName, bool shouldInsert);
    void DeleteRequestRecord(std::shared_ptr<std::list<std::shared_ptr<Request>>> requests);
    void HandleRequest(std::string abilityName, std::list<std::shared_ptr<Request>> list);
    void ProxySendLocationRequest(std::string abilityName, WorkRecord& workRecord);
    sptr<IRemoteObject> GetRemoteObject(std::string abilityName);
    bool IsUidInProcessing(int32_t uid);
    void UpdateUsingLocationPermission(std::shared_ptr<Request> request);
    void UpdateUsingApproximatelyPermission(std::shared_ptr<Request> request);
    void UpdateUsingBackgroundPermission(std::shared_ptr<Request> request);
    bool ActiveLocatingStrategies(const std::shared_ptr<Request>& request);
    bool AddRequestToWorkRecord(std::shared_ptr<Request>& request, std::shared_ptr<WorkRecord>& workRecord);
    bool IsRequestAvailable(std::shared_ptr<Request>& request);

    std::list<int32_t> runningUids_;
    static std::mutex requestMutex_;
    std::mutex permissionRecordMutex_;
};
} // namespace Location
} // namespace OHOS
#endif // REQUEST_MANAGER_H
