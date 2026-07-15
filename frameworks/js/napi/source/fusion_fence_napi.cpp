/*
* Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "fusion_fence_napi.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "location_log.h"
#include "iremote_object.h"
#include "geofence_definition.h"
#include "fusion_fence_request.h"
#include "napi_util.h"
#include "common_utils.h"
#include <regex>

namespace OHOS {
namespace Location {
static constexpr int MAX_BUF_LEN = 512;

bool ParsePoint(napi_env env, napi_value object, FusionFencePoint& point)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    if (JsObjectToDouble(env, object, "latitude", point.latitude) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "latitude is required");
        return false;
    }
    if (JsObjectToDouble(env, object, "longitude", point.longitude) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "longitude is required");
        return false;
    }
    if (point.latitude < MIN_LATITUDE || point.latitude > MAX_LATITUDE) {
        LBSLOGE(FUSION_FENCE, "invalid latitude");
        return false;
    }
    if (point.longitude < MIN_LONGITUDE || point.longitude > MAX_LONGITUDE) {
        LBSLOGE(FUSION_FENCE, "invalid longitude");
        return false;
    }
    return true;
}

bool ParseCellInfo(napi_env env, napi_value object, FusionFenceCellInfo& cellInfo)
{
    if (env == nullptr || object == nullptr) {
        LBSLOGE(FUSION_FENCE, "ParseCellInfo: env or object is null");
        return false;
    }
    if (JsObjectToInt64(env, object, "timeSinceBoot", cellInfo.timeSinceBoot) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "timeSinceBoot is required");
        return false;
    }
    if (JsObjectToInt64(env, object, "cellId", cellInfo.cellId) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "cellId is required");
        return false;
    }
    if (JsObjectToInt(env, object, "lac", cellInfo.lac) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "lac is required");
        return false;
    }
    if (JsObjectToInt(env, object, "mcc", cellInfo.mcc) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "mcc is required");
        return false;
    }
    if (JsObjectToInt(env, object, "mnc", cellInfo.mnc) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "mnc is required");
        return false;
    }
    if (JsObjectToInt(env, object, "rat", cellInfo.rat) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "rat is required");
        return false;
    }
    if (JsObjectToInt(env, object, "signalIntensity", cellInfo.signalIntensity) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "signalIntensity is required");
        return false;
    }
    if (JsObjectToInt(env, object, "arfcn", cellInfo.arfcn) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "arfcn is required");
        return false;
    }
    if (JsObjectToInt(env, object, "pci", cellInfo.pci) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "pci is required");
        return false;
    }
    JsObjectToInt(env, object, "tac", cellInfo.tac);
    return true;
}

static bool ValidatePolygonArrayLength(napi_env env, napi_value polygonArray, uint32_t& arrayLen)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, polygonArray, &valueType) != napi_ok || valueType != napi_object) {
        LBSLOGE(FUSION_FENCE, "polygonArray is not an object");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, polygonArray, &isArray) != napi_ok || !isArray) {
        LBSLOGE(FUSION_FENCE, "polygonArray is not an array");
        return false;
    }
    if (napi_get_array_length(env, polygonArray, &arrayLen) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "get array length failed");
        return false;
    }
    return true;
}

static bool ParsePolygonPoints(napi_env env, napi_value polygonArray, std::vector<FusionFencePoint>& polygon)
{
    if (env == nullptr || polygonArray == nullptr) {
        LBSLOGE(FUSION_FENCE, "ParsePolygonPoints: env or polygonArray is null");
        return false;
    }
    uint32_t arrayLen = 0;
    if (!ValidatePolygonArrayLength(env, polygonArray, arrayLen)) {
        return false;
    }
    constexpr uint32_t MAX_POLYGON_POINTS_COUNT = 10;
    constexpr uint32_t MIN_POLYGON_POINTS_COUNT = 3;
    if (arrayLen > MAX_POLYGON_POINTS_COUNT) {
        LBSLOGI(FUSION_FENCE, "polygon array length=%{public}u exceeds max=%{public}u, truncating to %{public}u",
            arrayLen, MAX_POLYGON_POINTS_COUNT, MAX_POLYGON_POINTS_COUNT);
        arrayLen = MAX_POLYGON_POINTS_COUNT;
    }
    if (arrayLen < MIN_POLYGON_POINTS_COUNT) {
        LBSLOGE(FUSION_FENCE, "polygon array must have at least %{public}u points, got: %{public}u",
            MIN_POLYGON_POINTS_COUNT, arrayLen);
        return false;
    }
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value pointValue = nullptr;
        if (napi_get_element(env, polygonArray, i, &pointValue) != napi_ok) {
            LBSLOGE(FUSION_FENCE, "failed to get element at index %{public}u", i);
            continue;
        }
        FusionFencePoint point;
        if (ParsePoint(env, pointValue, point)) {
            auto isDuplicate = std::find_if(polygon.begin(), polygon.end(),
                [&point](const FusionFencePoint& p) {
                    return p.latitude == point.latitude && p.longitude == point.longitude;
                });
            if (isDuplicate == polygon.end()) {
                polygon.push_back(point);
            }
        }
    }
    if (polygon.size() < MIN_POLYGON_POINTS_COUNT) {
        LBSLOGE(FUSION_FENCE, "polygon must have at least %{public}u valid points, got: %{public}zu",
            MIN_POLYGON_POINTS_COUNT, polygon.size());
        return false;
    }
    return true;
}

static bool ParseCircularFence(napi_env env, napi_value object, std::shared_ptr<Geofence>& circularFence)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    napi_value circularFenceValue = nullptr;
    if (napi_get_named_property(env, object, "circularFence", &circularFenceValue) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "get circularFence property failed");
        return false;
    }
    auto fence = std::make_shared<Geofence>();
    double latitude = 0.0;
    double longitude = 0.0;
    double radius = 0.0;
    double expiration = 0.0;
    if (JsObjectToDouble(env, circularFenceValue, "latitude", latitude) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "latitude is required");
        return false;
    }
    if (JsObjectToDouble(env, circularFenceValue, "longitude", longitude) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "longitude is required");
        return false;
    }
    if (JsObjectToDouble(env, circularFenceValue, "radius", radius) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "radius is required");
        return false;
    }
    if (JsObjectToDouble(env, circularFenceValue, "expiration", expiration) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "expiration is required");
        return false;
    }
    if (expiration <= 0) {
        LBSLOGE(FUSION_FENCE, "expiration must be greater than 0, got: %{public}f", expiration);
        return false;
    }
    fence->SetLatitude(latitude);
    fence->SetLongitude(longitude);
    fence->SetRadius(radius);
    fence->SetExpiration(expiration);
    circularFence = fence;
    return true;
}

bool ParseGnssFence(napi_env env, napi_value object, std::shared_ptr<FusionFenceGnss>& gnssFence)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "ParseGnssFence: env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "ParseGnssFence: object is null");
        return false;
    }
    auto fence = std::make_shared<FusionFenceGnss>();
    int32_t fenceType = 0;
    if (JsObjectToInt(env, object, "gnssFenceType", fenceType) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "ParseGnssFence: gnssFenceType is required");
        return false;
    }
    if (fenceType != static_cast<int32_t>(GnssFenceType::GNSS_FENCE_POLYGON) &&
        fenceType != static_cast<int32_t>(GnssFenceType::GNSS_FENCE_CIRCULAR)) {
        LBSLOGE(FUSION_FENCE, "ParseGnssFence: invalid gnssFenceType: %{public}d", fenceType);
        return false;
    }
    fence->gnssFenceType = static_cast<GnssFenceType>(fenceType);
 
    if (fence->gnssFenceType == GnssFenceType::GNSS_FENCE_CIRCULAR) {
        napi_value circularFenceValue = nullptr;
        if (napi_get_named_property(env, object, "circularFence", &circularFenceValue) != napi_ok) {
            LBSLOGE(FUSION_FENCE, "ParseGnssFence: circularFence is required for GNSS_FENCE_CIRCULAR");
            return false;
        }
        if (!ParseCircularFence(env, object, fence->circularFence)) {
            LBSLOGE(FUSION_FENCE, "ParseGnssFence: ParseCircularFence failed");
            return false;
        }
    } else if (fence->gnssFenceType == GnssFenceType::GNSS_FENCE_POLYGON) {
        napi_value polygonArray = nullptr;
        if (napi_get_named_property(env, object, "polygon", &polygonArray) != napi_ok) {
            LBSLOGE(FUSION_FENCE, "ParseGnssFence: polygon is required for GNSS_FENCE_POLYGON");
            return false;
        }
        if (!ParsePolygonPoints(env, polygonArray, fence->polygon)) {
            LBSLOGE(FUSION_FENCE, "ParseGnssFence: ParsePolygonPoints failed");
            return false;
        }
    }
    gnssFence = fence;
    return true;
}

static bool ParseWifiMacAddresses(napi_env env, napi_value featureValue, WirelessSignalFeature& feature)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    napi_value macArray = nullptr;
    if (napi_get_named_property(env, featureValue, "mac", &macArray) != napi_ok) {
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, macArray, &isArray) != napi_ok || !isArray) {
        LBSLOGE(FUSION_FENCE, "mac is not an array");
        return false;
    }
    uint32_t macLen = 0;
    if (napi_get_array_length(env, macArray, &macLen) != napi_ok) {
        return false;
    }
    if (macLen == 0) {
        LBSLOGE(FUSION_FENCE, "mac array is empty");
        return false;
    }
    LBSLOGI(FUSION_FENCE, "mac array length=%{public}u", macLen);
    for (uint32_t j = 0; j < macLen; j++) {
        napi_value macValue = nullptr;
        if (napi_get_element(env, macArray, j, &macValue) != napi_ok) {
            continue;
        }
        std::string mac;
        if (!GetStringFromValue(env, macValue, mac)) {
            LBSLOGE(FUSION_FENCE, "invalid mac at index %{public}u", j);
            return false;
        }
        std::regex macRegex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
        if (!std::regex_match(mac, macRegex)) {
            LBSLOGE(FUSION_FENCE, "invalid mac format at index %{public}u: %{public}s", j, mac.c_str());
            return false;
        }
        feature.mac.push_back(mac);
    }
    return true;
}

static bool ValidateWifiFeaturesArray(napi_env env, napi_value featuresArray, uint32_t& arrayLen)
{
    if (env == nullptr || featuresArray == nullptr) {
        LBSLOGE(FUSION_FENCE, "ValidateWifiFeaturesArray: env or featuresArray is null");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, featuresArray, &isArray) != napi_ok || !isArray) {
        LBSLOGE(FUSION_FENCE, "ValidateWifiFeaturesArray: featuresArray is not an array");
        return false;
    }
    if (napi_get_array_length(env, featuresArray, &arrayLen) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "ValidateWifiFeaturesArray: get array length failed");
        return false;
    }
    if (arrayLen == 0) {
        LBSLOGE(FUSION_FENCE, "ValidateWifiFeaturesArray: arrayLen is 0");
        return false;
    }
    return true;
}

static bool ParseWifiFeatures(napi_env env, napi_value featuresArray, std::vector<WirelessSignalFeature>& wifiFeatures)
{
    uint32_t arrayLen = 0;
    if (!ValidateWifiFeaturesArray(env, featuresArray, arrayLen)) {
        return false;
    }
    constexpr uint32_t MAX_WIFI_FEATURES_COUNT = 150;
    if (arrayLen > MAX_WIFI_FEATURES_COUNT) {
        LBSLOGI(FUSION_FENCE, "wifiFeatures array length=%{public}u exceeds max=%{public}u, truncating to %{public}u",
            arrayLen, MAX_WIFI_FEATURES_COUNT, MAX_WIFI_FEATURES_COUNT);
        arrayLen = MAX_WIFI_FEATURES_COUNT;
    }
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value featureValue = nullptr;
        if (napi_get_element(env, featuresArray, i, &featureValue) != napi_ok) {
            continue;
        }
        WirelessSignalFeature feature;
        if (JsObjectToInt(env, featureValue, "rssiAvg", feature.rssiAvg) != SUCCESS ||
            JsObjectToDouble(env, featureValue, "rssiStandardDeviation", feature.rssiStandardDeviation) != SUCCESS ||
            !ParseWifiMacAddresses(env, featureValue, feature)) {
            LBSLOGE(FUSION_FENCE, "ParseWifiFeatures: invalid feature at index %{public}u", i);
            return false;
        }
        wifiFeatures.push_back(feature);
    }
    return true;
}

bool ParseWifiFence(napi_env env, napi_value object, std::shared_ptr<FusionFenceWifi>& wifiFence)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    auto fence = std::make_shared<FusionFenceWifi>();
    int32_t wifiType = 0;
    if (JsObjectToInt(env, object, "type", wifiType) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "type is required");
        return false;
    }
    if (wifiType != static_cast<int32_t>(WifiFingerprintType::WIFI_FINGERPRINT_SEMANTIC) &&
        wifiType != static_cast<int32_t>(WifiFingerprintType::WIFI_FINGERPRINT_LOCATION)) {
        LBSLOGE(FUSION_FENCE, "invalid type: %{public}d", wifiType);
        return false;
    }
    fence->type = static_cast<WifiFingerprintType>(wifiType);
    LBSLOGI(FUSION_FENCE, "type=%{public}d", wifiType);
 
    napi_value featuresArray = nullptr;
    if (napi_get_named_property(env, object, "wifiFeatures", &featuresArray) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "wifiFeatures is required");
        return false;
    }
    if (!ParseWifiFeatures(env, featuresArray, fence->wifiFeatures)) {
        LBSLOGE(FUSION_FENCE, "ParseWifiFeatures failed");
        return false;
    }
    wifiFence = fence;
    return true;
}

static bool ParseCellInfos(napi_env env, napi_value cellInfosArray, std::vector<FusionFenceCellInfo>& cellInfos)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (cellInfosArray == nullptr) {
        LBSLOGE(FUSION_FENCE, "cellInfosArray is null");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, cellInfosArray, &isArray) != napi_ok || !isArray) {
        LBSLOGE(FUSION_FENCE, "cellInfosArray is not an array");
        return false;
    }
    uint32_t arrayLen = 0;
    if (napi_get_array_length(env, cellInfosArray, &arrayLen) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "get array length failed");
        return false;
    }
    const uint32_t MAX_CELL_INFOS_COUNT = 1000;
    if (arrayLen > MAX_CELL_INFOS_COUNT) {
        LBSLOGI(FUSION_FENCE, "cellInfos array length=%{public}u exceeds max=%{public}u, truncating to %{public}u",
            arrayLen, MAX_CELL_INFOS_COUNT, MAX_CELL_INFOS_COUNT);
        arrayLen = MAX_CELL_INFOS_COUNT;
    }
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value cellInfoValue = nullptr;
        if (napi_get_element(env, cellInfosArray, i, &cellInfoValue) != napi_ok) {
            continue;
        }
        FusionFenceCellInfo cellInfo;
        if (ParseCellInfo(env, cellInfoValue, cellInfo)) {
            LBSLOGI(FUSION_FENCE, "cellInfos[%{public}u] parsed", i);
            cellInfos.push_back(cellInfo);
        }
    }
    if (cellInfos.empty()) {
        LBSLOGE(FUSION_FENCE, "cellInfos array is empty");
        return false;
    }
    return true;
}

bool ParseCellFence(napi_env env, napi_value object, std::shared_ptr<FusionFenceCell>& cellFence)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    auto fence = std::make_shared<FusionFenceCell>();
    napi_value cellInfosArray = nullptr;
    if (napi_get_named_property(env, object, "cellInfos", &cellInfosArray) == napi_ok) {
        if (!ParseCellInfos(env, cellInfosArray, fence->cellInfos)) {
            LBSLOGE(FUSION_FENCE, "ParseCellInfos failed");
            return false;
        }
    }
    cellFence = fence;
    return true;
}

static bool ParseGnssFencesArray(
    napi_env env, napi_value gnssFencesArray, std::vector<std::shared_ptr<FusionFenceGnss>>& gnssFences)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (gnssFencesArray == nullptr) {
        LBSLOGE(FUSION_FENCE, "gnssFencesArray is null");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, gnssFencesArray, &isArray) != napi_ok || !isArray) {
        LBSLOGE(FUSION_FENCE, "gnssFencesArray is not an array");
        return false;
    }
    uint32_t arrayLen = 0;
    if (napi_get_array_length(env, gnssFencesArray, &arrayLen) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "get array length failed");
        return false;
    }
    const uint32_t MAX_GNSS_FENCES_COUNT = 30;
    if (arrayLen > MAX_GNSS_FENCES_COUNT) {
        LBSLOGI(FUSION_FENCE, "gnssFences array length=%{public}u exceeds max=%{public}u, truncating to %{public}u",
            arrayLen, MAX_GNSS_FENCES_COUNT, MAX_GNSS_FENCES_COUNT);
        arrayLen = MAX_GNSS_FENCES_COUNT;
    }
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value gnssFenceValue = nullptr;
        if (napi_get_element(env, gnssFencesArray, i, &gnssFenceValue) != napi_ok) {
            continue;
        }
        std::shared_ptr<FusionFenceGnss> gnssFence = nullptr;
        if (ParseGnssFence(env, gnssFenceValue, gnssFence)) {
            gnssFences.push_back(gnssFence);
        }
    }
    return true;
}

static bool ParseCellFencesArray(
    napi_env env, napi_value cellFencesArray, std::vector<std::shared_ptr<FusionFenceCell>>& cellFences)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (cellFencesArray == nullptr) {
        LBSLOGE(FUSION_FENCE, "cellFencesArray is null");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, cellFencesArray, &isArray) != napi_ok || !isArray) {
        LBSLOGE(FUSION_FENCE, "cellFencesArray is not an array");
        return false;
    }
    uint32_t arrayLen = 0;
    if (napi_get_array_length(env, cellFencesArray, &arrayLen) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "get array length failed");
        return false;
    }
    const uint32_t MAX_CELL_FENCES_COUNT = 30;
    if (arrayLen > MAX_CELL_FENCES_COUNT) {
        LBSLOGI(FUSION_FENCE, "cellFences array length=%{public}u exceeds max=%{public}u, truncating to %{public}u",
            arrayLen, MAX_CELL_FENCES_COUNT, MAX_CELL_FENCES_COUNT);
        arrayLen = MAX_CELL_FENCES_COUNT;
    }
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value cellFenceValue = nullptr;
        if (napi_get_element(env, cellFencesArray, i, &cellFenceValue) != napi_ok) {
            continue;
        }
        std::shared_ptr<FusionFenceCell> cellFence = nullptr;
        if (ParseCellFence(env, cellFenceValue, cellFence)) {
            LBSLOGI(FUSION_FENCE, "cellFences[%{public}u] parsed", i);
            cellFences.push_back(cellFence);
        }
    }
    return true;
}

static bool ParseWifiFencesArray(
    napi_env env, napi_value wifiFencesArray, std::vector<std::shared_ptr<FusionFenceWifi>>& wifiFences)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (wifiFencesArray == nullptr) {
        LBSLOGE(FUSION_FENCE, "wifiFencesArray is null");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, wifiFencesArray, &isArray) != napi_ok || !isArray) {
        LBSLOGE(FUSION_FENCE, "wifiFencesArray is not an array");
        return false;
    }
    uint32_t arrayLen = 0;
    if (napi_get_array_length(env, wifiFencesArray, &arrayLen) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "get array length failed");
        return false;
    }
    const uint32_t MAX_WIFI_FENCES_COUNT = 30;
    if (arrayLen > MAX_WIFI_FENCES_COUNT) {
        LBSLOGI(FUSION_FENCE, "wifiFences array length=%{public}u exceeds max=%{public}u, truncating to %{public}u",
            arrayLen, MAX_WIFI_FENCES_COUNT, MAX_WIFI_FENCES_COUNT);
        arrayLen = MAX_WIFI_FENCES_COUNT;
    }
    for (uint32_t i = 0; i < arrayLen; i++) {
        napi_value wifiFenceValue = nullptr;
        if (napi_get_element(env, wifiFencesArray, i, &wifiFenceValue) != napi_ok) {
            continue;
        }
        std::shared_ptr<FusionFenceWifi> wifiFence = nullptr;
        if (ParseWifiFence(env, wifiFenceValue, wifiFence)) {
            LBSLOGI(FUSION_FENCE, "wifiFences[%{public}u] parsed", i);
            wifiFences.push_back(wifiFence);
        }
    }
    return true;
}

static bool ParsePoiLocation(napi_env env, napi_value object, FusionFenceRequest& request)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    napi_value poiLocation = nullptr;
    if (napi_get_named_property(env, object, "poiLocation", &poiLocation) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "poiLocation is required but missing");
        return false;
    }
    FusionFencePoint poiPoint;
    if (!ParsePoint(env, poiLocation, poiPoint)) {
        return false;
    }
    request.SetPoiLocation(poiPoint);
    return true;
}

static bool ParseGnssFences(napi_env env, napi_value object, FusionFenceRequest& request)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    napi_value gnssFencesArray = nullptr;
    if (napi_get_named_property(env, object, "gnssFences", &gnssFencesArray) != napi_ok) {
        return false;
    }
    std::vector<std::shared_ptr<FusionFenceGnss>> gnssFences;
    if (!ParseGnssFencesArray(env, gnssFencesArray, gnssFences)) {
        LBSLOGE(FUSION_FENCE, "ParseGnssFencesArray failed");
        return false;
    }
    if (gnssFences.empty()) {
        LBSLOGE(FUSION_FENCE, "gnssFences array is empty");
        return false;
    }
    request.SetGnssFences(gnssFences);
    return true;
}

static bool ParseCellFences(napi_env env, napi_value object, FusionFenceRequest& request)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    napi_value cellFencesArray = nullptr;
    if (napi_get_named_property(env, object, "cellFences", &cellFencesArray) != napi_ok) {
        return false;
    }
    std::vector<std::shared_ptr<FusionFenceCell>> cellFences;
    if (!ParseCellFencesArray(env, cellFencesArray, cellFences)) {
        LBSLOGE(FUSION_FENCE, "ParseCellFencesArray failed");
        return false;
    }
    if (cellFences.empty()) {
        LBSLOGE(FUSION_FENCE, "cellFences array is empty");
        return false;
    }
    request.SetCellFences(cellFences);
    return true;
}

static bool ParseWifiFences(napi_env env, napi_value object, FusionFenceRequest& request)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    napi_value wifiFencesArray = nullptr;
    if (napi_get_named_property(env, object, "wifiFences", &wifiFencesArray) != napi_ok) {
        return false;
    }
    std::vector<std::shared_ptr<FusionFenceWifi>> wifiFences;
    if (!ParseWifiFencesArray(env, wifiFencesArray, wifiFences)) {
        LBSLOGE(FUSION_FENCE, "ParseWifiFencesArray failed");
        return false;
    }
    if (wifiFences.empty()) {
        LBSLOGE(FUSION_FENCE, "wifiFences array is empty");
        return false;
    }
    request.SetWifiFences(wifiFences);
    return true;
}

static bool ParseRequiredStrings(napi_env env, napi_value object, std::string& identifier, std::string& poiType)
{
    if (JsObjectToString(env, object, "identifier", MAX_BUF_LEN, identifier) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "identifier is required");
        return false;
    }
    if (identifier.empty()) {
        LBSLOGE(FUSION_FENCE, "identifier is empty");
        return false;
    }
    JsObjectToString(env, object, "poiType", MAX_BUF_LEN, poiType);
    return true;
}

static bool ParseAndValidateSceneAndFenceType(napi_env env, napi_value object, int32_t& scene, int32_t& fenceType)
{
    if (JsObjectToInt(env, object, "scene", scene) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "scene is required");
        return false;
    }
    if (scene != static_cast<int32_t>(FusionFenceScene::AIRPORT) &&
        scene != static_cast<int32_t>(FusionFenceScene::TRAIN_STATION) &&
        scene != static_cast<int32_t>(FusionFenceScene::SUBWAY) &&
        scene != static_cast<int32_t>(FusionFenceScene::SHOP)) {
        LBSLOGE(FUSION_FENCE, "invalid scene: %{public}d", scene);
        return false;
    }
    if (JsObjectToInt(env, object, "fenceType", fenceType) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "fenceType is required");
        return false;
    }
    constexpr int32_t VALID_FENCE_TYPE_MASK = static_cast<int32_t>(FusionFenceType::FUSION_FENCE_GNSS) |
        static_cast<int32_t>(FusionFenceType::FUSION_FENCE_CELLULAR) |
        static_cast<int32_t>(FusionFenceType::FUSION_FENCE_WIFI) |
        static_cast<int32_t>(FusionFenceType::FUSION_FENCE_BLUETOOTH);
    if (fenceType <= 0 || (fenceType & ~VALID_FENCE_TYPE_MASK) != 0) {
        LBSLOGE(FUSION_FENCE, "invalid fenceType: %{public}d", fenceType);
        return false;
    }
    return true;
}

static bool ParseTimingFields(napi_env env, napi_value object, std::shared_ptr<FusionFenceRequest>& fusionRequest)
{
    int32_t monitorTransitionEvents = 0;
    if (JsObjectToInt(env, object, "monitorTransitionEvents", monitorTransitionEvents) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "monitorTransitionEvents is required");
        return false;
    }
    constexpr int32_t VALID_TRANSITION_MASK =
        static_cast<int32_t>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER) |
        static_cast<int32_t>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_EXIT) |
        static_cast<int32_t>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_DWELL) |
        static_cast<int32_t>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_APPROACHING_GEOFENCE) |
        static_cast<int32_t>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_LEAVING_GEOFENCE) |
        static_cast<int32_t>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_NEAR_WANDER);
    if (monitorTransitionEvents <= 0 ||
        (monitorTransitionEvents & VALID_TRANSITION_MASK) != monitorTransitionEvents) {
        LBSLOGE(FUSION_FENCE, "invalid monitorTransitionEvents: %{public}d", monitorTransitionEvents);
        return false;
    }
    int64_t loiterTimeMs = 0;
    if (JsObjectToInt64(env, object, "loiterTimeMs", loiterTimeMs) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "loiterTimeMs is required");
        return false;
    }
    if (loiterTimeMs < 0) {
        LBSLOGE(FUSION_FENCE, "invalid loiterTimeMs: %{public}ld", loiterTimeMs);
        return false;
    }
    int64_t expirationMs = 0;
    if (JsObjectToInt64(env, object, "expirationMs", expirationMs) != SUCCESS) {
        LBSLOGE(FUSION_FENCE, "expirationMs is required");
        return false;
    }
    if (expirationMs <= 0) {
        LBSLOGE(FUSION_FENCE, "invalid expirationMs: %{public}ld", expirationMs);
        return false;
    }
    fusionRequest->SetMonitorTransitionEvents(monitorTransitionEvents);
    fusionRequest->SetLoiterTimeMs(loiterTimeMs);
    fusionRequest->SetExpirationMs(expirationMs);
    return true;
}

static bool ParseFenceArraysByType(napi_env env, napi_value object, int32_t fenceType,
    std::shared_ptr<FusionFenceRequest>& fusionRequest)
{
    if ((fenceType & static_cast<int32_t>(FusionFenceType::FUSION_FENCE_GNSS)) != 0) {
        if (!ParseGnssFences(env, object, *fusionRequest)) {
            LBSLOGE(FUSION_FENCE, "ParseGnssFences failed");
            return false;
        }
    }
    if ((fenceType & static_cast<int32_t>(FusionFenceType::FUSION_FENCE_CELLULAR)) != 0) {
        if (!ParseCellFences(env, object, *fusionRequest)) {
            LBSLOGE(FUSION_FENCE, "ParseCellFences failed");
            return false;
        }
    }
    if ((fenceType & static_cast<int32_t>(FusionFenceType::FUSION_FENCE_WIFI)) != 0) {
        if (!ParseWifiFences(env, object, *fusionRequest)) {
            LBSLOGE(FUSION_FENCE, "ParseWifiFences failed");
            return false;
        }
    }
    return true;
}

bool ParseFusionFenceRequest(napi_env env, napi_value object, std::shared_ptr<FusionFenceRequest>& request)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    auto fusionRequest = std::make_shared<FusionFenceRequest>();
    std::string identifier;
    std::string poiType;
    if (!ParseRequiredStrings(env, object, identifier, poiType)) {
        return false;
    }
    int32_t scene = 0;
    int32_t fenceType = 0;
    if (!ParseAndValidateSceneAndFenceType(env, object, scene, fenceType)) {
        return false;
    }
    if (!ParseTimingFields(env, object, fusionRequest)) {
        return false;
    }
    if (!ParsePoiLocation(env, object, *fusionRequest)) {
        return false;
    }
    if (!ParseFenceArraysByType(env, object, fenceType, fusionRequest)) {
        return false;
    }
    LBSLOGI(FUSION_FENCE, "identifier=%{public}s, scene=%{public}d, fenceType=%{public}d",
        identifier.c_str(), scene, fenceType);
    fusionRequest->SetIdentifier(identifier);
    fusionRequest->SetScene(static_cast<FusionFenceScene>(scene));
    fusionRequest->SetFenceType(fenceType);
    fusionRequest->SetPoiType(poiType);
    request = fusionRequest;
    return true;
}

napi_value CreateFusionFenceTransitionJsObj(napi_env env, const FusionFenceTransition& transition)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
 
    napi_value identifierValue = nullptr;
    if (napi_create_string_utf8(env, transition.identifier.c_str(), NAPI_AUTO_LENGTH, &identifierValue) != napi_ok) {
        return nullptr;
    }
    napi_set_named_property(env, result, "identifier", identifierValue);
 
    napi_value sceneValue = nullptr;
    if (napi_create_int32(env, static_cast<int32_t>(transition.scene), &sceneValue) != napi_ok) {
        return nullptr;
    }
    napi_set_named_property(env, result, "scene", sceneValue);
 
    napi_value transitionEventValue = nullptr;
    if (napi_create_int32(env, static_cast<int32_t>(transition.transitionEvent), &transitionEventValue) != napi_ok) {
        return nullptr;
    }
    napi_set_named_property(env, result, "transitionEvent", transitionEventValue);
 
    return result;
}

bool ParseFusionFenceTransitionCallback(napi_env env, napi_value object,
    sptr<FusionFenceCallbackNapi>& callbackHost)
{
    if (env == nullptr) {
        LBSLOGE(FUSION_FENCE, "env is null");
        return false;
    }
    if (object == nullptr) {
        LBSLOGE(FUSION_FENCE, "object is null");
        return false;
    }
    napi_ref handlerRef = nullptr;
    napi_value callbackNapiValue = nullptr;
    if (napi_get_named_property(env, object, "fenceTransitionCallback", &callbackNapiValue) != napi_ok) {
        LBSLOGE(FUSION_FENCE, "fenceTransitionCallback is required but missing");
        return false;
    }
    if (callbackNapiValue != nullptr) {
        napi_valuetype valueType;
        if (napi_typeof(env, callbackNapiValue, &valueType) != napi_ok || valueType != napi_function) {
            LBSLOGE(FUSION_FENCE, "fenceTransitionCallback is not a function");
            return false;
        }
        if (napi_create_reference(env, callbackNapiValue, 1, &handlerRef) != napi_ok) {
            LBSLOGE(FUSION_FENCE, "napi_create_reference failed");
            return false;
        }
    }
    callbackHost->SetEnv(env);
    callbackHost->SetHandleCb(handlerRef);
    return true;
}

bool GetStringFromValue(const napi_env& env, napi_value value, std::string& result)
{
    napi_valuetype valueType;
    if (napi_typeof(env, value, &valueType) != napi_ok || valueType != napi_string) {
        return false;
    }
    size_t bufsize = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &bufsize) != napi_ok) {
        return false;
    }
    if (bufsize == 0) {
        return false;
    }
    if (bufsize > SIZE_MAX - 1) {
        LBSLOGE(FUSION_FENCE, "bufsize overflow: %{public}zu", bufsize);
        return false;
    }
    result.resize(bufsize, '\0');
    if (napi_get_value_string_utf8(env, value, result.data(), bufsize + 1, &bufsize) != napi_ok) {
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS