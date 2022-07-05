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
#include <string>
#include "common_utils.h"
#include "geo_address.h"
#include "location_log.h"
#include "request_config.h"
#include "securec.h"
#include "string_ex.h"
#include "country_code.h"

namespace OHOS {
namespace Location {
static constexpr int MAX_BUF_LEN = 100;
static constexpr double MIN_LATITUDE = -90.0;
static constexpr double MAX_LATITUDE = 90.0;
static constexpr double MIN_LONGITUDE = -180.0;
static constexpr double MAX_LONGITUDE = 180.0;

napi_value UndefinedNapiValue(const napi_env& env)
{
    napi_value result;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

void SatelliteStatusToJs(const napi_env& env,
    const std::shared_ptr<SatelliteStatus>& statusInfo, napi_value& result)
{
    napi_value satelliteIdsArray;
    napi_value cn0Array;
    napi_value altitudesArray;
    napi_value azimuthsArray;
    napi_value carrierFrequenciesArray;
    SetValueDouble(env, "satellitesNumber", statusInfo->GetSatellitesNumber(), result);
    if (statusInfo->GetSatellitesNumber() > 0) {
        NAPI_CALL_RETURN_VOID(env,
            napi_create_array_with_length(env, statusInfo->GetSatellitesNumber(), &satelliteIdsArray));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_array_with_length(env, statusInfo->GetSatellitesNumber(), &cn0Array));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_array_with_length(env, statusInfo->GetSatellitesNumber(), &altitudesArray));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_array_with_length(env, statusInfo->GetSatellitesNumber(), &azimuthsArray));
        NAPI_CALL_RETURN_VOID(env,
            napi_create_array_with_length(env, statusInfo->GetSatellitesNumber(), &carrierFrequenciesArray));
        uint32_t idx1 = 0;
        for (int index = 0; index < statusInfo->GetSatellitesNumber(); index++) {
            napi_value value = nullptr;
            NAPI_CALL_RETURN_VOID(env, napi_create_double(env, statusInfo->GetSatelliteIds()[index], &value));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, satelliteIdsArray, idx1, value));
            NAPI_CALL_RETURN_VOID(env,
                napi_create_double(env, statusInfo->GetCarrierToNoiseDensitys()[index], &value));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, cn0Array, idx1, value));
            NAPI_CALL_RETURN_VOID(env, napi_create_double(env, statusInfo->GetAltitudes()[index], &value));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, altitudesArray, idx1, value));
            NAPI_CALL_RETURN_VOID(env, napi_create_double(env, statusInfo->GetAzimuths()[index], &value));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, azimuthsArray, idx1, value));
            NAPI_CALL_RETURN_VOID(env, napi_create_double(env, statusInfo->GetCarrierFrequencies()[index], &value));
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, carrierFrequenciesArray, idx1, value));
            idx1++;
        }
        SetValueStringArray(env, "satelliteIds", satelliteIdsArray, result);
        SetValueStringArray(env, "carrierToNoiseDensitys", cn0Array, result);
        SetValueStringArray(env, "altitudes", altitudesArray, result);
        SetValueStringArray(env, "azimuths", azimuthsArray, result);
        SetValueStringArray(env, "carrierFrequencies", carrierFrequenciesArray, result);
    }
}

void LocationsToJs(const napi_env& env, const std::vector<std::shared_ptr<Location>>& locations, napi_value& result)
{
    if (locations.size() > 0) {
        for (unsigned int index = 0; index < locations.size(); index++) {
            napi_value value;
            SetValueDouble(env, "latitude", locations[index]->GetLatitude(), value);
            SetValueDouble(env, "longitude", locations[index]->GetLongitude(), value);
            SetValueDouble(env, "altitude", locations[index]->GetAltitude(), value);
            SetValueDouble(env, "accuracy", locations[index]->GetAccuracy(), value);
            SetValueDouble(env, "speed", locations[index]->GetSpeed(), value);
            SetValueInt64(env, "timeStamp", locations[index]->GetTimeStamp(), value);
            SetValueDouble(env, "direction", locations[index]->GetDirection(), value);
            SetValueInt64(env, "timeSinceBoot", locations[index]->GetTimeSinceBoot(), value);
            SetValueUtf8String(env, "additions", "GNSS", value);
            SetValueInt64(env, "additionSize", 1, value);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, value));
        }
    }
}

