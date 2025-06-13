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
#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_napi.h"
#endif
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "country_code_callback_napi.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#include "geocode_convert_address_request.h"
#include "geocode_convert_location_request.h"
#include "geo_convert_callback_host.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_napi.h"
#endif
#include "ilocator_service.h"
#include "location.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "location_switch_callback_napi.h"
#include "locator.h"
#include "locator_callback_napi.h"
#include "locator_callback_proxy.h"
#define private public
#undef private
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_napi.h"
#endif
#include "permission_manager.h"
#include "geofence_request.h"
#include "location_data_rdb_manager.h"
#include "mock_i_remote_object.h"
#include "proxy_freeze_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 6;
#ifdef FEATURE_GEOCODE_SUPPORT
const double MOCK_LATITUDE = 99.0;
const double MOCK_LONGITUDE = 100.0;
#endif
const int REQUEST_MAX_NUM = 3;
const int UNKNOWN_SERVICE_ID = -1;
const int GET_SWITCH_STATE = 1;
const int DISABLED_SWITCHMODE = 0;
const int ENABLED_SWITCHMODE = 1;
const int DEFAULT_SWITCHMODE = 2;
const std::string ARGS_HELP = "-h";
const std::string RUNNING_STATE_OBSERVER = "ohos.permission.RUNNING_STATE_OBSERVER";
void LocatorServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    callbackStub_ = new (std::nothrow) LocatorCallbackStub();
    EXPECT_NE(nullptr, callbackStub_);
    backgroundProxy_ = LocatorBackgroundProxy::GetInstance();
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
    callbackStub_ = nullptr;
    backgroundProxy_ = nullptr;
}

void LocatorServiceTest::LoadSystemAbility()
{
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_LOCATOR_SA_ID);
#ifdef FEATURE_GNSS_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_GNSS_SA_ID);
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_GEO_CONVERT_SA_ID);
#endif
}

void LocatorServiceTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        RUNNING_STATE_OBSERVER.c_str(), ACCESS_CONTROL_LOCATION_SWITCH.c_str(),
        ACCESS_MOCK_LOCATION.c_str(), RSS_PROCESS_NAME.c_str()
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 8,
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
 * @tc.name: IsCallbackInProxyTest001
 * @tc.desc: Check if callback is in the proxy callback list
 * @tc.type: FUNC
 */
