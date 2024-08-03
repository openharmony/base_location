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

#ifndef GPS_NTP_TIME_CHECK_H
#define GPS_NTP_TIME_CHECK_H
#ifdef FEATURE_GNSS_SUPPORT
#ifdef TIME_SERVICE_ENABLE

#include "gps_time_manager.h"

namespace OHOS {
namespace Location {
const int64_t DEFAULT_UNCERTAINTY = 30;
class NtpTimeCheck {
public:
    static NtpTimeCheck* GetInstance();
    bool CheckNtpTime(int64_t ntpMsTime, int64_t msTimeSynsBoot);
    int GetUncertainty();
    void SetGpsTime(int64_t gpsMsTime, int64_t bootTimeMs);

private:
    NtpTimeCheck();
    ~NtpTimeCheck();
    bool CompareTime(int64_t currentNtpTime, int64_t compareTime);
    GpsTimeManager gpsTimeManager_;
    int difference_ = DEFAULT_UNCERTAINTY;
};
} // namespace Location
} // namespace OHOS

#endif // TIME_SERVICE_ENABLE
#endif // FEATURE_GNSS_SUPPORT
#endif // GPS_NTP_TIME_CHECK_H
