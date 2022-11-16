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
void LocatorServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
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
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
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
    pid_t pid = 0;
    pid_t uid = 0;
    std::unique_ptr<RequestConfig> requestConfig = RequestConfig::Unmarshalling(data);
    proxy_->StartLocating(requestConfig, callbackStub_, "ohos.unit", pid, uid);
    bool ret = (proxy_->StopLocating(callbackStub_)) != REPLY_CODE_SECURITY_EXCEPTION ? true : false;
    return ret;
}

std::vector<std::shared_ptr<GeocodingMockInfo>> LocatorServiceTest::SetGeocodingMockInfo()
{
    std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfos;
    std::shared_ptr<GeocodingMockInfo> geocodingMockInfo =
        std::make_shared<GeocodingMockInfo>();
    MessageParcel parcel;
    parcel.WriteString16(Str8ToStr16("locale"));
    parcel.WriteDouble(99.0); // latitude
    parcel.WriteDouble(100.0); // longitude
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
    parcel.WriteDouble(99.0); // latitude
    parcel.WriteInt32(1); // hasLongitude
    parcel.WriteDouble(100.0); // longitude
    parcel.WriteString("phoneNumber");
    parcel.WriteString("addressUrl");
    parcel.WriteBool(true);
    geocodingMockInfo->ReadFromParcel(parcel);
    geoMockInfos.emplace_back(std::move(geocodingMockInfo));
    return geoMockInfos;
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
    bool ret = (proxy_->StopLocating(callbackStub_) != REPLY_CODE_SECURITY_EXCEPTION) ? true : false;
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
        proxy_->GetCacheLocation(reply);
        ret = reply.ReadInt32() == REPLY_CODE_SECURITY_EXCEPTION;
        EXPECT_EQ(false, ret);
    } else {
        proxy_->GetCacheLocation(reply);
        ret = reply.ReadInt32() == REPLY_CODE_SECURITY_EXCEPTION;
        EXPECT_EQ(false, ret);
    }
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
    // no location permission
    EXPECT_EQ(false, result);
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
    requestManager_->HandlePermissionChanged(IPCSkeleton::GetCallingTokenID());
    bool result = backgroundProxy_->IsCallbackInProxy(callbackStub_);
    // no location permission
    EXPECT_EQ(false, result);
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
    proxy_->UpdateSaAbility();
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
    pid_t callinguid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register switch callback
     * @tc.expected: log exception: "register an invalid switch callback"
     */
    proxy_->RegisterSwitchCallback(client, callinguid);
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
    pid_t callinguid = 1;

    /*
     * @tc.steps: step2. test register switch callback
     * @tc.expected: no exception happens.
     */
    proxy_->RegisterSwitchCallback(callbackStub_->AsObject(), callinguid);

    /*
     * @tc.steps: step3. test unregister switch callback
     * @tc.steps: step4. continue to test unregister switch callback
     * @tc.expected: no exception happens.
     */
    proxy_->UnregisterSwitchCallback(callbackStub_->AsObject());

    proxy_->UnregisterSwitchCallback(callbackStub_->AsObject());
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
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister switch callback
     * @tc.expected: log exception: LOCATOR: unregister an invalid switch callback
     */
    proxy_->UnregisterSwitchCallback(client);
}

/*
 * @tc.name: RegisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterNmeaMessageCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    pid_t uid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: log info : "register an invalid nmea callback".
     */
    proxy_->RegisterNmeaMessageCallback(client, uid);
}

/*
 * @tc.name: RegisterAndUnregisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback and then unregister twice , the first will unreg success,
 * and the second will not return error.
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is not null.
     */
    pid_t uid = 1;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: no exception happens
     */
    proxy_->RegisterNmeaMessageCallback(callbackStub_->AsObject(), uid);

    /*
     * @tc.steps: step3. test unregister nmea message callback
     * @tc.steps: step4. continue to test unregister nmea message callback
     * @tc.expected: no exception happens.
     */
    proxy_->UnregisterNmeaMessageCallback(callbackStub_->AsObject());
    proxy_->UnregisterNmeaMessageCallback(callbackStub_->AsObject());
}

/*
 * @tc.name: UnregisterNmeaMessageCallback001
 * @tc.desc: Test unregister nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, UnregisterNmeaMessageCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister nmea message callback
     * @tc.expected: log info : "unregister an invalid nmea callback".
     */
    proxy_->UnregisterNmeaMessageCallback(client);
}

