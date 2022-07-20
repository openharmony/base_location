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
            for (int index = 0; index < geoAddress->m_descriptionsSize; index++) {
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
    JsObjectToInt(env, object, "reportingPeriodSec", request->reportingPeriodSec);
    JsObjectToBool(env, object, "wakeUpCacheQueueFull", request->wakeUpCacheQueueFull);
}

void JsObjToGeoFenceRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<GeofenceRequest>& request)
{
    int value = 0;
    double doubleValue = 0.0;
    if (JsObjectToInt(env, object, "priority", value)) {
        request->priority = value;
    }
    if (JsObjectToInt(env, object, "scenario", value)) {
        request->scenario = value;
    }
    if (JsObjectToDouble(env, object, "latitude", doubleValue)) {
        request->geofence.latitude = doubleValue;
    }
    if (JsObjectToDouble(env, object, "longitude", doubleValue)) {
        request->geofence.longitude = doubleValue;
    }
    if (JsObjectToDouble(env, object, "radius", doubleValue)) {
        request->geofence.radius = doubleValue;
    }
    if (JsObjectToDouble(env, object, "expiration", doubleValue)) {
        request->geofence.expiration = doubleValue;
    }
}

void JsObjToLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    int value = 0;
    double valueDouble = 0.0;
    if (JsObjectToInt(env, object, "priority", value)) {
        requestConfig->SetPriority(value);
    }
    if (JsObjectToInt(env, object, "scenario", value)) {
        requestConfig->SetScenario(value);
    }
    if (JsObjectToInt(env, object, "timeInterval", value)) {
        requestConfig->SetTimeInterval(value);
    }
    if (JsObjectToDouble(env, object, "maxAccuracy", valueDouble)) {
        requestConfig->SetMaxAccuracy(valueDouble);
    }
    if (JsObjectToInt(env, object, "distanceInterval", value)) {
        requestConfig->SetDistanceInterval(value);
    }
}

void JsObjToCurrentLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    int value = 0;
    double valueDouble = 0.0;
    if (JsObjectToInt(env, object, "priority", value)) {
        requestConfig->SetPriority(value);
    }
    if (JsObjectToInt(env, object, "scenario", value)) {
        requestConfig->SetScenario(value);
    }
    if (JsObjectToDouble(env, object, "maxAccuracy", valueDouble)) {
        requestConfig->SetMaxAccuracy(valueDouble);
    }
    if (JsObjectToInt(env, object, "timeoutMs", value)) {
        requestConfig->SetTimeOut(value);
    }
}

void JsObjToCommand(const napi_env& env, const napi_value& object,
    std::unique_ptr<LocationCommand>& commandConfig)
{
    if (commandConfig == nullptr) {
        return;
    }
    JsObjectToInt(env, object, "scenario", commandConfig->scenario);
    JsObjectToString(env, object, "command", MAX_BUF_LEN, commandConfig->command); // max bufLen
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

bool GetLocationInfo(const napi_env& env, const napi_value& object,
    const char* fieldStr, std::shared_ptr<ReverseGeocodeRequest>& request)
{
    bool result = false;
    napi_value value = nullptr;

    if (object == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "object is nullptr.");
        return false;
    }

    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &result), false);
    if (result) {
        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &value), false);
        JsObjectToString(env, value, "locale", MAX_BUF_LEN, request->locale);
        JsObjectToInt(env, value, "maxItems", request->maxItems);
        JsObjectToDouble(env, value, "latitude", request->latitude);
        JsObjectToDouble(env, value, "longitude", request->longitude);
        return true;
    }
    return false;
}

napi_value GetNapiValueByKey(napi_env env, const std::string keyChar, napi_value object)
{
    if (object == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetNapiValueByKey object is nullptr.");
        return nullptr;
    }
    bool result = false;
    NAPI_CALL(env, napi_has_named_property(env, object, keyChar.c_str(), &result));
    if (result) {
        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_named_property(env, object, keyChar.c_str(), &value));
        return value;
    }
    return nullptr;
}

bool GetStringArrayFromJsObj(napi_env env, napi_value value, std::vector<std::string>& outArray)
{
    uint32_t arrayLength = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, value, &arrayLength), false);
    if (arrayLength <= 0) {
        LBSLOGE(LOCATOR_STANDARD, "The array is empty.");
        return false;
    }
    for (size_t i = 0; i < arrayLength; i++) {
        napi_value napiElement = nullptr;
        NAPI_CALL_BASE(env, napi_get_element(env, value, i, &napiElement), false);
        napi_valuetype napiValueType = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, napiElement, &napiValueType), false);
        if (napiValueType != napi_string) {
            LBSLOGE(LOCATOR_STANDARD, "wrong argument type.");
            return false;
        }
        char type[64] = {0}; // max length
        size_t typeLen = 0;
        NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, napiElement, type, sizeof(type), &typeLen), false);
        std::string event = type;
        outArray.push_back(event);
    }
    return true;
}