HWTEST_F(LocatorServiceTest, IsCallbackInProxyTest001, TestSize.Level0)
{
    /*
     * @tc.steps: step1. Check if callback is in the proxy callback list
     * @tc.expected: step1. return false
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, IsCallbackInProxyTest001, TestSize.Level0";
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
 * @tc.name: GetAddressByCoordinate001
 * @tc.desc: Test get address by coordinate
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorServiceTest, GetAddressByCoordinate002, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build the data.
     */
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, GetAddressByCoordinate002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetAddressByCoordinate002 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    sptr<GeoConvertCallbackHost> callback = new (std::nothrow) GeoConvertCallbackHost();
    MessageParcel data;
    data.WriteDouble(10.5); // latitude
    data.WriteDouble(30.2); // longitude
    data.WriteInt32(10); // maxItems
    data.WriteInt32(1); // locale object size = 1
    data.WriteString16(Str8ToStr16("ZH")); // locale.getLanguage()
    data.WriteString16(Str8ToStr16("cn")); // locale.getCountry()
    data.WriteString16(Str8ToStr16("")); // locale.getVariant()
    data.WriteString16(Str8ToStr16("")); // ""
    auto request = GeocodeConvertLocationRequest::UnmarshallingMessageParcel(data);
    locatorAbility->GetAddressByCoordinate(callback, *request);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetAddressByCoordinate002 end");
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
    PermissionManager::GetPermissionLevel(callingTokenId, callingFirstTokenid);
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
    MockNativePermission();
    backgroundProxy_->RegisterAppStateObserver();
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

HWTEST_F(LocatorServiceTest, locatorServiceEnableAndDisable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceEnableAndDisable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceEnableAndDisable001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    int state = DISABLED;
    locatorAbility->EnableAbility(true);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    locatorAbility->EnableAbility(false);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    locatorAbility->EnableAbility(true);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    
    locatorAbility->GetSwitchState(state);

    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceEnableAndDisable001 end");
}

HWTEST_F(LocatorServiceTest, GetSwitchState001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, GetSwitchState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetSwitchState001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    int state = DISABLED;
    LocationDataRdbManager::SetSwitchStateToSysparaForCurrentUser(ENABLED_SWITCHMODE);
    locatorAbility->GetSwitchState(state);
    LocationDataRdbManager::SetSwitchStateToSysparaForCurrentUser(DISABLED_SWITCHMODE);
    locatorAbility->GetSwitchState(state);
    LocationDataRdbManager::SetSwitchStateToSysparaForCurrentUser(DEFAULT_SWITCHMODE);
    locatorAbility->GetSwitchState(state);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] GetSwitchState001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceEnableAndDisable002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceEnableAndDisable002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceEnableAndDisable002 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    int currentSwitchState = LocationDataRdbManager::QuerySwitchState();
    bool isEnable = currentSwitchState == 0 ? true: false;
    LocationDataRdbManager::SetSwitchStateToDb(0);
    locatorAbility->EnableAbility(isEnable);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceEnableAndDisable002 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorServiceTest, locatorServiceCallbackRegAndUnreg001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceCallbackRegAndUnreg001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceCallbackRegAndUnreg001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackNapi>(new (std::nothrow) CachedLocationsCallbackNapi());
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    auto cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
    int32_t reportingPeriodSec = 1;
    bool wakeUpCacheQueueFull = true;
    // uid pid not match locationhub process
    EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorAbility->RegisterCachedLocationCallback(
        reportingPeriodSec, wakeUpCacheQueueFull, cachedCallback, "unit.test"));
    sleep(1);
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED,
        locatorAbility->UnregisterCachedLocationCallback(cachedCallback)); // uid pid not match locationhub process
    sleep(1);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceCallbackRegAndUnreg001 end");
}
#endif

HWTEST_F(LocatorServiceTest, locatorServicePrivacyConfirmStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServicePrivacyConfirmStatus001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePrivacyConfirmStatus001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_STARTUP, true);
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
    auto locatorAbility = LocatorAbility::GetInstance();
    // uid pid not match locationhub process
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED, locatorAbility->SendCommand(SCENE_NAVIGATION, "cmd"));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSendCommand001 end");
}
#endif

HWTEST_F(LocatorServiceTest, locatorServiceLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceLocationMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceLocationMock001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    int timeInterval = 2;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->EnableLocationMock());
    std::vector<Location> locations;
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
    auto locatorAbility = LocatorAbility::GetInstance();
    int state = DISABLED;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    if (state == ENABLED) {
        EXPECT_EQ(REPLY_CODE_NO_EXCEPTION, locatorAbility->ReportLocationStatus(callbackStub_, 0));
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportLocationStatus001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceReportErrorStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceReportErrorStatus001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportErrorStatus001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    int state = DISABLED;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->GetSwitchState(state));
    if (state == ENABLED) {
        EXPECT_EQ(REPLY_CODE_NO_EXCEPTION, locatorAbility->ReportErrorStatus(callbackStub_, 0));
    }
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportErrorStatus001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceGetReceivers001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceGetReceivers001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGetReceivers001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    EXPECT_EQ(0, locatorAbility->GetReceivers()->size());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGetReceivers001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceProxyForFreeze001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceProxyForFreeze001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceProxyForFreeze001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    std::vector<int32_t> pidList;
    pidList.push_back(SYSTEM_UID);
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED, locatorAbility->ProxyForFreeze(pidList, true));
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED, locatorAbility->ProxyForFreeze(pidList, false));
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED, locatorAbility->ProxyForFreeze(pidList, true));
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED, locatorAbility->ResetAllProxy());
    EXPECT_EQ(false, ProxyFreezeManager::GetInstance()->IsProxyPid(SYSTEM_UID));
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceProxyForFreeze001 end");
}

HWTEST_F(LocatorServiceTest, LocatorAbilityStubDump001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, LocatorAbilityStubDump001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityStubDump001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
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
    auto locatorAbility = LocatorAbility::GetInstance();
    ASSERT_TRUE(locatorAbility != nullptr);
    locatorAbility->GetProxyMap();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityGetProxyMap001 end");
}

