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

#ifndef GEOLOCATIONMANAGER_FFI_H
#define GEOLOCATIONMANAGER_FFI_H

#include <functional>
#include <string>
#include <vector>
#include <cstdint>
#include "ffi_remote_data.h"
#include "geolocationmanager_utils.h"


namespace OHOS {
namespace GeoLocationManager {
extern "C" {
FFI_EXPORT CJLocation FfiOHOSGeoLocationManagerGetLastLocation(int32_t* errCode);

FFI_EXPORT bool FfiOHOSGeoLocationManagerIsLocationEnabled(int32_t* errCode);

FFI_EXPORT bool FfiOHOSGeoLocationManagerIsGeocoderAvailable(int32_t* errCode);

FFI_EXPORT int32_t FfiOHOSGeoLocationManagerGetCachedGnssLocationsSize(int32_t* errCode);

FFI_EXPORT CJLocation FfiOHOSGeoLocationManagerGetCurrentLocation(int32_t* errCode);

FFI_EXPORT CJLocation FfiOHOSGeoLocationManagerGetCurrentLocationCurrent(CJCurrentLocationRequest request,
    int32_t* errCode);

FFI_EXPORT CJLocation FfiOHOSGeoLocationManagerGetCurrentLocationSingle(CJSingleLocationRequest request,
    int32_t* errCode);

FFI_EXPORT void FfiOHOSGeoLocationManagerFlushCachedGnssLocations(int32_t* errCode);

FFI_EXPORT void FfiOHOSGeoLocationManagerSendCommand(CJLocationCommand command, int32_t* errCode);

FFI_EXPORT CJCountryCode FfiOHOSGeoLocationManagerGetCountryCode(int32_t* errCode);

FFI_EXPORT CJGeoAddressArr FfiOHOSGeoLocationManagerGetAddressesFromLocation(CJReverseGeoCodeRequest request,
    int32_t* errCode);

FFI_EXPORT CJGeoAddressArr FfiOHOSGeoLocationManagerGetAddressesFromLocationName(CJGeoCodeRequest request,
    int32_t* errCode);

FFI_EXPORT int32_t FfiGeoLocationManagerOnLocationChange(CJLocationRequest request, int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOnContinuousLocationRequest(CJContinuousLocationRequest request,
    int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffLocationChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffLocationChangeAll();

FFI_EXPORT int32_t FfiGeoLocationManagerOnLocationError(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffLocationError(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffLocationErrorAll();

FFI_EXPORT int32_t FfiGeoLocationManagerOnLocationEnabledChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffLocationEnabledChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffLocationEnabledChangeAll();

FFI_EXPORT int32_t FfiGeoLocationManagerOnCachedGnssLocationsChange(CJCachedGnssLocationsRequest request,
    int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffCachedGnssLocationsChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffCachedGnssLocationsChangeAll();

FFI_EXPORT int32_t FfiGeoLocationManagerOnSatelliteStatusChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffSatelliteStatusChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffSatelliteStatusChangeAll();

FFI_EXPORT int32_t FfiGeoLocationManagerOnNmeaMessage(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffNmeaMessage(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffNmeaMessageAll();

FFI_EXPORT int32_t FfiGeoLocationManagerOnCountryCodeChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffCountryCodeChange(int64_t callbackId);

FFI_EXPORT int32_t FfiGeoLocationManagerOffCountryCodeChangeAll();

FFI_EXPORT CArrI32 FfiGeoLocationManagerGetGeofenceSupportedCoordTypes(int32_t* errCode);
}
}
}

#endif // GEOLOCATIONMANAGER_FFI_H