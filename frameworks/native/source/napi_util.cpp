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

#include "securec.h"
#include "string_ex.h"

#include "country_code.h"
#include "common_utils.h"
#include "geo_address.h"
#include "location_log.h"
#include "locator_proxy.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
static constexpr int MAX_BUF_LEN = 100;
static constexpr int MIN_WIFI_SCAN_TIME = 3000;

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
            SetValueUtf8String(env, "additions", locations[index]->GetAdditions().c_str(), value);
            SetValueInt64(env, "additionSize", locations[index]->GetAdditionSize(), value);
            SetValueBool(env, "isFromMock", locations[index]->GetIsFromMock(), value);
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
    SetValueUtf8String(env, "additions", locationInfo->GetAdditions().c_str(), result);
    SetValueInt64(env, "additionSize", locationInfo->GetAdditionSize(), result);
    SetValueBool(env, "isFromMock", locationInfo->GetIsFromMock(), result);
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
        SetValueUtf8String(env, "locale", geoAddress->locale_.c_str(), eachObj);
        SetValueUtf8String(env, "placeName", geoAddress->placeName_.c_str(), eachObj);
        SetValueUtf8String(env, "countryCode", geoAddress->countryCode_.c_str(), eachObj);
        SetValueUtf8String(env, "countryName", geoAddress->countryName_.c_str(), eachObj);
        SetValueUtf8String(env, "administrativeArea", geoAddress->administrativeArea_.c_str(), eachObj);
        SetValueUtf8String(env, "subAdministrativeArea", geoAddress->subAdministrativeArea_.c_str(), eachObj);
        SetValueUtf8String(env, "locality", geoAddress->locality_.c_str(), eachObj);
        SetValueUtf8String(env, "subLocality", geoAddress->subLocality_.c_str(), eachObj);
        SetValueUtf8String(env, "roadName", geoAddress->roadName_.c_str(), eachObj);
        SetValueUtf8String(env, "subRoadName", geoAddress->subRoadName_.c_str(), eachObj);
        SetValueUtf8String(env, "premises", geoAddress->premises_.c_str(), eachObj);
        SetValueUtf8String(env, "postalCode", geoAddress->postalCode_.c_str(), eachObj);
        SetValueUtf8String(env, "phoneNumber", geoAddress->phoneNumber_.c_str(), eachObj);
        SetValueUtf8String(env, "addressUrl", geoAddress->addressUrl_.c_str(), eachObj);
        napi_value descriptionArray;
        if (geoAddress->descriptionsSize_ > 0) {
            NAPI_CALL_BASE(env,
                napi_create_array_with_length(env, geoAddress->descriptionsSize_, &descriptionArray), false);
            uint32_t idx1 = 0;
            for (int index = 0; index < geoAddress->descriptionsSize_; index++) {
                napi_value value;
                NAPI_CALL_BASE(env, napi_create_string_utf8(env, geoAddress->GetDescriptions(index).c_str(),
                    NAPI_AUTO_LENGTH, &value), false);
                NAPI_CALL_BASE(env, napi_set_element(env, descriptionArray, idx1++, value), false);
            }
            SetValueStringArray(env, "descriptions", descriptionArray, eachObj);
        }
        SetValueInt32(env, "descriptionsSize", geoAddress->descriptionsSize_, eachObj);
        SetValueBool(env, "isFromMock", geoAddress->isFromMock_, eachObj);
        NAPI_CALL_BASE(env, napi_set_element(env, arrayResult, idx++, eachObj), false);
    }
    return true;
}

