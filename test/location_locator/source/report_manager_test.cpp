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

#include "report_manager_test.h"

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "i_locator_callback.h"
#include "location.h"
#include "locator_ability.h"
#include "locator_callback_host.h"
#include "report_manager.h"
#include "request.h"
#include "request_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const std::string UNKNOWN_ABILITY = "unknown_ability";
void ReportManagerTest::SetUp()
{
    MockNativePermission();
    reportManager_ = DelayedSingleton<ReportManager>::GetInstance();
    EXPECT_NE(nullptr, reportManager_);
}

void ReportManagerTest::TearDown()
{
}

void ReportManagerTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "ReportManagerTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(ReportManagerTest, ReportRemoteCallbackTest001, TestSize.Level1)
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    auto locatorCallbackHostForTest =
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    sptr<ILocatorCallback> locatorCallback =
        sptr<ILocatorCallback>(locatorCallbackHostForTest);
    EXPECT_EQ(true, reportManager_->
        ReportRemoteCallback(locatorCallback, ILocatorCallback::RECEIVE_LOCATION_STATUS_EVENT, 1));
    EXPECT_EQ(true, reportManager_->
        ReportRemoteCallback(locatorCallback, ILocatorCallback::RECEIVE_ERROR_INFO_EVENT, 1));
    EXPECT_EQ(false, reportManager_->
        ReportRemoteCallback(locatorCallback, ILocatorCallback::RECEIVE_LOCATION_INFO_EVENT, 1));
}

HWTEST_F(ReportManagerTest, ResultCheckTest001, TestSize.Level1)
{
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetUid(1000);
    request->SetPid(0);
    request->SetTokenId(tokenId_);
    request->SetFirstTokenId(0);
    request->SetPackageName("ReportManagerTest");
    std::unique_ptr<Location> location = std::make_unique<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteFloat(1000.0); // accuracy
    parcel.WriteFloat(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock
    location->ReadFromParcel(parcel);
    EXPECT_EQ(true, reportManager_->ResultCheck(location, request));
    EXPECT_EQ(false, reportManager_->ResultCheck(nullptr, request)); // no location
    EXPECT_EQ(false, reportManager_->ResultCheck(location, nullptr)); // no request
    EXPECT_EQ(false, reportManager_->ResultCheck(nullptr, nullptr)); // no location & no request

    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetMaxAccuracy(1000.0);
    requestConfig->SetFixNumber(1);
    request->SetRequestConfig(*requestConfig);
    EXPECT_EQ(true, reportManager_->ResultCheck(location, request)); // no last location
    
    request->SetLastLocation(location);
    EXPECT_EQ(true, reportManager_->ResultCheck(location, request));
}

HWTEST_F(ReportManagerTest, SetLastLocationTest001, TestSize.Level1)
{
    EXPECT_EQ(nullptr, reportManager_->GetLastLocation());
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteFloat(1000.0); // accuracy
    parcel.WriteFloat(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    reportManager_->SetLastLocation(location);
    EXPECT_NE(nullptr, reportManager_->GetLastLocation());
}

HWTEST_F(ReportManagerTest, GetPermittedLocationTest001, TestSize.Level1)
{
    EXPECT_EQ(nullptr, reportManager_->GetPermittedLocation(tokenId_, 0, nullptr));
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteFloat(1000.0); // accuracy
    parcel.WriteFloat(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    EXPECT_NE(nullptr, reportManager_->GetPermittedLocation(tokenId_, 0, location));
    EXPECT_EQ(12.0, location->GetLatitude());
    EXPECT_EQ(13.0, location->GetLongitude());
    EXPECT_EQ(1000.0, location->GetAccuracy());
}

HWTEST_F(ReportManagerTest, UpdateRandomTest001, TestSize.Level1)
{
    reportManager_->UpdateRandom();
}

HWTEST_F(ReportManagerTest, OnReportLocationTest001, TestSize.Level1)
{
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteFloat(1000.0); // accuracy
    parcel.WriteFloat(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(false); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);

    reportManager_->OnReportLocation(location, UNKNOWN_ABILITY);
}
}  // namespace Location
}  // namespace OHOS