void LocationToJs(const napi_env& env, const std::unique_ptr<Location>& locationInfo, napi_value& result)
{
    SetValueDouble(env, "latitude", locationInfo->GetLatitude(), result);
    SetValueDouble(env, "longitude", locationInfo->GetLongitude(), result);
    SetValueDouble(env, "altitude", locationInfo->GetAltitude(), result);
    SetValueDouble(env, "accuracy", locationInfo->GetAccuracy(), result);
    SetValueDouble(env, "speed", locationInfo->GetSpeed(), result);
    SetValueInt64(env, "timeStamp", locationInfo->GetTimeStamp(), result);
    SetValueDouble(env, "direction", locationInfo->GetDirection(), result);
    SetValueInt64(env, "timeSinceBoot", locationInfo->GetTimeSinceBoot(), result);
    SetValueUtf8String(env, "additions", "GNSS", result);
    SetValueInt64(env, "additionSize", 1, result);
}

void CountryCodeToJs(const napi_env& env, const std::shared_ptr<CountryCode>& country, napi_value& result)
{
    SetValueUtf8String(env, "country", country->GetCountryCodeStr().c_str(), result);
    SetValueInt64(env, "type", country->GetCountryCodeType(), result);
}

void SystemLocationToJs(const napi_env& env, const std::unique_ptr<Location>& locationInfo, napi_value& result)
{
    SetValueDouble(env, "longitude", locationInfo->GetLongitude(), result);
    SetValueDouble(env, "latitude", locationInfo->GetLatitude(), result);
    SetValueDouble(env, "altitude", locationInfo->GetAltitude(), result);
    SetValueDouble(env, "accuracy", locationInfo->GetAccuracy(), result);
    SetValueInt64(env, "time", locationInfo->GetTimeStamp(), result);
}

bool GeoAddressesToJsObj(const napi_env& env,
    std::list<std::shared_ptr<GeoAddress>>& replyList, napi_value& arrayResult)
{
    uint32_t idx = 0;
    napi_status status = napi_ok;
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto geoAddress = *iter;
        napi_value eachObj;
        NAPI_CALL_BASE(env, napi_create_object(env, &eachObj), false);
        SetValueDouble(env, "latitude", geoAddress->GetLatitude(), eachObj);
        SetValueDouble(env, "longitude", geoAddress->GetLongitude(), eachObj);
        SetValueUtf8String(env, "locale", geoAddress->m_localeLanguage.c_str(), eachObj);
        SetValueUtf8String(env, "placeName", geoAddress->m_placeName.c_str(), eachObj);
        SetValueUtf8String(env, "countryCode", geoAddress->m_countryCode.c_str(), eachObj);
        SetValueUtf8String(env, "countryName", geoAddress->m_countryName.c_str(), eachObj);
        SetValueUtf8String(env, "administrativeArea", geoAddress->m_administrativeArea.c_str(), eachObj);
        SetValueUtf8String(env, "subAdministrativeArea", geoAddress->m_subAdministrativeArea.c_str(), eachObj);
        SetValueUtf8String(env, "locality", geoAddress->m_locality.c_str(), eachObj);
        SetValueUtf8String(env, "subLocality", geoAddress->m_subLocality.c_str(), eachObj);
        SetValueUtf8String(env, "roadName", geoAddress->m_roadName.c_str(), eachObj);
        SetValueUtf8String(env, "subRoadName", geoAddress->m_subRoadName.c_str(), eachObj);
        SetValueUtf8String(env, "premises", geoAddress->m_premises.c_str(), eachObj);
        SetValueUtf8String(env, "postalCode", geoAddress->m_postalCode.c_str(), eachObj);
        SetValueUtf8String(env, "phoneNumber", geoAddress->m_phoneNumber.c_str(), eachObj);
        SetValueUtf8String(env, "addressUrl", geoAddress->m_addressUrl.c_str(), eachObj);
        napi_value descriptionArray;
        if (geoAddress->m_descriptionsSize > 0) {
            NAPI_CALL_BASE(env,
                napi_create_array_with_length(env, geoAddress->m_descriptionsSize, &descriptionArray), false);
            uint32_t idx1 = 0;
            for (int index = 0; index < geoAddress->m_descriptionsSize && status == napi_ok; index++) {
                napi_value value;
                NAPI_CALL_BASE(env, napi_create_string_utf8(env, geoAddress->GetDescriptions(index).c_str(),
                    NAPI_AUTO_LENGTH, &value), false);
                NAPI_CALL_BASE(env, napi_set_element(env, descriptionArray, idx1++, value), false);
            }
            SetValueStringArray(env, "descriptions", descriptionArray, eachObj);
        }
        SetValueInt32(env, "descriptionsSize", geoAddress->m_descriptionsSize, eachObj);
        NAPI_CALL_BASE(env, napi_set_element(env, arrayResult, idx++, eachObj), false);
    }
    return true;
}

void JsObjToCachedLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<CachedGnssLocationsRequest>& request)
{
    int value = 0;
    bool valueBool;
    JsObjectToInt(env, object, "reportingPeriodSec", value);
    request->reportingPeriodSec = value;
    JsObjectToBool(env, object, "wakeUpCacheQueueFull", valueBool);
    request->wakeUpCacheQueueFull = valueBool;
}

void JsObjToGeoFenceRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<GeofenceRequest>& request)
{
    int value = 0;
    double doubleValue = 0.0;
    JsObjectToInt(env, object, "priority", value);
    request->priority = value;
    JsObjectToInt(env, object, "scenario", value);
    request->scenario = value;
    JsObjectToDouble(env, object, "latitude", doubleValue);
    request->geofence.latitude = doubleValue;
    JsObjectToDouble(env, object, "longitude", doubleValue);
    request->geofence.longitude = doubleValue;
    JsObjectToDouble(env, object, "radius", doubleValue);
    request->geofence.radius = doubleValue;
    JsObjectToDouble(env, object, "expiration", doubleValue);
    request->geofence.expiration = doubleValue;
}

void JsObjToLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    int value = 0;
    double valueDouble = 0.0;
    JsObjectToInt(env, object, "priority", value);
    requestConfig->SetPriority(value);
    JsObjectToInt(env, object, "scenario", value);
    requestConfig->SetScenario(value);
    JsObjectToInt(env, object, "timeInterval", value);
    requestConfig->SetTimeInterval(value);
    JsObjectToDouble(env, object, "maxAccuracy", valueDouble);
    requestConfig->SetMaxAccuracy(valueDouble);
    JsObjectToInt(env, object, "distanceInterval", value);
    requestConfig->SetDistanceInterval(value);
}

void JsObjToCurrentLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    int value = 0;
    double valueDouble = 0.0;
    JsObjectToInt(env, object, "priority", value);
    requestConfig->SetPriority(value);
    JsObjectToInt(env, object, "scenario", value);
    requestConfig->SetScenario(value);
    JsObjectToDouble(env, object, "maxAccuracy", valueDouble);
    requestConfig->SetMaxAccuracy(valueDouble);
    JsObjectToInt(env, object, "timeoutMs", value);
    requestConfig->SetTimeOut(value);
}

void JsObjToCommand(const napi_env& env, const napi_value& object,
    std::unique_ptr<LocationCommand>& commandConfig)
{
    int value = 0;
    std::string command = "";
    if (commandConfig == nullptr) {
        return;
    }
    JsObjectToInt(env, object, "scenario", value);
    commandConfig->scenario = value;
    JsObjectToString(env, object, "command", MAX_BUF_LEN, command); // max bufLen
    commandConfig->command = command;
}

