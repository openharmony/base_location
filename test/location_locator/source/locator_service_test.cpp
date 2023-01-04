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

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "app_identity.h"
#include "cached_locations_callback_host.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "country_code_callback_host.h"
#include "geo_address.h"
#include "gnss_status_callback_host.h"
#include "i_locator.h"
#include "location.h"
#include "location_log.h"
#include "location_switch_callback_host.h"
#include "locator.h"
#include "locator_ability.h"
#include "locator_callback_host.h"
#include "locator_callback_proxy.h"
#include "locator_skeleton.h"
#include "nmea_message_callback_host.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const int REQUEST_MAX_NUM = 3;
const int UNKNOWN_SERVICE_ID = -1;
const std::string ARGS_HELP = "-h";
void LocatorServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    locatorImpl_ = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl_);
    callbackStub_ = new (std::nothrow) LocatorCallbackStub();
    EXPECT_NE(nullptr, callbackStub_);
    backgroundProxy_ = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    EXPECT_NE(nullptr, backgroundProxy_);
    request_ = std::make_shared<Request>();
    EXPECT_NE(nullptr, request_);
    requestManager_ = DelayedSingleton<RequestManager>::GetInstance();
    EXPECT_NE(nullptr, requestManager_);
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
    callbackStub_ = nullptr;
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
        .processName = "LocatorTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
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
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    std::unique_ptr<RequestConfig> requestConfig = RequestConfig::Unmarshalling(data);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl->StartLocating(requestConfig, callbackStub_));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl->StopLocating(callbackStub_));
    return true;
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, CheckLocatingForScenario001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckLocatingForScenario001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetScenario(SCENE_NAVIGATION);
    MessageParcel data;
    requestConfig->Marshalling(data);
    bool ret = StartAndStopForLocating(data);
    EXPECT_EQ(true, ret);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckLocatingForScenario001 end");
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, CheckLocatingForConfig001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckLocatingForConfig001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    MessageParcel data;
    requestConfig->Marshalling(data);
    bool ret = StartAndStopForLocating(data);
    EXPECT_EQ(true, ret);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckLocatingForConfig001 end");
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, CheckStopLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckStopLocating001 begin");
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl->StopLocating(callbackStub_));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckStopLocating001 end");
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, IsCallbackInProxyTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsCallbackInProxyTest001 begin");
    bool result =  backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsCallbackInProxyTest001 end");
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, OnSuspendTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] OnSuspendTest001 begin");
    backgroundProxy_->OnSuspend(request_, 0);
    bool result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    // no location permission
    EXPECT_EQ(false, result);
    backgroundProxy_->OnSuspend(request_, 1);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] OnSuspendTest001 end");
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, OnPermissionChanged001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] OnPermissionChanged001 begin");
    backgroundProxy_->OnSuspend(request_, 0);
    requestManager_->HandlePermissionChanged(IPCSkeleton::GetCallingTokenID());
    bool result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    // no location permission
    EXPECT_EQ(false, result);
    backgroundProxy_->OnDeleteRequestRecord(request_);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] OnPermissionChanged001 end");
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, OnSaStateChange001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] OnSaStateChange001 begin");
    backgroundProxy_->OnSuspend(request_, 0);
    bool result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    // no location permission
    EXPECT_EQ(false, result);
    backgroundProxy_->OnSaStateChange(false);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    // no location permission
    EXPECT_EQ(false, result);
    backgroundProxy_->OnSaStateChange(true);
    result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    // no location permission
    EXPECT_EQ(false, result);
    backgroundProxy_->OnDeleteRequestRecord(request_);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] OnSaStateChange001 end");
}

/*
 * @tc.name: CheckPermission001
 * @tc.desc: Test the function CheckPermission
 * @tc.type: FUNC
 * @tc.require: issueI5OSHX
 */
HWTEST_F(LocatorServiceTest, CheckPermission001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. get callingTokenId and callingFirstTokenid.
     * @tc.steps: step2. Call GetPermissionLevel and get permission level.
     * @tc.expected: step1. get permission level is PERMISSION_ACCURATE.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, CheckPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckPermission001 begin");
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t callingFirstTokenid = IPCSkeleton::GetFirstTokenID();
    int permissionLevel = CommonUtils::GetPermissionLevel(callingTokenId, callingFirstTokenid);
    EXPECT_EQ(PERMISSION_ACCURATE, permissionLevel);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckPermission001 end");
}

/*
 * @tc.name: RegisterAppStateObserver001
 * @tc.desc: Test the function register app state observer
 * @tc.type: FUNC
 * @tc.require: issueI5PX7W
 */
HWTEST_F(LocatorServiceTest, RegisterAppStateObserver001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. get the request manager
     * @tc.steps: step2. register app state observer
     * @tc.expected: return false, permission denied
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterAppStateObserver001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAppStateObserver001 begin");
    bool ret = backgroundProxy_->RegisterAppStateObserver();
    EXPECT_EQ(true, ret);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAppStateObserver001 end");
}

/*
 * @tc.name: UnregisterAppStateObserver001
 * @tc.desc: Test the function unregister app state observer
 * @tc.type: FUNC
 * @tc.require: issueI5PX7W
 */
HWTEST_F(LocatorServiceTest, UnregisterAppStateObserver001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. get the request manager
     * @tc.steps: step2. unregister app state observer
     * @tc.expected: return true, unreg process is success
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, UnregisterAppStateObserver001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterAppStateObserver001 begin");
    bool ret = backgroundProxy_->UnregisterAppStateObserver();
    EXPECT_EQ(true, ret);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterAppStateObserver001 end");
}

/*
 * @tc.name: UpdateListOnRequestChange001
 * @tc.desc: Test update list on request change in normal scenario
 * @tc.type: FUNC
 * @tc.require: issueI5PX7W
 */
HWTEST_F(LocatorServiceTest, UpdateListOnRequestChange001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. get user id
     * @tc.steps: step2. get uid by bundle name and userId
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, UpdateListOnRequestChange001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UpdateListOnRequestChange001 begin");
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

    request_->SetUid(uid);
    request_->SetPackageName(name);

    /*
     * @tc.steps: step3. test update list on request change function
     * @tc.expected: normal scenario covered
     */
    backgroundProxy_->UpdateListOnRequestChange(request_);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UpdateListOnRequestChange001 end");
}

/*
 * @tc.name: UpdateListOnRequestChange002
 * @tc.desc: Test update list on request change in abnormal scenario
 * @tc.type: FUNC
 * @tc.require: issueI5PX7W
 */
HWTEST_F(LocatorServiceTest, UpdateListOnRequestChange002, TestSize.Level1)
{
    /*
     * @tc.steps: step1. cannot find bundle name if uid is not restored
     * @tc.expected: early return because bundleName can not be found
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, UpdateListOnRequestChange002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UpdateListOnRequestChange002 begin");
    backgroundProxy_->UpdateListOnRequestChange(request_);

    /*
     * @tc.steps: step2. request is null
     * @tc.expected: early return because request is nullptr
     */
    backgroundProxy_->UpdateListOnRequestChange(nullptr);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UpdateListOnRequestChange002 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceStartAndStop001, TestSize.Level1)
{
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->OnStart();
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, locatorAbility->QueryServiceState());
    locatorAbility->OnStart(); // after state running

    locatorAbility->OnStop();
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, locatorAbility->QueryServiceState());
    locatorAbility->OnStart(); // after stop
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, locatorAbility->QueryServiceState());
}

HWTEST_F(LocatorServiceTest, locatorServiceStartAndStopSA001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceStartAndStopSA001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceStartAndStopSA001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->OnAddSystemAbility(UNKNOWN_SERVICE_ID, "device-id");

    locatorAbility->RegisterAction();
    locatorAbility->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "device-id");

    locatorAbility->OnRemoveSystemAbility(UNKNOWN_SERVICE_ID, "device-id");
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceStartAndStopSA001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceInitSaAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceInitSaAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitSaAbility001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->InitSaAbility();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitSaAbility001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceInitRequestManager001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceInitRequestManager001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitRequestManager001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->InitRequestManagerMap();
    EXPECT_EQ(REQUEST_MAX_NUM, locatorAbility->GetRequests()->size());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitRequestManager001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceUpdateSaAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceUpdateSaAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceUpdateSaAbility001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->UpdateSaAbility());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceUpdateSaAbility001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceEnableAndDisable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceEnableAndDisable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceEnableAndDisable001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    int state = DISABLED;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->EnableAbility(true));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    EXPECT_EQ(ENABLED, state);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->EnableAbility(false));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    EXPECT_EQ(DISABLED, state);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->EnableAbility(true));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    EXPECT_EQ(ENABLED, state);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceEnableAndDisable001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceSwitchCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceSwitchCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSwitchCallback001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    locatorAbility->OnStart();
    EXPECT_EQ(ERRCODE_INVALID_PARAM, locatorAbility->RegisterSwitchCallback(nullptr, SYSTEM_UID));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->RegisterSwitchCallback(switchCallbackHost, SYSTEM_UID));
    EXPECT_EQ(ERRCODE_INVALID_PARAM, locatorAbility->UnregisterSwitchCallback(nullptr));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->UnregisterSwitchCallback(switchCallbackHost));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSwitchCallback001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceCountryCodeCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceCountryCodeCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceCountryCodeCallback001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    locatorAbility->OnStart();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->RegisterCountryCodeCallback(nullptr, SYSTEM_UID));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->RegisterCountryCodeCallback(countryCodeCallbackHost, SYSTEM_UID));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->UnregisterCountryCodeCallback(nullptr));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->UnregisterCountryCodeCallback(countryCodeCallbackHost));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceCountryCodeCallback001 end");
}

