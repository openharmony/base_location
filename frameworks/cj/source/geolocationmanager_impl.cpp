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

#include "constant_definition.h"
#include "geofence_sdk.h"
#include "geolocationmanager_impl.h"
#include "geolocationmanager_utils.h"
#include "cached_locations_callback.h"
#include "location_error_callback.h"
#include "location_switch_callback.h"
#include "locator_callback.h"
#include "gnss_status_callback.h"
#include "nmea_message_callback.h"
#include "country_code_callback.h"

namespace OHOS {
namespace GeoLocationManager {
std::list<sptr<LocatorCallback>> g_locationCallbacks;
std::list<sptr<LocationErrorCallback>> g_locationErrorCallbackHosts;
std::list<sptr<LocationSwitchCallback>> g_switchCallbacks;
std::list<sptr<CachedLocationsCallback>> g_cachedLocationCallbacks;
std::list<sptr<GnssStatusCallback>> g_gnssStatusInfoCallbacks;
std::list<sptr<NmeaMessageCallback>> g_nmeaCallbacks;
std::list<sptr<CountryCodeCallback>> g_countryCodeCallbacks;

std::unique_ptr<Location::CachedGnssLocationsRequest> cachedRequest =
    std::make_unique<Location::CachedGnssLocationsRequest>();

auto g_locatorProxy = Location::Locator::GetInstance();
auto g_geofenceClient = Location::GeofenceManager::GetInstance();

const int MIN_TIMEOUTMS_FOR_LOCATIONONCE = 1000;

sptr<LocatorCallback> CreateSingleLocationCallbackHost()
{
    auto callbackHost =
        sptr<LocatorCallback>(new (std::nothrow) LocatorCallback());
    if (callbackHost) {
        callbackHost->SetFixNumber(1);
    }
    return callbackHost;
}

bool CjIsRequestConfigValid(const std::unique_ptr<Location::RequestConfig>& config)
{
    if (config == nullptr) {
        return false;
    }
    if ((config->GetScenario() > Location::SCENE_NO_POWER || config->GetScenario() < Location::SCENE_UNSET) &&
        (config->GetScenario() > Location::LOCATION_SCENE_DAILY_LIFE_SERVICE ||
        config->GetScenario() < Location::LOCATION_SCENE_NAVIGATION) &&
        (config->GetScenario() > Location::LOCATION_SCENE_NO_POWER_CONSUMPTION ||
        config->GetScenario() < Location::LOCATION_SCENE_HIGH_POWER_CONSUMPTION)) {
        return false;
    }
    if ((config->GetPriority() > Location::PRIORITY_FAST_FIRST_FIX ||
        config->GetPriority() < Location::PRIORITY_UNSET) &&
        (config->GetPriority() > Location::LOCATION_PRIORITY_LOCATING_SPEED ||
        config->GetPriority() < Location::LOCATION_PRIORITY_ACCURACY)) {
        return false;
    }
    if (config->GetTimeOut() < MIN_TIMEOUTMS_FOR_LOCATIONONCE) {
        return false;
    }
    if (config->GetTimeInterval() < 0) {
        return false;
    }
    if (config->GetDistanceInterval() < 0) {
        return false;
    }
    if (config->GetMaxAccuracy() < 0) {
        return false;
    }
    return true;
}

Location::LocationErrCode CheckLocationSwitchEnable()
{
    bool isEnabled = false;
    Location::LocationErrCode errorCode = g_locatorProxy->IsLocationEnabledV9(isEnabled);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (!isEnabled) {
        return Location::LocationErrCode::ERRCODE_SWITCH_OFF;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int CjGetCurrentLocationType(std::unique_ptr<Location::RequestConfig>& config)
{
    if (config->GetPriority() == Location::LOCATION_PRIORITY_ACCURACY ||
        (config->GetScenario() == Location::SCENE_UNSET && config->GetPriority() == Location::PRIORITY_ACCURACY) ||
        config->GetScenario() == Location::SCENE_NAVIGATION ||
        config->GetScenario() == Location::SCENE_TRAJECTORY_TRACKING ||
        config->GetScenario() == Location::SCENE_CAR_HAILING) {
        return Location::LOCATION_PRIORITY_ACCURACY;
    } else {
        return Location::LOCATION_PRIORITY_LOCATING_SPEED;
    }
}

Location::LocationErrCode CheckLocationSwitchState()
{
    bool isEnabled = false;
    Location::LocationErrCode errorCode = g_locatorProxy->IsLocationEnabledV9(isEnabled);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (!isEnabled) {
        return Location::LocationErrCode::ERRCODE_SWITCH_OFF;
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

CJLocation GetLastLocation(int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return CJLocation{0};
    }
    errCode = CheckLocationSwitchState();
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return CJLocation{0};
    }
    std::unique_ptr<Location::Location> loc;
    errCode = g_locatorProxy->GetCachedLocationV9(loc);
    if (loc == nullptr) {
        return CJLocation{0};
    }
    return NativeLocationToCJLocation(*loc);
}

bool IsLocationEnabled(int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return false;
    }
    bool isEnabled = false;
    errCode = g_locatorProxy->IsLocationEnabledV9(isEnabled);
    return isEnabled;
}

bool IsGeocoderAvailable(int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return false;
    }

    bool isAvailable = false;
    errCode = g_locatorProxy->IsGeoServiceAvailableV9(isAvailable);
    return isAvailable;
}

int32_t GetCachedGnssLocationsSize(int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return 0;
    }
    errCode = CheckLocationSwitchState();
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return 0;
    }
    int32_t locationSize = -1;
    g_locatorProxy->GetCachedGnssLocationsSizeV9(locationSize);
    errCode = Location::LocationErrCode::ERRCODE_NOT_SUPPORTED;
    return locationSize;
}

