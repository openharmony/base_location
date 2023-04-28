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

#include "location_approximately_permission_test.h"

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_APPROXIMATELY_PERM = 3;
void LocationApproximatelyPermissionTest::SetUp()
{
    MockNativePermission();
    reportManager_ = DelayedSingleton<ReportManager>::GetInstance();
    EXPECT_NE(nullptr, reportManager_);
}

void LocationApproximatelyPermissionTest::TearDown()
{
    reportManager_ = nullptr;
    DelayedSingleton<ReportManager>::DestroyInstance();
}

void LocationApproximatelyPermissionTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_APPROXIMATELY_LOCATION.c_str(), ACCESS_BACKGROUND_LOCATION.c_str(),
        MANAGE_SECURE_SETTINGS.c_str()
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_APPROXIMATELY_PERM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorApproximatePermissionTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocationApproximatelyPermissionTest, ReportManagerApproximatelyLocationTest001, TestSize.Level1)
{
    EXPECT_EQ(nullptr, reportManager_->GetPermittedLocation(tokenId_, 0, nullptr));
    MessageParcel parcel;
    parcel.WriteDouble(MAX_LATITUDE + 1.0);  // latitude is out of range
    parcel.WriteDouble(MAX_LONGITUDE + 1.0);  // longitude is out of range
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    auto newLocation = reportManager_->GetPermittedLocation(tokenId_, 0, location);
    EXPECT_NE(nullptr, newLocation);
    EXPECT_EQ(MAX_LATITUDE, newLocation->GetLatitude());
    EXPECT_EQ(MAX_LONGITUDE, newLocation->GetLongitude());
    EXPECT_EQ(DEFAULT_APPROXIMATELY_ACCURACY, newLocation->GetAccuracy());
}

HWTEST_F(LocationApproximatelyPermissionTest, ReportManagerApproximatelyLocationTest002, TestSize.Level1)
{
    EXPECT_EQ(nullptr, reportManager_->GetPermittedLocation(tokenId_, 0, nullptr));
    MessageParcel parcel;
    parcel.WriteDouble(-MAX_LATITUDE - 1.0);  // latitude
    parcel.WriteDouble(-MAX_LONGITUDE - 1.0);  // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    auto newLocation = reportManager_->GetPermittedLocation(tokenId_, 0, location);
    EXPECT_NE(nullptr, newLocation);
    EXPECT_EQ(-MAX_LATITUDE, newLocation->GetLatitude());
    EXPECT_EQ(-MAX_LONGITUDE, newLocation->GetLongitude());
    EXPECT_EQ(DEFAULT_APPROXIMATELY_ACCURACY, newLocation->GetAccuracy());
}
}  // namespace Location
}  // namespace OHOS