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
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
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
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest002, TestSize.Level1)
{
    int32_t userId = 0;
    CommonUtils::GetCurrentUserId(userId);

    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, smgr);
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    EXPECT_NE(nullptr, remoteObject);
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy(new AppExecFwk::BundleMgrProxy(remoteObject));
    EXPECT_NE(nullptr, bundleMgrProxy);
    std::string name = "ohos.global.systemres";
    int32_t uid = bundleMgrProxy->GetUidByBundleName(name, userId);

    LBSLOGD(LOCATOR, "bundleName : %{public}s, uid = %{public}d", name.c_str(), uid);

    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(uid);
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
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest003, TestSize.Level1)
{
    int32_t userId = 0;
    CommonUtils::GetCurrentUserId(userId);

    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, smgr);
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    EXPECT_NE(nullptr, remoteObject);
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy(new AppExecFwk::BundleMgrProxy(remoteObject));
    EXPECT_NE(nullptr, bundleMgrProxy);
    std::string name = "ohos.global.systemres";
    int32_t uid = bundleMgrProxy->GetUidByBundleName(name, userId);

    LBSLOGD(LOCATOR, "bundleName : %{public}s, uid = %{public}d", name.c_str(), uid);

    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(uid);
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
}

HWTEST_F(LocatorBackgroundProxyTest, OnSuspendTest004, TestSize.Level1)
{
    int32_t userId = 0;
    CommonUtils::GetCurrentUserId(userId);

    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, smgr);
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    EXPECT_NE(nullptr, remoteObject);
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy(new AppExecFwk::BundleMgrProxy(remoteObject));
    EXPECT_NE(nullptr, bundleMgrProxy);
    std::string name = "ohos.global.systemres";
    int32_t uid = bundleMgrProxy->GetUidByBundleName(name, userId);

    LBSLOGD(LOCATOR, "bundleName : %{public}s, uid = %{public}d", name.c_str(), uid);

    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, locatorBackgroundProxy);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(uid);
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