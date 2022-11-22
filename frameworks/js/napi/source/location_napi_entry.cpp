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

#include "napi_util.h"
#include "location_log.h"
#include "location_napi_adapter.h"
#include "location_napi_event.h"
#include "location_napi_system.h"

namespace OHOS {
namespace Location {
#ifndef ENABLE_NAPI_MANAGER
/*
 * Module initialization function
 */
static napi_value Init(napi_env env, napi_value exports)
{
    LBSLOGI(LOCATION_NAPI, "Init,location_napi_entry");

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getLastLocation", GetLastLocation),
        DECLARE_NAPI_FUNCTION("isLocationEnabled", IsLocationEnabled),
        DECLARE_NAPI_FUNCTION("requestEnableLocation", RequestEnableLocation),
        DECLARE_NAPI_FUNCTION("enableLocation", EnableLocation),
        DECLARE_NAPI_FUNCTION("disableLocation", DisableLocation),
        DECLARE_NAPI_FUNCTION("getAddressesFromLocation", GetAddressesFromLocation),
        DECLARE_NAPI_FUNCTION("getAddressesFromLocationName", GetAddressesFromLocationName),
        DECLARE_NAPI_FUNCTION("isGeoServiceAvailable", IsGeoServiceAvailable),
        DECLARE_NAPI_FUNCTION("getCachedGnssLocationsSize", GetCachedGnssLocationsSize),
        DECLARE_NAPI_FUNCTION("flushCachedGnssLocations", FlushCachedGnssLocations),
        DECLARE_NAPI_FUNCTION("sendCommand", SendCommand),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getCurrentLocation", GetCurrentLocation),

        DECLARE_NAPI_FUNCTION("getLocation", GetLocation),
        DECLARE_NAPI_FUNCTION("getLocationType", GetLocationType),
        DECLARE_NAPI_FUNCTION("subscribe", Subscribe),
        DECLARE_NAPI_FUNCTION("unsubscribe", Unsubscribe),
        DECLARE_NAPI_FUNCTION("getSupportedCoordTypes", GetSupportedCoordTypes),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    return exports;
}

static napi_module g_locationModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "geolocation",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

#else

/*
 * Module initialization function
 */
static napi_value InitManager(napi_env env, napi_value exports)
{
    LBSLOGI(LOCATION_NAPI, "Init, location_napi_manager_entry");

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getCurrentLocation", GetCurrentLocation),
        DECLARE_NAPI_FUNCTION("getLastLocation", GetLastLocation),
        DECLARE_NAPI_FUNCTION("isLocationEnabled", IsLocationEnabled),
        DECLARE_NAPI_FUNCTION("enableLocation", EnableLocation),
        DECLARE_NAPI_FUNCTION("disableLocation", DisableLocation),
        DECLARE_NAPI_FUNCTION("requestEnableLocation", RequestEnableLocation),
        DECLARE_NAPI_FUNCTION("isGeocoderAvailable", IsGeoServiceAvailable),
        DECLARE_NAPI_FUNCTION("getAddressesFromLocation", GetAddressesFromLocation),
        DECLARE_NAPI_FUNCTION("getAddressesFromLocationName", GetAddressesFromLocationName),
        DECLARE_NAPI_FUNCTION("isLocationPrivacyConfirmed", IsLocationPrivacyConfirmed),
        DECLARE_NAPI_FUNCTION("setLocationPrivacyConfirmStatus", SetLocationPrivacyConfirmStatus),
        DECLARE_NAPI_FUNCTION("getCachedGnssLocationsSize", GetCachedGnssLocationsSize),
        DECLARE_NAPI_FUNCTION("flushCachedGnssLocations", FlushCachedGnssLocations),
        DECLARE_NAPI_FUNCTION("sendCommand", SendCommand),
        
        DECLARE_NAPI_FUNCTION("subscribe", Subscribe),
        DECLARE_NAPI_FUNCTION("unsubscribe", Unsubscribe),
        DECLARE_NAPI_FUNCTION("getLocation", GetLocation),
        DECLARE_NAPI_FUNCTION("getLocationType", GetLocationType),
        DECLARE_NAPI_FUNCTION("getCountryCode", GetIsoCountryCode),
        DECLARE_NAPI_FUNCTION("getSupportedCoordTypes", GetSupportedCoordTypes),
        DECLARE_NAPI_FUNCTION("enableLocationMock", EnableLocationMock),
        DECLARE_NAPI_FUNCTION("disableLocationMock", DisableLocationMock),
        DECLARE_NAPI_FUNCTION("setMockedLocations", SetMockedLocations),
        DECLARE_NAPI_FUNCTION("enableReverseGeocodingMock", EnableReverseGeocodingMock),
        DECLARE_NAPI_FUNCTION("disableReverseGeocodingMock", DisableReverseGeocodingMock),
        DECLARE_NAPI_FUNCTION("setReverseGeocodingMockInfo", SetReverseGeocodingMockInfo),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    return exports;
}

static napi_module g_locationManagerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = InitManager,
    .nm_modname = "geoLocationManager",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

#endif

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifndef ENABLE_NAPI_MANAGER
    napi_module_register(&g_locationModule);
#else
    napi_module_register(&g_locationManagerModule);
#endif
}
}  // namespace Location
}  // namespace OHOS
