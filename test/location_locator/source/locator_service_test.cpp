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
#include <singleton.h>

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
#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_host.h"
#endif
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "country_code_callback_host.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_host.h"
#endif
#include "i_locator.h"
#include "location.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "location_switch_callback_host.h"
#include "locator.h"
#include "locator_callback_host.h"
#include "locator_callback_proxy.h"
#include "locator_skeleton.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_host.h"
#endif

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
#ifdef FEATURE_GEOCODE_SUPPORT
const double MOCK_LATITUDE = 99.0;
const double MOCK_LONGITUDE = 100.0;
#endif
const int REQUEST_MAX_NUM = 3;
const int UNKNOWN_SERVICE_ID = -1;
const std::string ARGS_HELP = "-h";
void LocatorServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    LoadSystemAbility();
    MockNativePermission();
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
    proxy_ = nullptr;
    callbackStub_ = nullptr;
    backgroundProxy_ = nullptr;
    DelayedSingleton<LocatorBackgroundProxy>::DestroyInstance();
    requestManager_ = nullptr;
    DelayedSingleton<RequestManager>::DestroyInstance();
}

void LocatorServiceTest::LoadSystemAbility()
{
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_LOCATOR_SA_ID);
#ifdef FEATURE_GNSS_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_GNSS_SA_ID);
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_GEO_CONVERT_SA_ID);
#endif
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
    auto locatorImpl = Locator::GetInstance();
    std::unique_ptr<RequestConfig> requestConfig = RequestConfig::Unmarshalling(data);
    locatorImpl->StartLocating(requestConfig, callbackStub_);
    locatorImpl->StopLocating(callbackStub_);
    return true;
}

#ifdef FEATURE_GEOCODE_SUPPORT
std::vector<std::shared_ptr<GeocodingMockInfo>> LocatorServiceTest::SetGeocodingMockInfo()
{
    std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfos;
    std::shared_ptr<GeocodingMockInfo> geocodingMockInfo =
        std::make_shared<GeocodingMockInfo>();
    MessageParcel parcel;
    parcel.WriteString16(Str8ToStr16("locale"));
    parcel.WriteDouble(MOCK_LATITUDE); // latitude
    parcel.WriteDouble(MOCK_LONGITUDE); // longitude
    parcel.WriteInt32(1);
    parcel.WriteString("localeLanguage");
    parcel.WriteString("localeCountry");
    parcel.WriteInt32(1); // size
    parcel.WriteInt32(0); // line
    parcel.WriteString("line");
    parcel.WriteString("placeName");
    parcel.WriteString("administrativeArea");
    parcel.WriteString("subAdministrativeArea");
    parcel.WriteString("locality");
    parcel.WriteString("subLocality");
    parcel.WriteString("roadName");
    parcel.WriteString("subRoadName");
    parcel.WriteString("premises");
    parcel.WriteString("postalCode");
    parcel.WriteString("countryCode");
    parcel.WriteString("countryName");
    parcel.WriteInt32(1); // hasLatitude
    parcel.WriteDouble(MOCK_LATITUDE); // latitude
    parcel.WriteInt32(1); // hasLongitude
    parcel.WriteDouble(MOCK_LONGITUDE); // longitude
    parcel.WriteString("phoneNumber");
    parcel.WriteString("addressUrl");
    parcel.WriteBool(true);
    geocodingMockInfo->ReadFromParcel(parcel);
    geoMockInfos.emplace_back(std::move(geocodingMockInfo));
    return geoMockInfos;
}
#endif

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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, CheckSwitchState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckSwitchState001 begin");
    int result = proxy_->GetSwitchState();
    EXPECT_EQ(true, (result == ENABLED || result == DISABLED));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckSwitchState001 end");
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
    auto locatorImpl = Locator::GetInstance();
    ASSERT_TRUE(locatorImpl != nullptr);
    locatorImpl->StopLocating(callbackStub_);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckStopLocating001 end");
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
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, CheckGetCacheLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckGetCacheLocation001 begin");
    MessageParcel data;
    MessageParcel reply;
    bool ret = false;
    if (proxy_->GetSwitchState() == 1) {
        proxy_->GetCacheLocation(reply);
        ret = reply.ReadInt32() == REPLY_CODE_SECURITY_EXCEPTION;
        EXPECT_EQ(false, ret);
    } else {
        proxy_->GetCacheLocation(reply);
        ret = reply.ReadInt32() == REPLY_CODE_SECURITY_EXCEPTION;
        EXPECT_EQ(false, ret);
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] CheckGetCacheLocation001 end");
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
 * @tc.name: UpdateSaAbility001
 * @tc.desc: Test update sa ability
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, UpdateSaAbility001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test update sa ability
     * @tc.expected: step1. no exception happens
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, UpdateSaAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UpdateSaAbility001 begin");
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->UpdateSaAbility();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UpdateSaAbility001 end");
}

