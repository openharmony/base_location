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

#ifndef GEOLOCATIONMANAGER_UTILS_H
#define GEOLOCATIONMANAGER_UTILS_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "location.h"
#include "location_napi_event.h"
#include "napi_util.h"

const int32_t ERRCODE_MEMORY_ERROR = -1;

namespace OHOS {
namespace GeoLocationManager {
struct CMapStringString {
    char** keys;
    char** values;
};

struct CJLocation {
    double latitude;
    double longitude;
    double altitude;
    double accuracy;
    double speed;
    double direction;
    int64_t timeStamp;
    int64_t timeSinceBoot;
    char** additions;
    int64_t additionSize;
    CMapStringString additionsMap;
    double altitudeAccuracy;
    double speedAccuracy;
    double directionAccuracy;
    int64_t uncertaintyOfTimeSinceBoot;
    int32_t sourceType;
};

struct CJCurrentLocationRequest {
    int32_t priority;
    int32_t scenario;
    float maxAccuracy;
    int32_t timeoutMs;
};

struct CJSingleLocationRequest {
    int32_t locatingPriority;
    int32_t locatingTimeoutMs;
};

struct CJReverseGeocodeRequest {
    char* locale;
    char* country;
    double latitude;
    double longitude;
    int32_t maxItems;
};

struct CJLocationCommand {
    int32_t scenario;
    char* command;
};

struct CJCountryCode {
    char* country;
    int32_t type;
};

struct CJReverseGeoCodeRequest {
    char* locale;
    char* country;
    double latitude;
    double longitude;
    int32_t maxItems;
};

struct CJGeoCodeRequest {
    char* locale;
    char* country;
    char* description;
    int32_t maxItems;
    double minLatitude;
    double minLongitude;
    double maxLatitude;
    double maxLongitude;
};


struct CJGeoAddress {
    double latitude;
    double longitude;
    char* locale;
    char* placeName;
    char* countryCode;
    char* countryName;
    char* administrativeArea;
    char* subAdministrativeArea;
    char* locality;
    char* subLocality;
    char* roadName;
    char* subRoadName;
    char* premises;
    char* postalCode;
    char* phoneNumber;
    char* addressUrl;
    char** descriptions;
    int descriptionsSize;
};

struct CJGeoAddressArr {
    CJGeoAddress* head;
    int64_t size;
};

enum CallbackType {
    locationChange = 0
};

struct CJLocationRequest {
    int32_t priority;
    int32_t scenario;
    int32_t timeInterval;
    double distanceInterval;
    float maxAccuracy;
};

struct CJContinuousLocationRequest {
    int32_t interval;
    int32_t locationScenario;
};

struct CJCachedGnssLocationsRequest {
    int32_t reportingPeriodSec;
    bool wakeUpCacheQueueFull;
};

struct CJLocationArr {
    CJLocation* head;
    int64_t size;
};

struct CArrI32 {
    int32_t* head;
    int64_t size;
};

struct CArrF64 {
    double* head;
    int64_t size;
};

struct CJSatelliteStatusInfo {
    int32_t satellitesNumber;
    CArrI32 satelliteIds;
    CArrF64 carrierToNoiseDensitys;
    CArrF64 altitudes;
    CArrF64 azimuths;
    CArrF64 carrierFrequencies;
    CArrI32 constellationTypes;
    CArrI32 additionalInfoList;
};

char* MallocCString(const std::string& origin);

char** StringVectorToCPointer(const std::vector<std::string>& arr);

CMapStringString MapToCMapStringString(const std::map<std::string, std::string>& map);

CJLocation NativeLocationToCJLocation(const Location::Location& loc);

void CJCurrentLocationRequestToRequestConfig(const CJCurrentLocationRequest& request,
    std::unique_ptr<Location::RequestConfig>& requestConfig);

void CJSingleLocationRequestRequestToRequestConfig(const CJSingleLocationRequest& request,
    std::unique_ptr<Location::RequestConfig>& requestConfig);

bool CJReverseGeoCodeRequestToMessageParcel(CJReverseGeoCodeRequest& request, MessageParcel& dataParcel);

CJGeoAddressArr ListGeoAddressToCJGeoAddressArr(std::list<std::shared_ptr<Location::GeoAddress>>& replyList);

bool CJGeoCodeRequestToMessageParcel(CJGeoCodeRequest& request, MessageParcel& dataParcel);

void CJLocationRequestToRequestConfig(CJLocationRequest& request,
    std::unique_ptr<Location::RequestConfig>& requestConfig);

void CJContinuousLocationRequestToRequestConfig(CJContinuousLocationRequest request,
    std::unique_ptr<Location::RequestConfig>& requestConfig);

CJLocationArr LocationVectorToCJLocationArr(const std::vector<std::unique_ptr<Location::Location>>& locations);

void CJCachedGnssLocationsRequestToCachedLocationRequest(CJCachedGnssLocationsRequest& request,
    std::unique_ptr<Location::CachedGnssLocationsRequest>& requestConfig);

CArrI32 IntVectorToCArrI32(std::vector<int> arr);

CArrF64 DoubleVectorToCArrF64(std::vector<double> arr);

CJSatelliteStatusInfo SatelliteStatusInfoToCJSatelliteStatus(const std::unique_ptr<Location::SatelliteStatus>&
    statusInfo);

CJCountryCode CountryCodeToCJCountryCode(const std::shared_ptr<Location::CountryCode>& country);
}
}

#endif // GEOLOCATIONMANAGER_UTILS_H