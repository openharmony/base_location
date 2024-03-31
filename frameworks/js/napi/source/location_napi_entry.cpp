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

#include "constant_definition.h"
#include "napi_util.h"
#include "location_log.h"
#include "location_napi_adapter.h"
#include "location_napi_event.h"
#include "location_napi_system.h"

namespace OHOS {
namespace Location {
napi_value SvConstellationTypeConstructor(napi_env env)
{
    napi_value satelliteConstellationCategory = nullptr;
    NAPI_CALL(env, napi_create_object(env, &satelliteConstellationCategory));
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_UNKNOWN, "CONSTELLATION_CATEGORY_UNKNOWN");
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_GPS, "CONSTELLATION_CATEGORY_GPS");
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_SBAS, "CONSTELLATION_CATEGORY_SBAS");
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_GLONASS, "CONSTELLATION_CATEGORY_GLONASS");
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_QZSS, "CONSTELLATION_CATEGORY_QZSS");
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_BEIDOU, "CONSTELLATION_CATEGORY_BEIDOU");
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_GALILEO, "CONSTELLATION_CATEGORY_GALILEO");
    SetEnumPropertyByInteger(env, satelliteConstellationCategory,
        SatelliteConstellation::SV_CONSTELLATION_CATEGORY_IRNSS, "CONSTELLATION_CATEGORY_IRNSS");
    return satelliteConstellationCategory;
}

napi_value SatelliteAdditionalInfoTypeConstructor(napi_env env)
{
    napi_value satelliteAdditionalInfo = nullptr;
    NAPI_CALL(env, napi_create_object(env, &satelliteAdditionalInfo));
    SetEnumPropertyByInteger(env, satelliteAdditionalInfo,
        SatelliteAdditionalInfo::SV_ADDITIONAL_INFO_NULL, "SATELLITES_ADDITIONAL_INFO_NULL");
    SetEnumPropertyByInteger(env, satelliteAdditionalInfo,
        SatelliteAdditionalInfo::SV_ADDITIONAL_INFO_EPHEMERIS_DATA_EXIST,
        "SATELLITES_ADDITIONAL_INFO_EPHEMERIS_DATA_EXIST");
    SetEnumPropertyByInteger(env, satelliteAdditionalInfo,
        SatelliteAdditionalInfo::SV_ADDITIONAL_INFO_ALMANAC_DATA_EXIST,
        "SATELLITES_ADDITIONAL_INFO_ALMANAC_DATA_EXIST");
    SetEnumPropertyByInteger(env, satelliteAdditionalInfo,
        SatelliteAdditionalInfo::SV_ADDITIONAL_INFO_USED_IN_FIX,
        "SATELLITES_ADDITIONAL_INFO_USED_IN_FIX");
    SetEnumPropertyByInteger(env, satelliteAdditionalInfo,
        SatelliteAdditionalInfo::SV_ADDITIONAL_INFO_CARRIER_FREQUENCY_EXIST,
        "SATELLITES_ADDITIONAL_INFO_CARRIER_FREQUENCY_EXIST");
    return satelliteAdditionalInfo;
}

napi_value LocationRequestPriorityTypeConstructor(napi_env env)
{
    napi_value locationRequestPriority = nullptr;
    NAPI_CALL(env, napi_create_object(env, &locationRequestPriority));
    SetEnumPropertyByInteger(env, locationRequestPriority, PRIORITY_UNSET, "UNSET");
    SetEnumPropertyByInteger(env, locationRequestPriority, PRIORITY_ACCURACY, "ACCURACY");
    SetEnumPropertyByInteger(env, locationRequestPriority, PRIORITY_LOW_POWER, "LOW_POWER");
    SetEnumPropertyByInteger(env, locationRequestPriority, PRIORITY_FAST_FIRST_FIX, "FIRST_FIX");
    return locationRequestPriority;
}

napi_value LocationRequestScenarioTypeConstructor(napi_env env)
{
    napi_value locationRequestScenario = nullptr;
    NAPI_CALL(env, napi_create_object(env, &locationRequestScenario));
    SetEnumPropertyByInteger(env, locationRequestScenario, SCENE_UNSET, "UNSET");
    SetEnumPropertyByInteger(env, locationRequestScenario, SCENE_NAVIGATION, "NAVIGATION");
    SetEnumPropertyByInteger(env, locationRequestScenario, SCENE_TRAJECTORY_TRACKING, "TRAJECTORY_TRACKING");
    SetEnumPropertyByInteger(env, locationRequestScenario, SCENE_CAR_HAILING, "CAR_HAILING");
    SetEnumPropertyByInteger(env, locationRequestScenario, SCENE_DAILY_LIFE_SERVICE, "DAILY_LIFE_SERVICE");
    SetEnumPropertyByInteger(env, locationRequestScenario, SCENE_NO_POWER, "NO_POWER");
    return locationRequestScenario;
}

napi_value LocationPrivacyTypeConstructor(napi_env env)
{
    napi_value locationPrivacyType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &locationPrivacyType));
    SetEnumPropertyByInteger(env, locationPrivacyType, PRIVACY_TYPE_OTHERS, "OTHERS");
    SetEnumPropertyByInteger(env, locationPrivacyType, PRIVACY_TYPE_STARTUP, "STARTUP");
    SetEnumPropertyByInteger(env, locationPrivacyType, PRIVACY_TYPE_CORE_LOCATION, "CORE_LOCATION");
    return locationPrivacyType;
}