/*
 * @tc.name: SetEnableAndDisable001
 * @tc.desc: Test disable and enable system ability
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, SetEnableAndDisable001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.test the switch enable and disable function
     * @tc.expected: step1. switch set should be true, or setting will return error.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, SetEnableAndDisable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetEnableAndDisable001 begin");
    bool ret = false;
    if (proxy_->GetSwitchState() == 1) {
        proxy_->EnableAbility(false); // if the state is false
        ret = proxy_ -> GetSwitchState() == 0 ? true : false;
        EXPECT_EQ(true, ret);
        // reset the state
        proxy_->EnableAbility(true);
    } else {
        proxy_->EnableAbility(true); // if the state is false
        ret = proxy_ -> GetSwitchState() == 1 ? true : false;
        EXPECT_EQ(true, ret);
        // reset the state
        proxy_->EnableAbility(false);
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetEnableAndDisable001 end");
}

/*
 * @tc.name: RegisterSwitchCallback001
 * @tc.desc: Test register switch callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterSwitchCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterSwitchCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterSwitchCallback001 begin");
    pid_t callinguid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register switch callback
     * @tc.expected: log exception: "register an invalid switch callback"
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->RegisterSwitchCallback(client, callinguid);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterSwitchCallback001 end");
}

/*
 * @tc.name: RegisterSwitchCallback002
 * @tc.desc: Test register and unregister switch callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterSwitchCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the calling uid
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterAndUnregisterSwitchCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterSwitchCallback001 begin");
    pid_t callinguid = 1;

    /*
     * @tc.steps: step2. test register switch callback
     * @tc.expected: no exception happens.
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->RegisterSwitchCallback(callbackStub_->AsObject(), callinguid);

    /*
     * @tc.steps: step3. test unregister switch callback
     * @tc.steps: step4. continue to test unregister switch callback
     * @tc.expected: no exception happens.
     */
    proxy_->UnregisterSwitchCallback(callbackStub_->AsObject()); // the callback has been restored in the map

    proxy_->UnregisterSwitchCallback(callbackStub_->AsObject()); // the map is empty
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterSwitchCallback001 end");
}

/*
 * @tc.name: UnregisterSwitchCallback001
 * @tc.desc: Test unregister switch callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, UnregisterSwitchCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, UnregisterSwitchCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterSwitchCallback001 begin");
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister switch callback
     * @tc.expected: log exception: LOCATOR: unregister an invalid switch callback
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->UnregisterSwitchCallback(client);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterSwitchCallback001 end");
}

/*
 * @tc.name: RegisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback if client is null
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, RegisterNmeaMessageCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterNmeaMessageCallback001 begin");
    pid_t uid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: log info : "register an invalid nmea callback".
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->RegisterNmeaMessageCallback(client, uid);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterNmeaMessageCallback001 end");
}
#endif

/*
 * @tc.name: RegisterAndUnregisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback and then unregister twice , the first will unreg success,
 * and the second will not return error.
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is not null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterNmeaMessageCallback001 begin");
    pid_t uid = 1;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: no exception happens
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->RegisterNmeaMessageCallback(callbackStub_->AsObject(), uid);

    /*
     * @tc.steps: step3. test unregister nmea message callback
     * @tc.steps: step4. continue to test unregister nmea message callback
     * @tc.expected: no exception happens.
     */
    proxy_->UnregisterNmeaMessageCallback(callbackStub_->AsObject()); // callback in map
    proxy_->UnregisterNmeaMessageCallback(callbackStub_->AsObject()); // map is empty
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterNmeaMessageCallback001 end");
}
#endif

