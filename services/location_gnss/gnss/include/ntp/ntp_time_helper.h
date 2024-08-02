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

#ifndef NTP_TIME_HELPER_H
#define NTP_TIME_HELPER_H
#ifdef FEATURE_GNSS_SUPPORT
#ifdef TIME_SERVICE_ENABLE

#include <string>
#include "time_manager.h"

namespace OHOS {
namespace Location {

struct NtpTrustedTime {
    int64_t ntpTime = 0;
    int64_t ageMillis = 0;
    int64_t elapsedRealTime = 0;
};

class NtpTimeHelper {
public:
    static NtpTimeHelper* GetInstance();
    void RetrieveAndInjectNtpTime();

private:
    NtpTimeHelper() = default;
    ~NtpTimeHelper() = default;

    NtpTrustedTime ntpResult_;
};
} // namespace Location
} // namespace OHOS

#endif // TIME_SERVICE_ENABLE
#endif // FEATURE_GNSS_SUPPORT
#endif // NTP_TIME_HELPER_H