bool JsObjToGeoCodeRequest(const napi_env& env, const napi_value& object, MessageParcel& dataParcel)
{
    std::string description = "";
    int maxItems = 0;
    double minLatitude = 0.0;
    double minLongitude = 0.0;
    double maxLatitude = 0.0;
    double maxLongitude = 0.0;
    std::string locale = "";
    int bufLen = MAX_BUF_LEN;
    JsObjectToString(env, object, "locale", bufLen, locale);
    JsObjectToString(env, object, "description", bufLen, description);
    JsObjectToInt(env, object, "maxItems", maxItems);
    JsObjectToDouble(env, object, "minLatitude", minLatitude);
    JsObjectToDouble(env, object, "minLongitude", minLongitude);
    JsObjectToDouble(env, object, "maxLatitude", maxLatitude);
    JsObjectToDouble(env, object, "maxLongitude", maxLongitude);
    if (minLatitude < MIN_LATITUDE || minLatitude > MAX_LATITUDE) {
        return false;
    }
    if (minLongitude < MIN_LONGITUDE || minLongitude > MAX_LONGITUDE) {
        return false;
    }
    if (maxLatitude < MIN_LATITUDE || maxLatitude > MAX_LATITUDE) {
        return false;
    }
    if (maxLongitude < MIN_LONGITUDE || maxLongitude > MAX_LONGITUDE) {
        return false;
    }
    if (!dataParcel.WriteInterfaceToken(LocatorProxy::GetDescriptor())) {
        return false;
    }
    std::string str = "";
    dataParcel.WriteString(description);
    dataParcel.WriteDouble(minLatitude); // latitude
    dataParcel.WriteDouble(minLongitude); // longitude
    dataParcel.WriteDouble(maxLatitude); // latitude
    dataParcel.WriteDouble(maxLongitude); // longitude
    dataParcel.WriteInt32(maxItems); // maxItems
    dataParcel.WriteInt32(1); // locale object size = 1
    dataParcel.WriteString16(Str8ToStr16(locale)); // locale.getLanguage()
    dataParcel.WriteString16(Str8ToStr16(str)); // locale.getCountry()
    dataParcel.WriteString16(Str8ToStr16(str)); // locale.getVariant()
    dataParcel.WriteString16(Str8ToStr16(str)); // ""
    return true;
}

bool JsObjToReverseGeoCodeRequest(const napi_env& env, const napi_value& object, MessageParcel& dataParcel)
{
    double latitude = 0;
    double longitude = 0;
    int maxItems = 0;
    std::string locale = "";

    JsObjectToDouble(env, object, "latitude", latitude);
    JsObjectToDouble(env, object, "longitude", longitude);
    JsObjectToInt(env, object, "maxItems", maxItems);
    JsObjectToString(env, object, "locale", MAX_BUF_LEN, locale); // max bufLen

    if (latitude < MIN_LATITUDE || latitude > MAX_LATITUDE) {
        return false;
    }
    if (longitude < MIN_LONGITUDE || longitude > MAX_LONGITUDE) {
        return false;
    }
    std::string str = "";
    if (!dataParcel.WriteInterfaceToken(LocatorProxy::GetDescriptor())) {
        return false;
    }
    dataParcel.WriteDouble(latitude); // latitude
    dataParcel.WriteDouble(longitude); // longitude
    dataParcel.WriteInt32(maxItems); // maxItems
    dataParcel.WriteInt32(1); // locale object size = 1
    dataParcel.WriteString16(Str8ToStr16(locale)); // locale.getLanguage()
    dataParcel.WriteString16(Str8ToStr16(str)); // locale.getCountry()
    dataParcel.WriteString16(Str8ToStr16(str)); // locale.getVariant()
    dataParcel.WriteString16(Str8ToStr16(str)); // ""
    return true;
}

napi_value JsObjectToString(const napi_env& env, const napi_value& object,
    const char* fieldStr, const int bufLen, std::string& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr, &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL(env, napi_get_named_property(env, object, fieldStr, &field));
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
        if (bufLen <= 0) {
            return UndefinedNapiValue(env);
        }
        char *buf = (char *)malloc(bufLen);
        if (buf == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "Js object to str malloc failed!");
            return UndefinedNapiValue(env);
        }
        (void)memset_s(buf, bufLen, 0, bufLen);
        size_t result = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, field, buf, bufLen, &result));
        fieldRef = buf;
        free(buf);
        buf = nullptr;
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js obj to str no property: %{public}s", fieldStr);
    }
    return UndefinedNapiValue(env);
}