/*
 * @tc.name: GetAddressByLocationName001
 * @tc.desc: Test get address by location name
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, GetAddressByLocationName001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    MessageParcel data;
    MessageParcel reply;
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
    bool ret = false;
    if (proxy_->GetSwitchState() == 1) {
        proxy_->GetAddressByLocationName(data, reply);
        ret = (reply.ReadInt32() == REPLY_CODE_NO_EXCEPTION) ? true : false;
        EXPECT_TRUE(ret);
    } else {
        proxy_->GetAddressByLocationName(data, reply);
        ret = reply.ReadInt32() == REPLY_CODE_SWITCH_OFF_EXCEPTION ? true : false;
        // no location permission
        EXPECT_EQ(false, ret);
    }
}

/*
 * @tc.name: RegisterGnssStatusCallback001
 * @tc.desc: Test register gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterGnssStatusCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    pid_t lastCallingUid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: log info : "SendRegisterMsgToRemote callback is nullptr".
     */
    proxy_->RegisterGnssStatusCallback(client, lastCallingUid);
}

/*
 * @tc.name: RegisterAndUnregisterGnssStatusCallback001
 * @tc.desc: Test register and unregister gnss status callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the last calling uid
     */
    pid_t lastCallingUid = 1;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: no exception happens.
     */
    proxy_->RegisterGnssStatusCallback(callbackStub_->AsObject(), lastCallingUid);

    /*
     * @tc.steps: step3. test unregister gnss status callback
     * @tc.steps: step4. continue to test unregister gnss status callback
     * @tc.expected: no exception happens
     */
    proxy_->UnregisterGnssStatusCallback(callbackStub_->AsObject());
    proxy_->UnregisterGnssStatusCallback(callbackStub_->AsObject());
}

/*
 * @tc.name: UnregisterGnssStatusCallback001
 * @tc.desc: Test unregister gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, UnregisterGnssStatusCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister gnss status callback
     * @tc.expected: log info : "unregister an invalid gnssStatus callback".
     */
    proxy_->UnregisterGnssStatusCallback(client);
}

/*
 * @tc.name: IsGeoConvertAvailable001
 * @tc.desc: Test geo convert available
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, IsGeoConvertAvailable001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test geo convert available .
     * @tc.expected: step1. get reply state is true.
     */
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
}

/*
 * @tc.name: GetAddressByCoordinate001
 * @tc.desc: Test get address by coordinate
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, GetAddressByCoordinate001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build the data.
     */
    MessageParcel reply;
    MessageParcel data;
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
    bool ret = false;
    if (proxy_->GetSwitchState() == 1) {
        proxy_->GetAddressByCoordinate(data, reply);
        ret = reply.ReadInt32() == REPLY_CODE_NO_EXCEPTION ? true : false;
        // invaild token
        EXPECT_EQ(true, ret);
    } else {
        proxy_->GetAddressByCoordinate(data, reply);
        ret = reply.ReadInt32() == REPLY_CODE_SWITCH_OFF_EXCEPTION ? true : false;
        /// invaild token
        EXPECT_EQ(false, ret);
    }
}

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
    proxy_->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_OTHERS, true);
    proxy_->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_STARTUP, false);

    /*
     * @tc.steps: step4. location privacy confirm should be true when the type is PRIVACY_TYPE_OTHERS.
     * @tc.steps: step5. location privacy confirm should be false when the type is PRIVACY_TYPE_STARTUP.
     * @tc.steps: step6. location privacy confirm should be false when the type is PRIVACY_TYPE_CORE_LOCATION.
     * @tc.steps: step7. location privacy confirm should be false when the type is invalid.
     * @tc.expected: no exception happens
     */
    proxy_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_OTHERS);
    proxy_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_STARTUP);
    proxy_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_CORE_LOCATION);
    proxy_->IsLocationPrivacyConfirmed(-1);
}

/*
 * @tc.name: RegisterCountryCodeCallback001
 * @tc.desc: Test register country code callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterCountryCodeCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.the client is null.
     */
    pid_t callinguid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register country code callback
     * @tc.expected: log info : "RegisterCountryCodeCallback countryCodeManager_ is nullptr".
     */
    proxy_->RegisterCountryCodeCallback(client, callinguid);
}

