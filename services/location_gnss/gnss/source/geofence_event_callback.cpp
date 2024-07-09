/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
#include "geofence_event_callback.h"
#include "location_log.h"
#include "gnss_ability.h"

#include "common_utils.h"

namespace OHOS {
namespace Location {
int32_t GeofenceEventCallback::ReportGeofenceAvailability(bool isAvailable)
{
    return 0;
}

int32_t GeofenceEventCallback::ReportGeofenceEvent(int32_t fenceIndex,
    const HDI::Location::Geofence::V2_0::LocationInfo& location, GeofenceEvent event, int64_t timestamp)
{
    LBSLOGD(GNSS, "ReportGeofenceEvent enter");
    auto gnssAbility = GnssAbility::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "gnssAbility is nullptr");
        return -1;
    }
    gnssAbility->ReportGeofenceEvent(fenceIndex, event);
    return 0;
}

int32_t GeofenceEventCallback::ReportGeofenceOperateResult(
    int32_t fenceIndex, GeofenceOperateType type, GeofenceOperateResult result)
{
    LBSLOGD(GNSS, "ReportGeofenceOperateResult enter");
    auto gnssAbility = GnssAbility::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "gnssAbility is nullptr");
        return -1;
    }
    gnssAbility->ReportGeofenceOperationResult(fenceIndex, type, result);
    return 0;
}
}  // namespace Location
}  // namespace OHOS
#endif
