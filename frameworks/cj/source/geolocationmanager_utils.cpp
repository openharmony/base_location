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

#include "geolocationmanager_utils.h"

#include "locator_proxy.h"

namespace OHOS {
namespace GeoLocationManager {
char* MallocCString(const std::string& origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto len = origin.length() + 1;
    char* res = static_cast<char*>(malloc(sizeof(char) * len));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), len);
}

char** StringVectorToCPointer(const std::vector<std::string>& arr)
{
    if (arr.size() == 0) {
        return nullptr;
    }
    char** res = static_cast<char**>(malloc(sizeof(char*) * arr.size()));
    if (res == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < arr.size(); i++) {
        res[i] = MallocCString(arr[i]);
    }
    return res;
}

CMapStringString MapToCMapStringString(const std::map<std::string, std::string>& map)
{
    CMapStringString cMap{0};
    cMap.keys = static_cast<char**>(malloc(sizeof(char*) * map.size()));
    cMap.values = static_cast<char**>(malloc(sizeof(char*) * map.size()));
    if (cMap.keys == nullptr || cMap.values == nullptr) {
        free(cMap.keys);
        free(cMap.values);
        return CMapStringString{0};
    }
    int64_t index = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        cMap.keys[index] = MallocCString(it->first);
        cMap.values[index] = MallocCString(it->second);
        index++;
    }
    return cMap;
}

CJLocation NativeLocationToCJLocation(const Location::Location& loc)
{
    auto additions = loc.GetAdditions();
    uint32_t additionSize = static_cast<uint32_t>(loc.GetAdditionSize());
    uint32_t additionsMapSize = static_cast<uint32_t>(loc.GetAdditionsMap().size());
    uint32_t additionSizeMin = std::min({additionSize, static_cast<uint32_t>(additions.size()), additionsMapSize});
    return CJLocation{ .latitude = loc.GetLatitude(), .longitude = loc.GetLongitude(),
        .altitude = loc.GetAltitude(), .accuracy = loc.GetAccuracy(), .speed = loc.GetSpeed(),
        .direction = loc.GetDirection(), .timeStamp = loc.GetTimeStamp(), .timeSinceBoot = loc.GetTimeSinceBoot(),
        .additions = StringVectorToCPointer(additions), .additionSize = additionSizeMin,
        .additionsMap = MapToCMapStringString(loc.GetAdditionsMap()), .altitudeAccuracy = loc.GetAltitudeAccuracy(),
        .speedAccuracy = loc.GetSpeedAccuracy(), .directionAccuracy = loc.GetDirectionAccuracy(),
        .uncertaintyOfTimeSinceBoot = loc.GetUncertaintyOfTimeSinceBoot(), .sourceType = loc.GetLocationSourceType()
        };
}

void CJCurrentLocationRequestToRequestConfig(const CJCurrentLocationRequest& request,
    std::unique_ptr<Location::RequestConfig>& requestConfig)
{
    requestConfig->SetPriority(request.priority);
    requestConfig->SetScenario(request.scenario);
    requestConfig->SetMaxAccuracy(request.maxAccuracy);
    requestConfig->SetTimeOut(request.timeoutMs);
}

void CJSingleLocationRequestRequestToRequestConfig(const CJSingleLocationRequest& request,
    std::unique_ptr<Location::RequestConfig>& requestConfig)
{
    requestConfig->SetPriority(request.locatingPriority);
    requestConfig->SetTimeOut(request.locatingTimeoutMs);
}

bool CJReverseGeoCodeRequestToMessageParcel(CJReverseGeoCodeRequest& request, MessageParcel& dataParcel)
{
    double latitude = request.latitude;
    double longitude = request.longitude;
    int maxItems = request.maxItems;
    std::string locale = request.locale;
    std::string country = request.country;
    if (latitude < Location::MIN_LATITUDE || latitude > Location::MAX_LATITUDE) {
        return false;
    }
    if (longitude < Location::MIN_LONGITUDE || longitude > Location::MAX_LONGITUDE) {
        return false;
    }
    if (!dataParcel.WriteInterfaceToken(Location::LocatorProxy::GetDescriptor())) {
        return false;
    }
    dataParcel.WriteString16(Str8ToStr16(locale)); // locale
    dataParcel.WriteDouble(latitude); // latitude
    dataParcel.WriteDouble(longitude); // longitude
    dataParcel.WriteInt32(maxItems); // maxItems
    dataParcel.WriteString16(Str8ToStr16(Location::CommonUtils::GenerateUuid())); // transId
    dataParcel.WriteString16(Str8ToStr16(country)); // country
    return true;
}