/*
 * @tc.name: RegisterAndUnregisterCountryCodeCallback001
 * @tc.desc: Test register and unregister country code callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterCountryCodeCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the last calling uid
     */
    pid_t callinguid = 1;

    /*
     * @tc.steps: step2. test register country code callback
     * @tc.expected: no exception happens.
     */
    proxy_->RegisterCountryCodeCallback(callbackStub_->AsObject(), callinguid);

    /*
     * @tc.steps: step3. test unregister country code callback
     * @tc.steps: step4. continue to test unregister country code callback
     * @tc.expected: no exception happens
     */
    proxy_->UnregisterCountryCodeCallback(callbackStub_->AsObject());
    proxy_->UnregisterCountryCodeCallback(callbackStub_->AsObject());
}

/*
 * @tc.name: UnregisterCountryCodeCallback001
 * @tc.desc: Test unregister country code callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, UnregisterCountryCodeCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. the client is null.
     */
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister country code callback
     * @tc.expected: log exception : "UnregisterCountryCodeCallback countryCodeManager_ is nullptr".
     */
    proxy_->UnregisterCountryCodeCallback(client);
}

/*
 * @tc.name: RegisterAndUnregisterCachedLocationCallback001
 * @tc.desc: Test register and unregister cached location callback if the params are not null.
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterAndUnregisterCachedLocationCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the calling uid, cached call back, request config
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    auto cachedCallbackHost = sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    std::string bundleName = "test";

    /*
     * @tc.steps: step2. test register cached location callback
     * @tc.expected: no exception happens.
     */
    proxy_->RegisterCachedLocationCallback(requestConfig, cachedCallback, bundleName);

    /*
     * @tc.steps: step3. test unregister cached location callback
     * @tc.steps: step4. continue to test unregister cached location callback
     * @tc.expected: no exception happens
     */
    proxy_->UnregisterCachedLocationCallback(cachedCallback);
    proxy_->UnregisterCachedLocationCallback(cachedCallback);
}

/*
 * @tc.name: RegisterCachedLocationCallback001
 * @tc.desc: Test register cached location callback if params are null.
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RegisterCachedLocationCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. give the calling uid, cached call back, request config
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = nullptr;
    sptr<ICachedLocationsCallback> cachedCallback = nullptr;
    std::string bundleName = "test";

    /*
     * @tc.steps: step2. test register cached location callback
     * @tc.expected: no exception happens.
     */
    proxy_->RegisterCachedLocationCallback(requestConfig, cachedCallback, bundleName);
}

/*
 * @tc.name: GetCachedGnssLocationsSize001
 * @tc.desc: Test get cached gnss location size
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, GetCachedGnssLocationsSize001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test get cached gnss location size.
     * @tc.expected: step1. get the true size.
     */
    MessageParcel reply;
    if (proxy_->GetSwitchState() == 1) {
        proxy_->GetCachedGnssLocationsSize();
    } else {
        proxy_->GetCachedGnssLocationsSize();
    }
}

/*
 * @tc.name: FlushCachedGnssLocations001
 * @tc.desc: Test flush cached gnss location
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, FlushCachedGnssLocations001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test flush cached gnss location
     * @tc.expected: step1. get the true size.
     */
    proxy_->FlushCachedGnssLocations();
}

/*
 * @tc.name: SendCommand001
 * @tc.desc: Test send command
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, SendCommand001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build location command
     */
    MessageParcel data;
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario = data.ReadInt32();
    locationCommand->command = data.ReadBool();

    /*
     * @tc.steps: step2. test send command.
     * @tc.expected: current function is empty, nothing happens
     */
    proxy_->SendCommand(locationCommand);
}

/*
 * @tc.name: AddFence001
 * @tc.desc: Test add fence
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, AddFence001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build geo fence request
     */
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->priority = 1; // priority
    request->scenario = 2; // scenario
    request->geofence.latitude = 35.1; // latitude
    request->geofence.longitude = 40.2; // longitude
    request->geofence.radius = 2.2; // radius
    request->geofence.expiration = 12.2; // expiration

    /*
     * @tc.steps: step2. test add fence
     * @tc.expected: no exception happens
     */
    proxy_->AddFence(request);
}

