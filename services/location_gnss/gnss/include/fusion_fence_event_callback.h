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

#ifndef FUSION_FENCE_EVENT_CALLBACK_H
#define FUSION_FENCE_EVENT_CALLBACK_H

#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "i_fusion_fence_callback.h"
#include "fusion_fence_request.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {

class FusionFenceEventCallback : public IRemoteStub<IFusionFenceCallback> {
public:
    FusionFenceEventCallback();
    virtual ~FusionFenceEventCallback();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnTransitionStatusChange(FusionFenceTransition transition) override;
    void OnReportOperateResult(const std::string& fenceId, int type, int errorCode) override;
};

} // namespace Location
} // namespace OHOS
#endif // FUSION_FENCE_EVENT_CALLBACK_H