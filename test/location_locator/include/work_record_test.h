/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef WORK_RECORD_TEST_H
#define WORK_RECORD_TEST_H

#include <gtest/gtest.h>

#include "work_record.h"
#include "work_record_statistic.h"

namespace OHOS {
namespace Location {
class WorkRecordTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    void VerifyMarshalling(std::unique_ptr<WorkRecord>& workrecord);
};
} // namespace Location
} // namespace OHOS
#endif // WORK_RECORD_TEST_H