/*
 * @tc.name: UnregisterNmeaMessageCallback001
 * @tc.desc: Test unregister nmea message callback if client is null
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, UnregisterNmeaMessageCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, UnregisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterNmeaMessageCallback001 begin");
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister nmea message callback
     * @tc.expected: log info : "unregister an invalid nmea callback".
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->UnregisterNmeaMessageCallback(client);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterNmeaMessageCallback001 end");
}
#endif

/*
 * @tc.name: GetAddressByLocationName001
 * @tc.desc: Test get address by location name
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, GetAddressByLocationName001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, GetAddressByLocationName001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetAddressByLocationName001 begin");
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    data.WriteString16(Str8ToStr16("")); // description
    data.WriteDouble(10.0); // minLatitude
    data.WriteDouble(1.0); // minLongitude
    data.WriteDouble(10.0); // maxLatitude
    data.WriteDouble(10.0); // maxLongitude
    data.WriteInt32(10); // maxItems
    data.WriteInt32(1); // locale object size = 1
    data.WriteString16(Str8ToStr16("ZH")); // locale.getLanguage()
    data.WriteString16(Str8ToStr16("cn")); // locale.getCountry()
    data.WriteString16(Str8ToStr16("")); // locale.getVariant()
    data.WriteString16(Str8ToStr16("")); // ""

    /*
     * @tc.steps: step2. test get address by location name
     * @tc.expected: return REPLY_CODE_NO_EXCEPTION.
     */
    proxy_->GetAddressByLocationName(data, reply);
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, reply.ReadInt32());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetAddressByLocationName001 end");
}
#endif

/*
 * @tc.name: RegisterGnssStatusCallback001
 * @tc.desc: Test register gnss status callback if client is null
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, RegisterGnssStatusCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterGnssStatusCallback001 begin");
    pid_t lastCallingUid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: log info : "SendRegisterMsgToRemote callback is nullptr".
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->RegisterGnssStatusCallback(client, lastCallingUid);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterGnssStatusCallback001 end");
}
#endif

/*
 * @tc.name: RegisterAndUnregisterGnssStatusCallback001
 * @tc.desc: Test register and unregister gnss status callback if client is not null
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the last calling uid
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterGnssStatusCallback001 begin");
    pid_t lastCallingUid = 1;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: no exception happens.
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->RegisterGnssStatusCallback(callbackStub_->AsObject(), lastCallingUid);

    /*
     * @tc.steps: step3. test unregister gnss status callback
     * @tc.steps: step4. continue to test unregister gnss status callback
     * @tc.expected: no exception happens
     */
    proxy_->UnregisterGnssStatusCallback(callbackStub_->AsObject()); // callback in map
    proxy_->UnregisterGnssStatusCallback(callbackStub_->AsObject()); // map is empty
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterGnssStatusCallback001 end");
}
#endif

/*
 * @tc.name: UnregisterGnssStatusCallback001
 * @tc.desc: Test unregister gnss status callback if client is null
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, UnregisterGnssStatusCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, UnregisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterGnssStatusCallback001 begin");
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister gnss status callback
     * @tc.expected: log info : "unregister an invalid gnssStatus callback".
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->UnregisterGnssStatusCallback(client);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UnregisterGnssStatusCallback001 end");
}
#endif

/*
 * @tc.name: IsGeoConvertAvailable001
 * @tc.desc: Test geo convert available
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, IsGeoConvertAvailable001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test geo convert available .
     * @tc.expected: step1. get reply state is true.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, IsGeoConvertAvailable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsGeoConvertAvailable001 begin");
    MessageParcel reply;
    bool ret = false;
    if (proxy_->GetSwitchState() == 1) {
        proxy_->IsGeoConvertAvailable(reply);
        ret = reply.ReadInt32() == REPLY_CODE_NO_EXCEPTION ? true : false; // exception info
        // no location permission
        EXPECT_EQ(false, ret);
    } else {
        proxy_->IsGeoConvertAvailable(reply);
        ret = reply.ReadInt32() == REPLY_CODE_SWITCH_OFF_EXCEPTION ? true : false;
        // no location permission
        EXPECT_EQ(false, ret);
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsGeoConvertAvailable001 end");
}
#endif

/*
 * @tc.name: GetAddressByCoordinate001
 * @tc.desc: Test get address by coordinate
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, GetAddressByCoordinate001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build the data.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, GetAddressByCoordinate001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetAddressByCoordinate001 begin");
    MessageParcel reply;
    MessageParcel data;
    data.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    data.WriteDouble(10.5); // latitude
    data.WriteDouble(30.2); // longitude
    data.WriteInt32(10); // maxItems
    data.WriteInt32(1); // locale object size = 1
    data.WriteString16(Str8ToStr16("ZH")); // locale.getLanguage()
    data.WriteString16(Str8ToStr16("cn")); // locale.getCountry()
    data.WriteString16(Str8ToStr16("")); // locale.getVariant()
    data.WriteString16(Str8ToStr16("")); // ""

    /*
     * @tc.steps: step2. test get address by coordinate.
     * @tc.expected: step2. get reply state is true.
     */
    proxy_->GetAddressByCoordinate(data, reply);
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, reply.ReadInt32());

    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetAddressByCoordinate001 end");
}
#endif

