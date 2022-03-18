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

#include <list>
#include <map>
#include <mutex>
#include <singleton.h>
#include <string>

#include "iremote_stub.h"
#include "nocopyable.h"

#include "gnss_ability_proxy.h"
#include "network_ability_proxy.h"
#include "passive_ability_proxy.h"
#include "request.h"
#include "work_record.h"

#ifndef BASE_REQUEST_MANAGER_H
#define BASE_REQUEST_MANAGER_H

namespace OHOS {
namespace Location {
class RequestManager : public DelayedSingleton<RequestManager> {
public:
    RequestManager();
    ~RequestManager();
    bool InitSystemListeners();
    void HandleStartLocating(std::shared_ptr<Request> request);
    void HandleStopLocating(sptr<ILocatorCallback> callback);
    void HandlePermissionChanged(int32_t uid);
    void HandlePowerSuspendChanged(int32_t pid, int32_t uid, int32_t flag);
    void UpdateRequestRecord(std::shared_ptr<Request> request, bool shouldInsert);
    void HandleRequest();
private:
    bool RestorRequest(std::shared_ptr<Request> newRequest);
    void UpdateRequestRecord(std::shared_ptr<Request> request, std::string abilityName, bool shouldInsert);
    void DeleteRequestRecord(std::shared_ptr<std::list<std::shared_ptr<Request>>> requests);
    void HandleRequest(std::string abilityName);
    void ProxySendLocationRequest(std::string abilityName, WorkRecord& workRecord, int timeInterval);
    sptr<IRemoteObject> GetRemoteObject(std::string abilityName);
    bool IsUidInProcessing(int32_t uid);

    bool isPermissionRegistered_ = false;
    bool isPowerRegistered_ = false;
    std::list<int32_t> runningUids_;
    static std::mutex requestMutex;
};
} // namespace Location
} // namespace OHOS
#endif // BASE_REQUEST_MANAGER_H
