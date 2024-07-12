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

#ifndef GPS_TIME_MANAGER_H
#define GPS_TIME_MANAGER_H
#ifdef FEATURE_GNSS_SUPPORT
#ifdef TIME_SERVICE_ENABLE

#include "time_manager.h"

namespace OHOS {
namespace Location {
class GpsTimeManager {
public:
    GpsTimeManager();
    ~GpsTimeManager();
    int64_t GetGpsTime();
    void SetGpsTime(int64_t gpsTime, int64_t bootTimeMs);

private:
    bool CheckValid(int64_t gpsTime, int64_t lastSystemTime);
    TimeManager timeManager_;
    bool validFlag_ {false};
    int64_t lastSystemTime_;
};
} // namespace Location
} // namespace OHOS

#endif // TIME_SERVICE_ENABLE
#endif // FEATURE_GNSS_SUPPORT
#endif // GPS_TIME_MANAGER_H
