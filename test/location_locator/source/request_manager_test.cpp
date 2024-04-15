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

#include "request_manager_test.h"

#include "accesstoken_kit.h"
#include "app_mgr_constants.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "i_locator_callback.h"
#include "locator_ability.h"
#include "locator_callback_host.h"
#include "request.h"
#include "request_config.h"
#include "permission_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const int UNKNOWN_PRIORITY = 0x01FF;
const int UNKNOWN_SCENE = 0x02FF;
void RequestManagerTest::SetUp()
{
    MockNativePermission();
    requestManager_ = DelayedSingleton<RequestManager>::GetInstance();
    EXPECT_NE(nullptr, requestManager_);
    request_ = std::make_shared<Request>();
    EXPECT_NE(nullptr, request_);
    request_->SetUid(SYSTEM_UID);
    request_->SetPid(0);
    request_->SetTokenId(tokenId_);
    request_->SetFirstTokenId(0);
    request_->SetPackageName("RequestManagerTest");
    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(1);
    request_->SetRequestConfig(*requestConfig);
    sptr<LocatorCallbackHost> locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());
    callback_ = sptr<ILocatorCallback>(locatorCallbackHost);
    request_->SetLocatorCallBack(callback_);
}

void RequestManagerTest::TearDown()
{
    requestManager_ = nullptr;
    DelayedSingleton<RequestManager>::DestroyInstance();
}

void RequestManagerTest::MockNativePermission()
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
        .processName = "RequestManagerTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void RequestManagerTest::FillRequestField(std::shared_ptr<Request>& request)
{
    request->SetUid(SYSTEM_UID);
    request->SetPid(0);
    request->SetTokenId(0);
    request->SetFirstTokenId(0);
    request->SetPackageName("pkg.name");
    std::unique_ptr<RequestConfig> requestConfig =
        std::make_unique<RequestConfig>();
    request->SetRequestConfig(*requestConfig);
    sptr<LocatorCallbackHost> locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());
    auto callback = sptr<ILocatorCallback>(locatorCallbackHost);
    request->SetLocatorCallBack(callback);
    request->SetRequesting(false);
    std::unique_ptr<Location> location = std::make_unique<Location>();
    request->SetLastLocation(location);
    request->SetLocationPermState(true);
    request->SetBackgroundPermState(true);
    request->SetApproximatelyPermState(true);
}

void RequestManagerTest::VerifyRequestField(std::shared_ptr<Request>& request)
{
    EXPECT_EQ(SYSTEM_UID, request->GetUid());
    EXPECT_EQ(0, request->GetPid());
    EXPECT_EQ(0, request->GetTokenId());
    EXPECT_EQ(0, request->GetFirstTokenId());
    EXPECT_EQ("pkg.name", request->GetPackageName());
    EXPECT_NE(nullptr, request->GetRequestConfig());
    EXPECT_NE(nullptr, request->GetLocatorCallBack());
    EXPECT_EQ(false, request->GetIsRequesting());
    EXPECT_NE(nullptr, request->GetLastLocation());
    EXPECT_EQ(true, request->GetLocationPermState());
    EXPECT_EQ(true, request->GetBackgroundPermState());
    EXPECT_EQ(true, request->GetApproximatelyPermState());
}

HWTEST_F(RequestManagerTest, InitSystemListeners001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, InitSystemListeners001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] InitSystemListeners001 begin");
    EXPECT_EQ(true, requestManager_->InitSystemListeners());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] InitSystemListeners001 end");
}

HWTEST_F(RequestManagerTest, HandleStartAndStopLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandleStartAndStopLocating001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandleStartAndStopLocating001 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    requestManager_->HandleStartLocating(request_);
    requestManager_->HandleStopLocating(nullptr); // can't stop locating

    requestManager_->HandleStartLocating(request_); // can start locating
    requestManager_->HandleStopLocating(callback_); // can stop locating

    requestManager_->HandleStartLocating(nullptr); // can't start locating
    requestManager_->HandleStopLocating(callback_); // can stop locating, but not locating
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandleStartAndStopLocating001 end");
}

HWTEST_F(RequestManagerTest, HandlePowerSuspendChanged001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandlePowerSuspendChanged001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged001 begin");
    requestManager_->UpdateRequestRecord(request_, true);
    EXPECT_EQ(true, requestManager_->IsUidInProcessing(SYSTEM_UID));
    int32_t state1 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid(), state1);
    int32_t state2 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid(), state2);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged001 end");
}

HWTEST_F(RequestManagerTest, HandlePowerSuspendChanged002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandlePowerSuspendChanged002, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged002 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    requestManager_->UpdateRequestRecord(request_, false);
    EXPECT_EQ(false, requestManager_->IsUidInProcessing(SYSTEM_UID));

    int32_t state1 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid(), state1);
    int32_t state2 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid(), state2);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged002 end");
}