/*
 * @tc.name: SetAndCheckLocationPrivacyConfirmStatus001
 * @tc.desc: Test set and check the status
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, SetAndCheckLocationPrivacyConfirmStatus001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. set PRIVACY_TYPE_OTHERS type status true.
     * @tc.steps: step2. set PRIVACY_TYPE_STARTUP type status false.
     * @tc.steps: step3. set PRIVACY_TYPE_CORE_LOCATION type default.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, SetAndCheckLocationPrivacyConfirmStatus001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetAndCheckLocationPrivacyConfirmStatus001 begin");
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_OTHERS, true));
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_STARTUP, false));

    /*
     * @tc.steps: step4. location privacy confirm should be true when the type is PRIVACY_TYPE_OTHERS.
     * @tc.steps: step5. location privacy confirm should be false when the type is PRIVACY_TYPE_STARTUP.
     * @tc.steps: step6. location privacy confirm should be false when the type is PRIVACY_TYPE_CORE_LOCATION.
     * @tc.steps: step7. location privacy confirm should be false when the type is invalid.
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(true, proxy_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_OTHERS));
    EXPECT_EQ(false, proxy_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_STARTUP));
    EXPECT_EQ(false, proxy_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_CORE_LOCATION));
    EXPECT_EQ(false, proxy_->IsLocationPrivacyConfirmed(-1));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetAndCheckLocationPrivacyConfirmStatus001 end");
}

/*
 * @tc.name: RegisterAndUnregisterCachedLocationCallback001
 * @tc.desc: Test register and unregister cached location callback if the params are not null.
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterCachedLocationCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the calling uid, cached call back, request config
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterAndUnregisterCachedLocationCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterCachedLocationCallback001 begin");
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    auto cachedCallbackHost = sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    std::string bundleName = "test";

    /*
     * @tc.steps: step2. test register cached location callback
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        proxy_->RegisterCachedLocationCallback(requestConfig, cachedCallback, bundleName));

    /*
     * @tc.steps: step3. test unregister cached location callback
     * @tc.steps: step4. continue to test unregister cached location callback
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        proxy_->UnregisterCachedLocationCallback(cachedCallback)); // the callback has been restored in the map
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        proxy_->UnregisterCachedLocationCallback(cachedCallback)); // the map is empty
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterAndUnregisterCachedLocationCallback001 end");
}
#endif

/*
 * @tc.name: RegisterCachedLocationCallback001
 * @tc.desc: Test register cached location callback if params are null.
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, RegisterCachedLocationCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the calling uid, cached call back, request config
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RegisterCachedLocationCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterCachedLocationCallback001 begin");
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = nullptr;
    sptr<ICachedLocationsCallback> cachedCallback = nullptr;
    std::string bundleName = "test";

    /*
     * @tc.steps: step2. test register cached location callback
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        proxy_->RegisterCachedLocationCallback(requestConfig, cachedCallback, bundleName));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RegisterCachedLocationCallback001 end");
}
#endif

/*
 * @tc.name: GetCachedGnssLocationsSize001
 * @tc.desc: Test get cached gnss location size
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, GetCachedGnssLocationsSize001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test get cached gnss location size.
     * @tc.expected: step1. get the true size.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, GetCachedGnssLocationsSize001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetCachedGnssLocationsSize001 begin");
    MessageParcel reply;
    if (proxy_->GetSwitchState() == 1) {
        EXPECT_EQ(0, proxy_->GetCachedGnssLocationsSize()); // not support now
    } else {
        EXPECT_EQ(0, proxy_->GetCachedGnssLocationsSize()); // switch is off
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetCachedGnssLocationsSize001 end");
}
#endif

/*
 * @tc.name: FlushCachedGnssLocations001
 * @tc.desc: Test flush cached gnss location
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, FlushCachedGnssLocations001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test flush cached gnss location
     * @tc.expected: step1. get the true size.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, FlushCachedGnssLocations001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] FlushCachedGnssLocations001 begin");
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->FlushCachedGnssLocations());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] FlushCachedGnssLocations001 end");
}
#endif

/*
 * @tc.name: SendCommand001
 * @tc.desc: Test send command
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, SendCommand001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build location command
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, SendCommand001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SendCommand001 begin");
    MessageParcel data;
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario = data.ReadInt32();
    locationCommand->command = data.ReadBool();

    /*
     * @tc.steps: step2. test send command.
     * @tc.expected: current function is empty, nothing happens
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->SendCommand(locationCommand);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SendCommand001 end");
}
#endif

/*
 * @tc.name: AddFence001
 * @tc.desc: Test add fence
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, AddFence001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build geo fence request
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, AddFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] AddFence001 begin");
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = 2; // scenario
    request->geofence.latitude = 35.1; // latitude
    request->geofence.longitude = 40.2; // longitude
    request->geofence.radius = 2.2; // radius
    request->geofence.expiration = 12.2; // expiration

    /*
     * @tc.steps: step2. test add fence
     * @tc.expected: no exception happens
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->AddFence(request);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] AddFence001 end");
}
#endif

/*
 * @tc.name: RemoveFence001
 * @tc.desc: Test add fence
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, RemoveFence001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build geo fence request
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RemoveFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RemoveFence001 begin");
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = 2; // scenario
    request->geofence.latitude = 35.1; // latitude
    request->geofence.longitude = 40.2; // longitude
    request->geofence.radius = 2.2; // radius
    request->geofence.expiration = 12.2; // expiration

    /*
     * @tc.steps: step2. test remove fence
     * @tc.expected: no exception happens
     */
    ASSERT_TRUE(proxy_ != nullptr);
    proxy_->RemoveFence(request);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RemoveFence001 end");
}
#endif

