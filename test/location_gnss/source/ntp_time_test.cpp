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
#define private public
#include "ntp_time_check.h"
#include "time_manager.h"
#include "gps_time_manager.h"
#include "ntp_time_helper.h"
#include "net_conn_observer.h"
#undef private

#ifdef NET_MANAGER_ENABLE
#include "net_conn_observer.h"
#include "net_conn_client.h"
#include "net_specifier.h"
#endif

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

HWTEST_F(NtpTimeTest, NtpTimeCheckCheckNtpTimeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, NtpTimeCheckCheckNtpTimeTest002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckCheckNtpTimeTest002 begin");
    auto ntpTimeCheck = NtpTimeCheck::GetInstance();
    EXPECT_NE(nullptr, ntpTimeCheck);
    int64_t ntpMsTime = 1720522730000;
    int64_t msTimeSynsBoot = 1;
    ntpTimeCheck->CheckNtpTime(ntpMsTime, msTimeSynsBoot);
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckCheckNtpTimeTest002 end");
}

HWTEST_F(NtpTimeTest, NtpTimeCheckGetUncertaintyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, NtpTimeCheckGetUncertaintyTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckGetUncertaintyTest001 begin");
    auto ntpTimeCheck = NtpTimeCheck::GetInstance();
    EXPECT_NE(nullptr, ntpTimeCheck);
    ntpTimeCheck->GetUncertainty();
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NtpTimeCheckGetUncertaintyTest001 end");
}

HWTEST_F(NtpTimeTest, TimeManagerGetTimestampTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, TimeManagerGetTimestampTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] TimeManagerGetTimestampTest001 begin");
    TimeManager timeManager;
    int64_t timeStamp = timeManager.GetTimestamp();
    EXPECT_EQ(0, timeStamp);
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] TimeManagerGetTimestampTest001 end");
}

HWTEST_F(NtpTimeTest, GpsTimeManagerCheckValidTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, GpsTimeManagerCheckValidTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] GpsTimeManagerCheckValidTest001 begin");
    GpsTimeManager gpsTimeManager;
    gpsTimeManager.CheckValid(0, 0);
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] GpsTimeManagerCheckValidTest001 end");
}

HWTEST_F(NtpTimeTest, RetrieveAndInjectNtpTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, RetrieveAndInjectNtpTimeTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] RetrieveAndInjectNtpTimeTest001 begin");
    auto npTimeHelper = NtpTimeHelper::GetInstance();
    EXPECT_NE(nullptr, npTimeHelper);
    npTimeHelper->RetrieveAndInjectNtpTime();
    sleep(1);
    npTimeHelper->RetrieveAndInjectNtpTime();
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] RetrieveAndInjectNtpTimeTest001 end");
}

#ifdef NET_MANAGER_ENABLE
HWTEST_F(NtpTimeTest, NetConnObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NtpTimeTest, NetConnObserverTest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NetConnObserverTest001 begin");
    sptr<NetConnObserver> netWorkObserver = sptr<NetConnObserver>((new (std::nothrow) NetConnObserver()));
    EXPECT_NE(nullptr, netWorkObserver);
    sptr<NetManagerStandard::NetHandle> handle = nullptr;
    netWorkObserver->NetAvailable(handle);
    sptr<NetManagerStandard::NetAllCapabilities> netAllCap = nullptr;
    netWorkObserver->NetCapabilitiesChange(handle, netAllCap);
    sptr<NetManagerStandard::NetLinkInfo> info = nullptr;
    netWorkObserver->NetConnectionPropertiesChange(handle, info);
    netWorkObserver->NetLost(handle);
    netWorkObserver->NetUnavailable();
    netWorkObserver->NetBlockStatusChange(handle, false);
    LBSLOGI(GNSS_TEST, "[NtpTimeTest] NetConnObserverTest001 end");
}
#endif

}
}