napi_value JsObjectToDouble(const napi_env& env, const napi_value& object, const char* fieldStr, double& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr, &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL(env, napi_get_named_property(env, object, fieldStr, &field));
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. Number expected.");
        NAPI_CALL(env, napi_get_value_double(env, field, &fieldRef));
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js to int no property: %{public}s", fieldStr);
    }
    return UndefinedNapiValue(env);
}

napi_value JsObjectToInt(const napi_env& env, const napi_value& object, const char* fieldStr, int& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr, &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL(env, napi_get_named_property(env, object, fieldStr, &field));
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. Number expected.");
        NAPI_CALL(env, napi_get_value_int32(env, field, &fieldRef));
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js to int no property: %{public}s", fieldStr);
    }
    return UndefinedNapiValue(env);
}

napi_value JsObjectToBool(const napi_env& env, const napi_value& object, const char* fieldStr, bool& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr, &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL(env, napi_get_named_property(env, object, fieldStr, &field));
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_boolean, "Wrong argument type. Bool expected.");
        NAPI_CALL(env, napi_get_value_bool(env, field, &fieldRef));
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js to bool no property: %{public}s", fieldStr);
    }
    return UndefinedNapiValue(env);
}

napi_status SetValueUtf8String(const napi_env& env, const char* fieldStr, const char* str, napi_value& result)
{
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &value), napi_generic_failure);
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}

napi_status SetValueStringArray(const napi_env& env, const char* fieldStr, napi_value& value, napi_value& result)
{
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}

napi_status SetValueInt32(const napi_env& env, const char* fieldStr, const int intValue, napi_value& result)
{
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_create_int32(env, intValue, &value), napi_generic_failure);
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}

napi_status SetValueInt64(const napi_env& env, const char* fieldStr, const int64_t intValue, napi_value& result)
{
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_create_int64(env, intValue, &value), napi_generic_failure);
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}

napi_status SetValueDouble(const napi_env& env, const char* fieldStr, const double doubleValue, napi_value& result)
{
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_create_double(env, doubleValue, &value), napi_generic_failure);
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}

napi_status SetValueBool(const napi_env& env, const char* fieldStr, const bool boolvalue, napi_value& result)
{
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_get_boolean(env, boolvalue, &value), napi_generic_failure);
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}

static napi_value InitAsyncCallBackEnv(const napi_env& env, AsyncContext* asyncContext,
    const size_t argc, const napi_value* argv, const size_t objectArgsNum)
{
    if (asyncContext == nullptr || argv == nullptr) {
        return nullptr;
    }
    for (size_t i = objectArgsNum; i != argc; ++i) {
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, argv[i], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        NAPI_CALL(env, napi_create_reference(env, argv[i], 1, &asyncContext->callback[i - objectArgsNum]));
    }
    return UndefinedNapiValue(env);
}

static napi_value InitAsyncPromiseEnv(const napi_env& env, AsyncContext *asyncContext, napi_value& promise)
{
    napi_deferred deferred;
    if (asyncContext == nullptr) {
        return nullptr;
    }
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncContext->deferred = deferred;
    return UndefinedNapiValue(env);
}

void CreateFailCallBackParams(AsyncContext& context, std::string msg, int32_t errorCode)
{
    SetValueUtf8String(context.env, "data", msg.c_str(), context.result[PARAM0]);
    SetValueInt32(context.env, "code", errorCode, context.result[PARAM1]);
}

std::string GetErrorMsgByCode(int code)
{
    static std::map<int, std::string> errorCodeMap = {
        {SUCCESS, "SUCCESS"},
        {NOT_SUPPORTED, "NOT_SUPPORTED"},
        {INPUT_PARAMS_ERROR, "INPUT_PARAMS_ERROR"},
        {REVERSE_GEOCODE_ERROR, "REVERSE_GEOCODE_ERROR"},
        {GEOCODE_ERROR, "GEOCODE_ERROR"},
        {LOCATOR_ERROR, "LOCATOR_ERROR"},
        {LOCATION_SWITCH_ERROR, "LOCATION_SWITCH_ERROR"},
        {LAST_KNOWN_LOCATION_ERROR, "LAST_KNOWN_LOCATION_ERROR"},
        {LOCATION_REQUEST_TIMEOUT_ERROR, "LOCATION_REQUEST_TIMEOUT_ERROR"},
        {QUERY_COUNTRY_CODE_ERROR, "QUERY_COUNTRY_CODE_ERROR"},
    };

    auto iter = errorCodeMap.find(code);
    if (iter == errorCodeMap.end()) {
        return "";
    }
    return iter->second;
}