CJLocation GetCurrentLocation(int32_t& errCode)
{
    auto requestConfig = std::make_unique<Location::RequestConfig>();
    requestConfig->SetPriority(Location::PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(1);

    if (!CjIsRequestConfigValid(requestConfig)) {
        errCode = Location::LocationErrCode::ERRCODE_INVALID_PARAM;
        return CJLocation{0};
    }
    return GetCurrentLocation(requestConfig, errCode);
}

CJLocation GetCurrentLocationCurrent(CJCurrentLocationRequest request, int32_t& errCode)
{
    auto requestConfig = std::make_unique<Location::RequestConfig>();
    CJCurrentLocationRequestToRequestConfig(request, requestConfig);
    return GetCurrentLocation(requestConfig, errCode);
}

CJLocation GetCurrentLocationSingle(CJSingleLocationRequest request, int32_t& errCode)
{
    auto requestConfig = std::make_unique<Location::RequestConfig>();
    CJSingleLocationRequestRequestToRequestConfig(request, requestConfig);
    return GetCurrentLocation(requestConfig, errCode);
}

CJLocation GetCurrentLocation(std::unique_ptr<Location::RequestConfig>& requestConfig, int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return CJLocation{0};
    }
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    if (singleLocatorCallbackHost == nullptr) {
        errCode = Location::LocationErrCode::ERRCODE_INVALID_PARAM;
        return CJLocation{0};
    }
    singleLocatorCallbackHost->SetLocationPriority(CjGetCurrentLocationType(requestConfig));
    Location::LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        errCode = Location::LocationErrCode::ERRCODE_INVALID_PARAM;
        return CJLocation{0};
    }

    auto timeout = requestConfig->GetTimeOut();
    auto request = std::move(requestConfig);

    if (g_locatorProxy->IsLocationEnabled()) {
        auto callbackPtr = sptr<Location::ILocatorCallback>(singleLocatorCallbackHost);
        errCode = g_locatorProxy->StartLocatingV9(request, callbackPtr);
        if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            singleLocatorCallbackHost->SetCount(0);
            return CJLocation{0};
        }
        if (timeout > Location::DEFAULT_TIMEOUT_30S) {
            singleLocatorCallbackHost->Wait(Location::DEFAULT_TIMEOUT_30S);
            if (singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
                singleLocatorCallbackHost->Wait(timeout - Location::DEFAULT_TIMEOUT_30S);
            }
        } else {
            singleLocatorCallbackHost->Wait(timeout);
        }
        g_locatorProxy->StopLocating(callbackPtr);
        if (singleLocatorCallbackHost->GetCount() != 0 &&
            singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
            errCode = Location::ERRCODE_LOCATING_FAIL;
            return CJLocation{0};
        }
        singleLocatorCallbackHost->SetCount(1);
    }

    std::unique_ptr<Location::Location> location =
        std::make_unique<Location::Location>(*singleLocatorCallbackHost->GetSingleLocation());
    errCode = Location::LocationErrCode::ERRCODE_SUCCESS;
    if (singleLocatorCallbackHost) {
        singleLocatorCallbackHost = nullptr;
    }
    return NativeLocationToCJLocation(*location);
}

