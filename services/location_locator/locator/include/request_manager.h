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
#include "iremote_stub.h"
#include "nocopyable.h"
#include "gnss_ability_proxy.h"
#include "network_ability_proxy.h"
#include "passive_ability_proxy.h"
#include "request.h"
#include "work_record.h"
#include "app_status_change_callback.h"
#include "app_mgr_proxy.h"

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
    void DoSuspend(const std::vector<SuspendAppInfo>& info);
    void DoActive(const std::vector<SuspendAppInfo>& info);
    void NotifyRequestManager(const std::vector<SuspendAppInfo>& info, int32_t flage);
    bool RegisterLocationStatusChangeCallback();
    bool UnregisterLocationStatusChangeCallback();
    bool IsForegroundApp(std::string& bundleName);
private:
    bool RestorRequest(std::shared_ptr<Request> request);
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
    sptr<AppExecFwk::IAppMgr> iAppMgr_;
    sptr<AppStatusChangeCallback> appStateObserver_;
};
class SuspendInfo {
public:
    inline const std::string& GetName() const
    {
        return name_;
    }
    inline int32_t GetUid() const
    {
        return uid_;
    }
    inline const std::vector<int32_t>& GetPids() const
    {
        return pids_;
    }
    inline void SetName(const std::string& name)
    {
        name_ = name;
    }
    inline void SetUid(int32_t uid)
    {
        uid_ = uid;
    }
    inline void SetPids(const std::vector<int32_t>& pids)
    {
        pids_ = pid;
    }
private:
    std::string name_;
    int32_t uid_ {-1};
    std::vector<int32_t> pids_;
};
} // namespace Location
} // namespace OHOS
#endif // REQUEST_MANAGER_H
