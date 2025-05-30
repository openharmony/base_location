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

#ifndef POI_INFO_MANAGER
#define POI_INFO_MANAGER

#include <mutex>
#include <singleton.h>
#include <string>
#include <time.h>
#include <cJSON.h>

#include "location.h"

namespace OHOS {
namespace Location {

typedef struct StrPoiInfoStruct {
    uint64_t poiInfosTime = 0;
    std::string latestPoiInfos = "";
} StrPoiInfoStruct;

class PoiInfoManager {
public:
    PoiInfoManager();
    ~PoiInfoManager();
    static PoiInfoManager* GetInstance();
    void UpdateCachedPoiInfo(const std::unique_ptr<Location>& location);
    void ClearPoiInfos(const std::unique_ptr<Location>& finalLocation);
    void UpdateLocationPoiInfo(const std::unique_ptr<Location>& finalLocation);

    std::string GetLatestPoiInfo();
    void SetLatestPoiInfo(std::string poiInfo);
    uint64_t GetLatestPoiInfoTime();
    void SetLatestPoiInfoTime(uint64_t poiInfoTime);

    uint64_t GetPoiInfoTime(const std::string& poiInfos);
    bool IsPoiInfoValid(std::string poiInfos, uint64_t poiInfoTime);
    void AddCachedPoiInfoToLocation(const std::unique_ptr<Location>& finalLocation);
    Poi ParsePoiInfo(cJSON* poiJson);
    PoiInfo ParsePoiInfoFromStr(const std::string& jsonString);

private:
    std::mutex latestPoiInfoMutex_;
    StrPoiInfoStruct latestPoiInfoStruct_;
};
} // namespace OHOS
} // namespace Location
#endif // POI_INFO_MANAGER