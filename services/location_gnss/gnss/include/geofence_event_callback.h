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

#ifndef GEOFENCE_EVENT_CALLBACK_H
#define GEOFENCE_EVENT_CALLBACK_H

#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE

#include <v2_0/igeofence_callback.h>

namespace OHOS {
namespace Location {
using HDI::Location::Geofence::V2_0::LocationInfo;
using HDI::Location::Geofence::V2_0::IGeofenceCallback;
using HDI::Location::Geofence::V2_0::GeofenceEvent;
using HDI::Location::Geofence::V2_0::GeofenceOperateType;
using HDI::Location::Geofence::V2_0::GeofenceOperateResult;
using HDI::Location::Geofence::V2_0::TYPE_DELETE;
using HDI::Location::Geofence::V2_0::GEOFENCE_OPERATION_SUCCESS;
using HDI::Location::Geofence::V2_0::TYPE_ADD;

class GeofenceEventCallback : public IGeofenceCallback {
public:
    ~GeofenceEventCallback() override {}
    int32_t ReportGeofenceAvailability(bool isAvailable) override;
    int32_t ReportGeofenceEvent(
        int32_t fenceIndex, const LocationInfo& location, GeofenceEvent event, int64_t timestamp) override;
    int32_t ReportGeofenceOperateResult(
        int32_t fenceIndex, GeofenceOperateType type, GeofenceOperateResult result) override;
};
}  // namespace Location
}  // namespace OHOS
#endif // HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
#endif // GEOFENCE_EVENT_CALLBACK_H