/*
 * @tc.name: EnableLocationMock001
 * @tc.desc: Test enable location mock in SCENE_CAR_HAILING scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, EnableLocationMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test enable location mock
     * @tc.expected: no exception happens
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, EnableLocationMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] EnableLocationMock001 begin");
    EXPECT_EQ(true, proxy_->EnableLocationMock());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] EnableLocationMock001 end");
}

/*
 * @tc.name: DisableLocationMock001
 * @tc.desc: Test disable location mock in SCENE_CAR_HAILING scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, DisableLocationMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test disable location mock
     * @tc.expected: no exception happens
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, DisableLocationMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] DisableLocationMock001 begin");
    EXPECT_EQ(true, proxy_->DisableLocationMock());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] DisableLocationMock001 end");
}

/*
 * @tc.name: SetMockedLocations001
 * @tc.desc: Test set location mock in different scenarioes
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, SetMockedLocations001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, SetMockedLocations001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetMockedLocations001 begin");
    int timeInterval = 2;
    std::vector<std::shared_ptr<Location>> mockLocationArray;
    Parcel parcel;
    for (int i = 0; i < 2; i++) {
        parcel.WriteDouble(10.6); // latitude
        parcel.WriteDouble(10.5); // longitude
        parcel.WriteDouble(10.4); // altitude
        parcel.WriteDouble(1.0); // accuracy
        parcel.WriteDouble(5.0); // speed
        parcel.WriteDouble(10); // direction
        parcel.WriteInt64(1611000000); // timestamp
        parcel.WriteInt64(1611000000); // time since boot
        parcel.WriteString16(u"additions"); // additions
        parcel.WriteInt64(1); // additionSize
        parcel.WriteBool(true); // isFromMock
        parcel.WriteInt32(1); // source type
        parcel.WriteInt32(0); // floor no.
        parcel.WriteDouble(1000.0); // floor acc
        mockLocationArray.push_back(Location::UnmarshallingShared(parcel));
    }

    /*
     * @tc.steps: step2. test set mocked locations for different scenarioes
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(true, proxy_->SetMockedLocations(timeInterval, mockLocationArray));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetMockedLocations001 end");
}

/*
 * @tc.name: EnableReverseGeocodingMock001
 * @tc.desc: Test enable reverse geocoding mock
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, EnableReverseGeocodingMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test enable reverse geocoding mock
     * @tc.expected: no exception happens
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, EnableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] EnableReverseGeocodingMock001 begin");
    EXPECT_EQ(true, proxy_->EnableReverseGeocodingMock());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] EnableReverseGeocodingMock001 end");
}
#endif

/*
 * @tc.name: DisableReverseGeocodingMock001
 * @tc.desc: Test disable reverse geocoding mock
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, DisableReverseGeocodingMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test disable reverse geocoding mock
     * @tc.expected: no exception happens
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, DisableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] DisableReverseGeocodingMock001 begin");
    EXPECT_EQ(true, proxy_->DisableReverseGeocodingMock());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] DisableReverseGeocodingMock001 end");
}
#endif

/*
 * @tc.name: SetReverseGeocodingMockInfo001
 * @tc.desc: Test set reverse geocoding mock info
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, SetReverseGeocodingMockInfo001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, SetReverseGeocodingMockInfo001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetReverseGeocodingMockInfo001 begin");
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    std::shared_ptr<GeocodingMockInfo> info = std::make_shared<GeocodingMockInfo>();
    Parcel data;
    data.WriteString16(Str8ToStr16("locale")); // locale
    data.WriteDouble(10.5); // latitude
    data.WriteDouble(10.6); // longitude
    data.WriteInt32(2); // maxItems
    info->ReadFromParcel(data);
    mockInfo.push_back(info);
    /*
     * @tc.steps: step2. test set reverse geocoding mock info
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(true, proxy_->SetReverseGeocodingMockInfo(mockInfo));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] SetReverseGeocodingMockInfo001 end");
}
#endif

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
    ASSERT_TRUE(backgroundProxy_ != nullptr);
    backgroundProxy_->UpdateListOnRequestChange(request_);

    /*
     * @tc.steps: step2. request is null
     * @tc.expected: early return because request is nullptr
     */
    backgroundProxy_->UpdateListOnRequestChange(nullptr);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] UpdateListOnRequestChange002 end");
}