HWTEST_F(LocatorServiceTest, locatorServicePrivacyConfirmStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServicePrivacyConfirmStatus001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePrivacyConfirmStatus001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    EXPECT_EQ(ERRCODE_SUCCESS,
        locatorAbility->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_STARTUP, true));
    bool isConfirmed = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->IsLocationPrivacyConfirmed(PRIVACY_TYPE_STARTUP, isConfirmed));
    EXPECT_EQ(true, isConfirmed);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePrivacyConfirmStatus001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceIsoCountryCode001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceIsoCountryCode001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceIsoCountryCode001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->OnStart();
    std::shared_ptr<CountryCode> countryCode;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetIsoCountryCode(countryCode));
    EXPECT_NE(nullptr, countryCode);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceIsoCountryCode001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceLocationMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceLocationMock001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    int timeInterval = 2;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->EnableLocationMock());
    std::vector<std::shared_ptr<Location>> locations;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->SetMockedLocations(timeInterval, locations));

    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->DisableLocationMock());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->SetMockedLocations(timeInterval, locations));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceLocationMock001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceReportLocationStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceReportLocationStatus001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportLocationStatus001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    int state = DISABLED;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    if (state == ENABLED) {
        EXPECT_EQ(REPLY_CODE_NO_EXCEPTION, locatorAbility->ReportLocationStatus(callbackStub_, 0));
    } else {
        EXPECT_EQ(REPLY_CODE_EXCEPTION, locatorAbility->ReportLocationStatus(callbackStub_, 0));
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportLocationStatus001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceReportErrorStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceReportErrorStatus001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportErrorStatus001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    int state = DISABLED;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    if (state == ENABLED) {
        EXPECT_EQ(REPLY_CODE_NO_EXCEPTION, locatorAbility->ReportErrorStatus(callbackStub_, 0));
    } else {
        EXPECT_EQ(REPLY_CODE_EXCEPTION, locatorAbility->ReportErrorStatus(callbackStub_, 0));
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportErrorStatus001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceGetReceivers001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceGetReceivers001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGetReceivers001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    EXPECT_EQ(0, locatorAbility->GetReceivers()->size());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGetReceivers001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceProxyUidForFreeze001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceProxyUidForFreeze001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceProxyUidForFreeze001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->ProxyUidForFreeze(SYSTEM_UID, true));
    EXPECT_EQ(true, locatorAbility->IsProxyUid(SYSTEM_UID));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->ProxyUidForFreeze(SYSTEM_UID, false));
    EXPECT_EQ(false, locatorAbility->IsProxyUid(SYSTEM_UID));

    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->ProxyUidForFreeze(SYSTEM_UID, true));
    EXPECT_EQ(true, locatorAbility->IsProxyUid(SYSTEM_UID));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->ResetAllProxy());
    EXPECT_EQ(false, locatorAbility->IsProxyUid(SYSTEM_UID));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceProxyUidForFreeze001 end");
}

HWTEST_F(LocatorServiceTest, LocatorAbilityStubDump001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, LocatorAbilityStubDump001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityStubDump001 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    int32_t fd = 0;
    std::vector<std::u16string> args;
    std::u16string arg1 = Str8ToStr16("arg1");
    args.emplace_back(arg1);
    std::u16string arg2 = Str8ToStr16("arg2");
    args.emplace_back(arg2);
    std::u16string arg3 = Str8ToStr16("arg3");
    args.emplace_back(arg3);
    std::u16string arg4 = Str8ToStr16("arg4");
    args.emplace_back(arg4);
    EXPECT_EQ(ERR_OK, locatorAbility->Dump(fd, args));

    std::vector<std::u16string> emptyArgs;
    EXPECT_EQ(ERR_OK, locatorAbility->Dump(fd, emptyArgs));

    std::vector<std::u16string> helpArgs;
    std::u16string helpArg1 = Str8ToStr16(ARGS_HELP);
    helpArgs.emplace_back(helpArg1);
    EXPECT_EQ(ERR_OK, locatorAbility->Dump(fd, helpArgs));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityStubDump001 end");
}
}  // namespace Location
}  // namespace OHOS