/*
 * @tc.name: RemoveFence001
 * @tc.desc: Test add fence
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, RemoveFence001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build geo fence request
     */
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->priority = 1; // priority
    request->scenario = 2; // scenario
    request->geofence.latitude = 35.1; // latitude
    request->geofence.longitude = 40.2; // longitude
    request->geofence.radius = 2.2; // radius
    request->geofence.expiration = 12.2; // expiration

    /*
     * @tc.steps: step2. test remove fence
     * @tc.expected: no exception happens
     */
    proxy_->RemoveFence(request);
}

/*
 * @tc.name: GetIsoCountryCode001
 * @tc.desc: Test get iso country code
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, GetIsoCountryCode001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test get iso country code
     * @tc.expected: no exception happens
     */
    MessageParcel reply;
    auto country = proxy_->GetIsoCountryCode();
    EXPECT_EQ("CN", country->GetCountryCodeStr());
    EXPECT_EQ(COUNTRY_CODE_FROM_LOCALE, country->GetCountryCodeType());
}

/*
 * @tc.name: EnableLocationMock001
 * @tc.desc: Test enable location mock in SCENE_CAR_HAILING scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, EnableLocationMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_CAR_HAILING); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test enable location mock
     * @tc.expected: no exception happens
     */
    proxy_->EnableLocationMock(mockInfo);
}

/*
 * @tc.name: EnableLocationMock002
 * @tc.desc: Test enable location mock in SCENE_DAILY_LIFE_SERVICE scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, EnableLocationMock002, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_DAILY_LIFE_SERVICE); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test enable location mock
     * @tc.expected: no exception happens
     */
    proxy_->EnableLocationMock(mockInfo);
}

/*
 * @tc.name: EnableLocationMock003
 * @tc.desc: Test enable location mock in SCENE_NO_POWER scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, EnableLocationMock003, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_NO_POWER); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test enable location mock
     * @tc.expected: no exception happens
     */
    proxy_->EnableLocationMock(mockInfo);
}

/*
 * @tc.name: EnableLocationMock004
 * @tc.desc: Test enable location mock in SCENE_UNSET scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, EnableLocationMock004, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_UNSET); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test enable location mock
     * @tc.expected: no exception happens
     */
    proxy_->EnableLocationMock(mockInfo);
}

/*
 * @tc.name: DisableLocationMock001
 * @tc.desc: Test disable location mock in SCENE_CAR_HAILING scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, DisableLocationMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_CAR_HAILING); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test disable location mock
     * @tc.expected: no exception happens
     */
    proxy_->DisableLocationMock(mockInfo);
}

/*
 * @tc.name: DisableLocationMock002
 * @tc.desc: Test disable location mock in SCENE_DAILY_LIFE_SERVICE scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, DisableLocationMock002, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_DAILY_LIFE_SERVICE); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test disable location mock
     * @tc.expected: no exception happens
     */
    proxy_->DisableLocationMock(mockInfo);
}

/*
 * @tc.name: DisableLocationMock003
 * @tc.desc: Test disable location mock in SCENE_NO_POWER scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, DisableLocationMock003, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_NO_POWER); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test disable location mock
     * @tc.expected: no exception happens
     */
    proxy_->DisableLocationMock(mockInfo);
}

/*
 * @tc.name: DisableLocationMock004
 * @tc.desc: Test disable location mock in SCENE_UNSET scenario
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, DisableLocationMock004, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_UNSET); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    /*
     * @tc.steps: step1. test disable location mock
     * @tc.expected: no exception happens
     */
    proxy_->DisableLocationMock(mockInfo);
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
    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_CAR_HAILING); // scenario
    mockInfo.SetTimeInterval(2); // time interval

    std::vector<std::shared_ptr<Location>> mockLocationArray;
    Parcel parcel;
    for (int i = 0; i < 2; i++) {
        parcel.WriteDouble(10.6); // latitude
        parcel.WriteDouble(10.5); // longitude
        parcel.WriteDouble(10.4); // altitude
        parcel.WriteFloat(1.0); // accuracy
        parcel.WriteFloat(5.0); // speed
        parcel.WriteDouble(10); // direction
        parcel.WriteInt64(1611000000); // timestamp
        parcel.WriteInt64(1611000000); // time since boot
        parcel.WriteString("additions"); // additions
        parcel.WriteInt64(1); // additionSize
        parcel.WriteBool(true); // isFromMock
        mockLocationArray.push_back(Location::UnmarshallingShared(parcel));
    }

    /*
     * @tc.steps: step2. test set mocked locations for different scenarioes
     * @tc.expected: no exception happens
     */
    proxy_->SetMockedLocations(mockInfo, mockLocationArray);
    mockInfo.SetScenario(SCENE_DAILY_LIFE_SERVICE);
    proxy_->SetMockedLocations(mockInfo, mockLocationArray);
    mockInfo.SetScenario(SCENE_NO_POWER);
    proxy_->SetMockedLocations(mockInfo, mockLocationArray);
    mockInfo.SetScenario(SCENE_UNSET);
    proxy_->SetMockedLocations(mockInfo, mockLocationArray);
}

