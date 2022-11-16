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
#include "app_state_data.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "i_locator_callback.h"
#include "locator_callback_host.h"
#include "request_config.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
void RequestManagerTest::SetUp()
{
    MockNativePermission();
    requestManager_ = DelayedSingleton<RequestManager>::GetInstance();
    EXPECT_NE(nullptr, requestManager_);
    request_ = std::make_shared<Request>();
    EXPECT_NE(nullptr, request_);
    request_->SetUid(1000);
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
    request_->SetLocatorCallBack(locatorCallback);
}

void RequestManagerTest::TearDown()
{
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
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(RequestManagerTest, InitSystemListeners001, TestSize.Level1)
{
    EXPECT_EQ(true, requestManager_->InitSystemListeners());
}

HWTEST_F(RequestManagerTest, HandleStartAndStopLocating001, TestSize.Level1)
{
    requestManager_->HandleStartLocating(request_);
    requestManager_->HandleStopLocating(nullptr); // can't stop locating

    requestManager_->HandleStartLocating(request_); // can start locating
    requestManager_->HandleStopLocating(callback_); // can stop locating

    requestManager_->HandleStartLocating(nullptr); // can't start locating
    requestManager_->HandleStopLocating(callback_); // can stop locating, but not locating
}

HWTEST_F(RequestManagerTest, HandlePowerSuspendChanged001, TestSize.Level1)
{
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid(), AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    requestManager_->HandlePowerSuspendChanged(request_->GetPid(),
        request_->GetUid(), AppExecFwk::ApplicationState::APP_STATE_BACKGROUND);
}

HWTEST_F(RequestManagerTest, HandlePowerSuspendChanged001, TestSize.Level1)
{
    requestManager_->UpdateRequestRecord(request_, true); // uid = 1000 should be added to runningUids
    requestManager_->UpdateRequestRecord(request_, false); // uid = 1000 should be removed from runningUids
}

HWTEST_F(RequestManagerTest, UpdateUsingPermissionTest001, TestSize.Level1)
{
    requestManager_->UpdateUsingPermission(nullptr);

    EXPECT_EQ(false, request_->GetLocationPermState());
    EXPECT_EQ(false, request_->GetBackgroundPermState());
    EXPECT_EQ(false, request_->GetApproximatelyPermState());
    requestManager_->UpdateUsingPermission(request_);
    EXPECT_EQ(true, request_->GetLocationPermState());
    EXPECT_EQ(true, request_->GetBackgroundPermState());
    EXPECT_EQ(true, request_->GetApproximatelyPermState());

    std::shared_ptr<Request> requestWithoutPermission = std::make_shared<Request>();
    EXPECT_EQ(false, requestWithoutPermission->GetLocationPermState());
    EXPECT_EQ(false, requestWithoutPermission->GetBackgroundPermState());
    EXPECT_EQ(false, requestWithoutPermission->GetApproximatelyPermState());
    requestManager_->UpdateUsingPermission(requestWithoutPermission);
    EXPECT_EQ(false, requestWithoutPermission->GetLocationPermState());
    EXPECT_EQ(false, requestWithoutPermission->GetBackgroundPermState());
    EXPECT_EQ(false, requestWithoutPermission->GetApproximatelyPermState());
}

HWTEST_F(RequestManagerTest, HandlePermissionChangedTest001, TestSize.Level1)
{
    requestManager_->HandleStartLocating(request_);
    requestManager_->HandlePermissionChanged(request_->GetTokenId());

    requestManager_->HandleStopLocating(request_);
    requestManager_->HandlePermissionChanged(request_->GetTokenId());
}
}  // namespace Location
}  // namespace OHOS