HWTEST_F(LocatorServiceTest, locatorImpl001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorImpl001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImpl001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    locatorImpl->ShowNotification();
    locatorImpl->RequestPermission();
    locatorImpl->RequestEnableLocation();

    locatorImpl->EnableAbility(false);
    EXPECT_EQ(false, locatorImpl->IsLocationEnabled());
    locatorImpl->EnableAbility(true);
    EXPECT_EQ(true, locatorImpl->IsLocationEnabled());

    EXPECT_NE(nullptr, locatorImpl->GetCachedLocation());

    locatorImpl->SetLocationPrivacyConfirmStatus(1, true);
    EXPECT_EQ(true, locatorImpl->IsLocationPrivacyConfirmed(1));
    EXPECT_EQ(ERRCODE_INVALID_PARAM, locatorImpl->SetLocationPrivacyConfirmStatus(-1, true));
    EXPECT_EQ(false, locatorImpl->IsLocationPrivacyConfirmed(-1));
#ifdef FEATURE_GNSS_SUPPORT
    EXPECT_EQ(0, locatorImpl->GetCachedGnssLocationsSize());
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl->FlushCachedGnssLocations());
    std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
    command->scenario = SCENE_NAVIGATION;
    command->command = "cmd";
    EXPECT_EQ(true, locatorImpl->SendCommand(command));
    std::unique_ptr<GeofenceRequest> fenceRequest = std::make_unique<GeofenceRequest>();
    fenceRequest->scenario = SCENE_NAVIGATION;
    GeoFence geofence;
    geofence.latitude = 1.0;
    geofence.longitude = 2.0;
    geofence.radius = 3.0;
    geofence.expiration = 4.0;
    fenceRequest->geofence = geofence;
    EXPECT_EQ(true, locatorImpl->AddFence(fenceRequest));
    EXPECT_EQ(true, locatorImpl->RemoveFence(fenceRequest));
