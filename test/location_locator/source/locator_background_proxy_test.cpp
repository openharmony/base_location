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

#include "locator_background_proxy_test.h"

#include "accesstoken_kit.h"
#include "app_state_data.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "want.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location.h"
#include "location_log.h"
#include "locator_callback_napi.h"
#include "locator_callback_proxy.h"
#include "permission_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
using Want = OHOS::AAFwk::Want;
const int32_t LOCATION_PERM_NUM = 5;
const int VAL_UID = 20010044;
const std::string RUNNING_STATE_OBSERVER = "ohos.permission.RUNNING_STATE_OBSERVER";
void LocatorBackgroundProxyTest::SetUp()
{
    MockNativePermission();
}

void LocatorBackgroundProxyTest::TearDown()
{
}

void LocatorBackgroundProxyTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        RUNNING_STATE_OBSERVER.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorBackgroundProxyTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocatorBackgroundProxyTest, AppStateChangeCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, AppStateChangeCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] AppStateChangeCallbackTest001 begin");
    auto appStateObserver =
        sptr<AppStateChangeCallback>(new (std::nothrow) AppStateChangeCallback());
    int32_t state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    AppExecFwk::AppStateData appStateData;
    appStateData.uid = 1;
    appStateData.pid = 0;
    appStateData.state = state;
    ASSERT_TRUE(appStateObserver != nullptr);
    appStateObserver->OnForegroundApplicationChanged(appStateData);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] AppStateChangeCallbackTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->UpdateListOnRequestChange(request1);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->UpdateListOnRequestChange(nullptr);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspendTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->OnSuspend(request1, true); // cant find uid in requestMap

    locatorBackgroundProxy->OnSuspend(request1, false); // cant find uid in requestMap
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspendTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(VAL_UID);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("name");
    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(0);
    request1->SetRequestConfig(*requestConfig);
    locatorBackgroundProxy->OnSuspend(request1, true); // cant find request in list
    locatorBackgroundProxy->OnSuspend(request1, false); // add to requestsList
    locatorBackgroundProxy->OnSuspend(request1, false); // max num is 1, cant add request
    locatorBackgroundProxy->OnSuspend(request1, true); // remove from requestList
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspendTest003, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest003 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(VAL_UID);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("name");
    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(1); // fix number is 1
    request1->SetRequestConfig(*requestConfig);
    locatorBackgroundProxy->OnSuspend(request1, false); // add to requestsList
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest003 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspendTest004, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest004 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(VAL_UID);
    request1->SetPid(0);
    request1->SetTokenId(0); // invalid token id
    request1->SetFirstTokenId(0);
    request1->SetPackageName("name");
    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(0);
    request1->SetRequestConfig(*requestConfig);
    locatorBackgroundProxy->OnSuspend(request1, false); // permission denied, cant add to requestsList
    locatorBackgroundProxy->OnSuspend(request1, true); // permission denied, cant remove from requestList
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest004 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSaStateChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSaStateChangeTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSaStateChangeTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->OnSaStateChange(true);

    locatorBackgroundProxy->OnSaStateChange(false);

    locatorBackgroundProxy->OnSaStateChange(true);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSaStateChangeTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnDeleteRequestRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnDeleteRequestRecord001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnDeleteRequestRecord001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->OnDeleteRequestRecord(request1);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnDeleteRequestRecord001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, IsCallbackInProxyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, IsCallbackInProxyTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsCallbackInProxyTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    sptr<LocatorCallbackNapi> locatorCallbackHost =
        sptr<LocatorCallbackNapi>(new (std::nothrow)LocatorCallbackNapi());
    auto callback = sptr<ILocatorCallback>(locatorCallbackHost);
    EXPECT_EQ(false, locatorBackgroundProxy->IsCallbackInProxy(callback));

    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetLocatorCallBack(callback);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    auto requestConfig1 = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig1);
    requestConfig1->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig1->SetMaxAccuracy(1000.0);
    requestConfig1->SetFixNumber(0);
    request1->SetRequestConfig(*requestConfig1);
    locatorBackgroundProxy->OnSuspend(request1, false);
    EXPECT_EQ(false, locatorBackgroundProxy->IsCallbackInProxy(callback));
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsCallbackInProxyTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, IsAppBackgroundTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, IsAppBackgroundTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppBackgroundTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    EXPECT_EQ(true, locatorBackgroundProxy->IsAppBackground("LocatorBackgroundProxyTest"));
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppBackgroundTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, RegisterAppStateObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, RegisterAppStateObserverTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] RegisterAppStateObserverTest001 begin");
    MockNativePermission();
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver()); // unreg first
    locatorBackgroundProxy->RegisterAppStateObserver();
    EXPECT_EQ(true, locatorBackgroundProxy->RegisterAppStateObserver()); // register again
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver());
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] RegisterAppStateObserverTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UpdateListOnUserSwitchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UpdateListOnUserSwitchTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnUserSwitchTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    int userId = 0;
    CommonUtils::GetCurrentUserId(userId);
    locatorBackgroundProxy->UpdateListOnUserSwitch(userId);
    EXPECT_EQ(userId, locatorBackgroundProxy->curUserId_);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnUserSwitchTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UpdateListOnUserSwitchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UpdateListOnUserSwitchTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnUserSwitchTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    locatorBackgroundProxy->UpdateListOnUserSwitch(0); // requestsMap cant find userid
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnUserSwitchTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, IsCallbackInProxyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, IsCallbackInProxyTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsCallbackInProxyTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetLocatorCallBack(locatorBackgroundProxy->callback_);
    locatorBackgroundProxy->requestsList_->push_back(request);
    EXPECT_EQ(true,
        locatorBackgroundProxy->IsCallbackInProxy(locatorBackgroundProxy->callback_));
    sptr<ILocatorCallback> newCallback = new (std::nothrow) LocatorCallbackStub();
    EXPECT_EQ(false,
        locatorBackgroundProxy->IsCallbackInProxy(newCallback));
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsCallbackInProxyTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnUserSwitchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnUserSwitchTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserSwitchTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    int userId = 0;
    CommonUtils::GetCurrentUserId(userId);
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorBackgroundProxy->requestsList_->push_back(request);
    locatorBackgroundProxy->OnUserSwitch(userId);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserSwitchTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnUserSwitchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnUserSwitchTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserSwitchTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    int userId = 0;
    CommonUtils::GetCurrentUserId(userId);
    locatorBackgroundProxy->requestsList_->clear();
    locatorBackgroundProxy->OnUserSwitch(userId);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserSwitchTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnUserRemoveTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnUserRemoveTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserRemoveTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    int userId = 0;
    CommonUtils::GetCurrentUserId(userId);
    locatorBackgroundProxy->OnUserRemove(userId);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserRemoveTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnUserRemoveTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnUserRemoveTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserRemoveTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    locatorBackgroundProxy->OnUserRemove(0);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserRemoveTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, CheckMaxRequestNumTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, CheckMaxRequestNumTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    locatorBackgroundProxy->CheckMaxRequestNum(0, "PKG_FOR_TEST"); // cant find userId
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, CheckMaxRequestNumTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, CheckMaxRequestNumTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    int32_t userId = locatorBackgroundProxy->GetUserId(VAL_UID);
    auto list = std::make_shared<std::list<std::shared_ptr<Request>>>();
    auto request = std::make_shared<Request>();
    request->SetUid(VAL_UID);
    request->SetPackageName("PKG_FOR_TEST");
    list->push_back(request);
    (locatorBackgroundProxy->requestsMap_)->insert(make_pair(userId, list));
    EXPECT_EQ(false, locatorBackgroundProxy->CheckMaxRequestNum(VAL_UID, "PKG_FOR_TEST")); // matched request
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, CheckMaxRequestNumTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, CheckMaxRequestNumTest003, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest003 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    int32_t userId = locatorBackgroundProxy->GetUserId(VAL_UID);
    auto list = std::make_shared<std::list<std::shared_ptr<Request>>>();
    auto request = std::make_shared<Request>();
    request->SetUid(-1);
    request->SetPackageName("PKG_FOR_TEST");
    list->push_back(request);
    (locatorBackgroundProxy->requestsMap_)->insert(make_pair(userId, list));
    EXPECT_EQ(false, locatorBackgroundProxy->CheckMaxRequestNum(VAL_UID, "PKG_FOR_TEST")); // uid is wrong
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest003 end");
}

