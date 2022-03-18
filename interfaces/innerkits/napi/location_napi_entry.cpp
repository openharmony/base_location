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

#include "location_util.h"
#include "lbs_log.h"
#include "location_napi_adapter.h"
#include "location_napi_event.h"

namespace OHOS {
namespace Location {
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
        DECLARE_NAPI_FUNCTION("isLocationPrivacyConfirmed", IsLocationPrivacyConfirmed),
        DECLARE_NAPI_FUNCTION("setLocationPrivacyConfirmStatus", SetLocationPrivacyConfirmStatus),
        DECLARE_NAPI_FUNCTION("getCachedGnssLocationsSize", GetCachedGnssLocationsSize),
        DECLARE_NAPI_FUNCTION("flushCachedGnssLocations", FlushCachedGnssLocations),
        DECLARE_NAPI_FUNCTION("sendCommand", SendCommand),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getCurrentLocation", GetCurrentLocation),
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

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_locationModule);
}
}  // namespace Location
}  // namespace OHOS
