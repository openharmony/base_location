/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#include "geofence_event_callback_test.h"

#include <cstdlib>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "permission_manager.h"

#include "geofence_event_callback.h"
#include "common_utils.h"
#include "constant_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 6;
const std::string MANAGER_SETTINGS = "ohos.permission.MANAGE_SETTINGS";

void GeofenceEventCallbackTest::SetUp()
{
    MockNativePermission();
}

void GeofenceEventCallbackTest::TearDown()
{}

void GeofenceEventCallbackTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        MANAGER_SETTINGS.c_str(), ACCESS_CONTROL_LOCATION_SWITCH.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "GeofenceEventCallbackTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(GeofenceEventCallbackTest, GeofenceEventCallbackConstructor001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceEventCallbackTest, GeofenceEventCallbackConstructor001, TestSize.Level0";
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackConstructor001 begin");
    sptr<GeofenceEventCallback> geofenceCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceCallback);
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackConstructor001 end");
}

HWTEST_F(GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceAvailability001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceAvailability001, TestSize.Level0";
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceAvailability001 begin");
    sptr<GeofenceEventCallback> geofenceCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceCallback);
    int32_t result = geofenceCallback->ReportGeofenceAvailability(true);
    EXPECT_EQ(ERR_OK, result);
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceAvailability001 end");
}

HWTEST_F(GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceAvailability002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceAvailability002, TestSize.Level0";
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceAvailability002 begin");
    sptr<GeofenceEventCallback> geofenceCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceCallback);
    int32_t result = geofenceCallback->ReportGeofenceAvailability(false);
    EXPECT_EQ(ERR_OK, result);
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceAvailability002 end");
}

HWTEST_F(GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceEvent001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceEvent001, TestSize.Level0";
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceEvent001 begin");
    sptr<GeofenceEventCallback> geofenceCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceCallback);
    HDI::Location::Geofence::V2_0::LocationInfo location;
    location.latitude = 1.0;
    location.longitude = 2.0;
    location.altitude = 1.0;
    location.horizontalAccuracy = 1.0;
    location.speed = 1.0;
    location.bearing = 1.0;
    GeofenceEvent event = GeofenceEvent::GEOFENCE_EVENT_ENTERED;
    int32_t fenceIndex = 0;
    int64_t timestamp = 1000000000;
    int32_t result = geofenceCallback->ReportGeofenceEvent(fenceIndex, location, event, timestamp);
    EXPECT_EQ(ERR_OK, result);
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceEvent001 end");
}

HWTEST_F(GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceEvent002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceEvent002, TestSize.Level0";
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceEvent002 begin");
    sptr<GeofenceEventCallback> geofenceCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceCallback);
    HDI::Location::Geofence::V2_0::LocationInfo location;
    location.latitude = 45.0;
    location.longitude = 90.0;
    location.altitude = 100.0;
    location.horizontalAccuracy = 5.0;
    location.speed = 10.0;
    location.bearing = 180.0;
    GeofenceEvent event = GeofenceEvent::GEOFENCE_EVENT_EXITED;
    int32_t fenceIndex = 1;
    int64_t timestamp = 2000000000;
    int32_t result = geofenceCallback->ReportGeofenceEvent(fenceIndex, location, event, timestamp);
    EXPECT_EQ(ERR_OK, result);
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceEvent002 end");
}

HWTEST_F(GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceOperateResult001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceOperateResult001, TestSize.Level0";
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceOperateResult001 begin");
    sptr<GeofenceEventCallback> geofenceCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceCallback);
    GeofenceOperateType type = TYPE_ADD;
    GeofenceOperateResult result = GEOFENCE_OPERATION_SUCCESS;
    int32_t fenceIndex = 0;
    int32_t ret = geofenceCallback->ReportGeofenceOperateResult(fenceIndex, type, result);
    EXPECT_EQ(ERR_OK, ret);
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceOperateResult001 end");
}

HWTEST_F(GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceOperateResult002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceEventCallbackTest, GeofenceEventCallbackReportGeofenceOperateResult002, TestSize.Level0";
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceOperateResult002 begin");
    sptr<GeofenceEventCallback> geofenceCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceCallback);
    GeofenceOperateType type = TYPE_DELETE;
    GeofenceOperateResult result = GEOFENCE_OPERATION_SUCCESS;
    int32_t fenceIndex = 1;
    int32_t ret = geofenceCallback->ReportGeofenceOperateResult(fenceIndex, type, result);
    EXPECT_EQ(ERR_OK, ret);
    LBSLOGI(GNSS, "[GeofenceEventCallbackTest] GeofenceEventCallbackReportGeofenceOperateResult002 end");
}

}  // namespace Location
}  // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT