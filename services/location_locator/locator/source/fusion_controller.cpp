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
#include "hook_utils.h"
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
#endif
const long NANOS_PER_MILLI = 1000000L;
const long MAX_GNSS_LOCATION_COMPARISON_MS = 30 * MILLI_PER_SEC;
const long MAX_INDOOR_LOCATION_COMPARISON_MS = 5 * MILLI_PER_SEC;
const double MAX_INDOOR_LOCATION_SPEED = 3.0;

FusionController* FusionController::GetInstance()
{
    static FusionController data;
    return &data;
}

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

std::unique_ptr<Location> FusionController::chooseBestLocation(const std::unique_ptr<Location>& location,
    const std::unique_ptr<Location>& lastFuseLocation)
{
    if (location == nullptr) {
        return nullptr;
    }
    if (lastFuseLocation == nullptr) {
        return std::make_unique<Location>(*location);
    }
    LocationFusionInfo fusionInfo;
    fusionInfo.location = *location;
    fusionInfo.lastFuseLocation = *lastFuseLocation;
    fusionInfo.resultLocation = fusionInfo.location;
    if (fusionInfo.location.GetLocationSourceType() == LocationSourceType::NETWORK_TYPE) {
        if (CheckIfLastGnssLocationValid(location, std::make_unique<Location>(fusionInfo.lastFuseLocation))) {
            fusionInfo.resultLocation = fusionInfo.lastFuseLocation;
        }
    }
    HookUtils::ExecuteHook(
        LocationProcessStage::FUSION_REPORT_PROCESS, (void *)&fusionInfo, nullptr);
    return std::make_unique<Location>(fusionInfo.resultLocation);
}

bool FusionController::CheckIfLastGnssLocationValid(const std::unique_ptr<Location>& location,
    const std::unique_ptr<Location>& lastFuseLocation)
{
    if ((lastFuseLocation->GetLocationSourceType() == LocationSourceType::GNSS_TYPE ||
        lastFuseLocation->GetLocationSourceType() == LocationSourceType::RTK_TYPE) &&
        ((location->GetTimeSinceBoot() / NANOS_PER_MILLI -
        lastFuseLocation->GetTimeSinceBoot() / NANOS_PER_MILLI) < MAX_GNSS_LOCATION_COMPARISON_MS)) {
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