bool LocatingRequiredDataToJsObj(const napi_env& env,
    std::vector<std::shared_ptr<LocatingRequiredData>>& replyList, napi_value& arrayResult)
{
    uint32_t idx = 0;
    for (size_t i = 0; i < replyList.size(); i++) {
        napi_value eachObj;
        NAPI_CALL_BASE(env, napi_create_object(env, &eachObj), false);
        napi_value wifiObj;
        NAPI_CALL_BASE(env, napi_create_object(env, &wifiObj), false);
        SetValueUtf8String(env, "ssid", replyList[i]->GetWifiScanInfo()->GetSsid().c_str(), wifiObj);
        SetValueUtf8String(env, "bssid", replyList[i]->GetWifiScanInfo()->GetBssid().c_str(), wifiObj);
        SetValueInt32(env, "rssi", replyList[i]->GetWifiScanInfo()->GetRssi(), wifiObj);
        SetValueInt32(env, "frequency", replyList[i]->GetWifiScanInfo()->GetFrequency(), wifiObj);
        SetValueInt64(env, "timestamp", replyList[i]->GetWifiScanInfo()->GetTimestamp(), wifiObj);

        napi_value blueToothObj;
        NAPI_CALL_BASE(env, napi_create_object(env, &blueToothObj), false);
        SetValueUtf8String(env, "deviceName",
            replyList[i]->GetBluetoothScanInfo()->GetDeviceName().c_str(), blueToothObj);
        SetValueUtf8String(env, "macAddress", replyList[i]->GetBluetoothScanInfo()->GetMac().c_str(), blueToothObj);
        SetValueInt64(env, "rssi", replyList[i]->GetBluetoothScanInfo()->GetRssi(), blueToothObj);
        SetValueInt64(env, "timestamp", replyList[i]->GetBluetoothScanInfo()->GetTimeStamp(), blueToothObj);

        NAPI_CALL_BASE(env, napi_set_named_property(env, eachObj, "wifiData", wifiObj), napi_generic_failure);
        NAPI_CALL_BASE(env, napi_set_named_property(env, eachObj, "bluetoothData", blueToothObj), napi_generic_failure);
        napi_status status = napi_set_element(env, arrayResult, idx++, eachObj);
        if (status != napi_ok) {
            LBSLOGE(LOCATING_DATA_CALLBACK, "set element error: %{public}d, idx: %{public}d", status, idx - 1);
            return false;
        }
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
    const std::unique_ptr<GeofenceRequest>& request)
{
    int value = 0;
    double doubleValue = 0.0;
    if (JsObjectToInt(env, object, "scenario", value) == SUCCESS) {
        request->scenario = value;
    }
    if (JsObjectToDouble(env, object, "latitude", doubleValue) == SUCCESS) {
        request->geofence.latitude = doubleValue;
    }
    if (JsObjectToDouble(env, object, "longitude", doubleValue) == SUCCESS) {
        request->geofence.longitude = doubleValue;
    }
    if (JsObjectToDouble(env, object, "radius", doubleValue) == SUCCESS) {
        request->geofence.radius = doubleValue;
    }
    if (JsObjectToDouble(env, object, "expiration", doubleValue) == SUCCESS) {
        request->geofence.expiration = doubleValue;
    }
}

void JsObjToLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    int value = 0;
    double valueDouble = 0.0;
    if (JsObjectToInt(env, object, "priority", value) == SUCCESS) {
        requestConfig->SetPriority(value);
    }
    if (JsObjectToInt(env, object, "scenario", value) == SUCCESS) {
        requestConfig->SetScenario(value);
    }
    if (JsObjectToInt(env, object, "timeInterval", value) == SUCCESS) {
        if (value >= 0 && value < 1) {
            requestConfig->SetTimeInterval(1);
        } else {
            requestConfig->SetTimeInterval(value);
        }
    }
    if (JsObjectToDouble(env, object, "maxAccuracy", valueDouble) == SUCCESS) {
        requestConfig->SetMaxAccuracy(valueDouble);
    }
    if (JsObjectToDouble(env, object, "distanceInterval", valueDouble) == SUCCESS) {
        requestConfig->SetDistanceInterval(valueDouble);
    }
}

void JsObjToLocatingRequiredDataConfig(const napi_env& env, const napi_value& object,
    std::unique_ptr<LocatingRequiredDataConfig>& config)
{
    int valueInt = 0;
    bool valueBool = false;
    if (JsObjectToInt(env, object, "type", valueInt) == SUCCESS) {
        config->SetType(valueInt);
    }
    if (JsObjectToBool(env, object, "needStartScan", valueBool) == SUCCESS) {
        config->SetNeedStartScan(valueBool);
    }
    if (JsObjectToInt(env, object, "scanInterval", valueInt) == SUCCESS) {
        config->SetScanIntervalMs(valueInt < MIN_WIFI_SCAN_TIME ? MIN_WIFI_SCAN_TIME : valueInt);
    }
    if (JsObjectToInt(env, object, "scanTimeout", valueInt) == SUCCESS) {
        config->SetScanTimeoutMs(valueInt < MIN_WIFI_SCAN_TIME ? MIN_WIFI_SCAN_TIME : valueInt);
    }
}

void JsObjToCurrentLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig)
{
    int value = 0;
    double valueDouble = 0.0;
    if (JsObjectToInt(env, object, "priority", value) == SUCCESS) {
        requestConfig->SetPriority(value);
    }
    if (JsObjectToInt(env, object, "scenario", value) == SUCCESS) {
        requestConfig->SetScenario(value);
    }
    if (JsObjectToDouble(env, object, "maxAccuracy", valueDouble) == SUCCESS) {
        requestConfig->SetMaxAccuracy(valueDouble);
    }
    if (JsObjectToInt(env, object, "timeoutMs", value) == SUCCESS) {
        requestConfig->SetTimeOut(value);
    }
}

int JsObjToCommand(const napi_env& env, const napi_value& object,
    std::unique_ptr<LocationCommand>& commandConfig)
{
    if (commandConfig == nullptr) {
        return COMMON_ERROR;
    }
    CHK_ERROR_CODE("scenario", JsObjectToInt(env, object, "scenario", commandConfig->scenario), true);
    CHK_ERROR_CODE("command", JsObjectToString(env, object, "command", MAX_BUF_LEN, commandConfig->command), true);
    return SUCCESS;
}

int JsObjToGeoCodeRequest(const napi_env& env, const napi_value& object, MessageParcel& dataParcel)
{
    std::string description = "";
    int maxItems = 0;
    double minLatitude = 0.0;
    double minLongitude = 0.0;
    double maxLatitude = 0.0;
    double maxLongitude = 0.0;
    std::string locale = "";
    int bufLen = MAX_BUF_LEN;
    CHK_ERROR_CODE("locale", JsObjectToString(env, object, "locale", bufLen, locale), false);
    CHK_ERROR_CODE("description", JsObjectToString(env, object, "description", bufLen, description), true);
    if (description == "") {
        LBSLOGE(LOCATOR_STANDARD, "The required description field should not be empty.");
        return INPUT_PARAMS_ERROR;
    }
    CHK_ERROR_CODE("maxItems", JsObjectToInt(env, object, "maxItems", maxItems), false);
    CHK_ERROR_CODE("minLatitude", JsObjectToDouble(env, object, "minLatitude", minLatitude), false);
    CHK_ERROR_CODE("minLongitude", JsObjectToDouble(env, object, "minLongitude", minLongitude), false);
    CHK_ERROR_CODE("maxLatitude", JsObjectToDouble(env, object, "maxLatitude", maxLatitude), false);
    CHK_ERROR_CODE("maxLongitude", JsObjectToDouble(env, object, "maxLongitude", maxLongitude), false);
    if (minLatitude < MIN_LATITUDE || minLatitude > MAX_LATITUDE) {
        return INPUT_PARAMS_ERROR;
    }
    if (minLongitude < MIN_LONGITUDE || minLongitude > MAX_LONGITUDE) {
        return INPUT_PARAMS_ERROR;
    }
    if (maxLatitude < MIN_LATITUDE || maxLatitude > MAX_LATITUDE) {
        return INPUT_PARAMS_ERROR;
    }
    if (maxLongitude < MIN_LONGITUDE || maxLongitude > MAX_LONGITUDE) {
        return INPUT_PARAMS_ERROR;
    }
    if (!dataParcel.WriteInterfaceToken(LocatorProxy::GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "write interfaceToken fail!");
        return COMMON_ERROR;
    }
    dataParcel.WriteString16(Str8ToStr16(locale)); // locale
    dataParcel.WriteString16(Str8ToStr16(description)); // description
    dataParcel.WriteInt32(maxItems); // maxItems
    dataParcel.WriteDouble(minLatitude); // latitude
    dataParcel.WriteDouble(minLongitude); // longitude
    dataParcel.WriteDouble(maxLatitude); // latitude
    dataParcel.WriteDouble(maxLongitude); // longitude
    return SUCCESS;
}