HWTEST_F(RequestManagerTest, HandlePowerSuspendChanged003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandlePowerSuspendChanged003, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged003 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    requestManager_->UpdateRequestRecord(request_, false);
    EXPECT_EQ(false, requestManager_->IsUidInProcessing(SYSTEM_UID));

    int32_t state1 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid() + 1,
        request_->GetUid(), state1);
    int32_t state2 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid() + 1,
        request_->GetUid(), state2);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged003 end");
}

HWTEST_F(RequestManagerTest, HandlePowerSuspendChanged004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandlePowerSuspendChanged004, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged004 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    requestManager_->UpdateRequestRecord(request_, false);
    EXPECT_EQ(false, requestManager_->IsUidInProcessing(SYSTEM_UID));

    int32_t state1 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid() + 1, state1);
    int32_t state2 = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_BACKGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid() + 1, state2);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePowerSuspendChanged004 end");
}

HWTEST_F(RequestManagerTest, UpdateRequestRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, UpdateRequestRecord001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateRequestRecord001 begin");
    requestManager_->UpdateRequestRecord(request_, true); // uid = 1000 should be added to runningUids
    EXPECT_EQ(true, requestManager_->IsUidInProcessing(SYSTEM_UID));
    requestManager_->UpdateRequestRecord(request_, false); // uid = 1000 should be removed from runningUids
    EXPECT_EQ(false, requestManager_->IsUidInProcessing(SYSTEM_UID));
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateRequestRecord001 end");
}

HWTEST_F(RequestManagerTest, UpdateUsingPermissionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, UpdateUsingPermissionTest001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateUsingPermissionTest001 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    requestManager_->UpdateUsingPermission(nullptr);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateUsingPermissionTest001 end");
}

HWTEST_F(RequestManagerTest, UpdateUsingPermissionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, UpdateUsingPermissionTest002, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateUsingPermissionTest002 begin");
    EXPECT_EQ(false, request_->GetLocationPermState());
    EXPECT_EQ(false, request_->GetBackgroundPermState());
    EXPECT_EQ(false, request_->GetApproximatelyPermState());
    requestManager_->UpdateUsingPermission(request_);
    EXPECT_EQ(false, request_->GetLocationPermState());
    EXPECT_EQ(false, request_->GetBackgroundPermState());
    EXPECT_EQ(false, request_->GetApproximatelyPermState());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateUsingPermissionTest002 end");
}

HWTEST_F(RequestManagerTest, UpdateUsingPermissionTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, UpdateUsingPermissionTest003, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateUsingPermissionTest003 begin");
    requestManager_->UpdateRequestRecord(request_, true);
    EXPECT_EQ(false, request_->GetLocationPermState());
    EXPECT_EQ(false, request_->GetBackgroundPermState());
    EXPECT_EQ(false, request_->GetApproximatelyPermState());
    requestManager_->UpdateUsingPermission(request_);
    // location permission is not recorded
    EXPECT_EQ(false, request_->GetLocationPermState());
    EXPECT_EQ(false, request_->GetBackgroundPermState());
    // background location permission is not recorded
    EXPECT_EQ(true, request_->GetApproximatelyPermState());

    requestManager_->UpdateRequestRecord(request_, false);
    // location permission is not recorded
    EXPECT_EQ(false, request_->GetLocationPermState());
    // background location permission is not recorded
    EXPECT_EQ(false, request_->GetBackgroundPermState());
    EXPECT_EQ(true, request_->GetApproximatelyPermState());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] UpdateUsingPermissionTest003 end");
}

HWTEST_F(RequestManagerTest, HandlePermissionChangedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandlePermissionChangedTest001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePermissionChangedTest001 begin");
    requestManager_->HandleStartLocating(request_);
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    EXPECT_NE(0, locatorAbility->GetActiveRequestNum());
    requestManager_->HandlePermissionChanged(request_->GetTokenId());

    requestManager_->HandleStopLocating(callback_);
    requestManager_->HandlePermissionChanged(request_->GetTokenId());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandlePermissionChangedTest001 end");
}

