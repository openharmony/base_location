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

#include "fusion_controller.h"

#include "system_ability_definition.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "lbs_log.h"
#include "network_ability_proxy.h"

namespace OHOS {
namespace Location {
const uint32_t FUSION_DEFAULT_FLAG = 0;
const uint32_t FUSION_BASE_FLAG = 1;
const uint32_t REPORT_FUSED_LOCATION_FLAG = FUSION_BASE_FLAG;
const uint32_t QUICK_FIX_FLAG = FUSION_BASE_FLAG << 1;
const uint32_t NETWORK_SELF_REQUEST = 4;

void FusionController::ActiveFusionStrategies(int type)
{
    if (needReset_) {
        fusedFlag_ = FUSION_DEFAULT_FLAG;
        needReset_ = false;
    }
    switch (type) {
        case SCENE_NAVIGATION:
        case SCENE_TRAJECTORY_TRACKING:
            fusedFlag_ = fusedFlag_ | QUICK_FIX_FLAG;
            LBSLOGI(FUSION_CONTROLLER, "enable quick first fix");
            break;
        case PRIORITY_FAST_FIRST_FIX:
            fusedFlag_ = fusedFlag_ | REPORT_FUSED_LOCATION_FLAG;
            LBSLOGI(FUSION_CONTROLLER, "enable basic fused report");
            break;
        default:
            break;
    }
}

void FusionController::Process(std::string abilityName)
{
    needReset_ = true;
    if (GNSS_ABILITY.compare(abilityName) != 0) {
        return;
    }
    LBSLOGI(FUSION_CONTROLLER, "fused flag:%{public}d", fusedFlag_);
    RequestQuickFix(fusedFlag_ & QUICK_FIX_FLAG);
}

void FusionController::FuseResult(std::string abilityName, const std::unique_ptr<Location>& location)
{
    if (GNSS_ABILITY.compare(abilityName) == 0) {
        RequestQuickFix(false);
    }
}

void FusionController::RequestQuickFix(bool state)
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID);
    if (remoteObject == nullptr) {
        LBSLOGW(FUSION_CONTROLLER, "can not get network ability remote object");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor())) {
        return;
    }
    data.WriteBool(state);
    remoteObject->SendRequest(NETWORK_SELF_REQUEST, data, reply, option);
}
} // namespace Location
} // namespace OHOS