/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "poi_info_manager.h"

#include "location_log.h"
#include "common_utils.h"
#ifdef MOVEMENT_CLIENT_ENABLE
#include "locator_msdp_monitor_manager.h"
#endif

namespace OHOS {
namespace Location {

static constexpr int STILL_POI_EXPIRED_TIME = 30 * 60 * 1000; // Unit ms
static constexpr int POI_EXPIRED_TIME = 25 * 1000; // Unit ms
static constexpr int MAX_UTC_TIME_SIZE = 16;
static constexpr int MAX_POI_ARRAY_SIZE = 20;

PoiInfoManager* PoiInfoManager::GetInstance()
{
    static PoiInfoManager data;
    return &data;
}

PoiInfoManager::PoiInfoManager() {}

PoiInfoManager::~PoiInfoManager() {}

void PoiInfoManager::UpdateCachedPoiInfo(const std::unique_ptr<Location>& location)
{
    std::map<std::string, std::string> additionMap = location->GetAdditionsMap();
    if (additionMap["poiInfos"] != "") {
        std::string poiInfos = std::string("poiInfos:") + additionMap["poiInfos"];
        uint64_t poiInfoTime = GetPoiInfoTime(poiInfos);
        SetLatestPoiInfo(poiInfos);
        SetLatestPoiInfoTime(poiInfoTime);
    }
}

void PoiInfoManager::ClearPoiInfos(const std::unique_ptr<Location>& finalLocation)
{
    std::vector<std::string> additions;
    for (auto& str : finalLocation->GetAdditions()) {
        if (str.find("poiInfos") != std::string::npos) {
            continue;
        }
        additions.push_back(str);
    }
    finalLocation->SetAdditions(additions, false);
    finalLocation->SetAdditionSize(finalLocation->GetAdditions().size());
    return;
}

void PoiInfoManager::UpdateLocationPoiInfo(const std::unique_ptr<Location>& finalLocation)
{
    std::map<std::string, std::string> additionMap = finalLocation->GetAdditionsMap();
    std::string poiInfos = additionMap["poiInfos"];
    uint64_t poiInfoTime = GetPoiInfoTime(poiInfos);
    if (poiInfos != "") {
        if (IsPoiInfoValid(poiInfos, poiInfoTime)) {
            PoiInfo poiInfo = ParsePoiInfoFromStr(poiInfos);
            finalLocation->SetPoiInfo(poiInfo);
            return;
        } else {
            LBSLOGI(REPORT_MANAGER, "PoiInfos Expied");
            ClearPoiInfos(finalLocation);
        }
    }
    std::string latestPoiInfos = GetLatestPoiInfo();
    uint64_t latestPoiInfoTime = GetLatestPoiInfoTime();
    if (!IsPoiInfoValid(latestPoiInfos, latestPoiInfoTime)) {
        std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
        latestPoiInfoStruct_ = {0};
        return;
    }
    AddCachedPoiInfoToLocation(finalLocation);
}

void PoiInfoManager::AddCachedPoiInfoToLocation(const std::unique_ptr<Location>& finalLocation)
{
    LBSLOGI(REPORT_MANAGER, "Add Latest PoiInfos");
    std::string poiInfos = GetLatestPoiInfo();
    std::vector<std::string> addition = finalLocation->GetAdditions();
    addition.push_back(poiInfos);
    finalLocation->SetAdditions(addition, false);
    finalLocation->SetAdditionSize(finalLocation->GetAdditions().size());
    std::string poiKey = "poiInfos:";
    std::size_t pos = poiInfos.find(poiKey);
    if (pos != std::string::npos) {
        std::string result = poiInfos.substr(pos + poiKey.size()); // ����"poiInfos:"����
        PoiInfo poiInfo = ParsePoiInfoFromStr(result);
        finalLocation->SetPoiInfo(poiInfo);
    } else {
        LBSLOGE(REPORT_MANAGER, "Not Found poiInfos");
    }
}

bool PoiInfoManager::IsPoiInfoValid(std::string poiInfos, uint64_t poiInfoTime)
{
    uint64_t curTimeStamp = CommonUtils::GetCurrentTimeMilSec();
    uint64_t enterStillTime = 0;
    bool isInStillState = false;
#ifdef MOVEMENT_CLIENT_ENABLE
    isInStillState = LocatorMsdpMonitorManager::GetInstance()->GetStillMovementState();
    enterStillTime = LocatorMsdpMonitorManager::GetInstance()->GetEnterStillTime();
#endif
    if (isInStillState && poiInfoTime > enterStillTime && curTimeStamp - poiInfoTime < STILL_POI_EXPIRED_TIME) {
        return true;
    }
    if (curTimeStamp - poiInfoTime < POI_EXPIRED_TIME) {
        return true;
    }
    return false;
}

std::string PoiInfoManager::GetLatestPoiInfo()
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    return latestPoiInfoStruct_.latestPoiInfos;
}

void PoiInfoManager::SetLatestPoiInfo(std::string poiInfo)
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    latestPoiInfoStruct_.latestPoiInfos = poiInfo;
}

