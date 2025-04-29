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

PoiInfoManager* PoiInfoManager::GetInstance()
{
    static PoiInfoManager data;
    return &data;
}

PoiInfoManager::PoiInfoManager() {}

PoiInfoManager::~PoiInfoManager() {}

void PoiInfoManager::UpdatePoiInfo(const std::unique_ptr<Location>& location)
{
    std::map<std::string, std::string> additionMap = location->GetAdditionsMap();
    if (additionMap["poiInfos"] != "") {
        LBSLOGI(REPORT_MANAGER, "Update PoiInfos");
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

void PoiInfoManager::PoiInfoReportCheck(const std::unique_ptr<Location>& finalLocation)
{
    LBSLOGI(REPORT_MANAGER, "PoiInfos Report Check");
    std::map<std::string, std::string> additionMap = finalLocation->GetAdditionsMap();
    std::string poiInfos = additionMap["poiInfos"];
    uint64_t poiInfoTime = GetPoiInfoTime(poiInfos);
    if (poiInfos != "") {
        if (IsPoiInfoValid(poiInfos, poiInfoTime)) {
            return;
        } else {
            LBSLOGI(REPORT_MANAGER, "PoiInfos Expied");
            ClearPoiInfos(finalLocation);
        }
    }
    std::string latestPoiInfos = GetLatestPoiInfo();
    uint64_t latestpoiInfoTime = GetLatestPoiInfoTime();
    if (!IsPoiInfoValid(latestPoiInfos, latestpoiInfoTime)) {
        std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
        LBSLOGI(REPORT_MANAGER, "Latest PoiInfos Expied");
        latestPoiInfoStruct_ = {0};
        return;
    }
    AddPoiInfo(finalLocation);
}

void PoiInfoManager::AddPoiInfo(const std::unique_ptr<Location>& finalLocation)
{
    LBSLOGI(REPORT_MANAGER, "Add Latest PoiInfos");
    std::string poiInfos = GetLatestPoiInfo();
    std::vector<std::string> addition = finalLocation->GetAdditions();
    addition.push_back(poiInfos);
    finalLocation->SetAdditions(addition, false);
    finalLocation->SetAdditionSize(finalLocation->GetAdditions().size());
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
    if ((isInStillState && poiInfoTime + POI_EXPIRED_TIME < enterStillTime)
            || (isInStillState && curTimeStamp - poiInfoTime >= STILL_POI_EXPIRED_TIME)) {
        LBSLOGI(REPORT_MANAGER, "Clear PoiInfos When Still");
        return false;
    }
    if (!isInStillState && curTimeStamp - poiInfoTime >= POI_EXPIRED_TIME) {
        LBSLOGI(REPORT_MANAGER, "Clear PoiInfos When Move");
        return false;
    }
    return true;
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

} // namespace OHOS
} // namespace Location