#endif
    EXPECT_NE(nullptr, locatorImpl->GetIsoCountryCode());

    EXPECT_EQ(true, locatorImpl->ProxyUidForFreeze(1000, false));

    int timeInterval = 2;
    EXPECT_EQ(true, locatorImpl->EnableLocationMock());
    std::vector<std::shared_ptr<Location>> locations;
    EXPECT_EQ(true, locatorImpl->SetMockedLocations(timeInterval, locations));
    EXPECT_EQ(true, locatorImpl->DisableLocationMock());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImpl001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, locatorImplGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorImplGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGeocodingMock001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    EXPECT_EQ(true, locatorImpl->EnableReverseGeocodingMock());
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    EXPECT_EQ(true, locatorImpl->SetReverseGeocodingMockInfo(mockInfos));
    EXPECT_EQ(true, locatorImpl->DisableReverseGeocodingMock());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGeocodingMock001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, locatorImplIsGeoServiceAvailable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorImplIsGeoServiceAvailable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplIsGeoServiceAvailable001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    EXPECT_EQ(true, locatorImpl->EnableReverseGeocodingMock());
    EXPECT_EQ(true, locatorImpl->IsGeoServiceAvailable());

    EXPECT_EQ(true, locatorImpl->DisableReverseGeocodingMock());
    EXPECT_EQ(false, locatorImpl->IsGeoServiceAvailable());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplIsGeoServiceAvailable001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, locatorImplGetAddressByCoordinate001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorImplGetAddressByCoordinate001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGetAddressByCoordinate001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    MessageParcel request001;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList001;
    EXPECT_EQ(true, locatorImpl->EnableReverseGeocodingMock());

    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    EXPECT_EQ(true, locatorImpl->SetReverseGeocodingMockInfo(mockInfos));
    request001.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request001.WriteDouble(MOCK_LATITUDE); // latitude
    request001.WriteDouble(MOCK_LONGITUDE); // longitude
    request001.WriteInt32(3); // maxItems
    request001.WriteInt32(1); // locale object size = 1
    request001.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request001.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request001.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request001.WriteString16(Str8ToStr16("")); // ""
    locatorImpl->GetAddressByCoordinate(request001, geoAddressList001);
    EXPECT_EQ(false, geoAddressList001.empty());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGetAddressByCoordinate001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, locatorImplGetAddressByCoordinate002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorImplGetAddressByCoordinate002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGetAddressByCoordinate002 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    MessageParcel request002;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList002;
    EXPECT_EQ(true, locatorImpl->DisableReverseGeocodingMock());
    request002.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request002.WriteDouble(1.0); // latitude
    request002.WriteDouble(2.0); // longitude
    request002.WriteInt32(3); // maxItems
    request002.WriteInt32(1); // locale object size = 1
    request002.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request002.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request002.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request002.WriteString16(Str8ToStr16("")); // ""
    locatorImpl->GetAddressByCoordinate(request002, geoAddressList002);
    EXPECT_EQ(true, geoAddressList002.empty());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGetAddressByCoordinate002 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, locatorImplGetAddressByLocationName001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorImplGetAddressByLocationName001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGetAddressByLocationName001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    MessageParcel request003;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList003;
    request003.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request003.WriteString16(Str8ToStr16("description")); // description
    request003.WriteDouble(1.0); // minLatitude
    request003.WriteDouble(2.0); // minLongitude
    request003.WriteDouble(3.0); // maxLatitude
    request003.WriteDouble(4.0); // maxLongitude
    request003.WriteInt32(3); // maxItems
    request003.WriteInt32(1); // locale object size = 1
    request003.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request003.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request003.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request003.WriteString16(Str8ToStr16("")); // ""
    locatorImpl->GetAddressByLocationName(request003, geoAddressList003);
    EXPECT_EQ(true, geoAddressList003.empty());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplGetAddressByLocationName001 end");
}
#endif

