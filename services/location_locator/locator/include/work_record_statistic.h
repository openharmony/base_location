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

#ifndef WORK_RECORD_STATISTIC_H
#define WORK_RECORD_STATISTIC_H

#include <mutex>
#include <string>
#include <map>

namespace OHOS {
namespace Location {
class WorkRecordStatistic {
public:
    static std::shared_ptr<WorkRecordStatistic> GetInstance();
    bool Update(std::string name, int requestNum);
private:
    bool UpdateLocationWorkingState();
    std::map<std::string, int> workRecordStatisticMap_;
    static std::shared_ptr<WorkRecordStatistic> instance_;
    int location_working_state_ = 0;
    std::mutex mutex_;
    static std::mutex workRecordStatisticMutex_;
};
} // namespace Location
} // namespace OHOS
#endif // WORK_RECORD_STATISTIC_H
