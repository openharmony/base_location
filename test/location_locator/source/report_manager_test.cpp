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
#include "locator.h"
#define private public
#include "locator_ability.h"
#include "request.h"
#undef private
#include "locator_callback_napi.h"
#include "locator_callback_proxy.h"
#include "request_manager.h"
#include "permission_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const std::string UNKNOWN_ABILITY = "unknown_ability";
void ReportManagerTest::SetUp()
{
    MockNativePermission();
    reportManager_ = ReportManager::GetInstance();
    EXPECT_NE(nullptr, reportManager_);
}

void ReportManagerTest::TearDown()
{
    reportManager_ = nullptr;
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

std::unique_ptr<Location> ReportManagerTest::MockLocation()
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000100); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock
    location->ReadFromParcel(parcel);
    return location;
}

HWTEST_F(ReportManagerTest, ReportRemoteCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, ReportRemoteCallbackTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ReportRemoteCallbackTest001 begin");
    std::unique_ptr<Location> location = std::make_unique<Location>();
    auto locatorCallbackHostForTest =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    sptr<ILocatorCallback> locatorCallback =
        sptr<ILocatorCallback>(locatorCallbackHostForTest);
    EXPECT_EQ(true, reportManager_->
        ReportRemoteCallback(locatorCallback, ILocatorCallback::RECEIVE_LOCATION_STATUS_EVENT, 1));
    EXPECT_EQ(true, reportManager_->
        ReportRemoteCallback(locatorCallback, ILocatorCallback::RECEIVE_ERROR_INFO_EVENT, 1));
    EXPECT_EQ(false, reportManager_->
        ReportRemoteCallback(locatorCallback, ILocatorCallback::RECEIVE_LOCATION_INFO_EVENT, 1));
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ReportRemoteCallbackTest001 end");
}

HWTEST_F(ReportManagerTest, ResultCheckTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, ResultCheckTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ResultCheckTest001 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetUid(1000);
    request->SetPid(0);
    request->SetTokenId(tokenId_);
    request->SetFirstTokenId(0);
    request->SetPackageName("ReportManagerTest");
    auto location = MockLocation();

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

    std::unique_ptr<Location> lastLocation1 = std::make_unique<Location>(*location);
    lastLocation1->SetLatitude(-91.0);
    request->SetLastLocation(lastLocation1);
    EXPECT_EQ(true, reportManager_->ResultCheck(location, request)); // no need to check

    std::unique_ptr<Location> lastLocation2 = std::make_unique<Location>(*location);
    request->SetLastLocation(lastLocation2);
    reportManager_->ResultCheck(location, request); // time interval check failed

    std::unique_ptr<Location> lastLocation3 = std::make_unique<Location>(*location);
    lastLocation3->SetTimeSinceBoot(1000000000);
    requestConfig->SetDistanceInterval(1.0);
    request->SetRequestConfig(*requestConfig);
    request->SetLastLocation(lastLocation3);
    reportManager_->ResultCheck(location, request); // distance interval check failed

    std::unique_ptr<Location> lastLocation4 = std::make_unique<Location>(*location);
    lastLocation4->SetTimeSinceBoot(1000000000);
    requestConfig->SetDistanceInterval(0.0);
    requestConfig->SetMaxAccuracy(10.0);
    request->SetRequestConfig(*requestConfig);
    request->SetLastLocation(lastLocation4);
    EXPECT_EQ(false, reportManager_->ResultCheck(location, request)); // acc check failed
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ResultCheckTest001 end");
}

HWTEST_F(ReportManagerTest, ResultCheckTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, ResultCheckTest002, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ResultCheckTest002 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetUid(1000);
    request->SetPid(0);
    request->SetTokenId(tokenId_);
    request->SetFirstTokenId(0);
    request->SetPackageName("ReportManagerTest");
    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetMaxAccuracy(1000.0);
    requestConfig->SetFixNumber(1);
    request->SetRequestConfig(*requestConfig);
    auto location = MockLocation();

    std::unique_ptr<Location> lastLocation5 = std::make_unique<Location>(*location);
    lastLocation5->SetTimeSinceBoot(1000000000);
    requestConfig->SetDistanceInterval(0.0);
    requestConfig->SetMaxAccuracy(0.0);
    request->SetRequestConfig(*requestConfig);
    request->SetLastLocation(lastLocation5);
    reportManager_->ResultCheck(location, request); // check pass
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ResultCheckTest002 end");
}

