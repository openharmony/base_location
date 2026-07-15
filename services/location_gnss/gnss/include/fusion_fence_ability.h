/*
* Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef FUSION_FENCE_ABILITY_H
#define FUSION_FENCE_ABILITY_H

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <ffrt.h>

#include "geofence_sdk.h"
#include "fusion_fence_request.h"
#include "iremote_object.h"
#include "hook_utils.h"
#include "fusion_fence_event_callback.h"
#include "geofence_definition.h"
#include "message_parcel.h"
#include "message_option.h"
#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace Location {

struct FusionFenceStruct{
    std::shared_ptr<FusionFenceRequest> request;
    sptr<IRemoteObject> callback;
    int requestCode;
    bool retCode;
};

class FusionFenceHandler;

class FusionFenceAbility {
public:
    static FusionFenceAbility* GetInstance();
    ~FusionFenceAbility();
 
    LocationErrCode AddFusionFence(std::shared_ptr<FusionFenceRequest>& request);
    LocationErrCode RemoveFusionFence(std::shared_ptr<FusionFenceRequest>& request);
    bool ExecuteFusionFenceProcess(std::shared_ptr<FusionFenceRequest>& request, int code);
    bool IsFusionFenceSupported();
    bool HasDuplicateAddFusionFenceRequest(const std::shared_ptr<FusionFenceRequest>& request);
    bool IsFusionFenceExists(const std::string& identifier);
    size_t GetGnssFenceCount();
    int GetGnssFenceCountForOneApp(const std::string& bundleName);
    LocationErrCode CheckFenceLimit(std::shared_ptr<FusionFenceRequest>& request);
    void AddFenceCount(std::shared_ptr<FusionFenceRequest>& request);
    void RemoveFenceCount(std::shared_ptr<FusionFenceRequest>& request);
    
    void RemoveFusionFenceByCallbackWhenAppDie(const sptr<IRemoteObject>& callback);
    void ReportFusionFenceEvent(const FusionFenceTransition& transition);
    void ReportOperateResult(std::shared_ptr<FusionFenceRequest> request, GnssGeofenceOperateType type,
        LocationErrCode errorCode);
    std::shared_ptr<FusionFenceRequest> GetFusionFenceRequest(const std::string& identifier);

private:
    FusionFenceAbility();
    void SubscribeSwingEvent();
    void UnSubscribeSwingEvent();
 
    std::vector<std::shared_ptr<FusionFenceRequest>> fusionFenceRequestList_;
    ffrt::mutex fusionFenceMutex_;
    sptr<FusionFenceEventCallback> fusionFenceEventCallback_;
    std::shared_ptr<FusionFenceHandler> fusionFenceHandler_;
    std::map<std::string, int> gnssFenceCountMap_;
    int totalGnssFenceCount_;
};

class FusionFenceHandler : public AppExecFwk::EventHandler {
public:
    explicit FusionFenceHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~FusionFenceHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    void InitFusionFenceEventProcessMap();
    void HandleRemoveFusionFence(const AppExecFwk::InnerEvent::Pointer& event);
 
    using FusionFenceEventProcessHandle = std::function<void(const AppExecFwk::InnerEvent::Pointer &)>;
    using FusionFenceEventProcessMap = std::map<uint32_t, FusionFenceEventProcessHandle>;
    FusionFenceEventProcessMap fusionFenceEventProcessMap_;
};

class FusionFenceCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    FusionFenceCallbackDeathRecipient();
    ~FusionFenceCallbackDeathRecipient() override;
};
} // namespace Location
} // namespace OHOS
#endif // FUSION_FENCE_ABILITY_H