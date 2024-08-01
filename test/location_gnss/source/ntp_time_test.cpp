/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ntp_time_test.h"

#include "location_log.h"
#include "ntp_time_check.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void NtpTimeTest::SetUp()
{
}

void NtpTimeTest::TearDown()
{
}

HWTEST_F(NtpTimeTest, NtpTimeCheckSetGpsTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, NtpTimeCheckSetGpsTimeTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckSetGpsTimeTest001 begin");
    auto ntpTimeCheck = NtpTimeCheck::GetInstance();
    EXPECT_NE(nullptr, ntpTimeCheck);
    int64_t gpsMsTime = -1;
    int64_t bootTimeMs = -1;
    ntpTimeCheck->SetGpsTime(gpsMsTime, bootTimeMs);
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckSetGpsTimeTest001 end");
}

HWTEST_F(NtpTimeTest, NtpTimeCheckCheckNtpTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, NtpTimeCheckCheckNtpTimeTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckCheckNtpTimeTest001 begin");
    auto ntpTimeCheck = NtpTimeCheck::GetInstance();
    EXPECT_NE(nullptr, ntpTimeCheck);
    int64_t gpsMsTime = 1720522730000;
    int64_t bootTimeMs = 1;
    ntpTimeCheck->SetGpsTime(gpsMsTime, bootTimeMs);
    int64_t ntpMsTime = 1720522730000;
    int64_t msTimeSynsBoot = 1;
    ntpTimeCheck->CheckNtpTime(ntpMsTime, msTimeSynsBoot);
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckCheckNtpTimeTest001 end");
}

}
}