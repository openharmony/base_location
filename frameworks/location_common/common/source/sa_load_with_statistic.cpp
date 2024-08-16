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

#include "location_sa_load_manager.h"

#include "common_utils.h"
#include "common_hisysevent.h"
#include "location_log_event_ids.h"

namespace OHOS {
namespace Location {

SaLoadWithStatistic::SaLoadWithStatistic()
{
}

SaLoadWithStatistic::~SaLoadWithStatistic()
{
}

bool SaLoadWithStatistic::InitLocationSa(int32_t systemAbilityId)
{
    auto startTime = CommonUtils::GetCurrentTimeStamp();
    bool ret = LocationSaLoadManager::InitLocationSa(systemAbilityId);
    auto endTime = CommonUtils::GetCurrentTimeStamp();
    WriteLocationInnerEvent(SA_LOAD, {"saId", std::to_string(systemAbilityId), "type", "load",
        "ret", std::to_string(ret), "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
    return ret;
}

bool SaLoadWithStatistic::UnInitLocationSa(int32_t systemAbilityId)
{
    auto startTime = CommonUtils::GetCurrentTimeStamp();
    bool ret = LocationSaLoadManager::UnInitLocationSa(systemAbilityId);
    auto endTime = CommonUtils::GetCurrentTimeStamp();
    WriteLocationInnerEvent(SA_LOAD, {"saId", std::to_string(systemAbilityId), "type", "unload",
        "ret", std::to_string(ret), "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
    return ret;
}
}; // namespace Location
}; // namespace OHOS