bool JsObjToReverseGeoCodeRequest(const napi_env& env, const napi_value& object, MessageParcel& dataParcel)
{
    double latitude = 0;
    double longitude = 0;
    int maxItems = 0;
    std::string locale = "";

    CHK_ERROR_CODE("latitude", JsObjectToDouble(env, object, "latitude", latitude), true);
    CHK_ERROR_CODE("longitude", JsObjectToDouble(env, object, "longitude", longitude), true);
    CHK_ERROR_CODE("maxItems", JsObjectToInt(env, object, "maxItems", maxItems), false);
    CHK_ERROR_CODE("locale", JsObjectToString(env, object, "locale", MAX_BUF_LEN, locale), false); // max bufLen

    if (latitude < MIN_LATITUDE || latitude > MAX_LATITUDE) {
        return false;
    }
    if (longitude < MIN_LONGITUDE || longitude > MAX_LONGITUDE) {
        return false;
    }
    if (!dataParcel.WriteInterfaceToken(LocatorProxy::GetDescriptor())) {
        return false;
    }
    dataParcel.WriteString16(Str8ToStr16(locale)); // locale
    dataParcel.WriteDouble(latitude); // latitude
    dataParcel.WriteDouble(longitude); // longitude
    dataParcel.WriteInt32(maxItems); // maxItems
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

napi_value GetNapiValueByKey(napi_env env, const std::string& keyChar, napi_value object)
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
    if (arrayLength == 0) {
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
    napi_env env, napi_value jsObject, const std::string& key, std::vector<std::string>& outArray)
{
    napi_value array = GetNapiValueByKey(env, key, jsObject);
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
    const std::string& fieldStr, std::shared_ptr<GeoAddress> address)
{
    napi_value value = GetNapiValueByKey(env, fieldStr, object);
    if (value == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetNapiValueByKey is nullptr.");
        return false;
    }
    int bufLen = MAX_BUF_LEN;
    JsObjectToDouble(env, value, "latitude", address->latitude_);
    JsObjectToDouble(env, value, "longitude", address->longitude_);
    JsObjectToString(env, value, "locale", bufLen, address->locale_);
    JsObjectToString(env, value, "placeName", bufLen, address->placeName_);
    JsObjectToString(env, value, "countryCode", bufLen, address->countryCode_);
    JsObjectToString(env, value, "countryName", bufLen, address->countryName_);
    JsObjectToString(env, value, "administrativeArea", bufLen, address->administrativeArea_);
    JsObjectToString(env, value, "subAdministrativeArea", bufLen, address->subAdministrativeArea_);
    JsObjectToString(env, value, "locality", bufLen, address->locality_);
    JsObjectToString(env, value, "subLocality", bufLen, address->subLocality_);
    JsObjectToString(env, value, "roadName", bufLen, address->roadName_);
    JsObjectToString(env, value, "subRoadName", bufLen, address->subRoadName_);
    JsObjectToString(env, value, "premises", bufLen, address->premises_);
    JsObjectToString(env, value, "postalCode", bufLen, address->postalCode_);
    JsObjectToString(env, value, "phoneNumber", bufLen, address->phoneNumber_);
    JsObjectToString(env, value, "addressUrl", bufLen, address->addressUrl_);
    JsObjectToInt(env, value, "descriptionsSize", address->descriptionsSize_);
    JsObjectToBool(env, value, "isFromMock", address->isFromMock_);
    std::vector<std::string> descriptions;
    GetStringArrayValueByKey(env, value, "descriptions", descriptions);
    size_t size = static_cast<size_t>(address->descriptionsSize_) > descriptions.size() ?
        descriptions.size() : static_cast<size_t>(address->descriptionsSize_);
    for (size_t i = 0; i < size; i++) {
        address->descriptions_.insert(std::make_pair(i, descriptions[i]));
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
    if (arrayLength == 0) {
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
    if (arrayLength == 0) {
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
        double accuracy = 0.0;
        JsObjectToDouble(env, elementValue, "accuracy", accuracy);
        locationAdapter->SetAccuracy(accuracy);
        double speed = 0.0;
        JsObjectToDouble(env, elementValue, "speed", speed);
        locationAdapter->SetSpeed(speed);
        double direction = 0.0;
        JsObjectToDouble(env, elementValue, "direction", direction);
        locationAdapter->SetDirection(direction);
        int64_t timeStamp = 0;
        JsObjectToInt64(env, elementValue, "timeStamp", timeStamp);
        locationAdapter->SetTimeStamp(timeStamp);
        int64_t timeSinceBoot = 0;
        JsObjectToInt64(env, elementValue, "timeSinceBoot", timeSinceBoot);
        locationAdapter->SetTimeSinceBoot(timeSinceBoot);
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

int JsObjectToString(const napi_env& env, const napi_value& object,
    const char* fieldStr, const int bufLen, std::string& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), COMMON_ERROR);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), COMMON_ERROR);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), COMMON_ERROR);
        if (valueType != napi_string) {
            LBSLOGE(LOCATOR_STANDARD, "JsObjectToString, valueType != napi_string.");
            return INPUT_PARAMS_ERROR;
        }
        if (bufLen <= 0) {
            LBSLOGE(LOCATOR_STANDARD, "The length of buf should be greater than 0.");
            return COMMON_ERROR;
        }
        int32_t actBuflen = bufLen + 1;
        std::unique_ptr<char[]> buf = std::make_unique<char[]>(actBuflen);
        (void)memset_s(buf.get(), actBuflen, 0, actBuflen);
        size_t result = 0;
        NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, field, buf.get(), actBuflen, &result), COMMON_ERROR);
        fieldRef = buf.get();
        return SUCCESS;
    }
    LBSLOGD(LOCATOR_STANDARD, "Js obj to str no property: %{public}s", fieldStr);
    return PARAM_IS_EMPTY;
}