/*
 * @tc.name: EnableReverseGeocodingMock001
 * @tc.desc: Test enable reverse geocoding mock
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, EnableReverseGeocodingMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test enable reverse geocoding mock
     * @tc.expected: no exception happens
     */
    proxy_->EnableReverseGeocodingMock();
}

/*
 * @tc.name: DisableReverseGeocodingMock001
 * @tc.desc: Test disable reverse geocoding mock
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, DisableReverseGeocodingMock001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test disable reverse geocoding mock
     * @tc.expected: no exception happens
     */
    proxy_->DisableReverseGeocodingMock();
}

/*
 * @tc.name: SetReverseGeocodingMockInfo001
 * @tc.desc: Test set reverse geocoding mock info
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, SetReverseGeocodingMockInfo001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. prepare mock info
     */
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
    proxy_->SetReverseGeocodingMockInfo(mockInfo);
}

/*
 * @tc.name: CheckGetCacheLocation002
 * @tc.desc: Test the function GetCacheLocation and get approximately location
 * @tc.type: FUNC
 * @tc.require: issueI5OSHX
 */
HWTEST_F(LocatorServiceTest, CheckGetCacheLocation002, TestSize.Level1)
{
    /*
     * @tc.steps: step1. get approximately location permission.
     * @tc.steps: step2. Call system ability and get cache location.
     * @tc.expected: step1. get reply location is nullptr.
     */
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(0);
    identity.SetUid(LOCATOR_UID);
    identity.SetTokenId(0);
    identity.SetFirstTokenId(0);
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    bool ret = false;
    if (proxy_->GetSwitchState() == 1) {
        ret = locatorAbility->GetCacheLocation(reply, identity);
        ret = reply.ReadInt32() == REPLY_CODE_EXCEPTION;
        EXPECT_EQ(true, ret);
    }
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
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t callingFirstTokenid = IPCSkeleton::GetFirstTokenID();
    int permissionLevel = CommonUtils::GetPermissionLevel(callingTokenId, callingFirstTokenid);
    EXPECT_EQ(PERMISSION_ACCURATE, permissionLevel);
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
    bool ret = backgroundProxy_->RegisterAppStateObserver();
    EXPECT_EQ(true, ret);
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
    bool ret = backgroundProxy_->UnregisterAppStateObserver();
    EXPECT_EQ(true, ret);
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
    backgroundProxy_->UpdateListOnRequestChange(request_);

    /*
     * @tc.steps: step2. request is null
     * @tc.expected: early return because request is nullptr
     */
    backgroundProxy_->UpdateListOnRequestChange(nullptr);
}

