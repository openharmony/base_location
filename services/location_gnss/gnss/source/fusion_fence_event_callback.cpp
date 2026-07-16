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

#include "fusion_fence_event_callback.h"
#include "fusion_fence_ability.h"
#include "location_log.h"

namespace OHOS {
namespace Location {

FusionFenceEventCallback::FusionFenceEventCallback()
{
    LBSLOGI(FUSION_FENCE, "%{public}s called", __func__);
}

FusionFenceEventCallback::~FusionFenceEventCallback()
{
    LBSLOGI(FUSION_FENCE, "%{public}s called", __func__);
}

void FusionFenceEventCallback::OnTransitionStatusChange(FusionFenceTransition transition)
{
    LBSLOGI(FUSION_FENCE, "%{public}s called, identifier: %{public}s, scene: %{public}d",
        __func__, transition.identifier.c_str(), static_cast<int>(transition.scene));
    auto fusionFenceAbility = FusionFenceAbility::GetInstance();
    if (fusionFenceAbility == nullptr) {
        LBSLOGE(FUSION_FENCE, "FusionFenceAbility is nullptr");
        return;
    }
    fusionFenceAbility->ReportFusionFenceEvent(transition);
}

int FusionFenceEventCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGI(FUSION_FENCE, "%{public}s called, code: %{public}d", __func__, code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(FUSION_FENCE, "%{public}s: token error", __func__);
        return -1;
    }
    switch (code) {
        case static_cast<uint32_t>(FusionFenceInterfaceCode::ON_TRANSITION_STATUS_CHANGE): {
            auto transition = FusionFenceTransition::Unmarshalling(data);
            if (transition != nullptr) {
                OnTransitionStatusChange(*transition);
                delete transition;
            } else {
                LBSLOGE(FUSION_FENCE, "Failed to unmarshal FusionFenceTransition");
                return -1;
            }
            break;
        }
        case static_cast<uint32_t>(FusionFenceInterfaceCode::REPORT_OPERATE_RESULT): {
            std::string fenceId = data.ReadString();
            int32_t type = data.ReadInt32();
            int32_t errorCode = data.ReadInt32();
            OnReportOperateResult(fenceId, type, errorCode);
            break;
        }
        default:
            LBSLOGE(FUSION_FENCE, "%{public}s: unknown code: %{public}d", __func__, code);
            return IRemoteStub<IFusionFenceCallback>::OnRemoteRequest(code, data, reply, option);
    }
    return 0;
}

void FusionFenceEventCallback::OnReportOperateResult(const std::string& fenceId, int type, int errorCode)
{
    LBSLOGI(FUSION_FENCE, "%{public}s called, fenceId: %{public}s, type: %{public}d, errorCode: %{public}d",
        __func__, fenceId.c_str(), type, errorCode);
}

} // namespace Location
} // namespace OHOS