void HandleDescriptions(std::shared_ptr<Location::GeoAddress>& geoAddress, CJGeoAddress& eachObj)
{
    if (geoAddress->descriptionsSize_ > 0) {
        char** descriptionArray = static_cast<char**>(malloc(geoAddress->descriptionsSize_ * sizeof(char*)));
        if (descriptionArray == nullptr) {
            eachObj.descriptionsSize = 0;
        } else {
            for (int index = 0; index < geoAddress->descriptionsSize_; index++) {
                descriptionArray[index] = MallocCString(geoAddress->GetDescriptions(index).c_str());
            }
        }
        eachObj.descriptions = descriptionArray;
    }
}

CJGeoAddressArr ListGeoAddressToCJGeoAddressArr(std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    CJGeoAddressArr ret{0};
    if (replyList.size() == 0) {
        return ret;
    }
    ret.head = static_cast<CJGeoAddress*>(malloc(replyList.size() * sizeof(CJGeoAddress)));
    if (ret.head == nullptr) {
        return ret;
    }
    ret.size = replyList.size();
    uint32_t idx = 0;
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto geoAddress = *iter;
        CJGeoAddress eachObj{0};
        eachObj.latitude = geoAddress->GetLatitude();
        eachObj.longitude = geoAddress->GetLongitude();
        eachObj.locale = MallocCString(geoAddress->locale_.c_str());
        eachObj.placeName = MallocCString(geoAddress->placeName_.c_str());
        eachObj.countryCode = MallocCString(geoAddress->countryCode_.c_str());
        eachObj.countryName = MallocCString(geoAddress->countryName_.c_str());
        eachObj.administrativeArea = MallocCString(geoAddress->administrativeArea_.c_str());
        eachObj.subAdministrativeArea = MallocCString(geoAddress->subAdministrativeArea_.c_str());
        eachObj.locality = MallocCString(geoAddress->locality_.c_str());
        eachObj.subLocality = MallocCString(geoAddress->subLocality_.c_str());
        eachObj.roadName = MallocCString(geoAddress->roadName_.c_str());
        eachObj.subRoadName = MallocCString(geoAddress->subRoadName_.c_str());
        eachObj.premises = MallocCString(geoAddress->premises_.c_str());
        eachObj.postalCode = MallocCString(geoAddress->postalCode_.c_str());
        eachObj.phoneNumber = MallocCString(geoAddress->phoneNumber_.c_str());
        eachObj.addressUrl = MallocCString(geoAddress->addressUrl_.c_str());
        eachObj.descriptionsSize = geoAddress->descriptionsSize_;
        HandleDescriptions(geoAddress, eachObj);
        ret.head[idx] = eachObj;
        idx++;
    }
    return ret;
}

bool CJGeoCodeRequestToMessageParcel(CJGeoCodeRequest& request, MessageParcel& dataParcel)
{
    std::string locale = request.locale;
    std::string country = request.country;
    std::string description = request.description;
    if (description == "") {
        LBSLOGE(Location::LOCATOR_STANDARD, "The required description field should not be empty.");
        return false;
    }
    int32_t maxItems = request.maxItems;
    double minLatitude = request.minLatitude;
    double minLongitude = request.minLongitude;
    double maxLatitude = request.maxLatitude;
    double maxLongitude = request.maxLongitude;
    if (minLatitude < Location::MIN_LATITUDE || minLatitude > Location::MAX_LATITUDE) {
        return false;
    }
    if (minLongitude < Location::MIN_LONGITUDE || minLongitude > Location::MAX_LONGITUDE) {
        return false;
    }
    if (maxLatitude < Location::MIN_LATITUDE || maxLatitude > Location::MAX_LATITUDE) {
        return false;
    }
    if (maxLongitude < Location::MIN_LONGITUDE || maxLongitude > Location::MAX_LONGITUDE) {
        return false;
    }
    if (!dataParcel.WriteInterfaceToken(Location::LocatorProxy::GetDescriptor())) {
        LBSLOGE(Location::LOCATOR_STANDARD, "write interfaceToken fail!");
        return false;
    }
    dataParcel.WriteString16(Str8ToStr16(locale)); // locale
    dataParcel.WriteString16(Str8ToStr16(description)); // description
    dataParcel.WriteInt32(maxItems); // maxItems
    dataParcel.WriteDouble(minLatitude); // latitude
    dataParcel.WriteDouble(minLongitude); // longitude
    dataParcel.WriteDouble(maxLatitude); // latitude
    dataParcel.WriteDouble(maxLongitude); // longitude
    dataParcel.WriteString16(Str8ToStr16(Location::CommonUtils::GenerateUuid())); // transId
    dataParcel.WriteString16(Str8ToStr16(country)); // country
    return true;
}

