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

#ifdef FEATURE_GNSS_SUPPORT
#ifdef TIME_SERVICE_ENABLE

#include "gps_time_manager.h"

#include "elapsed_real_time_check.h"
#include "location_log.h"
#include "time_service_client.h"

namespace OHOS {
namespace Location {
const int64_t INVALID_TIME = 0L;
const int64_t MAX_MISSTAKE_TIME = 10 * 1000L;

GpsTimeManager::GpsTimeManager()
{
    timeManager_ = TimeManager(EXPIRT_TIME);
}

GpsTimeManager::~GpsTimeManager() {}

bool GpsTimeManager::CheckValid(int64_t gpsTime, int64_t lastSystemTime)
{
    int64_t bootTimeDiff = gpsTime - timeManager_.GetTimestamp();
    int64_t systemTimeDiff = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs() - lastSystemTime;
    if (abs(bootTimeDiff - systemTimeDiff) > MAX_MISSTAKE_TIME) {
        return false;
    }
    return true;
}

int64_t GpsTimeManager::GetGpsTime()
{
    int64_t tmpTime = timeManager_.GetCurrentTime();
    if ((!validFlag_) && (tmpTime > 0)) {
        if (!CheckValid(tmpTime, lastSystemTime_)) {
            return INVALID_TIME;
        }
    }
    return tmpTime;
}

void GpsTimeManager::SetGpsTime(int64_t gpsMsTime, int64_t bootTimeMs)
{
    validFlag_ = true;
    lastSystemTime_ = MiscServices::TimeServiceClient::GetInstance()->GetWallTimeMs();
    timeManager_.SetCurrentTime(gpsMsTime, bootTimeMs);
    auto elapsedRealTimeCheck = ElapsedRealTimeCheck::GetInstance();
    if (elapsedRealTimeCheck != nullptr) {
        elapsedRealTimeCheck->CheckRealTime(gpsMsTime);
    }
}

}  // namespace Location
}  // namespace OHOS
#endif // TIME_SERVICE_ENABLE
#endif
