/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "locator_service_test.h"

#include <cstdlib>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location_log.h"
#include "locator_ability.h"
#include "locator_skeleton.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Location;

void LocatorServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, systemAbilityManager);
    sptr<IRemoteObject> object = systemAbilityManager->GetSystemAbility(LOCATION_LOCATOR_SA_ID);
    EXPECT_NE(nullptr, object);
    proxy_ = new (std::nothrow) LocatorProxy(object);
    EXPECT_NE(nullptr, proxy_);
    callbackStub_ = new (std::nothrow) LocatorCallbackStub();
    EXPECT_NE(nullptr, callbackStub_);
    backgroundProxy_ = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, backgroundProxy_);
    request_ = std::make_shared<Request>();
    EXPECT_NE(nullptr, request_);
    request_->SetLocatorCallBack(callbackStub_);
    request_->SetUid(SYSTEM_UID);
    request_->SetPid(getpid());

    SetStartUpConfirmed(true);
    ChangedLocationMode(true);
}

void LocatorServiceTest::TearDown()
{
    /*
     * @tc.teardown: release memory.
     */
    proxy_ = nullptr;
    callbackStub_ = nullptr;
}

void LocatorServiceTest::SetStartUpConfirmed(bool isAuthorized)
{
    std::string value = isAuthorized ? "1" : "0";
    std::string executeCmd = "settings put SECURE high_accuracy_startup_comfirm " + value;
    system(executeCmd.c_str());
}

void LocatorServiceTest::ChangedLocationMode(bool isEnable)
{
    std::string value = isEnable ? "3" : "0";
    std::string executeCmd = "settings put SECURE location_mode " + value;
    system(executeCmd.c_str());
}

bool LocatorServiceTest::StartAndStopForLocating(MessageParcel& data)
{
    pid_t pid = 0;
    pid_t uid = 0;
    std::unique_ptr<RequestConfig> requestConfig = RequestConfig::Unmarshalling(data);
    proxy_->StartLocating(requestConfig, callbackStub_, "ohos.unit", pid, uid);
    bool ret = (proxy_->StopLocating(callbackStub_)) == REPLY_CODE_NO_EXCEPTION ? true : false;
    return ret;
}

/*
 * @tc.name: CheckSwitchState001
 * @tc.desc: Check location switch state expect success
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, CheckSwitchState001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call system ability and check switch state whether available.
     * @tc.expected: step1. get switch state is available.
     */
    int result = proxy_->GetSwitchState();
    EXPECT_EQ(true, (result == ENABLED || result == DISABLED));
}

/*
 * @tc.name: CheckLocatingForScenario001
 * @tc.desc: Check start locating based on scenario expect success
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, CheckLocatingForScenario001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call system ability and start locating for SCENE_VEHICLE_NAVIGATION.
     * @tc.expected: step1. get reply state is successful.
     */
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetScenario(SCENE_NAVIGATION);
    MessageParcel data;
    requestConfig->Marshalling(data);
    bool ret = StartAndStopForLocating(data);
    EXPECT_EQ(true, ret);
}

/*
 * @tc.name: CheckLocatingForConfig001
 * @tc.desc: Check start locating based on config expect success
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, CheckLocatingForConfig001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call system ability and start locating for HIGHT_ACCURACY/HIGHT_POWER_COST.
     * @tc.expected: step1. get reply state is successful.
     */
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    MessageParcel data;
    requestConfig->Marshalling(data);
    bool ret = StartAndStopForLocating(data);
    EXPECT_EQ(true, ret);
}

/*
 * @tc.name: CheckStopLocating001
 * @tc.desc: Check stop locating with illegal param and expect fail
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, CheckStopLocating001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call system ability and stop locating whit illegal param.
     * @tc.expected: step1. get reply state is false.
     */
    bool ret = (proxy_->StopLocating(callbackStub_) == REPLY_CODE_NO_EXCEPTION) ? true : false;
    EXPECT_EQ(true, ret);
}

/*
 * @tc.name: CheckGetCacheLocation001
 * @tc.desc: Check get cache location and expect success
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, CheckGetCacheLocation001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call system ability and get cache location.
     * @tc.expected: step1. get reply state is true.
     */
    MessageParcel data;
    MessageParcel reply;
    bool ret = false;
    if (proxy_->GetSwitchState() == 1) {
        ret = (proxy_->GetCacheLocation(data, reply) == REPLY_CODE_NO_EXCEPTION) ? true : false;
    } else {
        ret = (proxy_->GetCacheLocation(data, reply) != REPLY_CODE_NO_EXCEPTION) ? true : false;
    }
    EXPECT_EQ(true, ret);
}

/*
 * @tc.name: IsCallbackInProxyTest001
 * @tc.desc: Check if callback is in the proxy callback list
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, IsCallbackInProxyTest001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Check if callback is in the proxy callback list
     * @tc.expected: step1. return false
     */
    bool result =  backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(false, result);
}

/*
 * @tc.name: OnSuspendTest001
 * @tc.desc: Test the function of the process enter and exit frozen state
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, OnSuspendTest001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call the onsuspend function, the process enter frozen state
     * @tc.expected: step1. return true, the callback of the process is in the proxy list
     * @tc.steps: step2. Call the onsuspend function, the process exit frozen state
     * @tc.expected: step2. return false, the callback of the process is remove from the proxy list
     */
    backgroundProxy_->OnSuspend(request_, 0);
    bool result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(true, result);
    backgroundProxy_->OnSuspend(request_, 1);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(false, result);
}

/*
 * @tc.name: OnPermissionChanged001
 * @tc.desc: Test the function onPermissionChanged and OnDeleteRequestRecord
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, OnPermissionChanged001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call the onsuspend function, the process enter frozen state
     * @tc.steps: step2. Call onPermissionChanged, the process exit frozen state
     * @tc.expected: step2. return true, the callback of the process is in the proxy list
     */
    backgroundProxy_->OnSuspend(request_, 0);
    backgroundProxy_->OnPermissionChanged(SYSTEM_UID);
    bool result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(true, result);
    backgroundProxy_->OnDeleteRequestRecord(request_);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(false, result);
}

/*
 * @tc.name: OnSaStateChange001
 * @tc.desc: Test the function OnSaStateChange
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, OnSaStateChange001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call the onsuspend function, the process enter frozen state
     * @tc.steps: step2. Call OnSaStateChange, disable locator ability
     * @tc.expected: step2. return true, do not change proxy list
     * @tc.steps: step3. Call OnSaStateChange, enable locator ability
     * @tc.expected: step3. return true, do not change proxy list
     */
    backgroundProxy_->OnSuspend(request_, 0);
    bool result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(true, result);
    backgroundProxy_->OnSaStateChange(false);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(true, result);
    backgroundProxy_->OnSaStateChange(true);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(true, result);
    backgroundProxy_->OnDeleteRequestRecord(request_);
}