HWTEST_F(ReportManagerTest, SetLastLocationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, SetLastLocationTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] SetLastLocationTest001 begin");
    reportManager_->GetLastLocation();
    int64_t curTime = CommonUtils::GetCurrentTimeStamp();
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(curTime * MILLI_PER_SEC); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    reportManager_->UpdateCacheLocation(location, GNSS_ABILITY);
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetUid(1000);
    request->SetPid(0);
    request->SetTokenId(tokenId_);
    request->SetFirstTokenId(0);
    request->SetPackageName("ReportManagerTest");
    EXPECT_NE(nullptr, reportManager_->GetLastLocation());
    reportManager_->GetCacheLocation(request);
    reportManager_->UpdateCacheLocation(location, NETWORK_ABILITY);
    reportManager_->GetCacheLocation(request);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] SetLastLocationTest001 end");
}

HWTEST_F(ReportManagerTest, GetPermittedLocationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, GetPermittedLocationTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] GetPermittedLocationTest001 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetTokenId(tokenId_);
    EXPECT_EQ(nullptr, reportManager_->GetPermittedLocation(request, nullptr));
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    auto newLocation = reportManager_->GetPermittedLocation(request, location);
    EXPECT_NE(nullptr, newLocation);
    if (newLocation != nullptr) {
        EXPECT_EQ(12.0, newLocation->GetLatitude());
        EXPECT_EQ(13.0, newLocation->GetLongitude());
        EXPECT_EQ(1000.0, newLocation->GetAccuracy());
        LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] GetPermittedLocationTest001 end");
    }
}

HWTEST_F(ReportManagerTest, OnReportLocationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, OnReportLocationTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest001 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(0); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    location->SetUuid("35279");
    std::list<std::shared_ptr<Request>> networkList;
    int num = 2;
    for (int i = 0; i < num; i++) {
        std::shared_ptr<Request> request = std::make_shared<Request>();
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetTimeInterval(i);
        request->SetUid(i + 1);
        request->SetPid(i + 2);
        request->SetPackageName("nameForTest");
        request->SetRequestConfig(*requestConfig);
        request->SetUuid(std::to_string(i + 35279));
        request->SetNlpRequestType(0);
        networkList.push_back(request);
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->requests_->insert(make_pair(NETWORK_ABILITY, networkList));

    EXPECT_EQ(true, reportManager_->OnReportLocation(location, NETWORK_ABILITY));
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest001 end");
}

HWTEST_F(ReportManagerTest, OnReportLocationTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, OnReportLocationTest002, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest002 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(0); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    EXPECT_EQ(true, reportManager_->OnReportLocation(location, GNSS_ABILITY)); // is not requesting
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest002 end");
}

HWTEST_F(ReportManagerTest, OnReportLocationTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, OnReportLocationTest003, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest003 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteInt32(0);          // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    requestConfig->SetFixNumber(0);
    requestConfig->SetTimeInterval(1);
    requestConfig->SetScenario(SCENE_DAILY_LIFE_SERVICE);
    auto locatorImpl = Locator::GetInstance();
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    locatorImpl->EnableAbility(true);
    locatorImpl->StartLocating(requestConfig, callbackStub); // start locating
    sleep(1);
    EXPECT_EQ(true, reportManager_->OnReportLocation(location, GNSS_ABILITY)); // report location successfully
    EXPECT_EQ(true,
        reportManager_->OnReportLocation(location, GNSS_ABILITY)); // report the same location, result check is false
    locatorImpl->StopLocating(callbackStub);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest003 end");
}

HWTEST_F(ReportManagerTest, OnReportLocationTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, OnReportLocationTest004, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest004 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteInt32(0);          // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    requestConfig->SetFixNumber(1); // locating once
    requestConfig->SetTimeOut(120000);
    requestConfig->SetScenario(SCENE_DAILY_LIFE_SERVICE);
    auto locatorImpl = Locator::GetInstance();
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    locatorImpl->EnableAbility(true);
    locatorImpl->StartLocating(requestConfig, callbackStub); // start locating
    sleep(1);
    EXPECT_EQ(true, reportManager_->OnReportLocation(location, GNSS_ABILITY)); // will resolve deadRequests
    locatorImpl->StopLocating(callbackStub);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] OnReportLocationTest004 end");
}