int JsObjectToDouble(const napi_env& env, const napi_value& object, const char* fieldStr, double& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), COMMON_ERROR);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), COMMON_ERROR);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), COMMON_ERROR);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "Wrong argument type.", INPUT_PARAMS_ERROR);
        NAPI_CALL_BASE(env, napi_get_value_double(env, field, &fieldRef), COMMON_ERROR);
        return SUCCESS;
    }
    LBSLOGD(LOCATOR_STANDARD, "Js to int no property: %{public}s", fieldStr);
    return PARAM_IS_EMPTY;
}

int JsObjectToInt(const napi_env& env, const napi_value& object, const char* fieldStr, int& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), COMMON_ERROR);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), COMMON_ERROR);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), COMMON_ERROR);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "Wrong argument type.", INPUT_PARAMS_ERROR);
        NAPI_CALL_BASE(env, napi_get_value_int32(env, field, &fieldRef), COMMON_ERROR);
        return SUCCESS;
    }
    LBSLOGD(LOCATOR_STANDARD, "Js to int no property: %{public}s", fieldStr);
    return PARAM_IS_EMPTY;
}

int JsObjectToInt64(const napi_env& env, const napi_value& object, const char* fieldStr, int64_t& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), COMMON_ERROR);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), COMMON_ERROR);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), COMMON_ERROR);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "Wrong argument type.", INPUT_PARAMS_ERROR);
        NAPI_CALL_BASE(env, napi_get_value_int64(env, field, &fieldRef), COMMON_ERROR);
        return SUCCESS;
    }
    LBSLOGD(LOCATOR_STANDARD, "Js to int no property: %{public}s", fieldStr);
    return PARAM_IS_EMPTY;
}

int JsObjectToBool(const napi_env& env, const napi_value& object, const char* fieldStr, bool& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, fieldStr, &hasProperty), COMMON_ERROR);
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        NAPI_CALL_BASE(env, napi_get_named_property(env, object, fieldStr, &field), COMMON_ERROR);
        NAPI_CALL_BASE(env, napi_typeof(env, field, &valueType), COMMON_ERROR);
        NAPI_ASSERT_BASE(env, valueType == napi_boolean, "Wrong argument type.", INPUT_PARAMS_ERROR);
        NAPI_CALL_BASE(env, napi_get_value_bool(env, field, &fieldRef), COMMON_ERROR);
        return SUCCESS;
    }
    LBSLOGD(LOCATOR_STANDARD, "Js to bool no property: %{public}s", fieldStr);
    return PARAM_IS_EMPTY;
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
    if (asyncContext == nullptr || argv == nullptr ||
        argc > MAXIMUM_JS_PARAMS || objectArgsNum > MAXIMUM_JS_PARAMS) {
        return false;
    }
    size_t startLoop = objectArgsNum;
    size_t endLoop = argc;
    for (size_t i = startLoop; i < endLoop; ++i) {
        napi_valuetype valuetype;
        NAPI_CALL_BASE(env, napi_typeof(env, argv[i], &valuetype), false);
        NAPI_ASSERT_BASE(env, valuetype == napi_function,  "Wrong argument type.", false);
        size_t index = i - startLoop;
        if (index >= MAX_CALLBACK_NUM) {
            break;
        }
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[i], 1, &asyncContext->callback[index]), false);
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