bool GetStringArrayValueByKey(
    napi_env env, napi_value jsObject, std::string key, std::vector<std::string>& outArray)
{
    napi_value array = GetNapiValueByKey(env, key.c_str(), jsObject);
    if (array == nullptr) {
        return false;
    }
    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, array, &isArray), false);
    if (!isArray) {
        LBSLOGE(LOCATOR_STANDARD, "not an array!");
        return false;
    }
    return GetStringArrayFromJsObj(env, array, outArray);
}

bool GetGeoAddressInfo(const napi_env& env, const napi_value& object,
    const std::string fieldStr, std::shared_ptr<GeoAddress> address)
{
    napi_value value = GetNapiValueByKey(env, fieldStr, object);
    if (value == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetNapiValueByKey is nullptr.");
        return false;
    }
    double latitude = 0.0;
    double longitude = 0.0;
    JsObjectToDouble(env, value, "latitude", latitude);
    if (CommonUtils::DoubleEqual(latitude, 0.0)) {
        address->m_hasLatitude = false;
    } else {
        address->m_hasLatitude = true;
        address->m_latitude = latitude;
    }
    JsObjectToDouble(env, value, "longitude", longitude);
    if (CommonUtils::DoubleEqual(longitude, 0.0)) {
        address->m_hasLongitude = false;
    } else {
        address->m_hasLongitude = true;
        address->m_longitude = longitude;
    }
    int bufLen = MAX_BUF_LEN;
    JsObjectToString(env, value, "locale", bufLen, address->m_localeLanguage);
    JsObjectToString(env, value, "placeName", bufLen, address->m_placeName);
    JsObjectToString(env, value, "countryCode", bufLen, address->m_countryCode);
    JsObjectToString(env, value, "countryName", bufLen, address->m_countryName);
    JsObjectToString(env, value, "administrativeArea", bufLen, address->m_administrativeArea);
    JsObjectToString(env, value, "subAdministrativeArea", bufLen, address->m_subAdministrativeArea);
    JsObjectToString(env, value, "locality", bufLen, address->m_locality);
    JsObjectToString(env, value, "subLocality", bufLen, address->m_subLocality);
    JsObjectToString(env, value, "roadName", bufLen, address->m_roadName);
    JsObjectToString(env, value, "subRoadName", bufLen, address->m_subRoadName);
    JsObjectToString(env, value, "premises", bufLen, address->m_premises);
    JsObjectToString(env, value, "postalCode", bufLen, address->m_postalCode);
    JsObjectToString(env, value, "phoneNumber", bufLen, address->m_phoneNumber);
    JsObjectToString(env, value, "addressUrl", bufLen, address->m_addressUrl);
    JsObjectToInt(env, value, "descriptionsSize", address->m_descriptionsSize);
    JsObjectToBool(env, value, "isFromMock", address->m_isFromMock);
    std::vector<std::string> descriptions;
    GetStringArrayValueByKey(env, value, "descriptions", descriptions);
    size_t size = address->m_descriptionsSize > descriptions.size() ?
        descriptions.size() : address->m_descriptionsSize;
    for (size_t i = 0; i < size; i++) {
        address->m_descriptions.insert(std::make_pair(i, descriptions[i]));
    }
    return true;
}

bool JsObjToRevGeocodeMock(const napi_env& env, const napi_value& object,
    std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, object, &isArray), false);
    if (!isArray) {
        LBSLOGE(LOCATOR_STANDARD, "JsObjToRevGeocodeMock:not an array!");
        return false;
    }
    uint32_t arrayLength = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, object, &arrayLength), false);
    if (arrayLength <= 0) {
        LBSLOGE(LOCATOR_STANDARD, "JsObjToRevGeocodeMock:The array is empty.");
        return false;
    }
    for (size_t i = 0; i < arrayLength; i++) {
        napi_value napiElement = nullptr;
        NAPI_CALL_BASE(env, napi_get_element(env, object, i, &napiElement), false);
        std::shared_ptr<GeocodingMockInfo> info = std::make_shared<GeocodingMockInfo>();
        std::shared_ptr<ReverseGeocodeRequest> request = std::make_shared<ReverseGeocodeRequest>();
        std::shared_ptr<GeoAddress> geoAddress = std::make_shared<GeoAddress>();
        GetLocationInfo(env, napiElement, "location", request);
        GetGeoAddressInfo(env, napiElement, "geoAddress", geoAddress);
        info->SetLocation(request);
        info->SetGeoAddressInfo(geoAddress);
        mockInfo.push_back(info);
    }
    return true;
}