void FlushCachedGnssLocations(int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return;
    }
    Location::LocationErrCode errorCode = CheckLocationSwitchState();
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        errCode = errorCode;
        return;
    }
    g_locatorProxy->FlushCachedGnssLocationsV9();
    errCode = Location::LocationErrCode::ERRCODE_NOT_SUPPORTED;
}

void SendCommand(CJLocationCommand command, int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return;
    }
    auto ptr = std::make_unique<Location::LocationCommand>();
    ptr->scenario = command.scenario;
    ptr->command = command.command;
    errCode = g_locatorProxy->SendCommandV9(ptr);
}

CJCountryCode GetCountryCode(int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return CJCountryCode{0};
    }
    std::shared_ptr<Location::CountryCode> country = std::make_shared<Location::CountryCode>();
    errCode = g_locatorProxy->GetIsoCountryCodeV9(country);
    return CJCountryCode{ .country = MallocCString(country->GetCountryCodeStr()),
        .type = country->GetCountryCodeType() };
}

CJGeoAddressArr GetAddressesFromLocation(CJReverseGeoCodeRequest request, int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return CJGeoAddressArr{0};
    }
    MessageParcel reverseGeoCodeRequest;
    bool ret = CJReverseGeoCodeRequestToMessageParcel(request, reverseGeoCodeRequest);
    if (!ret) {
        errCode = Location::LocationErrCode::ERRCODE_INVALID_PARAM;
        return CJGeoAddressArr{0};
    }

    bool isAvailable = false;
    errCode = g_locatorProxy->IsGeoServiceAvailableV9(isAvailable);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return CJGeoAddressArr{0};
    }
    if (!isAvailable) {
        errCode = Location::LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL;
        return CJGeoAddressArr{0};
    }
    std::list<std::shared_ptr<Location::GeoAddress>> replyList;
    errCode = g_locatorProxy->GetAddressByCoordinateV9(reverseGeoCodeRequest, replyList);
    if (replyList.empty() || errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return CJGeoAddressArr{0};
    }
    return ListGeoAddressToCJGeoAddressArr(replyList);
}

CJGeoAddressArr GetAddressesFromLocationName(CJGeoCodeRequest request, int32_t& errCode)
{
    if (g_locatorProxy == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return CJGeoAddressArr{0};
    }
    MessageParcel geoCodeRequest;
    bool ret = CJGeoCodeRequestToMessageParcel(request, geoCodeRequest);
    if (!ret) {
        errCode = Location::LocationErrCode::ERRCODE_INVALID_PARAM;
        return CJGeoAddressArr{0};
    }

    bool isAvailable = false;
    errCode = g_locatorProxy->IsGeoServiceAvailableV9(isAvailable);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return CJGeoAddressArr{0};
    }
    if (!isAvailable) {
        errCode = Location::LocationErrCode::ERRCODE_GEOCODING_FAIL;
        return CJGeoAddressArr{0};
    }
    std::list<std::shared_ptr<Location::GeoAddress>> replyList;
    errCode = g_locatorProxy->GetAddressByLocationNameV9(geoCodeRequest, replyList);
    if (replyList.empty() || errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return CJGeoAddressArr{0};
    }
    return ListGeoAddressToCJGeoAddressArr(replyList);
}