HWTEST_F(LocatorBackgroundProxyTest, CheckMaxRequestNumTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, CheckMaxRequestNumTest004, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest004 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    int32_t userId = locatorBackgroundProxy->GetUserId(VAL_UID);
    auto list = std::make_shared<std::list<std::shared_ptr<Request>>>();
    auto request = std::make_shared<Request>();
    request->SetUid(VAL_UID);
    request->SetPackageName("INVALID_PKG_NAME");
    list->push_back(request);
    (locatorBackgroundProxy->requestsMap_)->insert(make_pair(userId, list));
    EXPECT_EQ(false, locatorBackgroundProxy->CheckMaxRequestNum(VAL_UID, "PKG_FOR_TEST")); // pkg name is wrong
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] CheckMaxRequestNumTest004 end");
}

HWTEST_F(LocatorBackgroundProxyTest, MLocatorCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, MLocatorCallbackTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] MLocatorCallbackTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    auto callback =
        sptr<LocatorBackgroundProxy::mLocatorCallback>(new (std::nothrow) LocatorBackgroundProxy::mLocatorCallback());
    ASSERT_TRUE(callback != nullptr);
    auto request = std::make_shared<Request>();
    request->SetUid(VAL_UID);
    request->SetPackageName("PKG_NAME");
    (locatorBackgroundProxy->requestsList_)->clear();

    auto location = std::make_unique<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(1.0); // latitude
    parcel.WriteDouble(2.0); // longitude
    parcel.WriteDouble(3.0); // altitude
    parcel.WriteDouble(4.0); // accuracy
    parcel.WriteDouble(5.0); // speed
    parcel.WriteDouble(6.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock
    EXPECT_NE(nullptr, location);
    location->ReadFromParcel(parcel);
    callback->OnLocationReport(location);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] MLocatorCallbackTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, MLocatorCallbackTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, MLocatorCallbackTest003, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] MLocatorCallbackTest003 begin");
    auto callback =
        sptr<LocatorBackgroundProxy::mLocatorCallback>(new (std::nothrow) LocatorBackgroundProxy::mLocatorCallback());
    ASSERT_TRUE(callback != nullptr);
    callback->OnLocatingStatusChange(0);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] MLocatorCallbackTest003 end");
}