HWTEST_F(ReportManagerTest, UpdateRandomTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, UpdateRandomTest004, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] UpdateRandomTest004 begin");
    std::list<std::shared_ptr<Request>> gnssList;
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->requests_->insert(make_pair(GNSS_ABILITY, gnssList));
    reportManager_->UpdateRandom();

    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    reportManager_->lastUpdateTime_.tv_sec = now.tv_sec + LONG_TIME_INTERVAL +1;
    locatorAbility->requests_->clear();
    reportManager_->UpdateRandom();
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] UpdateRandomTest004 end");
}

HWTEST_F(ReportManagerTest, IsRequestFuseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, IsRequestFuseTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] IsRequestFuseTest001 begin");
    EXPECT_EQ(false, reportManager_->IsRequestFuse(nullptr));

    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetScenario(SCENE_UNSET);
    request->SetRequestConfig(*requestConfig);
    EXPECT_EQ(true, reportManager_->IsRequestFuse(request));

    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] IsRequestFuseTest001 end");
}

HWTEST_F(ReportManagerTest, IsRequestFuseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, IsRequestFuseTest002, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] IsRequestFuseTest002 begin");
    EXPECT_EQ(false, reportManager_->IsRequestFuse(nullptr));

    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_UNSET);
    requestConfig->SetScenario(SCENE_UNSET);
    request->SetRequestConfig(*requestConfig);
    reportManager_->IsRequestFuse(request);

    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] IsRequestFuseTest002 end");
}

HWTEST_F(ReportManagerTest, UpdateLocationByRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, UpdateLocationByRequestTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] UpdateLocationByRequestTest001 begin");
    auto location = MockLocation();
    reportManager_->UpdateLocationByRequest(tokenId_, tokenId_, location);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] UpdateLocationByRequestTest001 end");
}

HWTEST_F(ReportManagerTest, UpdateLocationByRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, UpdateLocationByRequestTest002, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] UpdateLocationByRequestTest002 begin");
    std::unique_ptr<Location> location = nullptr;
    reportManager_->UpdateLocationByRequest(tokenId_, tokenId_, location);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] UpdateLocationByRequestTest002 end");
}

HWTEST_F(ReportManagerTest, ProcessRequestForReport001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, ProcessRequestForReport001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ProcessRequestForReport001 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetTimeInterval(1);
    request->SetUid(111);
    request->SetPid(222);
    request->SetPackageName("nameForTest");
    request->SetRequestConfig(*requestConfig);
    request->SetRequesting(true);
    request->SetUuid(std::to_string(35279));
    request->SetNlpRequestType(0);
    auto deadRequests = std::make_unique<std::list<std::shared_ptr<Request>>>();
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->SetUuid("35279");
    reportManager_->ProcessRequestForReport(request, deadRequests, location, NETWORK_ABILITY);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ProcessRequestForReport001 end");
}

HWTEST_F(ReportManagerTest, ProcessRequestForReport002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, ProcessRequestForReport002, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ProcessRequestForReport002 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetTimeInterval(1);
    request->SetUid(111);
    request->SetPid(222);
    request->SetPackageName("nameForTest");

    request->SetRequesting(true);
    request->SetUuid(std::to_string(35279));
    request->SetNlpRequestType(0);
    auto deadRequests = std::make_unique<std::list<std::shared_ptr<Request>>>();
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->SetUuid("35279");
    reportManager_->ProcessRequestForReport(request, deadRequests, location, NETWORK_ABILITY);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] ProcessRequestForReport002 end");
}

HWTEST_F(ReportManagerTest, WriteNetWorkReportEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "ReportManagerTest, WriteNetWorkReportEvent, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] WriteNetWorkReportEvent begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetTimeInterval(1);
    request->SetUid(111);
    request->SetPid(222);
    request->SetPackageName("nameForTest");
    request->SetRequestConfig(*requestConfig);
    request->SetRequesting(true);
    request->SetUuid(std::to_string(35279));
    request->SetNlpRequestType(0);
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->SetUuid("35279");
    reportManager_->WriteNetWorkReportEvent(NETWORK_ABILITY, request, location);
    LBSLOGI(REPORT_MANAGER, "[ReportManagerTest] WriteNetWorkReportEvent end");
}
}  // namespace Location
}  // namespace OHOS