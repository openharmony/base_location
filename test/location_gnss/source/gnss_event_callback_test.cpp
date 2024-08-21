/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "gnss_event_callback_test.h"

#include <cstdlib>

#include "gnss_event_callback.h"
#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "agnss_ni_manager.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "location.h"
#include "permission_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
using HDI::Location::Gnss::V2_0::IGnssCallback;
using HDI::Location::Gnss::V2_0::LocationInfo;
using HDI::Location::Gnss::V2_0::ConstellationCategory;
const int32_t LOCATION_PERM_NUM = 5;
const std::string MANAGER_SETTINGS = "ohos.permission.MANAGE_SETTINGS";
void GnssEventCallbackTest::SetUp()
{
    MockNativePermission();
}

void GnssEventCallbackTest::TearDown()
{}

void GnssEventCallbackTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        MANAGER_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "GnssEventCallbackTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportLocation001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportLocation001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    gnssCallback->ReportLocation(locationInfo);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportLocation001 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportGnssWorkingStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportGnssWorkingStatus001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssWorkingStatus001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssWorkingStatus status = HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_NONE;
    gnssCallback->ReportGnssWorkingStatus(status);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssWorkingStatus001 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportGnssWorkingStatus002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportGnssWorkingStatus002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssWorkingStatus002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssWorkingStatus status = HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_NONE;
    gnssCallback->ReportGnssWorkingStatus(status);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssWorkingStatus002 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportNmea001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportNmea001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportNmea001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    gnssCallback->ReportNmea(0, "nmea", 0);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportNmea001 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportNmea002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportNmea002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportNmea002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    gnssCallback->ReportNmea(0, "nmea", 0);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportNmea002 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportGnssCapabilities001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportGnssCapabilities001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssCapabilities001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssCapabilities capabilities = HDI::Location::Gnss::V2_0::GNSS_CAP_SUPPORT_MSB;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportGnssCapabilities(capabilities));
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssCapabilities001 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportGnssCapabilities002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportGnssCapabilities002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssCapabilities002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssCapabilities capabilities = HDI::Location::Gnss::V2_0::GNSS_CAP_SUPPORT_MSB;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportGnssCapabilities(capabilities));
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssCapabilities002 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportSatelliteStatusInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportSatelliteStatusInfo002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportSatelliteStatusInfo002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    SatelliteStatusInfo statusInfo;
    statusInfo.satellitesNumber = 0;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportSatelliteStatusInfo(statusInfo));
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportSatelliteStatusInfo002 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportSatelliteStatusInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportSatelliteStatusInfo003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportSatelliteStatusInfo003 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    SatelliteStatusInfo statusInfo;
    statusInfo.satellitesNumber = 1;
    statusInfo.elevation.push_back(12);
    statusInfo.azimuths.push_back(30);
    statusInfo.carrierFrequencies.push_back(40);
    statusInfo.carrierToNoiseDensitys.push_back(40);
    statusInfo.satelliteIds.push_back(1);
    statusInfo.constellation.push_back(static_cast<ConstellationCategory>(1));
    statusInfo.additionalInfo.push_back(
        HDI::Location::Gnss::V2_0::SATELLITES_ADDITIONAL_INFO_EPHEMERIS_DATA_EXIST);
    EXPECT_EQ(ERR_OK, gnssCallback->ReportSatelliteStatusInfo(statusInfo));
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportLocation(locationInfo));
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportSatelliteStatusInfo003 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackRequestGnssReferenceInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackRequestGnssReferenceInfo001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestGnssReferenceInfo001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssRefInfoType type = HDI::Location::Gnss::V2_0::GNSS_REF_INFO_TIME;
    gnssCallback->RequestGnssReferenceInfo(type);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestGnssReferenceInfo001 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackRequestPredictGnssData001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackRequestPredictGnssData001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestPredictGnssData001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    gnssCallback->RequestPredictGnssData();
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestPredictGnssData001 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportCachedLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportCachedLocation001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportCachedLocation001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    std::vector<LocationInfo> gnssLocations;
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    gnssLocations.push_back(locationInfo);
    gnssCallback->ReportCachedLocation(gnssLocations);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportCachedLocation001 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackReportGnssNiNotification003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackReportGnssNiNotification003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssNiNotification003 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssNiNotificationRequest notif;
    notif.notificationCategory = GNSS_NI_NOTIFICATION_REQUIRE_VERIFY | GNSS_NI_NOTIFICATION_REQUIRE_NOTIFY;
    gnssCallback->ReportGnssNiNotification(notif);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackReportGnssNiNotification003 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackRequestGnssReferenceInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackRequestGnssReferenceInfo002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestGnssReferenceInfo002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssRefInfoType type = HDI::Location::Gnss::V2_0::GNSS_REF_INFO_LOCATION;
    gnssCallback->RequestGnssReferenceInfo(type);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestGnssReferenceInfo002 end");
}

HWTEST_F(GnssEventCallbackTest, GnssEventCallbackRequestGnssReferenceInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssEventCallbackTest, GnssEventCallbackRequestGnssReferenceInfo003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestGnssReferenceInfo003 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssRefInfoType type = HDI::Location::Gnss::V2_0::GNSS_REF_INFO_BEST_LOCATION;
    gnssCallback->RequestGnssReferenceInfo(type);
    LBSLOGI(GNSS_TEST, "[GnssEventCallbackTest] GnssEventCallbackRequestGnssReferenceInfo003 end");
}

}  // namespace Location
}  // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