HWTEST_F(LocatorServiceTest, locatorImplRegisterAndUnregisterCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorImplRegisterAndUnregisterCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplRegisterAndUnregisterCallback001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject()));
#ifdef FEATURE_GNSS_SUPPORT
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_NE(nullptr, gnssCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject()));
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_NE(nullptr, nmeaCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject()));
#endif
    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    EXPECT_NE(nullptr, countryCodeCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterCountryCodeCallback(countryCodeCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterCountryCodeCallback(countryCodeCallbackHost->AsObject()));
#ifdef FEATURE_GNSS_SUPPORT
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    EXPECT_NE(nullptr, cachedLocationsCallbackHost);
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    EXPECT_NE(nullptr, cachedCallback);
    auto request = std::make_unique<CachedGnssLocationsRequest>();
    EXPECT_NE(nullptr, request);
    request->reportingPeriodSec = 10;
    request->wakeUpCacheQueueFull = true;
    locatorImpl->RegisterCachedLocationCallback(request, cachedCallback);
    locatorImpl->UnregisterCachedLocationCallback(cachedCallback);
#endif
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorImplRegisterAndUnregisterCallback001 end");
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
    EXPECT_EQ(true, locatorAbility->isActionRegistered);
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
    ASSERT_TRUE(locatorAbility != nullptr);
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

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, locatorServiceCallbackRegAndUnreg001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceCallbackRegAndUnreg001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceCallbackRegAndUnreg001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    auto cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
    // uid pid not match locationhub process
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED,
        locatorAbility->RegisterCachedLocationCallback(cachedRequest, cachedCallback, "unit.test"));

    sleep(1);
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED,
        locatorAbility->UnregisterCachedLocationCallback(cachedCallback)); // uid pid not match locationhub process
    sleep(1);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceCallbackRegAndUnreg001 end");
}
#endif

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

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, locatorServiceGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGnssStatusCallback001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());

    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    // uid pid not match locationhub process
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED,
        locatorAbility->RegisterGnssStatusCallback(nullptr, SYSTEM_UID)); // invalid callback
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->RegisterGnssStatusCallback(gnssCallbackHost, SYSTEM_UID));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->UnregisterGnssStatusCallback(nullptr)); // invalid callback
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->UnregisterGnssStatusCallback(gnssCallbackHost));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGnssStatusCallback001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, locatorServiceNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceNmeaMessageCallback001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED,
        locatorAbility->RegisterNmeaMessageCallback(nullptr, SYSTEM_UID)); // invalid callback
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->RegisterNmeaMessageCallback(nmeaCallbackHost, SYSTEM_UID));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->UnregisterNmeaMessageCallback(nullptr)); // invalid callback
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->UnregisterNmeaMessageCallback(nmeaCallbackHost));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceNmeaMessageCallback001 end");
}
#endif

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

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, locatorServiceSendCommand001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceSendCommand001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSendCommand001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
    command->scenario = SCENE_NAVIGATION;
    command->command = "cmd";
    // uid pid not match locationhub process
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->SendCommand(command));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSendCommand001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, locatorServiceFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceFence001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::unique_ptr<GeofenceRequest> fenceRequest = std::make_unique<GeofenceRequest>();
    fenceRequest->scenario = SCENE_NAVIGATION;
    GeoFence geofence;
    geofence.latitude = 1.0;
    geofence.longitude = 2.0;
    geofence.radius = 3.0;
    geofence.expiration = 4.0;
    fenceRequest->geofence = geofence;
    // uid pid not match locationhub process
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->AddFence(fenceRequest));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorAbility->RemoveFence(fenceRequest));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceFence001 end");
}
#endif

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

HWTEST_F(LocatorServiceTest, LocatorAbilityGetProxyMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, LocatorAbilityGetProxyMap001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityGetProxyMap001 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    locatorAbility->GetProxyMap();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityGetProxyMap001 end");
}
}  // namespace Location
}  // namespace OHOS
