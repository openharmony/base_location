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
 
#ifndef LOCATION_HIAPPEVENT_H
#define LOCATION_HIAPPEVENT_H
 
#include <mutex>
#include <string>
#include <map>
 
namespace OHOS {
namespace Location {
const int32_t HA_CONFIG_TIMEOUT = 90;
const int32_t HA_CONFIG_ROW = 30;
const int32_t HA_NOT_SUPPORT_PROCESS_ID = -200;
const int32_t HA_REPORT_INTERVAL = 60 * 1000; // 1min

static const std::string KEY_KIT_REPORT_APPID = "kit_report_appId";

typedef struct {
    std::vector<int64_t> runTime;
    int64_t sumTime;
    int64_t succCount;
    std::vector<std::string> errCodeType;
    std::vector<int64_t> errCodeNum;
    std::map<int, int64_t> errCodes;
    int64_t beginTime;
    int64_t lastReportTime;
} HaEventInfo;
 
class LocationHiAppEvent {
public:
    LocationHiAppEvent();
    ~LocationHiAppEvent();
    static LocationHiAppEvent* GetInstance();
    void AddProcessor();
    void WriteEndEvent(const int64_t beginTime, const int result, const int errCode, const std::string& apiName);
    void CountEventTimeAndNum(const std::string apiName, const int64_t startTime, const int errCode);
private:
    bool Init();
    void WriteCallStatusEvent(const std::string apiName, HaEventInfo eventInfo);
    int64_t processorId_{-1};
    std::map<std::string, HaEventInfo> haEventInfoMap_;
    std::mutex processorIdMutex_;
    std::mutex haEventInfoMapMutex_;
};
 
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_HIAPPEVENT_H