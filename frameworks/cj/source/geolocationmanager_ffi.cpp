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

#include "geolocationmanager_ffi.h"
#include "geolocationmanager_impl.h"

namespace OHOS {
namespace GeoLocationManager {
extern "C" {
CJLocation FfiOHOSGeoLocationManagerGetLastLocation(int32_t* errCode)
{
    return GetLastLocation(*errCode);
}

bool FfiOHOSGeoLocationManagerIsLocationEnabled(int32_t* errCode)
{
    return IsLocationEnabled(*errCode);
}

bool FfiOHOSGeoLocationManagerIsGeocoderAvailable(int32_t* errCode)
{
    return IsGeocoderAvailable(*errCode);
}

int32_t FfiOHOSGeoLocationManagerGetCachedGnssLocationsSize(int32_t* errCode)
{
    return GetCachedGnssLocationsSize(*errCode);
}

CJLocation FfiOHOSGeoLocationManagerGetCurrentLocation(int32_t* errCode)
{
    return GetCurrentLocation(*errCode);
}

CJLocation FfiOHOSGeoLocationManagerGetCurrentLocationCurrent(CJCurrentLocationRequest request, int32_t* errCode)
{
    return GetCurrentLocationCurrent(request, *errCode);
}

CJLocation FfiOHOSGeoLocationManagerGetCurrentLocationSingle(CJSingleLocationRequest request, int32_t* errCode)
{
    return GetCurrentLocationSingle(request, *errCode);
}

FFI_EXPORT void FfiOHOSGeoLocationManagerFlushCachedGnssLocations(int32_t* errCode)
{
    return FlushCachedGnssLocations(*errCode);
}

void FfiOHOSGeoLocationManagerSendCommand(CJLocationCommand command, int32_t* errCode)
{
    return SendCommand(command, *errCode);
}

CJCountryCode FfiOHOSGeoLocationManagerGetCountryCode(int32_t* errCode)
{
    return GetCountryCode(*errCode);
}

CJGeoAddressArr FfiOHOSGeoLocationManagerGetAddressesFromLocation(CJReverseGeoCodeRequest request,
    int32_t* errCode)
{
    return GetAddressesFromLocation(request, *errCode);
}

CJGeoAddressArr FfiOHOSGeoLocationManagerGetAddressesFromLocationName(CJGeoCodeRequest request,
    int32_t* errCode)
{
    return GetAddressesFromLocationName(request, *errCode);
}

int32_t FfiGeoLocationManagerOnLocationChange(CJLocationRequest request, int64_t callbackId)
{
    return OnLocationRequest(request, callbackId);
}

int32_t FfiGeoLocationManagerOnContinuousLocationRequest(CJContinuousLocationRequest request, int64_t callbackId)
{
    return OnContinuousLocationRequest(request, callbackId);
}

int32_t FfiGeoLocationManagerOffLocationChange(int64_t callbackId)
{
    return OffLocationChange(callbackId);
}

int32_t FfiGeoLocationManagerOffLocationChangeAll()
{
    return OffLocationChangeAll();
}

int32_t FfiGeoLocationManagerOnLocationError(int64_t callbackId)
{
    return OnLocationError(callbackId);
}

int32_t FfiGeoLocationManagerOffLocationError(int64_t callbackId)
{
    return OffLocationError(callbackId);
}

int32_t FfiGeoLocationManagerOffLocationErrorAll()
{
    return OffLocationErrorAll();
}

int32_t FfiGeoLocationManagerOnLocationEnabledChange(int64_t callbackId)
{
    return OnLocationEnabledChange(callbackId);
}

int32_t FfiGeoLocationManagerOffLocationEnabledChange(int64_t callbackId)
{
    return OffLocationEnabledChange(callbackId);
}

int32_t FfiGeoLocationManagerOffLocationEnabledChangeAll()
{
    return OffLocationEnabledChangeAll();
}

int32_t FfiGeoLocationManagerOnCachedGnssLocationsChange(CJCachedGnssLocationsRequest request,
    int64_t callbackId)
{
    return OnCachedGnssLocationsChange(request, callbackId);
}

int32_t FfiGeoLocationManagerOffCachedGnssLocationsChange(int64_t callbackId)
{
    return OffCachedGnssLocationsChange(callbackId);
}

int32_t FfiGeoLocationManagerOffCachedGnssLocationsChangeAll()
{
    return OffCachedGnssLocationsChangeAll();
}

int32_t FfiGeoLocationManagerOnSatelliteStatusChange(int64_t callbackId)
{
    return OnSatelliteStatusChange(callbackId);
}

int32_t FfiGeoLocationManagerOffSatelliteStatusChange(int64_t callbackId)
{
    return OffSatelliteStatusChange(callbackId);
}

int32_t FfiGeoLocationManagerOffSatelliteStatusChangeAll()
{
    return OffSatelliteStatusChangeAll();
}

int32_t FfiGeoLocationManagerOnNmeaMessage(int64_t callbackId)
{
    return OnNmeaMessage(callbackId);
}

int32_t FfiGeoLocationManagerOffNmeaMessage(int64_t callbackId)
{
    return OffNmeaMessage(callbackId);
}

int32_t FfiGeoLocationManagerOffNmeaMessageAll()
{
    return OffNmeaMessageAll();
}

int32_t FfiGeoLocationManagerOnCountryCodeChange(int64_t callbackId)
{
    return OnCountryCodeChange(callbackId);
}

int32_t FfiGeoLocationManagerOffCountryCodeChange(int64_t callbackId)
{
    return OffCountryCodeChange(callbackId);
}

int32_t FfiGeoLocationManagerOffCountryCodeChangeAll()
{
    return OffCountryCodeChangeAll();
}

CArrI32 FfiGeoLocationManagerGetGeofenceSupportedCoordTypes(int32_t* errCode)
{
    return GetGeofenceSupportedCoordTypes(*errCode);
}
}
}
}