HWTEST_F(LocatorBackgroundProxyTest, MLocatorCallbackTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, MLocatorCallbackTest004, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] MLocatorCallbackTest004 begin");
    auto callback =
        sptr<LocatorBackgroundProxy::mLocatorCallback>(new (std::nothrow) LocatorBackgroundProxy::mLocatorCallback());
    ASSERT_TRUE(callback != nullptr);
    callback->OnErrorReport(0);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] MLocatorCallbackTest004 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UserSwitchSubscriberOnReceiveEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UserSwitchSubscriberOnReceiveEventTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UserSwitchSubscriberOnReceiveEventTest001 begin");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<LocatorBackgroundProxy::UserSwitchSubscriber> subscriber =
        std::make_shared<LocatorBackgroundProxy::UserSwitchSubscriber>(subscriberInfo);
    Want want;
    want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventData event;
    event.SetWant(want);
    event.SetCode(0);
    ASSERT_TRUE(subscriber != nullptr);
    subscriber->OnReceiveEvent(event);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UserSwitchSubscriberOnReceiveEventTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UserSwitchSubscriberOnReceiveEventTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UserSwitchSubscriberOnReceiveEventTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UserSwitchSubscriberOnReceiveEventTest002 begin");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<LocatorBackgroundProxy::UserSwitchSubscriber> subscriber =
        std::make_shared<LocatorBackgroundProxy::UserSwitchSubscriber>(subscriberInfo);
    Want want;
    want.SetAction(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    OHOS::EventFwk::CommonEventData event;
    event.SetWant(want);
    event.SetCode(0);
    subscriber->OnReceiveEvent(event);
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy != nullptr);
    auto iter = (locatorBackgroundProxy->requestsMap_)->find(0);
    EXPECT_EQ(true, iter == (locatorBackgroundProxy->requestsMap_)->end());
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UserSwitchSubscriberOnReceiveEventTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread001 begin");
    auto locatorBackgroundProxy1 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy1 != nullptr);
    locatorBackgroundProxy1->isLocating_ = true;
    locatorBackgroundProxy1->proxySwtich_ = true;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorBackgroundProxy1->requestsList_->push_back(request);
    locatorBackgroundProxy1->StartLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread002 begin");
    auto locatorBackgroundProxy2 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy2 != nullptr);
    locatorBackgroundProxy2->isLocating_ = true;
    locatorBackgroundProxy2->proxySwtich_ = true;
    locatorBackgroundProxy2->StartLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread003, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread003 begin");
    auto locatorBackgroundProxy3 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy3 != nullptr);
    locatorBackgroundProxy3->isLocating_ = true;
    locatorBackgroundProxy3->proxySwtich_ = false;
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    locatorBackgroundProxy3->requestsList_->push_back(request1);
    locatorBackgroundProxy3->StartLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread003 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread004, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread004 begin");
    auto locatorBackgroundProxy4 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy4 != nullptr);
    locatorBackgroundProxy4->isLocating_ = false;
    locatorBackgroundProxy4->proxySwtich_ = true;
    std::shared_ptr<Request> request2 = std::make_shared<Request>();
    locatorBackgroundProxy4->requestsList_->push_back(request2);
    locatorBackgroundProxy4->StartLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread004 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread005, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread005 begin");
    auto locatorBackgroundProxy5 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy5 != nullptr);
    locatorBackgroundProxy5->isLocating_ = true;
    locatorBackgroundProxy5->proxySwtich_ = false;
    locatorBackgroundProxy5->StartLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread005 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread006, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread006 begin");
    auto locatorBackgroundProxy6 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy6 != nullptr);
    locatorBackgroundProxy6->isLocating_ = false;
    locatorBackgroundProxy6->proxySwtich_ = false;
    std::shared_ptr<Request> request3 = std::make_shared<Request>();
    locatorBackgroundProxy6->requestsList_->push_back(request3);
    locatorBackgroundProxy6->StartLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread006 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread007, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread007 begin");
    auto locatorBackgroundProxy7 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy7 != nullptr);
    locatorBackgroundProxy7->isLocating_ = false;
    locatorBackgroundProxy7->proxySwtich_ = false;
    locatorBackgroundProxy7->StartLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread007 end");
}

