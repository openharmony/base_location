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

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H
#ifdef FEATURE_GNSS_SUPPORT
#ifdef TIME_SERVICE_ENABLE

#include <string>

namespace OHOS {
namespace Location {
const int64_t EXPIRT_TIME = 14 * 24 * 60 * 60 * 1000L;  // 14 day, two week

class TimeManager {
public:
    TimeManager() = default;
    explicit TimeManager(int64_t expireTime) : expireTime_(expireTime) {}
    ~TimeManager() = default;
    int64_t GetCurrentTime();
    void SetCurrentTime(int64_t msTime, int64_t msTimeSynsBoot);
    int64_t GetTimestamp();

private:
    int64_t timestamp_ = 0;
    int64_t timeSynsBoot_ = 0;
    int64_t expireTime_ = EXPIRT_TIME;
};
} // namespace Location
} // namespace OHOS

#endif // TIME_SERVICE_ENABLE
#endif // FEATURE_GNSS_SUPPORT
#endif // TIME_MANAGER_H