HWTEST_F(LocatorServiceTest, LocatorAbilityGetProxyMap002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, LocatorAbilityGetProxyMap002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityGetProxyMap002 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    ASSERT_TRUE(locatorAbility != nullptr);
    locatorAbility->InitRequestManagerMap();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] LocatorAbilityGetProxyMap002 end");
}

HWTEST_F(LocatorServiceTest, locatorServicePreProxyForFreeze001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServicePreProxyForFreeze001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePreProxyForFreeze001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    std::vector<int32_t> pidList;
    pidList.push_back(1);
    bool isProxy = false;
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED, locatorAbility->ProxyForFreeze(pidList, isProxy));
    EXPECT_EQ(LOCATION_ERRCODE_PERMISSION_DENIED, locatorAbility->ResetAllProxy());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePreProxyForFreeze001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceOnAddSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceOnAddSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceOnAddSystemAbility001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->locatorEventSubscriber_ = nullptr;
    locatorAbility->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "device-id");
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceOnAddSystemAbility001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceOnRemoveSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceOnRemoveSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceOnRemoveSystemAbility001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->locatorEventSubscriber_ = nullptr;
    locatorAbility->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "device-id");

    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    locatorAbility->locatorEventSubscriber_ = std::make_shared<LocatorEventSubscriber>(subscriberInfo);
    locatorAbility->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "device-id");
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceOnRemoveSystemAbility001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceOnRemoveSystemAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceOnRemoveSystemAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceOnRemoveSystemAbility002 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->locationPrivacyEventSubscriber_ = nullptr;
    locatorAbility->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "device-id");
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceOnRemoveSystemAbility002 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceInit001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceInit001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInit001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->registerToAbility_ = true;
    locatorAbility->Init();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInit001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceInitRequestManagerMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceInitRequestManagerMap001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitRequestManagerMap001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->requests_ = nullptr;
    locatorAbility->InitRequestManagerMap();
    locatorAbility->requests_ = std::make_shared<std::map<std::string, std::list<std::shared_ptr<Request>>>>();
    locatorAbility->GetActiveRequestNum();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitRequestManagerMap001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceInitSaAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceInitSaAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitSaAbility002 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->proxyMap_ = nullptr;
    locatorAbility->InitSaAbility();

    locatorAbility->proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    locatorAbility->InitSaAbility();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitSaAbility002 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceSendLocationMockMsgToGnssSa001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceSendLocationMockMsgToGnssSa001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSendLocationMockMsgToGnssSa001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
#ifdef FEATURE_GNSS_SUPPORT
    locatorAbility->SendLocationMockMsgToGnssSa(nullptr, 0, locations, 0);
#endif
    locatorAbility->SendLocationMockMsgToNetworkSa(nullptr, 0, locations, 0);
    locatorAbility->SendLocationMockMsgToPassiveSa(nullptr, 0, locations, 0);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSendLocationMockMsgToGnssSa001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceStartLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceStartLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceStartLocating001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();

    locatorAbility->proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetFixNumber(1);
    requestConfig->SetPriority(LOCATION_PRIORITY_LOCATING_SPEED);
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    location->SetLocationSourceType(1);
    locatorAbility->reportManager_->UpdateCacheLocation(location, NETWORK_ABILITY);

    LocationDataRdbManager::SetSwitchStateToDb(ENABLED);
    locatorAbility->StartLocating(*requestConfig, callbackStub);
    LocationDataRdbManager::SetSwitchStateToDb(DISABLED);
    locatorAbility->StartLocating(*requestConfig, callbackStub);

    sptr<IRemoteObject> objectGnss = CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    locatorAbility->proxyMap_->insert(make_pair(GNSS_ABILITY, objectGnss));
    locatorAbility->reportManager_ = nullptr;
    locatorAbility->StartLocating(*requestConfig, callbackStub);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceStartLocating001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceGetCacheLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceGetCacheLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGetCacheLocation001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    AppIdentity identity;
    MessageParcel parcel;
    parcel.WriteDouble(0.0);         // latitude
    parcel.WriteDouble(0.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteInt32(1);          // isFromMock
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceGetCacheLocation001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceReportLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceReportLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportLocation001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->requests_ = nullptr;
    std::unique_ptr<Location> location = std::make_unique<Location>();
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->ReportLocation("test", *location));
    locatorAbility->requests_ = std::make_shared<std::map<std::string, std::list<std::shared_ptr<Request>>>>();
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportLocation001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceRegisterPermissionCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceReportLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportLocation001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->permissionMap_ = nullptr;
    std::vector<std::string> permissionNameList;
    locatorAbility->RegisterPermissionCallback(0, permissionNameList);
    locatorAbility->UnregisterPermissionCallback(0);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceReportLocation001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceRemoveUnloadTask001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceRemoveUnloadTask001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceRemoveUnloadTask001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->RemoveUnloadTask(DEFAULT_CODE);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceRemoveUnloadTask001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceRemoveUnloadTask002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceRemoveUnloadTask002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceRemoveUnloadTask002 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->RemoveUnloadTask(GET_SWITCH_STATE);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceRemoveUnloadTask002 end");
}