HWTEST_F(RequestManagerTest, RequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestTest001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestTest001 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    FillRequestField(request);
    VerifyRequestField(request);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestTest001 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest001 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::shared_ptr<std::list<std::string>> proxyList = nullptr;
    request->GetProxyName(proxyList);
    EXPECT_EQ(nullptr, proxyList);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest001 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest002, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest002 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig1 =
        std::make_unique<RequestConfig>();
    requestConfig1->SetScenario(SCENE_NAVIGATION);
    request->SetRequestConfig(*requestConfig1);
    auto proxyList1 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList1);
    EXPECT_NE(true, proxyList1->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest002 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest003, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest003 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig2 =
        std::make_unique<RequestConfig>();
    requestConfig2->SetScenario(SCENE_TRAJECTORY_TRACKING);
    request->SetRequestConfig(*requestConfig2);
    auto proxyList2 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList2);
    EXPECT_NE(true, proxyList2->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest003 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest004, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest004 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig3 =
        std::make_unique<RequestConfig>();
    requestConfig3->SetScenario(SCENE_CAR_HAILING);
    request->SetRequestConfig(*requestConfig3);
    auto proxyList3 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList3);
    EXPECT_NE(true, proxyList3->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest004 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest005, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest005 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig4 =
        std::make_unique<RequestConfig>();
    requestConfig4->SetScenario(SCENE_DAILY_LIFE_SERVICE);
    request->SetRequestConfig(*requestConfig4);
    auto proxyList4 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList4);
    EXPECT_NE(true, proxyList4->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest005 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest006, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest006 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig5 =
        std::make_unique<RequestConfig>();
    requestConfig5->SetScenario(SCENE_NO_POWER);
    request->SetRequestConfig(*requestConfig5);
    auto proxyList5 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList5);
    EXPECT_NE(true, proxyList5->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest006 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest007, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest007 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig6 =
        std::make_unique<RequestConfig>();
    requestConfig6->SetScenario(SCENE_UNSET);
    requestConfig6->SetPriority(PRIORITY_ACCURACY);
    request->SetRequestConfig(*requestConfig6);
    auto proxyList6 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList6);
    EXPECT_NE(true, proxyList6->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest007 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest008, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest008 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig7 =
        std::make_unique<RequestConfig>();
    requestConfig7->SetScenario(SCENE_UNSET);
    requestConfig7->SetPriority(PRIORITY_LOW_POWER);
    request->SetRequestConfig(*requestConfig7);
    auto proxyList7 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList7);
    EXPECT_NE(true, proxyList7->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest008 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest009, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest009 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig8 =
        std::make_unique<RequestConfig>();
    requestConfig8->SetScenario(SCENE_UNSET);
    requestConfig8->SetPriority(PRIORITY_FAST_FIRST_FIX);
    request->SetRequestConfig(*requestConfig8);
    auto proxyList8 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList8);
    EXPECT_NE(true, proxyList8->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest009 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest010, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest010 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig9 =
        std::make_unique<RequestConfig>();
    requestConfig9->SetScenario(SCENE_UNSET);
    requestConfig9->SetPriority(UNKNOWN_PRIORITY);
    request->SetRequestConfig(*requestConfig9);
    auto proxyList9 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList9);
    EXPECT_EQ(true, proxyList9->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest010 end");
}

HWTEST_F(RequestManagerTest, RequestGetProxyNameTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, RequestGetProxyNameTest011, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest011 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    std::unique_ptr<RequestConfig> requestConfig10 =
        std::make_unique<RequestConfig>();
    requestConfig10->SetScenario(UNKNOWN_SCENE);
    request->SetRequestConfig(*requestConfig10);
    auto proxyList10 = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxyList10);
    EXPECT_EQ(true, proxyList10->empty());
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] RequestGetProxyNameTest011 end");
}

HWTEST_F(RequestManagerTest, GetRemoteObject001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, GetRemoteObject001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] GetRemoteObject001 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    requestManager_->GetRemoteObject(GNSS_ABILITY);

    requestManager_->GetRemoteObject("");
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] GetRemoteObject001 end");
}

HWTEST_F(RequestManagerTest, HandleChrEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandleChrEvent001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandleChrEvent001 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    std::list<std::shared_ptr<Request>> requests;
    requestManager_->HandleChrEvent(requests);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandleChrEvent001 end");
}

HWTEST_F(RequestManagerTest, HandleChrEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, HandleChrEvent002, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandleChrEvent002 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    std::list<std::shared_ptr<Request>> requests;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    requests.push_back(request);
    requestManager_->HandleChrEvent(requests);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] HandleChrEvent002 end");
}


HWTEST_F(RequestManagerTest, IsUidInProcessing001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "RequestManagerTest, IsUidInProcessing001, TestSize.Level1";
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] IsUidInProcessing001 begin");
    ASSERT_TRUE(requestManager_ != nullptr);
    bool ret = requestManager_->IsUidInProcessing(0);
    EXPECT_EQ(false, ret);
    LBSLOGI(REQUEST_MANAGER, "[RequestManagerTest] IsUidInProcessing001 end");
}
}  // namespace Location
}  // namespace OHOS