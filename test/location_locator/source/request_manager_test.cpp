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

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Location;

void RequestManagerTest::SetUp()
{
    MockNativePermission();
    requestManager_ = DelayedSingleton<RequestManager>::GetInstance();
}

void RequestManagerTest::TearDown()
{
}

void LocatorServiceTest::MockNativePermission()
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

HWTEST_F(RequestManagerTest, RequestManagerTest001, TestSize.Level1)
{
    requestManager_->InitSystemListeners();
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetUid(uid);
    request->SetPid(0);
    request->SetTokenId(tokenId_);
    request->SetFirstTokenId(0);
    request->SetPackageName("RequestManagerTest");
    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetFixNumber(1);
    request->SetRequestConfig(*requestConfig);
    sptr<LocatorCallbackHost> locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());
    auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
    request->SetLocatorCallBack(locatorCallback);
    requestManager_->HandleStartLocating(request);
    
    requestManager_->HandleStopLocating(locatorCallback);

    requestManager_->HandlePowerSuspendChanged(int32_t pid, int32_t uid, int32_t flag);
    requestManager_->UpdateRequestRecord(std::shared_ptr<Request> request, bool shouldInsert);
    requestManager_->HandleRequest();
    requestManager_->UpdateUsingPermission(std::shared_ptr<Request> request);
    requestManager_->HandlePermissionChanged(uint32_t tokenId);
}
