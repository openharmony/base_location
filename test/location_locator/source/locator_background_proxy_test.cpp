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

#include "common_utils.h"
#include "constant_definition.h"
#include "location.h"
#include "location_log.h"
#include "locator_background_proxy.h"
#include "locator_callback_host.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const int VAL_UID = 20010044;
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
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocatorBackgroundProxyTest, AppStateChangeCallbackTest001, TestSize.Level1)
{
    auto appStateObserver =
        sptr<AppStateChangeCallback>(new (std::nothrow) AppStateChangeCallback());
    int32_t state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    AppExecFwk::AppStateData appStateData;
    appStateData.uid = 1;
    appStateData.pid = 0;
    appStateData.state = state;
    appStateObserver->OnForegroundApplicationChanged(appStateData);
}

HWTEST_F(LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest001, TestSize.Level1";
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest001 begin");
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->UpdateListOnRequestChange(request1);
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, UpdateListOnRequestChangeTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest002 begin");
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->UpdateListOnRequestChange(nullptr);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] UpdateListOnRequestChangeTest002 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspendTest001, TestSize.Level1";
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] OnSuspendTest001 begin");
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->OnSuspend(request1, true); // cant find uid in requestMap

    locatorBackgroundProxy->OnSuspend(request1, false); // cant find uid in requestMap
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] OnSuspendTest001 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspendTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "[LocatorBackgroundProxyTest] OnSuspendTest002 begin");
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(VAL_UID);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName(name);
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
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] OnSuspendTest003 begin");
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(VAL_UID);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName(name);
    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(1); // fix number is 1
    request1->SetRequestConfig(*requestConfig);
    locatorBackgroundProxy->OnSuspend(request1, false); // add to requestsList
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] OnSuspendTest003 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorBackgroundProxyTest, OnSuspendTest004, TestSize.Level1";
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] OnSuspendTest004 begin");
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(VAL_UID);
    request1->SetPid(0);
    request1->SetTokenId(0); // invalid token id
    request1->SetFirstTokenId(0);
    request1->SetPackageName(name);
    auto requestConfig = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(0);
    request1->SetRequestConfig(*requestConfig);
    locatorBackgroundProxy->OnSuspend(request1, false); // permission denied, cant add to requestsList
    locatorBackgroundProxy->OnSuspend(request1, true); // permission denied, cant remove from requestList
    LBSLOGI(GNSS, "[LocatorBackgroundProxyTest] OnSuspendTest004 end");
}

HWTEST_F(LocatorBackgroundProxyTest, OnSaStateChangeTest001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->OnSaStateChange(true);
    
    locatorBackgroundProxy->OnSaStateChange(false);

    locatorBackgroundProxy->OnSaStateChange(true);
}

HWTEST_F(LocatorBackgroundProxyTest, OnDeleteRequestRecord001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->OnDeleteRequestRecord(request1);
}

HWTEST_F(LocatorBackgroundProxyTest, IsCallbackInProxyTest001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    sptr<LocatorCallbackHost> locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());
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
}

HWTEST_F(LocatorBackgroundProxyTest, IsAppBackgroundTest001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    EXPECT_EQ(true, locatorBackgroundProxy->IsAppBackground("LocatorBackgroundProxyTest"));
}

HWTEST_F(LocatorBackgroundProxyTest, RegisterAppStateObserverTest001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver()); // unreg first
    EXPECT_EQ(true, locatorBackgroundProxy->RegisterAppStateObserver());
    EXPECT_EQ(true, locatorBackgroundProxy->RegisterAppStateObserver()); // register again
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver());
}
}  // namespace Location
}  // namespace OHOS