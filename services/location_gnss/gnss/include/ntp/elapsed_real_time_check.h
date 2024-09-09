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

#ifndef ELAPSED_REAL_TIME_CHECK_H
#define ELAPSED_REAL_TIME_CHECK_H
#ifdef FEATURE_GNSS_SUPPORT
#ifdef TIME_SERVICE_ENABLE

#include <cstdint>

namespace OHOS {
namespace Location {
class ElapsedRealTimeCheck {
public:
    static ElapsedRealTimeCheck* GetInstance();
    void CheckRealTime(int64_t time);
    bool CanTrustElapsedRealTime();

private:
    ElapsedRealTimeCheck();
    ~ElapsedRealTimeCheck();

    int64_t timeBegin_ = 0;
    int64_t timeBeginElapsed_ = 0;
    int64_t timeCheck_ = 0;
    int64_t timeCheckElapsed_ = 0;
    bool canTrustElapsedRealTime_ = true; // default is true, usually we trust this time.
};
} // namespace Location
} // namespace OHOS

#endif // TIME_SERVICE_ENABLE
#endif // FEATURE_GNSS_SUPPORT
#endif // ELAPSED_REAL_TIME_CHECK_H
