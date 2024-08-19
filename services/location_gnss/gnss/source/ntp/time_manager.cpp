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
#include "time_manager.h"

#include <string>
#include "common_utils.h"
#include "elapsed_real_time_check.h"
#include "location_log.h"
#include "time_service_client.h"

namespace OHOS {
namespace Location {
const int64_t INVALID_TIME = 0;
const int64_t GPS_UTC_REFERENCE_TIME = 946656000;  // 946656000:UTC time of 2000-01-01 00:00:00

int64_t TimeManager::GetCurrentTime()
{
    LBSLOGI(GNSS, "beginning timestamp_ is %{public}s", std::to_string(timestamp_).c_str());
    if (timestamp_ < GPS_UTC_REFERENCE_TIME) {
        return INVALID_TIME;
    }
    if (!ElapsedRealTimeCheck::GetInstance()->CanTrustElapsedRealTime()) {
        LBSLOGI(GNSS, "getCurrentTime ElapsedRealTime INVALID_TIME");
        return INVALID_TIME;
    }

    int64_t timeTillNow = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs() - timeSinceBoot_;
    if (timeTillNow >= expireTime_) {
        LBSLOGI(GNSS, "getCurrentTime INVALID_TIME");
        return INVALID_TIME;
    } else {
        LBSLOGI(GNSS, "getCurrentTime:%{public}s", std::to_string(timestamp_ + timeTillNow).c_str());
        return timestamp_ + timeTillNow;
    }
}

void TimeManager::SetCurrentTime(int64_t msTime, int64_t msTimeSynsBoot)
{
    timestamp_ = msTime;
    timeSinceBoot_ = msTimeSynsBoot;
    LBSLOGI(GNSS, "setCurrentTime timestamp_:%{public}s,  mTimeReference:%{public}s",
        std::to_string(timestamp_).c_str(), std::to_string(timeSinceBoot_).c_str());
}

int64_t TimeManager::GetTimestamp()
{
    return timestamp_;
}

}  // namespace Location
}  // namespace OHOS
#endif  // TIME_SERVICE_ENABLE
#endif