HWTEST_F(LocatorServiceTest, locatorServicePostUnloadTask001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServicePostUnloadTask001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePostUnloadTask001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->PostUnloadTask(static_cast<uint16_t>(LocatorInterfaceCode::PROXY_PID_FOR_FREEZE));
    locatorAbility->PostUnloadTask(static_cast<uint16_t>(LocatorInterfaceCode::RESET_ALL_PROXY));
    locatorAbility->PostUnloadTask(DEFAULT_CODE);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePostUnloadTask001 end");
}

HWTEST_F(LocatorServiceTest, locatorServicePostUnloadTask002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServicePostUnloadTask002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePostUnloadTask002 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->PostUnloadTask(GET_SWITCH_STATE);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServicePostUnloadTask002 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceSendSwitchState001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceSendSwitchState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSendSwitchState001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->SendSwitchState(0);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceSendSwitchState001 end");
}

HWTEST_F(LocatorServiceTest, locatorServiceInitRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, locatorServiceInitRequest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitRequest001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    AppIdentity identity;
    std::shared_ptr<Request> request = std::make_shared<Request>(requestConfig, callbackStub_, identity);
    EXPECT_NE(nullptr, request);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] locatorServiceInitRequest001 end");
}

HWTEST_F(LocatorServiceTest, IsCacheVaildScenario001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, IsCacheVaildScenario001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsCacheVaildScenario001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    AppIdentity identity;
    std::shared_ptr<Request> request = std::make_shared<Request>(requestConfig, callbackStub_, identity);
    locatorAbility->IsCacheVaildScenario(request->GetRequestConfig());
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsCacheVaildScenario001 end");
}

HWTEST_F(LocatorServiceTest, IsSingleRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, IsSingleRequest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsSingleRequest001 begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    AppIdentity identity;
    std::shared_ptr<Request> request = std::make_shared<Request>(requestConfig, callbackStub_, identity);
    bool res = locatorAbility->IsSingleRequest(request->GetRequestConfig());
    EXPECT_EQ(false, res);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsSingleRequest001 end");
}

HWTEST_F(LocatorServiceTest, RemoveInvalidRequests, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, RemoveInvalidRequests, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RemoveInvalidRequests begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    auto ret = locatorAbility->RemoveInvalidRequests();
    EXPECT_EQ(ERRCODE_SUCCESS, ret);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] RemoveInvalidRequests end");
}

HWTEST_F(LocatorServiceTest, IsInvalidRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, IsInvalidRequest, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsInvalidRequest begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorAbility->IsInvalidRequest(request);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsInvalidRequest end");
}

HWTEST_F(LocatorServiceTest, IsPorcessRunning, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorServiceTest, IsPorcessRunning, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsPorcessRunning begin");
    auto locatorAbility = LocatorAbility::GetInstance();
    auto result = locatorAbility->IsProcessRunning(1000, 1000);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[LocatorServiceTest] IsPorcessRunning end");
}
}  // namespace Location
}  // namespace OHOS
