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
#include "elapsed_real_time_check.h"

#include <cmath>
#include <string>

#include "common_utils.h"
#include "location_log.h"
#include "time_service_client.h"

namespace OHOS {
namespace Location {
const int64_t MAX_MISS_MS = 24 * 1000;  // we only allow 24s misstake

ElapsedRealTimeCheck *ElapsedRealTimeCheck::GetInstance()
{
    static ElapsedRealTimeCheck data;
    return &data;
}

ElapsedRealTimeCheck::ElapsedRealTimeCheck() {}

ElapsedRealTimeCheck::~ElapsedRealTimeCheck() {}

void ElapsedRealTimeCheck::CheckRealTime(int64_t time)
{
    if (timeBegin_ == 0) {
        timeBegin_ = time;
        timeBeginElapsed_ = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    } else if (timeCheck_ == 0) {
        timeCheck_ = time;
        timeCheckElapsed_ = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    } else {
        LBSLOGD(GNSS, "checkRealTime");
    }

    if ((timeBegin_ != 0) && (timeCheck_ != 0)) {
        int64_t missTime = (timeCheck_ - timeBegin_) - (timeCheckElapsed_ - timeBeginElapsed_);
        LBSLOGD(GNSS,
            "checkRealTime timeBegin_:%{public}s, timeCheck_:%{public}s, timeBeginElapsed_:%{public}s, "
            "timeCheckElapsed_:%{public}s",
            std::to_string(timeBegin_).c_str(), std::to_string(timeCheck_).c_str(),
            std::to_string(timeBeginElapsed_).c_str(), std::to_string(timeCheckElapsed_).c_str());
        LBSLOGI(GNSS, "checkRealTime missTime:%{public}s", std::to_string(missTime).c_str());
        // we found missTime usually should not grow, results should be simillar on any moment.
        // so if very high missTime should be problem.
        if (std::abs(missTime) >= MAX_MISS_MS) {
            LBSLOGI(GNSS, "checkRealTime false");
            canTrustElapsedRealTime_ = false;
        } else {
            // sometimes just once system communication block, this could recover automaticly,
            // elapsedRealtime still be ok.
            canTrustElapsedRealTime_ = true;
        }
        timeCheck_ = 0;
        timeCheckElapsed_ = 0;
    }
}

bool ElapsedRealTimeCheck::CanTrustElapsedRealTime()
{
    return canTrustElapsedRealTime_;
}

}  // namespace Location
}  // namespace OHOS
#endif  // TIME_SERVICE_ENABLE
#endif