void GetLocationArray(const napi_env& env, LocationMockAsyncContext *asyncContext, const napi_value& object)
{
    uint32_t arrayLength = 0;
    NAPI_CALL_RETURN_VOID(env, napi_get_array_length(env, object, &arrayLength));
    if (arrayLength <= 0) {
        LBSLOGE(LOCATOR_STANDARD, "The array is empty.");
        return;
    }
    for (uint32_t i = 0; i < arrayLength; i++) {
        napi_value elementValue = nullptr;
        std::shared_ptr<Location> locationAdapter = std::make_shared<Location>();
        NAPI_CALL_RETURN_VOID(env, napi_get_element(env, object, i, &elementValue));
        double latitude = 0.0;
        JsObjectToDouble(env, elementValue, "latitude", latitude);
        locationAdapter->SetLatitude(latitude);
        double longitude = 0.0;
        JsObjectToDouble(env, elementValue, "longitude", longitude);
        locationAdapter->SetLongitude(longitude);
        double altitude = 0.0;
        JsObjectToDouble(env, elementValue, "altitude", altitude);
        locationAdapter->SetAltitude(altitude);
        int32_t accuracy = 0;
        JsObjectToInt(env, elementValue, "accuracy", accuracy);
        locationAdapter->SetAccuracy(static_cast<float>(accuracy));
        int32_t speed = 0;
        JsObjectToInt(env, elementValue, "speed", speed);
        locationAdapter->SetSpeed(static_cast<float>(speed));
        double direction = 0.0;
        JsObjectToDouble(env, elementValue, "direction", direction);
        locationAdapter->SetDirection(direction);
        int32_t timeStamp = 0;
        JsObjectToInt(env, elementValue, "timeStamp", timeStamp);
        locationAdapter->SetTimeStamp(static_cast<int64_t>(accuracy));
        int32_t timeSinceBoot = 0;
        JsObjectToInt(env, elementValue, "timeSinceBoot", timeSinceBoot);
        locationAdapter->SetTimeSinceBoot(static_cast<int64_t>(timeSinceBoot));
        std::string additions = " ";
        int buffLen = 100;
        JsObjectToString(env, elementValue, "additions", buffLen, additions);
        locationAdapter->SetAdditions(additions);
        int32_t additionSize = 0;
        JsObjectToInt(env, elementValue, "additionSize", additionSize);
        locationAdapter->SetAdditionSize(static_cast<int64_t>(additionSize));
        bool isFromMock = false;
        JsObjectToBool(env, elementValue, "isFromMock", isFromMock);
        locationAdapter->SetIsFromMock(isFromMock);
        asyncContext->LocationNapi.push_back(locationAdapter);
    }
}

bool JsObjectToString(const napi_env& env, const napi_value& object,
    const char* fieldStr, const int bufLen, std::string& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), false);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), false);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), false);
        NAPI_ASSERT_BASE(env, valueType == napi_string, "Wrong argument type.", false);
        if (bufLen <= 0) {
            return false;
        }
        char *buf = (char *)malloc(bufLen);
        if (buf == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "Js object to str malloc failed!");
            return false;
        }
        (void)memset_s(buf, bufLen, 0, bufLen);
        size_t result = 0;
        NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, field, buf, bufLen, &result), false);
        fieldRef = buf;
        free(buf);
        buf = nullptr;
        return true;
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js obj to str no property: %{public}s", fieldStr);
    }
    return false;
}

bool JsObjectToDouble(const napi_env& env, const napi_value& object, const char* fieldStr, double& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), false);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), false);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), false);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "Wrong argument type.", false);
        NAPI_CALL_BASE(env, napi_get_value_double(env, field, &fieldRef), false);
        return true;
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js to int no property: %{public}s", fieldStr);
    }
    return false;
}

bool JsObjectToInt(const napi_env& env, const napi_value& object, const char* fieldStr, int& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), false);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), false);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), false);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "Wrong argument type.", false);
        NAPI_CALL_BASE(env, napi_get_value_int32(env, field, &fieldRef), false);
        return true;
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js to int no property: %{public}s", fieldStr);
    }
    return false;
}

bool JsObjectToBool(const napi_env& env, const napi_value& object, const char* fieldStr, bool& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), false);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), false);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), false);
        NAPI_ASSERT_BASE(env, valueType == napi_boolean, "Wrong argument type.", false);
        NAPI_CALL_BASE(env, napi_get_value_bool(env, field, &fieldRef), false);
        return true;
    } else {
        LBSLOGD(LOCATOR_STANDARD, "Js to bool no property: %{public}s", fieldStr);
    }
    return false;
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

static bool InitAsyncCallBackEnv(const napi_env& env, AsyncContext* asyncContext,
    const size_t argc, const napi_value* argv, const size_t objectArgsNum)
{
    if (asyncContext == nullptr || argv == nullptr) {
        return false;
    }
    for (size_t i = objectArgsNum; i != argc; ++i) {
        napi_valuetype valuetype;
        NAPI_CALL_BASE(env, napi_typeof(env, argv[i], &valuetype), false);
        NAPI_ASSERT_BASE(env, valuetype == napi_function,  "Wrong argument type.", false);
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[i], 1, &asyncContext->callback[i - objectArgsNum]), false);
    }
    return true;
}

static bool InitAsyncPromiseEnv(const napi_env& env, AsyncContext *asyncContext, napi_value& promise)
{
    napi_deferred deferred;
    if (asyncContext == nullptr) {
        return false;
    }
    NAPI_CALL_BASE(env, napi_create_promise(env, &deferred, &promise), false);
    asyncContext->deferred = deferred;
    return true;
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

    if (context->result[PARAM1] == 0) {
        LBSLOGD(LOCATOR_STANDARD, "empty message, just return.");
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