void CJLocationRequestToRequestConfig(CJLocationRequest& request,
    std::unique_ptr<Location::RequestConfig>& requestConfig)
{
    requestConfig->SetPriority(request.priority);
    requestConfig->SetScenario(request.scenario);
    if (request.timeInterval >= 0 && request.timeInterval < 1) {
        requestConfig->SetTimeInterval(1);
    } else {
        requestConfig->SetTimeInterval(request.timeInterval);
    }
    requestConfig->SetMaxAccuracy(request.maxAccuracy);
    requestConfig->SetDistanceInterval(request.distanceInterval);
}

void CJContinuousLocationRequestToRequestConfig(CJContinuousLocationRequest request,
    std::unique_ptr<Location::RequestConfig>& requestConfig)
{
    requestConfig->SetScenario(request.locationScenario);
    if (request.interval >= 0 && request.interval < 1) {
        requestConfig->SetTimeInterval(1);
    } else {
        requestConfig->SetTimeInterval(request.interval);
    }
}

CJLocationArr LocationVectorToCJLocationArr(const std::vector<std::unique_ptr<Location::Location>>& locations)
{
    if (locations.size() == 0) {
        return CJLocationArr{0};
    }
    CJLocation* head = static_cast<CJLocation*>(malloc(sizeof(CJLocation) * locations.size()));
    if (head == nullptr) {
        return CJLocationArr{0};
    }
    for (size_t i = 0; i < locations.size(); i++) {
        head[i] = NativeLocationToCJLocation(*(locations[i]));
    }
    return CJLocationArr{ .head = head, .size = locations.size() };
}

void CJCachedGnssLocationsRequestToCachedLocationRequest(CJCachedGnssLocationsRequest& request,
    std::unique_ptr<Location::CachedGnssLocationsRequest>& requestConfig)
{
    requestConfig->reportingPeriodSec = request.reportingPeriodSec;
    requestConfig->wakeUpCacheQueueFull = request.wakeUpCacheQueueFull;
}

CArrI32 IntVectorToCArrI32(std::vector<int> arr)
{
    if (arr.size() == 0) {
        return CArrI32{0};
    }
    int32_t* head = static_cast<int32_t*>(malloc(sizeof(int32_t) * arr.size()));
    if (head == nullptr) {
        return CArrI32{0};
    }
    for (size_t i = 0; i < arr.size(); i++) {
        head[i] = arr[i];
    }
    return CArrI32{ .head = head, .size = arr.size() };
}

CArrF64 DoubleVectorToCArrF64(std::vector<double> arr)
{
    if (arr.size() == 0) {
        return CArrF64{0};
    }
    double* head = static_cast<double*>(malloc(sizeof(double) * arr.size()));
    if (head == nullptr) {
        return CArrF64{0};
    }
    for (size_t i = 0; i < arr.size(); i++) {
        head[i] = arr[i];
    }
    return CArrF64{ .head = head, .size = arr.size() };
}

CJSatelliteStatusInfo SatelliteStatusInfoToCJSatelliteStatus(const std::unique_ptr<Location::SatelliteStatus>&
    statusInfo)
{
    CJSatelliteStatusInfo res = CJSatelliteStatusInfo{0};
    res.satellitesNumber = statusInfo->GetSatellitesNumber();
    res.satelliteIds = IntVectorToCArrI32(statusInfo->GetSatelliteIds());
    res.carrierToNoiseDensitys = DoubleVectorToCArrF64(statusInfo->GetCarrierToNoiseDensitys());
    res.altitudes = DoubleVectorToCArrF64(statusInfo->GetAltitudes());
    res.azimuths = DoubleVectorToCArrF64(statusInfo->GetAzimuths());
    res.carrierFrequencies = DoubleVectorToCArrF64(statusInfo->GetCarrierFrequencies());
    res.constellationTypes = IntVectorToCArrI32(statusInfo->GetConstellationTypes());
    res.additionalInfoList = IntVectorToCArrI32(statusInfo->GetSatelliteAdditionalInfoList());
    return res;
}

CJCountryCode CountryCodeToCJCountryCode(const std::shared_ptr<Location::CountryCode>& country)
{
    return CJCountryCode{ .country = MallocCString(country->GetCountryCodeStr()),
        .type = country->GetCountryCodeType() };
}
}
}