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
#include "ntp_time_helper.h"

#include <string>
#include "gnss_ability.h"
#include "location_log.h"
#include "ntp_time_check.h"
#include "time_service_client.h"

namespace OHOS {
namespace Location {
static const int64_t NTP_INTERVAL = 24 * 60 * 60 * 1000;

NtpTimeHelper *NtpTimeHelper::GetInstance()
{
    static NtpTimeHelper data;
    return &data;
}

void NtpTimeHelper::RetrieveAndInjectNtpTime()
{
    if (ntpResult_.elapsedRealTime > 0) {
        ntpResult_.ageMillis =
            MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs() - ntpResult_.elapsedRealTime;
        LBSLOGD(GNSS, "ntp age millis %{public}s", std::to_string(ntpResult_.ageMillis).c_str());
    }

    auto gnssAbility = GnssAbility::GetInstance();
    if (ntpResult_.ntpTime == 0 || ntpResult_.ageMillis > NTP_INTERVAL) {
        auto ret = MiscServices::TimeServiceClient::GetInstance()->GetNtpTimeMs(ntpResult_.ntpTime);
        if (ret != ERR_OK) {
            ntpResult_.ntpTime = 0;
            LBSLOGE(GNSS, "failed to get ntp time %{public}d", ret);
            return;
        }
        LBSLOGI(GNSS, "get ntp time %{public}s", std::to_string(ntpResult_.ntpTime).c_str());
        ntpResult_.elapsedRealTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
        ntpResult_.ageMillis = 0;
        if (gnssAbility != nullptr) {
            gnssAbility->UpdateNtpTime(ntpResult_.ntpTime, ntpResult_.elapsedRealTime);
        }
    }
    if (gnssAbility != nullptr) {
        gnssAbility->InjectTime();
    }
}

}  // namespace Location
}  // namespace OHOS
#endif // TIME_SERVICE_ENABLE
#endif
