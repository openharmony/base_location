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

#ifndef GEOLOCATIONMANAGER_IMPL_H
#define GEOLOCATIONMANAGER_IMPL_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "locator.h"
#include "location_napi_event.h"
#include "geolocationmanager_utils.h"

namespace OHOS {
namespace GeoLocationManager {
CJLocation GetLastLocation(int32_t& errCode);

bool IsLocationEnabled(int32_t& errCode);

bool IsGeocoderAvailable(int32_t& errCode);

int32_t GetCachedGnssLocationsSize(int32_t& errCode);

CJLocation GetCurrentLocation(int32_t& errCode);

CJLocation GetCurrentLocationCurrent(CJCurrentLocationRequest request, int32_t& errCode);

CJLocation GetCurrentLocationSingle(CJSingleLocationRequest request, int32_t& errCode);

CJLocation GetCurrentLocation(std::unique_ptr<Location::RequestConfig>& requestConfig, int32_t& errCode);

void FlushCachedGnssLocations(int32_t& errCode);

void SendCommand(CJLocationCommand command, int32_t& errCode);

CJCountryCode GetCountryCode(int32_t& errCode);

CJGeoAddressArr GetAddressesFromLocation(CJReverseGeoCodeRequest request, int32_t& errCode);

CJGeoAddressArr GetAddressesFromLocationName(CJGeoCodeRequest request, int32_t& errCode);

int32_t OnLocationRequest(CJLocationRequest request, int64_t callbackId);

int32_t OnContinuousLocationRequest(CJContinuousLocationRequest request, int64_t callbackId);

int32_t OffLocationChange(int64_t callbackId);

int32_t OffLocationChangeAll();

int32_t OnLocationError(int64_t callbackId);

int32_t OffLocationError(int64_t callbackId);

int32_t OffLocationErrorAll();

int32_t OnLocationEnabledChange(int64_t callbackId);

int32_t OffLocationEnabledChange(int64_t callbackId);

int32_t OffLocationEnabledChangeAll();

int32_t OnCachedGnssLocationsChange(CJCachedGnssLocationsRequest request,
    int64_t callbackId);

int32_t OffCachedGnssLocationsChange(int64_t callbackId);

int32_t OffCachedGnssLocationsChangeAll();

int32_t OnSatelliteStatusChange(int64_t callbackId);

int32_t OffSatelliteStatusChange(int64_t callbackId);

int32_t OffSatelliteStatusChangeAll();

int32_t OnNmeaMessage(int64_t callbackId);

int32_t OffNmeaMessage(int64_t callbackId);

int32_t OffNmeaMessageAll();

int32_t OnCountryCodeChange(int64_t callbackId);

int32_t OffCountryCodeChange(int64_t callbackId);

int32_t OffCountryCodeChangeAll();

CArrI32 GetGeofenceSupportedCoordTypes(int32_t& errCode);
}
}

#endif // GEOLOCATIONMANAGER_IMPL_H