void CreateFailCallBackParams(AsyncContext& context, const std::string& msg, int32_t errorCode)
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
        {LocationErrCode::ERRCODE_SUCCESS, "SUCCESS."},
        {LocationErrCode::ERRCODE_PERMISSION_DENIED, "Permission denied."},
        {LocationErrCode::ERRCODE_SYSTEM_PERMISSION_DENIED, "System API is not allowed called by third HAP."},
        {LocationErrCode::ERRCODE_INVALID_PARAM, "Parameter error."},
        {LocationErrCode::ERRCODE_NOT_SUPPORTED, "Capability not supported."},
        {LocationErrCode::ERRCODE_SERVICE_UNAVAILABLE, "Location service is unavailable."},
        {LocationErrCode::ERRCODE_SWITCH_OFF, "The location switch is off."},
        {LocationErrCode::ERRCODE_LOCATING_FAIL, "Failed to obtain the geographical location."},
        {LocationErrCode::ERRCODE_REVERSE_GEOCODING_FAIL, "Reverse geocoding query failed."},
        {LocationErrCode::ERRCODE_GEOCODING_FAIL, "Geocoding query failed."},
        {LocationErrCode::ERRCODE_COUNTRYCODE_FAIL, "Failed to query the area information."},
        {LocationErrCode::ERRCODE_GEOFENCE_FAIL, "Failed to operate the geofence."},
        {LocationErrCode::ERRCODE_NO_RESPONSE, "No response to the request."},
    };

    auto iter = errorCodeMap.find(code);
    if (iter != errorCodeMap.end()) {
        std::string errMessage = "BussinessError ";
        errMessage.append(std::to_string(code)).append(": ").append(iter->second);
        return errMessage;
    }
    return "undefined error.";
}

napi_value GetErrorObject(napi_env env, const int32_t errCode, const std::string& errMsg)
{
    napi_value businessError = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(), errMsg.length(), &eMsg));
    NAPI_CALL(env, napi_create_object(env, &businessError));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "code", eCode));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "message", eMsg));
    return businessError;
}

void CreateResultObject(const napi_env& env, AsyncContext* context)
{
    if (context == nullptr || env == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "CreateResultObject input para error");
        return;
    }
    if (context->errCode != SUCCESS) {
        std::string errMsg = GetErrorMsgByCode(context->errCode);
        context->result[PARAM0] = GetErrorObject(env, context->errCode, errMsg);
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
            AsyncContext* context = static_cast<AsyncContext *>(data);
            context->executeFunc(context);
        },
        [](napi_env env, napi_status status, void* data) {
            if (data == nullptr) {
                LBSLOGE(LOCATOR_STANDARD, "Async data parameter is null");
                return;
            }
            AsyncContext* context = static_cast<AsyncContext *>(data);
            context->completeFunc(data);
            CreateResultObject(env, context);
            SendResultToJs(env, context);
            MemoryReclamation(env, context);
        }, static_cast<void*>(asyncContext), &asyncContext->work));
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

void DeleteQueueWork(AsyncContext* context)
{
    uv_loop_s *loop = nullptr;
    if (context->env == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "env is nullptr.");
        delete context;
        return;
    }
    NAPI_CALL_RETURN_VOID(context->env, napi_get_uv_event_loop(context->env, &loop));
    if (loop == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "loop == nullptr.");
        delete context;
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "work == nullptr.");
        delete context;
        return;
    }
    work->data = context;
    DeleteCallbackHandler(loop, work);
}

void DeleteCallbackHandler(uv_loop_s *&loop, uv_work_t *&work)
{
    uv_queue_work(loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            AsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<AsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            if (context->callback[SUCCESS_CALLBACK] != nullptr) {
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_delete_reference(context->env, context->callback[SUCCESS_CALLBACK]),
                    scope, context, work);
            }
            if (context->callback[FAIL_CALLBACK] != nullptr) {
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_delete_reference(context->env, context->callback[FAIL_CALLBACK]),
                    scope, context, work);
            }
            if (context->callback[COMPLETE_CALLBACK] != nullptr) {
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_delete_reference(context->env, context->callback[COMPLETE_CALLBACK]),
                    scope, context, work);
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

bool CheckIfParamIsFunctionType(napi_env env, napi_value param)
{
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, param, &valueType), false);
    if (valueType != napi_function) {
        return false;
    }
    return true;
}

napi_value SetEnumPropertyByInteger(napi_env env, napi_value dstObj, int32_t enumValue, const char *enumName)
{
    napi_value enumProp = nullptr;
    NAPI_CALL(env, napi_create_int32(env, enumValue, &enumProp));
    NAPI_CALL(env, napi_set_named_property(env, dstObj, enumName, enumProp));
    return enumProp;
}

bool CheckIfParamIsObjectType(napi_env env, napi_value param)
{
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, param, &valueType), false);
    if (valueType != napi_object) {
        return false;
    }
    return true;
}
}  // namespace Location
}  // namespace OHOS