napi_value CountryCodeTypeConstructor(napi_env env)
{
    napi_value countryCodeType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &countryCodeType));
    SetEnumPropertyByInteger(env, countryCodeType, COUNTRY_CODE_FROM_LOCALE, "COUNTRY_CODE_FROM_LOCALE");
    SetEnumPropertyByInteger(env, countryCodeType, COUNTRY_CODE_FROM_SIM, "COUNTRY_CODE_FROM_SIM");
    SetEnumPropertyByInteger(env, countryCodeType, COUNTRY_CODE_FROM_LOCATION, "COUNTRY_CODE_FROM_LOCATION");
    SetEnumPropertyByInteger(env, countryCodeType, COUNTRY_CODE_FROM_NETWORK, "COUNTRY_CODE_FROM_NETWORK");
    return countryCodeType;
}

napi_value GeoLocationErrorCodeTypeConstructor(napi_env env)
{
    napi_value geoLocationErrorCode = nullptr;
    NAPI_CALL(env, napi_create_object(env, &geoLocationErrorCode));
    SetEnumPropertyByInteger(env, geoLocationErrorCode, INPUT_PARAMS_ERROR, "INPUT_PARAMS_ERROR");
    SetEnumPropertyByInteger(env, geoLocationErrorCode, REVERSE_GEOCODE_ERROR, "REVERSE_GEOCODE_ERROR");
    SetEnumPropertyByInteger(env, geoLocationErrorCode, GEOCODE_ERROR, "GEOCODE_ERROR");
    SetEnumPropertyByInteger(env, geoLocationErrorCode, LOCATOR_ERROR, "LOCATOR_ERROR");
    SetEnumPropertyByInteger(env, geoLocationErrorCode, LOCATION_SWITCH_ERROR, "LOCATION_SWITCH_ERROR");
    SetEnumPropertyByInteger(env, geoLocationErrorCode, LAST_KNOWN_LOCATION_ERROR, "LAST_KNOWN_LOCATION_ERROR");
    SetEnumPropertyByInteger(env, geoLocationErrorCode,
        LOCATION_REQUEST_TIMEOUT_ERROR, "LOCATION_REQUEST_TIMEOUT_ERROR");
    return geoLocationErrorCode;
}

napi_value LocatingRequiredDataTypeConstructor(napi_env env)
{
    napi_value locatingPriority = nullptr;
    NAPI_CALL(env, napi_create_object(env, &locatingPriority));
    SetEnumPropertyByInteger(env, locatingPriority, LocatingRequiredDataType::WIFI, "WIFI");
    SetEnumPropertyByInteger(env, locatingPriority, LocatingRequiredDataType::BLUE_TOOTH, "BLUE_TOOTH");
    return locatingPriority;
}

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

        DECLARE_NAPI_PROPERTY("LocationRequestPriority", LocationRequestPriorityTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("LocationRequestScenario", LocationRequestScenarioTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("LocationPrivacyType", LocationPrivacyTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("GeoLocationErrorCode", GeoLocationErrorCodeTypeConstructor(env)),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    return exports;
}

static napi_module g_locationModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
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
    LBSLOGD(LOCATION_NAPI, "Init, location_napi_manager_entry");

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
        DECLARE_NAPI_FUNCTION("getCountryCode", GetIsoCountryCode),
        DECLARE_NAPI_FUNCTION("enableLocationMock", EnableLocationMock),
        DECLARE_NAPI_FUNCTION("disableLocationMock", DisableLocationMock),
        DECLARE_NAPI_FUNCTION("setMockedLocations", SetMockedLocations),
        DECLARE_NAPI_FUNCTION("enableReverseGeocodingMock", EnableReverseGeocodingMock),
        DECLARE_NAPI_FUNCTION("disableReverseGeocodingMock", DisableReverseGeocodingMock),
        DECLARE_NAPI_FUNCTION("setReverseGeocodingMockInfo", SetReverseGeocodingMockInfo),
        DECLARE_NAPI_FUNCTION("getLocatingRequiredData", GetLocatingRequiredData),

        DECLARE_NAPI_PROPERTY("LocationRequestPriority", LocationRequestPriorityTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("LocationRequestScenario", LocationRequestScenarioTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("LocationPrivacyType", LocationPrivacyTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("CountryCodeType", CountryCodeTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("LocatingRequiredDataType", LocatingRequiredDataTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("SatelliteConstellationCategory", SvConstellationTypeConstructor(env)),
        DECLARE_NAPI_PROPERTY("SatelliteAdditionalInfo", SatelliteAdditionalInfoTypeConstructor(env)),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    return exports;
}

static napi_module g_locationManagerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitManager,
    .nm_modname = "geoLocationManager",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};
#endif

#ifndef ENABLE_NAPI_MANAGER
extern "C" __attribute__((constructor)) void RegisterGeolocationModule(void)
{
    napi_module_register(&g_locationModule);
}
#else
extern "C" __attribute__((constructor)) void RegisterGeolocationManagerModule(void)
{
    napi_module_register(&g_locationManagerModule);
}
#endif
}  // namespace Location
}  // namespace OHOS