int32_t OnLocationChange(std::unique_ptr<OHOS::Location::RequestConfig>& requestConfig, int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    Location::LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (!CjIsRequestConfigValid(requestConfig)) {
        return Location::LocationErrCode::ERRCODE_INVALID_PARAM;
    }
    auto locatorCallbackHost = sptr<LocatorCallback>(new (std::nothrow) LocatorCallback(callbackId));
    if (locatorCallbackHost == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto locatorCallback = sptr<Location::ILocatorCallback>(locatorCallbackHost);
    errorCode = g_locatorProxy->StartLocatingV9(requestConfig, locatorCallback);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    g_locationCallbacks.push_back(locatorCallbackHost);
    return errorCode;
}

int32_t OnLocationRequest(CJLocationRequest request, int64_t callbackId)
{
    auto requestConfig = std::make_unique<Location::RequestConfig>();
    CJLocationRequestToRequestConfig(request, requestConfig);
    return OnLocationChange(requestConfig, callbackId);
}

int32_t OnContinuousLocationRequest(CJContinuousLocationRequest request, int64_t callbackId)
{
    auto requestConfig = std::make_unique<Location::RequestConfig>();
    CJContinuousLocationRequestToRequestConfig(request, requestConfig);
    return OnLocationChange(requestConfig, callbackId);
}

int32_t OffLocationChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_locationCallbacks.begin(); it != g_locationCallbacks.end();) {
        if (*it == nullptr) {
            it = g_locationCallbacks.erase(it);
            continue;
        }
        if (((**it).GetCallBackId() != callbackId)) {
            ++it;
            continue;
        }
        auto locatorCallback = sptr<Location::ILocatorCallback>(*it);
        Location::LocationErrCode errorCode = g_locatorProxy->StopLocatingV9(locatorCallback);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_locationCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OffLocationChangeAll()
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_locationCallbacks.begin(); it != g_locationCallbacks.end();) {
        if (*it == nullptr) {
            it = g_locationCallbacks.erase(it);
            continue;
        }
        auto locatorCallback = sptr<Location::ILocatorCallback>(*it);
        Location::LocationErrCode errorCode = g_locatorProxy->StopLocatingV9(locatorCallback);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_locationCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OnLocationError(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto locationErrorCallbackHost = sptr<LocationErrorCallback>(new (std::nothrow) LocationErrorCallback(callbackId));
    if (locationErrorCallbackHost == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto locationErrorCallback = sptr<Location::ILocatorCallback>(locationErrorCallbackHost);
    Location::LocationErrCode errorCode = g_locatorProxy->SubscribeLocationError(locationErrorCallback);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    g_locationErrorCallbackHosts.push_back(locationErrorCallbackHost);
    return errorCode;
}

int32_t OffLocationError(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_locationErrorCallbackHosts.begin(); it != g_locationErrorCallbackHosts.end();) {
        if (*it == nullptr) {
            it = g_locationErrorCallbackHosts.erase(it);
            continue;
        }
        if (((**it).GetCallBackId() != callbackId)) {
            ++it;
            continue;
        }
        auto locatorCallback = sptr<Location::ILocatorCallback>(*it);
        Location::LocationErrCode errorCode = g_locatorProxy->UnSubscribeLocationError(locatorCallback);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_locationErrorCallbackHosts.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OffLocationErrorAll()
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_locationErrorCallbackHosts.begin(); it != g_locationErrorCallbackHosts.end();) {
        if (*it == nullptr) {
            it = g_locationErrorCallbackHosts.erase(it);
            continue;
        }
        auto locatorCallback = sptr<Location::ILocatorCallback>(*it);
        Location::LocationErrCode errorCode = g_locatorProxy->UnSubscribeLocationError(locatorCallback);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_locationErrorCallbackHosts.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OnLocationEnabledChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto switchCallbackHost = sptr<LocationSwitchCallback>(new (std::nothrow) LocationSwitchCallback(callbackId));
    if (switchCallbackHost == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    Location::LocationErrCode errorCode = g_locatorProxy->RegisterSwitchCallbackV9(switchCallbackHost);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    g_switchCallbacks.push_back(switchCallbackHost);
    return errorCode;
}

int32_t OffLocationEnabledChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_switchCallbacks.begin(); it != g_switchCallbacks.end();) {
        if (*it == nullptr) {
            it = g_switchCallbacks.erase(it);
            continue;
        }
        if (((**it).GetCallBackId() != callbackId)) {
            ++it;
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterSwitchCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_switchCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OffLocationEnabledChangeAll()
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_switchCallbacks.begin(); it != g_switchCallbacks.end();) {
        if (*it == nullptr) {
            it = g_switchCallbacks.erase(it);
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterSwitchCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_switchCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OnCachedGnssLocationsChange(CJCachedGnssLocationsRequest request,
    int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    CJCachedGnssLocationsRequestToCachedLocationRequest(request, cachedRequest);
    auto cachedCallbackHost = sptr<CachedLocationsCallback>(new (std::nothrow) CachedLocationsCallback(callbackId));
    if (cachedCallbackHost == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    Location::LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    auto cachedCallback = sptr<Location::ICachedLocationsCallback>(cachedCallbackHost);
    errorCode = g_locatorProxy->RegisterCachedLocationCallbackV9(cachedRequest, cachedCallback);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return Location::LocationErrCode::ERRCODE_NOT_SUPPORTED;
    }
    g_cachedLocationCallbacks.push_back(cachedCallbackHost);
    return errorCode;
}

int32_t OffCachedGnssLocationsChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_cachedLocationCallbacks.begin(); it != g_cachedLocationCallbacks.end();) {
        if (*it == nullptr) {
            it = g_cachedLocationCallbacks.erase(it);
            continue;
        }
        if (((**it).GetCallBackId() != callbackId)) {
            ++it;
            continue;
        }
        auto cachedCallback = sptr<Location::ICachedLocationsCallback>(*it);
        g_locatorProxy->UnregisterCachedLocationCallbackV9(cachedCallback);
        Location::LocationErrCode errorCode = Location::LocationErrCode::ERRCODE_NOT_SUPPORTED;
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_cachedLocationCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OffCachedGnssLocationsChangeAll()
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_cachedLocationCallbacks.begin(); it != g_cachedLocationCallbacks.end();) {
        if (*it == nullptr) {
            it = g_cachedLocationCallbacks.erase(it);
            continue;
        }
        auto cachedCallback = sptr<Location::ICachedLocationsCallback>(*it);
        g_locatorProxy->UnregisterCachedLocationCallbackV9(cachedCallback);
        Location::LocationErrCode errorCode = Location::LocationErrCode::ERRCODE_NOT_SUPPORTED;
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_cachedLocationCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OnSatelliteStatusChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto gnssCallbackHost = sptr<GnssStatusCallback>(new (std::nothrow) GnssStatusCallback(callbackId));
    if (gnssCallbackHost == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    Location::LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    errorCode = g_locatorProxy->RegisterGnssStatusCallbackV9(gnssCallbackHost);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    g_gnssStatusInfoCallbacks.push_back(gnssCallbackHost);
    return errorCode;
}

int32_t OffSatelliteStatusChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_gnssStatusInfoCallbacks.begin(); it != g_gnssStatusInfoCallbacks.end();) {
        if (*it == nullptr) {
            it = g_gnssStatusInfoCallbacks.erase(it);
            continue;
        }
        if (((**it).GetCallBackId() != callbackId)) {
            ++it;
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterGnssStatusCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_gnssStatusInfoCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OffSatelliteStatusChangeAll()
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_gnssStatusInfoCallbacks.begin(); it != g_gnssStatusInfoCallbacks.end();) {
        if (*it == nullptr) {
            it = g_gnssStatusInfoCallbacks.erase(it);
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterGnssStatusCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_gnssStatusInfoCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OnNmeaMessage(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto nmeaCallbackHost = sptr<NmeaMessageCallback>(new (std::nothrow) NmeaMessageCallback(callbackId));
    if (nmeaCallbackHost == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    Location::LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    errorCode = g_locatorProxy->RegisterNmeaMessageCallbackV9(nmeaCallbackHost);
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    g_nmeaCallbacks.push_back(nmeaCallbackHost);
    return errorCode;
}

int32_t OffNmeaMessage(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_nmeaCallbacks.begin(); it != g_nmeaCallbacks.end();) {
        if (*it == nullptr) {
            it = g_nmeaCallbacks.erase(it);
            continue;
        }
        if (((**it).GetCallBackId() != callbackId)) {
            ++it;
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterNmeaMessageCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_nmeaCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OffNmeaMessageAll()
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_nmeaCallbacks.begin(); it != g_nmeaCallbacks.end();) {
        if (*it == nullptr) {
            it = g_nmeaCallbacks.erase(it);
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterNmeaMessageCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_nmeaCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OnCountryCodeChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto callbackHost = sptr<CountryCodeCallback>(new (std::nothrow) CountryCodeCallback(callbackId));
    if (callbackHost == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    auto callbackPtr = sptr<Location::ICountryCodeCallback>(callbackHost);
    Location::LocationErrCode errorCode = g_locatorProxy->RegisterCountryCodeCallbackV9(callbackPtr->AsObject());
    if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return errorCode;
    }
    g_countryCodeCallbacks.push_back(callbackHost);
    return errorCode;
}

int32_t OffCountryCodeChange(int64_t callbackId)
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_countryCodeCallbacks.begin(); it != g_countryCodeCallbacks.end();) {
        if (*it == nullptr) {
            it = g_countryCodeCallbacks.erase(it);
            continue;
        }
        if (((**it).GetCallBackId() != callbackId)) {
            ++it;
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterCountryCodeCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_countryCodeCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

int32_t OffCountryCodeChangeAll()
{
    if (g_locatorProxy == nullptr) {
        return ERRCODE_MEMORY_ERROR;
    }
    for (auto it = g_countryCodeCallbacks.begin(); it != g_countryCodeCallbacks.end();) {
        if (*it == nullptr) {
            it = g_countryCodeCallbacks.erase(it);
            continue;
        }
        Location::LocationErrCode errorCode = g_locatorProxy->UnregisterCountryCodeCallbackV9(*it);
        if (errorCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
            return errorCode;
        }
        it = g_countryCodeCallbacks.erase(it);
    }
    return Location::LocationErrCode::ERRCODE_SUCCESS;
}

CArrI32 GetGeofenceSupportedCoordTypes(int32_t& errCode)
{
    CArrI32 res = CArrI32{0};
    if (g_geofenceClient == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return res;
    }
    std::vector<Location::CoordinateSystemType> coordinateSystemTypes;
    errCode = g_geofenceClient->GetGeofenceSupportedCoordTypes(coordinateSystemTypes);
    if (errCode != Location::LocationErrCode::ERRCODE_SUCCESS) {
        return res;
    }
    if (coordinateSystemTypes.size() == 0) {
        return res;
    }
    int32_t* head = static_cast<int32_t*>(malloc(sizeof(int32_t) * coordinateSystemTypes.size()));
    if (head == nullptr) {
        errCode = ERRCODE_MEMORY_ERROR;
        return res;
    }
    for (size_t i = 0; i < coordinateSystemTypes.size(); i++) {
        head[i] = coordinateSystemTypes[i];
    }
    res.head = head;
    res.size = static_cast<int64_t>(coordinateSystemTypes.size());
    return res;
}
}
}
