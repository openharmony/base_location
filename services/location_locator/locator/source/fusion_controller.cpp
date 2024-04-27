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
#include "location_log.h"
#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability_proxy.h"
#endif

namespace OHOS {
namespace Location {
const uint32_t FUSION_DEFAULT_FLAG = 0;
const uint32_t FUSION_BASE_FLAG = 1;
const uint32_t REPORT_FUSED_LOCATION_FLAG = FUSION_BASE_FLAG;

#ifdef FEATURE_NETWORK_SUPPORT
const uint32_t QUICK_FIX_FLAG = FUSION_BASE_FLAG << 1;
const uint32_t NETWORK_SELF_REQUEST = 4;
#endif
const long NANOS_PER_MILLI = 1000000L;
const long MAX_GNSS_LOCATION_COMPARISON_MS = 120 * MILLI_PER_SEC;
const long MAX_INDOOR_LOCATION_COMPARISON_MS = 5 * MILLI_PER_SEC;

void FusionController::ActiveFusionStrategies(int type)
{
    if (needReset_) {
        fusedFlag_ = FUSION_DEFAULT_FLAG;
        needReset_ = false;
    }
    switch (type) {
        case SCENE_NAVIGATION:
        case SCENE_TRAJECTORY_TRACKING:
            break;
        case PRIORITY_FAST_FIRST_FIX:
            fusedFlag_ = fusedFlag_ | REPORT_FUSED_LOCATION_FLAG;
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
    LBSLOGD(FUSION_CONTROLLER, "fused flag:%{public}u", fusedFlag_);
#ifdef FEATURE_NETWORK_SUPPORT
    RequestQuickFix(fusedFlag_ & QUICK_FIX_FLAG);
#endif
}

void FusionController::FuseResult(std::string abilityName, const std::unique_ptr<Location>& location)
{
    if (GNSS_ABILITY.compare(abilityName) == 0) {
#ifdef FEATURE_NETWORK_SUPPORT
        RequestQuickFix(false);
#endif
    }
}

#ifdef FEATURE_NETWORK_SUPPORT
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
#endif

std::unique_ptr<Location> FusionController::chooseBestLocation(const std::unique_ptr<Location>& location,
    const std::unique_ptr<Location>& lastFuseLocation)
{
    if (location == nullptr) {
        return nullptr;
    }
    if (lastFuseLocation == nullptr) {
        return std::make_unique<Location>(*location);
    }
    if (location->GetLocationSourceType() == LocationSourceType::INDOOR_TYPE) {
        return std::make_unique<Location>(*location);
    } else if (location->GetLocationSourceType() == LocationSourceType::GNSS_TYPE) {
        if (CheckIfLastIndoorLocationValid(location, std::make_unique<Location>(*lastFuseLocation))) {
            return std::make_unique<Location>(*lastFuseLocation);
        }
    } else if (location->GetLocationSourceType() == LocationSourceType::NETWORK_TYPE) {
        if (CheckIfLastIndoorLocationValid(location, std::make_unique<Location>(*lastFuseLocation))) {
            return std::make_unique<Location>(*lastFuseLocation);
        } else if (CheckIfLastGnssLocationValid(location, std::make_unique<Location>(*lastFuseLocation))) {
            return std::make_unique<Location>(*lastFuseLocation);
        }
    }
    return std::make_unique<Location>(*location);
}

bool FusionController::CheckIfLastIndoorLocationValid(const std::unique_ptr<Location>& location,
    const std::unique_ptr<Location>& lastFuseLocation)
{
    if (lastFuseLocation->GetLocationSourceType() == LocationSourceType::INDOOR_TYPE &&
            lastFuseLocation->GetTimeSinceBoot() / NANOS_PER_MILLI +
            MAX_INDOOR_LOCATION_COMPARISON_MS >= location->GetTimeSinceBoot() / NANOS_PER_MILLI) {
        return true;
    } else {
        return false;
    }
}

bool FusionController::CheckIfLastGnssLocationValid(const std::unique_ptr<Location>& location,
    const std::unique_ptr<Location>& lastFuseLocation)
{
    if (lastFuseLocation->GetLocationSourceType() == LocationSourceType::GNSS_TYPE &&
            lastFuseLocation->GetTimeSinceBoot() / NANOS_PER_MILLI +
            MAX_GNSS_LOCATION_COMPARISON_MS >= location->GetTimeSinceBoot() / NANOS_PER_MILLI) {
        return true;
    } else {
        return false;
    }
}

std::unique_ptr<Location> FusionController::GetFuseLocation(const std::unique_ptr<Location>& location,
    const sptr<Location>& lastFuseLocation)
{
    LBSLOGD(FUSION_CONTROLLER, " GetFuseLocation enter");
    auto bestLocation = chooseBestLocation(location, std::make_unique<Location>(*lastFuseLocation));
    return std::make_unique<Location>(*bestLocation);
}
} // namespace Location
} // namespace OHOS