void CreateResultObject(const napi_env& env, AsyncContext* context)
{
    if (context == nullptr || env == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "CreateResultObject input para error");
        return;
    }
    if (context->errCode != SUCCESS) {
        std::string msg = GetErrorMsgByCode(context->errCode);
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &context->result[PARAM0]));
        SetValueInt32(env, "code", context->errCode, context->result[PARAM0]);
        SetValueUtf8String(env, "data", msg.c_str(), context->result[PARAM0]);
        NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &context->result[PARAM1]));
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &context->result[PARAM0]));
    }
}

void SendResultToJs(const napi_env& env, AsyncContext* context)
{
    if (context == nullptr || env == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendResultToJs input para error");
        return;
    }

    bool isPromise = context->deferred != nullptr;
    if (isPromise) {
        if (context->errCode != SUCCESS) {
            NAPI_CALL_RETURN_VOID(env,
                napi_reject_deferred(env, context->deferred, context->result[PARAM0]));
        } else {
            NAPI_CALL_RETURN_VOID(env,
                napi_resolve_deferred(env, context->deferred, context->result[PARAM1]));
        }
    } else {
        napi_value undefine;
        NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefine));
        napi_value callback;
        NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, context->callback[0], &callback));
        NAPI_CALL_RETURN_VOID(env,
            napi_call_function(env, nullptr, callback, RESULT_SIZE, context->result, &undefine));
    }
}

void MemoryReclamation(const napi_env& env, AsyncContext* context)
{
    if (context == nullptr || env == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "MemoryReclamation input para error");
        return;
    }

    if (context->callback[SUCCESS_CALLBACK] != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, context->callback[SUCCESS_CALLBACK]));
    }
    if (context->callback[FAIL_CALLBACK] != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, context->callback[FAIL_CALLBACK]));
    }
    if (context->callback[COMPLETE_CALLBACK] != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, context->callback[COMPLETE_CALLBACK]));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, context->work));
    delete context;
}

static napi_value CreateAsyncWork(const napi_env& env, AsyncContext* asyncContext)
{
    if (asyncContext == nullptr) {
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_create_async_work(
        env, nullptr, asyncContext->resourceName,
        [](napi_env env, void* data) {
            if (data == nullptr) {
                LBSLOGE(LOCATOR_STANDARD, "Async data parameter is null");
                return;
            }
            AsyncContext* context = (AsyncContext *)data;
            context->executeFunc(context);
        },
        [](napi_env env, napi_status status, void* data) {
            if (data == nullptr) {
                LBSLOGE(LOCATOR_STANDARD, "Async data parameter is null");
                return;
            }
            AsyncContext* context = (AsyncContext *)data;
            context->completeFunc(data);
            CreateResultObject(env, context);
            SendResultToJs(env, context);
            MemoryReclamation(env, context);
        }, (void*)asyncContext, &asyncContext->work));
    NAPI_CALL(env, napi_queue_async_work(env, asyncContext->work));
    return UndefinedNapiValue(env);
}

napi_value DoAsyncWork(const napi_env& env, AsyncContext* asyncContext,
    const size_t argc, const napi_value* argv, const size_t objectArgsNum)
{
    if (asyncContext == nullptr || argv == nullptr) {
        return UndefinedNapiValue(env);
    }
    if (argc > objectArgsNum) {
        InitAsyncCallBackEnv(env, asyncContext, argc, argv, objectArgsNum);
        return CreateAsyncWork(env, asyncContext);
    } else {
        napi_value promise;
        InitAsyncPromiseEnv(env, asyncContext, promise);
        CreateAsyncWork(env, asyncContext);
        return promise;
    }
}
}  // namespace Location
}  // namespace OHOS