HWTEST_F(LocatorBackgroundProxyTest, StartLocatorThread008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, StartLocatorThread008, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread008 begin");
    auto locatorBackgroundProxy1 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy1 != nullptr);
    locatorBackgroundProxy1->isLocating_ = false;
    locatorBackgroundProxy1->StopLocator();

    auto locatorBackgroundProxy2 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy2 != nullptr);
    locatorBackgroundProxy2->isLocating_ = true;
    locatorBackgroundProxy2->StopLocator();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] StartLocatorThread008 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspend001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspend001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspend001 begin");
    auto locatorBackgroundProxy1 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy1 != nullptr);
    locatorBackgroundProxy1->featureSwitch_ = true;
    locatorBackgroundProxy1->isUserSwitchSubscribed_ = true;
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    locatorBackgroundProxy1->OnSuspend(request1, true);

    auto locatorBackgroundProxy2 = LocatorBackgroundProxy::GetInstance();
    ASSERT_TRUE(locatorBackgroundProxy2 != nullptr);
    locatorBackgroundProxy2->featureSwitch_ = true;
    locatorBackgroundProxy1->isUserSwitchSubscribed_ = false;
    std::shared_ptr<Request> request2 = std::make_shared<Request>();
    locatorBackgroundProxy2->OnSuspend(request2, false);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspend001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSaStateChangeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSaStateChangeTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSaStateChangeTest002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->proxySwtich_ = true;
    locatorBackgroundProxy->OnSaStateChange(true);

    locatorBackgroundProxy->proxySwtich_ = false;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorBackgroundProxy->requestsList_->push_back(request);
    locatorBackgroundProxy->OnSaStateChange(true);

    locatorBackgroundProxy->proxySwtich_ = true;
    locatorBackgroundProxy->OnSaStateChange(false);

    locatorBackgroundProxy->proxySwtich_ = false;
    locatorBackgroundProxy->OnSaStateChange(false);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSaStateChangeTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnDeleteRequestRecord002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnDeleteRequestRecord002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnDeleteRequestRecord002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();

    locatorBackgroundProxy->featureSwitch_ = false;
    locatorBackgroundProxy->OnDeleteRequestRecord(request1);

    locatorBackgroundProxy->featureSwitch_ = true;
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->OnDeleteRequestRecord(request1);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnDeleteRequestRecord002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UpdateListOnSuspend001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UpdateListOnSuspend001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnSuspend001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->UpdateListOnSuspend(nullptr, true);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    request1->requestConfig_ = nullptr;
    locatorBackgroundProxy->UpdateListOnSuspend(request1, true);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnSuspend001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, IsCallbackInProxyTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, IsCallbackInProxyTest003, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsCallbackInProxyTest003 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->featureSwitch_ = false;
    sptr<ILocatorCallback> newCallback = new (std::nothrow) LocatorCallbackStub();
    bool ret = locatorBackgroundProxy->IsCallbackInProxy(newCallback);
    EXPECT_EQ(false, ret);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsCallbackInProxyTest003 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnUserSwitchTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnUserSwitchTest003, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserSwitchTest003 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->requestsMap_->clear();
    int32_t userId = 0;
    auto list = std::make_shared<std::list<std::shared_ptr<Request>>>();
    auto request = std::make_shared<Request>();
    request->SetUid(VAL_UID);
    request->SetPackageName("INVALID_PKG_NAME");
    list->push_back(request);
    (locatorBackgroundProxy->requestsMap_)->insert(make_pair(userId, list));
    locatorBackgroundProxy->OnUserSwitch(userId);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnUserSwitchTest003 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UnregisterAppStateObserverTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UnregisterAppStateObserverTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UnregisterAppStateObserverTest001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->iAppMgr_  = nullptr;
    locatorBackgroundProxy->appStateObserver_  = nullptr;
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver());

    locatorBackgroundProxy->RegisterAppStateObserver();
    locatorBackgroundProxy->iAppMgr_  = nullptr;
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver());

    EXPECT_EQ(true, locatorBackgroundProxy->RegisterAppStateObserver());
    locatorBackgroundProxy->appStateObserver_  = nullptr;
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver());

    EXPECT_EQ(true, locatorBackgroundProxy->RegisterAppStateObserver());
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver());
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UnregisterAppStateObserverTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, IsAppHasFormVisible001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, IsAppHasFormVisible001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppHasFormVisible001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    auto ret = locatorBackgroundProxy->IsAppHasFormVisible(tokenId_, 0);
    EXPECT_EQ(false, ret);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppHasFormVisible001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, IsAppHasFormVisible002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, IsAppHasFormVisible002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppHasFormVisible002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    auto ret = locatorBackgroundProxy->IsAppHasFormVisible(0, 0);
    EXPECT_EQ(false, ret);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppHasFormVisible002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnAddSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnAddSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnAddSystemAbility001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    auto systemAbilityStatusChangeListener = locatorBackgroundProxy->statusChangeListener_;
    int32_t systemAbilityId = 1000;
    const std::string& deviceId = "0123456789ABCDEF";
    systemAbilityStatusChangeListener->OnAddSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnAddSystemAbility001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnRemoveSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnRemoveSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnRemoveSystemAbility001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    auto systemAbilityStatusChangeListener = locatorBackgroundProxy->statusChangeListener_;
    int32_t systemAbilityId = COMMON_EVENT_SERVICE_ID;
    const std::string& deviceId = "0123456789ABCDEF";
    systemAbilityStatusChangeListener->OnRemoveSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnRemoveSystemAbility001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnRemoveSystemAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnRemoveSystemAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnRemoveSystemAbility002 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    auto systemAbilityStatusChangeListener = locatorBackgroundProxy->statusChangeListener_;
    int32_t systemAbilityId = 1;
    const std::string& deviceId = "0123456789ABCDEF";
    systemAbilityStatusChangeListener->OnRemoveSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnRemoveSystemAbility002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, IsAppInLocationContinuousTasks001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, IsAppInLocationContinuousTasks001, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppInLocationContinuousTasks001 begin");
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();

    locatorBackgroundProxy->IsAppInLocationContinuousTasks(1000);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] IsAppInLocationContinuousTasks001 end");
}
}  // namespace Location
}  // namespace OHOS