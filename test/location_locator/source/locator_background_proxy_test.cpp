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
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "locator_background_proxy.h"
#include "locator_callback_host.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
void LocatorBackgroundProxyTest::SetUp()
{
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
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->UpdateListOnRequestChange(nullptr);

    locatorBackgroundProxy->UpdateListOnRequestChange(request1);
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(tokenId_);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorBackgroundProxyTest");
    locatorBackgroundProxy->OnSuspend(request1, true);
    
    locatorBackgroundProxy->OnSuspend(request1, false);
}

HWTEST_F(LocatorBackgroundProxyTest, OnSaStateChangeTest001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    locatorBackgroundProxy->OnSaStateChange(true);
    
    locatorBackgroundProxy->OnSaStateChange(false);

    locatorBackgroundProxy->OnSaStateChange(true);
}

HWTEST_F(LocatorBackgroundProxyTest, OnDeleteRequestRecord001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
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
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
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
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    EXPECT_EQ(true, locatorBackgroundProxy->IsAppBackground("LocatorBackgroundProxyTest"));
}

HWTEST_F(LocatorBackgroundProxyTest, RegisterAppStateObserverTest001, TestSize.Level1)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver()); // unreg first
    EXPECT_EQ(false, locatorBackgroundProxy->RegisterAppStateObserver());
    EXPECT_EQ(false, locatorBackgroundProxy->RegisterAppStateObserver()); // register again
    EXPECT_EQ(true, locatorBackgroundProxy->UnregisterAppStateObserver());
}
}  // namespace Location
}  // namespace OHOS