HWTEST_F(LocatorServiceTest, locatorImpl001, TestSize.Level1)
{
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    locatorImpl->ShowNotification();
    locatorImpl->RequestPermission();
    locatorImpl->RequestEnableLocation();

    locatorImpl->EnableAbility(false);
    EXPECT_EQ(false, locatorImpl->IsLocationEnabled());
    locatorImpl->EnableAbility(true);
    EXPECT_EQ(true, locatorImpl->IsLocationEnabled());

    EXPECT_EQ(nullptr, locatorImpl->GetCachedLocation());

    EXPECT_EQ(false, locatorImpl->IsLocationPrivacyConfirmed(1));
    EXPECT_EQ(false, locatorImpl->IsLocationPrivacyConfirmed(-1));
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION, locatorImpl->SetLocationPrivacyConfirmStatus(1, true));
    EXPECT_EQ(REPLY_CODE_EXCEPTION, locatorImpl->SetLocationPrivacyConfirmStatus(-1, true));
    EXPECT_EQ(true, locatorImpl->IsLocationPrivacyConfirmed(1));
    EXPECT_EQ(false, locatorImpl->IsLocationPrivacyConfirmed(-1));

    EXPECT_EQ(-1, locatorImpl->GetCachedGnssLocationsSize());

    EXPECT_EQ(REPLY_CODE_UNSUPPORT, locatorImpl->FlushCachedGnssLocations());

    std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
    command->scenario = SCENE_NAVIGATION;
    command->command = "cmd";
    EXPECT_EQ(true, locatorImpl->SendCommand(command));

    std::unique_ptr<GeofenceRequest> fenceRequest = std::make_unique<GeofenceRequest>();
    fenceRequest->priority = 1;
    fenceRequest->scenario = SCENE_NAVIGATION;
    GeoFence geofence;
    geofence.latitude = 1.0;
    geofence.longitude = 2.0;
    geofence.radius = 3.0;
    geofence.expiration = 4.0;
    fenceRequest->geofence = geofence;
    EXPECT_EQ(true, locatorImpl->AddFence(fenceRequest));
    EXPECT_EQ(true, locatorImpl->RemoveFence(fenceRequest));

    EXPECT_NE(nullptr, locatorImpl->GetIsoCountryCode());

    EXPECT_EQ(true, locatorImpl->ProxyUidForFreeze(1000, true));
    EXPECT_EQ(true, locatorImpl->ProxyUidForFreeze(1000, false));
    EXPECT_EQ(true, locatorImpl->ResetAllProxy());

    LocationMockConfig mockInfo;
    mockInfo.SetScenario(SCENE_NAVIGATION);
    mockInfo.SetTimeInterval(2);
    EXPECT_EQ(true, locatorImpl->EnableLocationMock(mockInfo));
    std::vector<std::shared_ptr<Location>> locations;
    EXPECT_EQ(true, locatorImpl->SetMockedLocations(mockInfo, locations));
    EXPECT_EQ(true, locatorImpl->DisableLocationMock(mockInfo));
}

HWTEST_F(LocatorServiceTest, locatorImplGeocodingMock001, TestSize.Level1)
{
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    EXPECT_EQ(true, locatorImpl->EnableReverseGeocodingMock());
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    EXPECT_EQ(true, locatorImpl->SetReverseGeocodingMockInfo(mockInfos));
    EXPECT_EQ(true, locatorImpl->DisableReverseGeocodingMock());
}

HWTEST_F(LocatorServiceTest, locatorImplIsGeoServiceAvailable001, TestSize.Level1)
{
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    EXPECT_EQ(true, locatorImpl->EnableReverseGeocodingMock());
    EXPECT_EQ(true, locatorImpl->IsGeoServiceAvailable());

    EXPECT_EQ(true, locatorImpl->DisableReverseGeocodingMock());
    EXPECT_EQ(false, locatorImpl->IsGeoServiceAvailable());
}

HWTEST_F(LocatorServiceTest, locatorImplGetAddressByCoordinate001, TestSize.Level1)
{
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    MessageParcel request001;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList001;
    EXPECT_EQ(true, locatorImpl->EnableReverseGeocodingMock());

    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    EXPECT_EQ(true, locatorImpl->SetReverseGeocodingMockInfo(mockInfos));
    request001.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request001.WriteDouble(99.0); // latitude
    request001.WriteDouble(100.0); // longitude
    request001.WriteInt32(3); // maxItems
    request001.WriteInt32(1); // locale object size = 1
    request001.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request001.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request001.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request001.WriteString16(Str8ToStr16("")); // ""
    locatorImpl->GetAddressByCoordinate(request001, geoAddressList001);
    EXPECT_EQ(false, geoAddressList001.empty());
}

HWTEST_F(LocatorServiceTest, locatorImplGetAddressByCoordinate002, TestSize.Level1)
{
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
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
}

HWTEST_F(LocatorServiceTest, locatorImplGetAddressByLocationName001, TestSize.Level1)
{
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
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
}

HWTEST_F(LocatorServiceTest, locatorImplRegisterAndUnregisterCallback001, TestSize.Level1)
{
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject()));

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

    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    EXPECT_NE(nullptr, countryCodeCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterCountryCodeCallback(countryCodeCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterCountryCodeCallback(countryCodeCallbackHost->AsObject()));

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
}
}  // namespace Location
}  // namespace OHOS