uint64_t PoiInfoManager::GetLatestPoiInfoTime()
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    return latestPoiInfoStruct_.poiInfosTime;
}

void PoiInfoManager::SetLatestPoiInfoTime(uint64_t poiInfoTime)
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    latestPoiInfoStruct_.poiInfosTime = poiInfoTime;
}

uint64_t PoiInfoManager::GetPoiInfoTime(const std::string& poiInfos)
{
    std::string key = "\"time\":";
    size_t pos = poiInfos.find(key);
    if (pos != std::string::npos) {
        pos += key.length();
        std::string number;
        for (; pos < poiInfos.length() && isdigit(poiInfos[pos]); ++pos) {
            number += poiInfos[pos];
        }
        if (CommonUtils::IsValidForStoull(number, MAX_UTC_TIME_SIZE)) {
            return std::stoull(number);
        }
    }
    return 0;
}

Poi PoiInfoManager::ParsePoiInfo(cJSON* poiJson)
{
    Poi poi;
    cJSON *item = cJSON_GetObjectItem(poiJson, "id");
    poi.id = (item && item->valuestring) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "confidence");
    poi.confidence = (item) ? item->valuedouble : 0.0;
    item = cJSON_GetObjectItem(poiJson, "name");
    poi.name = (item && item->valuestring) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "lat");
    poi.latitude = (item) ? item->valuedouble : 0.0;
    item = cJSON_GetObjectItem(poiJson, "lon");
    poi.longitude = (item) ? item->valuedouble : 0.0;
    item = cJSON_GetObjectItem(poiJson, "administrativeArea");
    poi.administrativeArea = (item && item->valuestring) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "subAdministrativeArea");
    poi.subAdministrativeArea = (item && item->valuestring) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "locality");
    poi.locality = (item && item->valuestring) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "subLocality");
    poi.subLocality = (item && item->valuestring) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "address");
    poi.address = (item && item->valuestring) ? item->valuestring : "";
    return poi;
}

PoiInfo PoiInfoManager::ParsePoiInfoFromStr(const std::string& jsonString)
{
    PoiInfo poiInfo;
    cJSON* cJsonObj = cJSON_Parse(jsonString.c_str());
    if (!cJsonObj) {
        LBSLOGI(REPORT_MANAGER, "Poi cJson Parse Failed");
        return poiInfo;
    }
    cJSON* item = cJSON_GetObjectItem(cJsonObj, "time");
    poiInfo.timestamp = (item) ? static_cast<uint64_t>(item->valuedouble) : 0;
    cJSON* poisArray = cJSON_GetObjectItem(cJsonObj, "pois");
    if (poisArray && cJSON_IsArray(poisArray)) {
        int arraySize = cJSON_GetArraySize(poisArray);
        if (arraySize > MAX_POI_ARRAY_SIZE) {
            arraySize = MAX_POI_ARRAY_SIZE;
        }
        for (int i = 0; i < arraySize; ++i) {
            cJSON* poiJson = cJSON_GetArrayItem(poisArray, i);
            if (poiJson) {
                poiInfo.poiArray.push_back(ParsePoiInfo(poiJson));
            }
        }
    }
    cJSON_Delete(cJsonObj);
    return poiInfo;
}

} // namespace OHOS
} // namespace Location