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
#include "ntp_time_check.h"

#include <cmath>
#include <string>

#include "common_utils.h"
#include "location_log.h"
#include "time_service_client.h"

namespace OHOS {
namespace Location {
const int64_t INVALID_TIME = 0;
const int64_t MISSTAKE_TIME = 50 * 1000;  // 50s

NtpTimeCheck::NtpTimeCheck() {}

NtpTimeCheck::~NtpTimeCheck() {}

NtpTimeCheck *NtpTimeCheck::GetInstance()
{
    static NtpTimeCheck data;
    return &data;
}

bool NtpTimeCheck::CheckNtpTime(int64_t ntpMsTime, int64_t msTimeSynsBoot)
{
    int64_t currentNtpTime =
        ntpMsTime + MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs() - msTimeSynsBoot;
    if (gpsTimeManager_.GetGpsTime() != INVALID_TIME) {
        return CompareTime(currentNtpTime, gpsTimeManager_.GetGpsTime());
    }

    LBSLOGI(GNSS, "checkNtpTime return false");
    return false;
}

bool NtpTimeCheck::CompareTime(int64_t currentNtpTime, int64_t compareTime)
{
    LBSLOGI(GNSS, "compareTime currentNtpTime:%{public}s, compareTime:%{public}s",
        std::to_string(currentNtpTime).c_str(), std::to_string(compareTime).c_str());
    int64_t misstake = std::abs(currentNtpTime - compareTime);
    if (misstake > MISSTAKE_TIME) {
        LBSLOGI(GNSS, "find error ntp time:%{public}s", std::to_string(misstake).c_str());
        return false;
    } else {
        difference_ = misstake;
        return true;
    }
}

void NtpTimeCheck::SetGpsTime(int64_t gpsMsTime, int64_t bootTimeMs)
{
    if (gpsMsTime <= 0 || bootTimeMs <= 0) {
        LBSLOGE(GNSS, "set gps time failed :%{public}s, %{public}s", std::to_string(gpsMsTime).c_str(),
            std::to_string(bootTimeMs).c_str());
        return;
    }
    gpsTimeManager_.SetGpsTime(gpsMsTime, bootTimeMs);
}

int NtpTimeCheck::GetUncertainty()
{
    return (difference_ > DEFAULT_UNCERTAINTY) ? difference_ : DEFAULT_UNCERTAINTY;
}

}  // namespace Location
}  // namespace OHOS
#endif  // TIME_SERVICE_ENABLE
#endif
