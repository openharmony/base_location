/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "work_record_statistic.h"
#include "common_utils.h"
#include "location_data_rdb_manager.h"

namespace OHOS {
namespace Location {
std::shared_ptr<WorkRecordStatistic> WorkRecordStatistic::instance_ = nullptr;
std::mutex WorkRecordStatistic::workRecordStatisticMutex_;

const uint32_t WORKING_STATE = 1;
const uint32_t NO_WORKING_STATE = 0;

std::shared_ptr<WorkRecordStatistic> WorkRecordStatistic::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(workRecordStatisticMutex_);
        if (instance_ == nullptr) {
            std::shared_ptr<WorkRecordStatistic> workRecordStatistic = std::make_shared<WorkRecordStatistic>();
            instance_ = workRecordStatistic;
        }
    }
    return instance_;
}

std::shared_ptr<WorkRecordStatistic> WorkRecordStatistic::DesTroyInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(workRecordStatisticMutex_);
        if (instance_ == nullptr) {
            std::shared_ptr<WorkRecordStatistic> workRecordStatistic = std::make_shared<WorkRecordStatistic>();
            instance_ = workRecordStatistic;
        }
    }
    return instance_;
}

bool WorkRecordStatistic::Update(std::string name, int requestNum)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (name == "CacheLocation") {
        workRecordStatisticMap_[name] = workRecordStatisticMap_[name] += requestNum;
    } else {
        workRecordStatisticMap_[name] = requestNum;
    }
    if (!UpdateLocationWorkingState()) {
        return false;
    }
    return true;
}

bool WorkRecordStatistic::UpdateLocationWorkingState()
{
    int requestNum = 0;
    for (const auto& pair : workRecordStatisticMap_) {
        requestNum += pair.second;
    }

    if (requestNum == 1 && location_working_state_ != WORKING_STATE) {
        if (!LocationDataRdbManager::SetLocationWorkingState(WORKING_STATE)) {
            return false;
        }
        location_working_state_ = WORKING_STATE;
    } else if (requestNum == 0 && location_working_state_ != NO_WORKING_STATE) {
        if (!LocationDataRdbManager::SetLocationWorkingState(NO_WORKING_STATE)) {
            return false;
        }
        location_working_state_ = NO_WORKING_STATE;
    }
    return true;
}
} // namespace Location
} // namespace OHOS