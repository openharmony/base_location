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

#ifndef FUSION_FENCE_CALLBACK_NAPI_H
#define FUSION_FENCE_CALLBACK_NAPI_H

#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "uv.h"
#include "common_utils.h"
#include "geofence_definition.h"
#include "i_fusion_fence_callback.h"
#include <atomic>

namespace OHOS {
namespace Location {
class FusionFenceCallbackNapi : public IRemoteStub<IFusionFenceCallback> {
public:
    FusionFenceCallbackNapi();
    virtual ~FusionFenceCallbackNapi();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool IsRemoteDied();
    void DeleteHandler();
    std::string GetIdentifier();
    void SetIdentifier(const std::string& identifier);
    LocationErrCode GetLastOperationErrorCode();
    void OnTransitionStatusChange(FusionFenceTransition transition) override;
    void OnReportOperateResult(const std::string& fenceId, int type, int errorCode) override;
    void CountDown();
    int GetCount() const;
    void SetCount(int count);
    void Wait(int timeMs);
    napi_ref GetHandleCb();
    void SetHandleCb(const napi_ref& handlerCb);
    napi_env GetEnv();
    void SetEnv(const napi_env& env);
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);

private:
    void InitLatch();
    void ResetLatch();
    napi_env env_;
    napi_ref handlerCb_;
    std::atomic<bool> remoteDied_;
    mutable std::mutex mutex_;
    mutable std::mutex operationResultMutex_;
    std::string identifier_;
    LocationErrCode errorCode_;
    CountDownLatch* latch_;
};
} // namespace Location
} // namespace OHOS
#endif // FUSION_FENCE_CALLBACK_NAPI_H