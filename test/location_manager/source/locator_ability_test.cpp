/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "locator_ability_test.h"
#include <cstdlib>

#define private public
#include "request.h"
#include "request_config.h"
#undef private

#include "event_handler.h"
#include "event_runner.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "country_code_callback_host.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "locator.h"
#include "permission_manager.h"
#include "report_manager.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_napi.h"
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_napi.h"
#endif
#include "ilocator_service.h"
#include "location.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "location_switch_callback_host.h"
#include "locator.h"
#include "locator_callback_host.h"
#include "locator_callback_proxy.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_napi.h"
#endif
#include "permission_manager.h"
#include "geofence_request.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "request_manager.h"
#include "app_identity.h"

#define private public
#include "locator_ability.h"
#undef private

#include "mock_i_remote_object.h"
#include "bluetooth_scan_result_callback_napi.h"
#include "locating_required_data_callback_napi.h"
using namespace testing::ext;
namespace OHOS {
namespace Location {
const uint32_t EVENT_SEND_SWITCHSTATE_TO_HIFENCE = 0x0006;
const int32_t LOCATION_PERM_NUM = 5;
const std::string ARGS_HELP = "-h";
const int REQUEST_MAX_NUM = 3;
const int UNKNOWN_SERVICE_ID = -1;
const int GET_SWITCH_STATE = 1;
void LocatorAbilityTest::SetUp()
{
    LoadSystemAbility();
    MockNativePermission();
    locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
}

void LocatorAbilityTest::TearDown()
{
    locatorAbility = nullptr;
}

void LocatorAbilityTest::LoadSystemAbility()
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

void LocatorAbilityTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        ACCESS_CONTROL_LOCATION_SWITCH.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorAbilityTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorAbilityTest, locatorServiceGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, locatorServiceGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceGnssStatusCallback001 begin");
    sptr<MockIRemoteObject> callback = sptr<MockIRemoteObject>(new (std::nothrow) MockIRemoteObject());
    // uid pid not match locationhub process
    if (!locatorAbility->CheckLocationSwitchState()) {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorAbility->RegisterGnssStatusCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterGnssStatusCallback(callback));
        locatorAbility->SetSwitchState(true);
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->RegisterGnssStatusCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterGnssStatusCallback(callback));
        locatorAbility->SetSwitchState(false);
    } else {
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->RegisterGnssStatusCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterGnssStatusCallback(callback));
        locatorAbility->SetSwitchState(false);
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorAbility->RegisterGnssStatusCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterGnssStatusCallback(callback));
        locatorAbility->SetSwitchState(true);
    }
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceGnssStatusCallback001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorAbilityTest, locatorServiceNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, locatorServiceNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceNmeaMessageCallback001 begin");
    sptr<MockIRemoteObject> callback = sptr<MockIRemoteObject>(new (std::nothrow) MockIRemoteObject());
    if (!locatorAbility->CheckLocationSwitchState()) {
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorAbility->RegisterNmeaMessageCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterNmeaMessageCallback(callback));
        locatorAbility->SetSwitchState(true);
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->RegisterNmeaMessageCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterNmeaMessageCallback(callback));
        locatorAbility->SetSwitchState(false);
    } else {
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->RegisterNmeaMessageCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterNmeaMessageCallback(callback));
        locatorAbility->SetSwitchState(false);
        EXPECT_EQ(ERRCODE_SWITCH_OFF, locatorAbility->RegisterNmeaMessageCallback(callback));
        EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, locatorAbility->UnregisterNmeaMessageCallback(callback));
        locatorAbility->SetSwitchState(true);
    }
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceNmeaMessageCallback001 end");
}
#endif

HWTEST_F(LocatorAbilityTest, locatorServiceStartAndStop001, TestSize.Level1)
{
    locatorAbility->OnStart();
    locatorAbility->OnStop();
}

HWTEST_F(LocatorAbilityTest, locatorServiceStartAndStopSA001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, locatorServiceStartAndStopSA001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceStartAndStopSA001 begin");
    locatorAbility->OnAddSystemAbility(UNKNOWN_SERVICE_ID, "device-id");
    locatorAbility->OnRemoveSystemAbility(UNKNOWN_SERVICE_ID, "device-id");
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceStartAndStopSA001 end");
}

HWTEST_F(LocatorAbilityTest, locatorServiceInitSaAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, locatorServiceInitSaAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceInitSaAbility001 begin");
    ASSERT_TRUE(locatorAbility != nullptr);
    locatorAbility->InitSaAbility();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceInitSaAbility001 end");
}

HWTEST_F(LocatorAbilityTest, locatorServiceInitRequestManager001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, locatorServiceInitRequestManager001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceInitRequestManager001 begin");
    locatorAbility->InitRequestManagerMap();
    EXPECT_EQ(REQUEST_MAX_NUM, locatorAbility->GetRequests()->size());
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceInitRequestManager001 end");
}

HWTEST_F(LocatorAbilityTest, locatorServiceUpdateSaAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, locatorServiceUpdateSaAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceUpdateSaAbility001 begin");
    EXPECT_EQ(ERRCODE_SUCCESS, locatorAbility->UpdateSaAbility());
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] locatorServiceUpdateSaAbility001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityApplyRequestsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityApplyRequestsTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityApplyRequestsTest001 begin");
    int delay = 1;
    locatorAbility->ApplyRequests(delay);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityApplyRequestsTest001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityUpdateSaAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityUpdateSaAbilityTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUpdateSaAbilityTest001 begin");
    locatorAbility->UpdateSaAbility();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUpdateSaAbilityTest001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityRemoveUnloadTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRemoveUnloadTaskTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveUnloadTask001 begin");
    int code = 1;
    locatorAbility->RemoveUnloadTask(code);
    locatorAbility->RemoveUnloadTask(code);
    code = static_cast<uint16_t>(LocatorInterfaceCode::PROXY_PID_FOR_FREEZE);
    locatorAbility->RemoveUnloadTask(code);
    code = static_cast<uint16_t>(LocatorInterfaceCode::RESET_ALL_PROXY);
    locatorAbility->RemoveUnloadTask(code);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveUnloadTaskTest001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilityAddFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityAddFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddFence001 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    locatorAbility->AddFence(*request);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddFence001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityRemoveFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRemoveFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveFence001 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    locatorAbility->RemoveFence(*request);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveFence001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityAddGnssGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityAddGnssGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddGnssGeofence001 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    locatorAbility->AddGnssGeofence(*request);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddGnssGeofence001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityRemoveGnssGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRemoveGnssGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveGnssGeofence001 begin");
    int32_t fenceId = 1;
    locatorAbility->RemoveGnssGeofence(fenceId);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveGnssGeofence001 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityStartLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityStartLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStartLocating001 begin");
    locatorAbility->proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    locatorAbility->StartLocating(*requestConfig, callbackStub);
    sptr<IRemoteObject> objectGnss = CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    locatorAbility->proxyMap_->insert(make_pair(GNSS_ABILITY, objectGnss));
    locatorAbility->reportManager_ = ReportManager::GetInstance();
    locatorAbility->requestManager_ = RequestManager::GetInstance();
    locatorAbility->StartLocating(*requestConfig, callbackStub);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStartLocating001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityIsCacheVaildScenario001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityIsCacheVaildScenario001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityIsCacheVaildScenario001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    AppIdentity identity;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->requestConfig_->scenario_ = SCENE_UNSET;
    request->requestConfig_->priority_ = PRIORITY_LOW_POWER;
    bool res = locatorAbility->IsCacheVaildScenario(request->GetRequestConfig());
    EXPECT_EQ(true, res);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityIsCacheVaildScenario001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityIsCacheVaildScenario002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityIsCacheVaildScenario002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityIsCacheVaildScenario002 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    AppIdentity identity;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->requestConfig_->scenario_ = SCENE_UNSET;
    request->requestConfig_->priority_ = LOCATION_PRIORITY_ACCURACY;
    bool res = locatorAbility->IsCacheVaildScenario(request->GetRequestConfig());
    EXPECT_EQ(false, res);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityIsCacheVaildScenario002 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityIsSingleRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityIsSingleRequest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityIsSingleRequest001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    AppIdentity identity;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->requestConfig_->fixNumber_ = 1;
    bool res = locatorAbility->IsSingleRequest(request->GetRequestConfig());
    EXPECT_EQ(true, res);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityIsSingleRequest001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation001 begin");
    sptr<ILocatorCallback> callback;
    std::shared_ptr<Request> request = nullptr;
    locatorAbility->reportManager_ = nullptr;
    locatorAbility->NeedReportCacheLocation(request, callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation002 begin");
    sptr<ILocatorCallback> callback;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorAbility->reportManager_ = ReportManager::GetInstance();
    request->requestConfig_->fixNumber_ = 1;
    request->requestConfig_->scenario_ = SCENE_DAILY_LIFE_SERVICE;
    locatorAbility->NeedReportCacheLocation(request, callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation002 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation003 begin");
    sptr<ILocatorCallback> callback;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorAbility->reportManager_ = ReportManager::GetInstance();
    request->requestConfig_->fixNumber_ = 0;
    request->requestConfig_->scenario_ = SCENE_DAILY_LIFE_SERVICE;
    locatorAbility->NeedReportCacheLocation(request, callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation003 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation004, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation004 begin");
    sptr<ILocatorCallback> callback;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorAbility->reportManager_ = ReportManager::GetInstance();
    request->requestConfig_->fixNumber_ = 0;
    request->requestConfig_->scenario_ = LOCATION_PRIORITY_ACCURACY;
    locatorAbility->NeedReportCacheLocation(request, callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation004 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityNeedReportCacheLocation005, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation005 begin");
    sptr<ILocatorCallback> callback;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    locatorAbility->reportManager_ = ReportManager::GetInstance();
    request->requestConfig_->fixNumber_ = 1;
    request->requestConfig_->scenario_ = LOCATION_PRIORITY_ACCURACY;
    locatorAbility->NeedReportCacheLocation(request, callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityNeedReportCacheLocation005 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityHandleStartLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityHandleStartLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityHandleStartLocating001 begin");
    std::shared_ptr<Request> request = std::make_shared<Request>();
    sptr<LocatorCallbackHost> locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());
    sptr<ILocatorCallback> callback = sptr<ILocatorCallback>(locatorCallbackHost);
    request->SetLocatorCallBack(callback);
    locatorAbility->HandleStartLocating(request, callback);
    locatorAbility->HandleStartLocating(request, callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityHandleStartLocating001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityStopLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityStopLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStopLocating001 begin");
    sptr<ILocatorCallback> callback;
    locatorAbility->requestManager_ = nullptr;
    locatorAbility->StopLocating(callback);
    locatorAbility->requestManager_ = RequestManager::GetInstance();
    locatorAbility->StopLocating(callback);
    locatorAbility->StopLocating(callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStopLocating001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityGetCacheLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityGetCacheLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityGetCacheLocation001 begin");
    std::unique_ptr<Location> loc = std::make_unique<Location>();;
    locatorAbility->GetCacheLocation(*loc);
    locatorAbility->GetCacheLocation(*loc);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityGetCacheLocation001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock001 begin");
    locatorAbility->EnableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock002 begin");
    locatorAbility->EnableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock002 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock001 begin");
    locatorAbility->DisableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock002 begin");
    locatorAbility->DisableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock002 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo001 begin");
    std::vector<GeocodingMockInfo> mockInfo;
    locatorAbility->SetReverseGeocodingMockInfo(mockInfo);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo002 begin");
    std::vector<GeocodingMockInfo> mockInfo;
    locatorAbility->SetReverseGeocodingMockInfo(mockInfo);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo002 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityRegisterPermissionCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRegisterPermissionCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRegisterPermissionCallback001 begin");
    auto permissionMap =
        std::make_shared<std::map<uint32_t, std::shared_ptr<PermissionStatusChangeCb>>>();
    locatorAbility->permissionMap_ = permissionMap;
    uint32_t callingTokenId = 10;
    std::vector<std::string> permissionNameList;
    locatorAbility->RegisterPermissionCallback(callingTokenId, permissionNameList);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRegisterPermissionCallback001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityUnRegisterPermissionCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityUnRegisterPermissionCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUnRegisterPermissionCallback001 begin");
    auto permissionMap =
        std::make_shared<std::map<uint32_t, std::shared_ptr<PermissionStatusChangeCb>>>();
    locatorAbility->permissionMap_ = permissionMap;
    uint32_t callingTokenId = 10;
    std::vector<std::string> permissionNameList;
    locatorAbility->UnregisterPermissionCallback(callingTokenId);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUnRegisterPermissionCallback001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityReportDataToResSched001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityReportDataToResSched001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityReportDataToResSched001 begin");
    std::string state("state");
    locatorAbility->ReportDataToResSched(state);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityReportDataToResSched001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilitySendNetworkLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySendNetworkLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySendNetworkLocation001 begin");
    std::unique_ptr<Location> location = std::make_unique<Location>();
    locatorAbility->SendNetworkLocation(location);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySendNetworkLocation001 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityRegisterLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRegisterLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRegisterLocationError001 begin");
    sptr<ILocatorCallback> callback;
    AppIdentity identity;
    locatorAbility->RegisterLocationError(callback, identity);
    locatorAbility->UnregisterLocationError(callback, identity);
    locatorAbility->RegisterLocationError(callback, identity);
    locatorAbility->UnregisterLocationError(callback, identity);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRegisterLocationError001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityReportLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityReportLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityReportLocationError001 begin");
    std::string uuid;
    std::string errMsg;
    int32_t errCode = 10;
    locatorAbility->ReportLocationError(errCode, errMsg, uuid);
    locatorAbility->ReportLocationError(errCode, errMsg, uuid);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUnRegisterLocationError001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilitySyncIdleState001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySyncIdleState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySyncIdleState001 begin");
    bool state = true;
    locatorAbility->SyncIdleState(state);
    state = false;
    locatorAbility->SyncIdleState(state);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySyncIdleState001 end");
}

HWTEST_F(LocatorAbilityTest, LocationMessageGetAbilityName001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocationMessageGetAbilityName001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocationMessageGetAbilityName001 begin");
    auto locationMessage =
        new (std::nothrow) LocationMessage();
    locationMessage->GetAbilityName();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocationMessageGetAbilityName001 end");
}

HWTEST_F(LocatorAbilityTest, LocationMessageSetLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocationMessageSetLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocationMessageSetLocation001 begin");
    auto locationMessage =
        new (std::nothrow) LocationMessage();
    locationMessage->location_ = nullptr;
    std::unique_ptr<Location> location;
    locationMessage->SetLocation(location);
    location = std::make_unique<Location>();
    locationMessage->SetLocation(location);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocationMessageSetLocation001 end");
}

HWTEST_F(LocatorAbilityTest, LocationMessageGetLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocationMessageGetLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocationMessageGetLocation001 begin");
    auto locationMessage =
        new (std::nothrow) LocationMessage();
    locationMessage->location_ = nullptr;
    locationMessage->GetLocation();
    locationMessage->location_ = std::make_unique<Location>();
    locationMessage->GetLocation();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocationMessageGetLocation001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerInitLocatorHandlerEventMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerInitLocatorHandlerEventMap001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerInitLocatorHandlerEventMap001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    locatorHandler->InitLocatorHandlerEventMap();
    locatorHandler->InitLocatorHandlerEventMap();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerInitLocatorHandlerEventMap001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerGetCachedLocationSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerGetCachedLocationSuccess001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerGetCachedLocationSuccess001 begin");
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(1);
    identity.SetTokenId(1);
    identity.SetTokenIdEx(1);
    identity.SetFirstTokenId(1);
    identity.SetBundleName("com.example.lbs.test");
    std::shared_ptr<AppIdentity> identityInfo = std::make_shared<AppIdentity>(identity);
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, identityInfo);
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler = new (std::nothrow) LocatorHandler(runner);
    locatorHandler->GetCachedLocationSuccess(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerGetCachedLocationSuccess001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerGetCachedLocationFailed001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerGetCachedLocationFailed001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerGetCachedLocationFailed001 begin");
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(1);
    identity.SetTokenId(1);
    identity.SetTokenIdEx(1);
    identity.SetFirstTokenId(1);
    identity.SetBundleName("com.example.lbs.test");
    std::shared_ptr<AppIdentity> identityInfo = std::make_shared<AppIdentity>(identity);
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, identityInfo);
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler = new (std::nothrow) LocatorHandler(runner);
    locatorHandler->GetCachedLocationFailed(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerGetCachedLocationFailed001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerUpdateSaEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerUpdateSaEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerUpdateSaEvent001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->UpdateSaEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerUpdateSaEvent001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerApplyRequirementsEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerApplyRequirementsEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerApplyRequirementsEvent001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->ApplyRequirementsEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerApplyRequirementsEvent001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerRetryRegisterActionEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerRetryRegisterActionEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerRetryRegisterActionEvent001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->RetryRegisterActionEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerRetryRegisterActionEvent001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerReportLocationMessageEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerReportLocationMessageEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerReportLocationMessageEvent001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->ReportLocationMessageEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerReportLocationMessageEvent001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerStartAndStopLocatingEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerStartLocatingEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerStartLocatingEvent001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->StartLocatingEvent(event);
    locatorHandler->StopLocatingEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerStartLocatingEvent001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerUpdateLastLocationRequestNum001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerUpdateLastLocationRequestNum001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerUpdateLastLocationRequestNum001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerUpdateLastLocationRequestNum001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerUnloadSaEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerUnloadSaEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerUnloadSaEvent001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->UnloadSaEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerUnloadSaEvent001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandler001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandler001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->RegLocationErrorEvent(event);
    locatorHandler->UnRegLocationErrorEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandler002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandler002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler002 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->SyncIdleState(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler002 end");
}

HWTEST_F(LocatorAbilityTest, LocatorCallbackDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorCallbackDeathRecipient001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorCallbackDeathRecipient001 begin");
    int32_t tokenId = 1;
    auto recipient =
        sptr<LocatorCallbackDeathRecipient>(new (std::nothrow) LocatorCallbackDeathRecipient(tokenId));
    wptr<IRemoteObject> remote;
    recipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorCallbackDeathRecipient001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandler003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandler003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler003 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->RequestCheckEvent(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler003 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandler004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandler004, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler004 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->SyncStillMovementState(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandler004 end");
}

HWTEST_F(LocatorAbilityTest, TestLocalPermission, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] TestLocalPermission start");
    // test local permisson
    AppIdentity identity;
    locatorAbility->GetAppIdentityInfo(identity);
    LBSLOGI(LOCATOR, "identity: %{public}s", identity.ToString().c_str());

    if (!PermissionManager::CheckSystemPermission(identity.GetTokenId(), identity.GetTokenIdEx())) {
        LBSLOGE(LOCATOR, "CheckSystemPermission return false, [%{public}s]", identity.ToString().c_str());
    }
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] TestLocalPermission end");
}

HWTEST_F(LocatorAbilityTest, EnableAbilityForUser_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] EnableAbilityForUser_Test_001 start");
    // Arrange
    int32_t userId = 100;

    // Act
    ErrCode result = locatorAbility->EnableAbilityForUser(false, userId);
    result = locatorAbility->EnableAbilityForUser(true, userId);

    // Assert
    EXPECT_EQ(result, ERRCODE_SUCCESS);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] EnableAbilityForUser_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, GetCachedGnssLocationsSize_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCachedGnssLocationsSize_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    int32_t size = -1;

    // Act
    ErrCode result = locatorAbility->GetCachedGnssLocationsSize(size);
    // Assert
    EXPECT_EQ(true, size >= -1);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCachedGnssLocationsSize_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, FlushCachedGnssLocations_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] FlushCachedGnssLocations_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    // Act
    ErrCode result = locatorAbility->FlushCachedGnssLocations();
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] FlushCachedGnssLocations_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, Add_RemoveFence_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] RemoveFence_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();

    // Act
    ErrCode result = locatorAbility->AddFence(*request);
    result = locatorAbility->RemoveFence(*request);
    
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] RemoveFence_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, Add_RemoveGnssFence_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] Add_RemoveGnssFence_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    int32_t fenceId = 100;
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    request->SetFenceId(fenceId);

    // Act
    ErrCode result = locatorAbility->AddGnssGeofence(*request);
    result = locatorAbility->RemoveGnssGeofence(fenceId);
    
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] Add_RemoveGnssFence_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, StartLocating_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] StartLocating_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    locatorAbility->proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    // Act
    ErrCode result = locatorAbility->StartLocating(*requestConfig, callbackStub);
    result = locatorAbility->StopLocating(callbackStub);
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] StartLocating_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, NeedReportCacheLocation_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] NeedReportCacheLocation_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    locatorAbility->proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->requestConfig_->scenario_ = SCENE_UNSET;
    request->requestConfig_->priority_ = PRIORITY_LOW_POWER;

    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    // Act
    ErrCode result = locatorAbility->NeedReportCacheLocation(request, callbackStub);
    
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] NeedReportCacheLocation_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, ReportSingleCacheLocation_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] ReportSingleCacheLocation_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    locatorAbility->proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->requestConfig_->scenario_ = SCENE_UNSET;
    request->requestConfig_->priority_ = PRIORITY_LOW_POWER;
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    
    auto cacheLocation = locatorAbility->reportManager_->GetCacheLocation(request);
    // Act
    ErrCode result = locatorAbility->ReportSingleCacheLocation(request, callbackStub, cacheLocation);
    
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] ReportSingleCacheLocation_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, GetCacheLocation_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCacheLocation_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    std::unique_ptr<Location> location = std::make_unique<Location>();
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    
    // Act
    ErrCode result = locatorAbility->GetCacheLocation(*location);
    result = locatorAbility->ReportLocation(NETWORK_ABILITY, *location);
    result = locatorAbility->ReportLocationStatus(callbackStub, result);
    result = locatorAbility->ReportErrorStatus(callbackStub, result);
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCacheLocation_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, GeoConvert_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GeoConvert_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    bool isAvailable = false;
    // Act
    ErrCode result = locatorAbility->IsGeoConvertAvailable(isAvailable);
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GeoConvert_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, GetAddressByLocationName_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetAddressByLocationName_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    auto geoConvertRequest = std::make_unique<GeocodeConvertAddressRequest>();
    sptr<MockIRemoteObject> callback = sptr<MockIRemoteObject>(new (std::nothrow) MockIRemoteObject());
    // Act
    ErrCode result = locatorAbility->GetAddressByLocationName(callback, *geoConvertRequest);
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetAddressByLocationName_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, BluetoothScan_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] BluetoothScan_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    std::vector<CoordinateType> coordinateTypes;
    auto bluetoothScanResultCallbackHost =
        sptr<BluetoothScanResultCallbackNapi>(new (std::nothrow) BluetoothScanResultCallbackNapi());
    auto bluetoothScanResultCallback = sptr<IBluetoothScanResultCallback>(bluetoothScanResultCallbackHost);
    // Act
    ErrCode result = locatorAbility->QuerySupportCoordinateSystemType(coordinateTypes);
    result = locatorAbility->SubscribeBluetoothScanResultChange(nullptr);
    result = locatorAbility->SubscribeBluetoothScanResultChange(bluetoothScanResultCallback);
    result = locatorAbility->UnSubscribeBluetoothScanResultChange(bluetoothScanResultCallback);
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] BluetoothScan_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, RemoveInvalidRequests_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] RemoveInvalidRequests_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    AppIdentity identity;
    auto requestManager_ = RequestManager::GetInstance();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetTimeStamp(0);
    requestConfig->SetFixNumber(1);
    requestConfig->SetScenario(SCENE_NAVIGATION);
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    std::shared_ptr<Request> request = std::make_shared<Request>(requestConfig, callbackStub, identity);
    
    // Act
    ErrCode result = locatorAbility->IsInvalidRequest(request);
    locatorAbility->StartLocating(*requestConfig, callbackStub);
    result = locatorAbility->RemoveInvalidRequests();
    locatorAbility->StopLocating(callbackStub);
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] RemoveInvalidRequests_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, SyncStillMovementState_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SyncStillMovementState_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);

    // Act
    locatorAbility->SyncStillMovementState(true);

    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SyncStillMovementState_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, SetLocationSwitchIgnored_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SetLocationSwitchIgnored_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    ErrCode result = ERRCODE_SUCCESS;
    AppIdentity identity;
    locatorAbility->GetAppIdentityInfo(identity);

    // Act
    bool switchFlag = locatorAbility->GetLocationSwitchIgnoredFlag(identity.GetTokenId());
    result = locatorAbility->SetLocationSwitchIgnored(true);
    sleep(1);
    result = locatorAbility->SetLocationSwitchIgnored(false);
    sleep(1);
    locatorAbility->SetLocationSwitchIgnoredFlag(identity.GetTokenId(), true);
    sleep(1);
    locatorAbility->SetLocationSwitchIgnoredFlag(identity.GetTokenId(), false);

    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SetLocationSwitchIgnored_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, SetLocationWorkingStateEvent_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SetLocationWorkingStateEvent_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    // Act
    ErrCode result = ERRCODE_SUCCESS;
    result = locatorAbility->SetSwitchState(true);
    sleep(1);
    result = locatorAbility->SetSwitchState(false);
    sleep(1);
    result = locatorAbility->SetSwitchState(true);
    sleep(1);

    result = locatorAbility->SetSwitchStateForUser(true, 100);
    sleep(1);
    result = locatorAbility->SetSwitchStateForUser(false, 100);
    sleep(1);
    result = locatorAbility->SetSwitchStateForUser(true, 100);

    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SetLocationWorkingStateEvent_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, RegisterLocatingRequiredDataCallback_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] RegisterLocatingRequiredDataCallback_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    std::unique_ptr<LocatingRequiredDataConfig> requestConfig = std::make_unique<LocatingRequiredDataConfig>();
    requestConfig->SetFixNumber(1);
    auto callbackHost =
        sptr<LocatingRequiredDataCallbackNapi>(new (std::nothrow) LocatingRequiredDataCallbackNapi());
    callbackHost->SetFixNumber(1);
    auto callbackPtr = sptr<ILocatingRequiredDataCallback>(callbackHost);
    // Act
    ErrCode result = ERRCODE_SUCCESS;
    result = locatorAbility->RegisterLocatingRequiredDataCallback(*requestConfig, nullptr);
    result = locatorAbility->RegisterLocatingRequiredDataCallback(*requestConfig, callbackPtr);

    result = locatorAbility->UnRegisterLocatingRequiredDataCallback(callbackPtr);
    // Assert
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] RegisterLocatingRequiredDataCallback_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, SubscribeLocationError_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SubscribeLocationError_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();

    // Act
    ErrCode result = ERRCODE_SUCCESS;
    result = locatorAbility->SubscribeLocationError(nullptr);
    result = locatorAbility->UnSubscribeLocationError(nullptr);

    result = locatorAbility->SubscribeLocationError(callbackStub);
    result = locatorAbility->UnSubscribeLocationError(callbackStub);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] SubscribeLocationError_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, GetCurrentWifiBssidForLocating_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCurrentWifiBssidForLocating_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    std::string bssid = "0112:110:123";
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();

    // Act
    ErrCode result = ERRCODE_SUCCESS;
    result = locatorAbility->GetCurrentWifiBssidForLocating(bssid);
    result = locatorAbility->GetCurrentWifiBssidForLocating(bssid);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCurrentWifiBssidForLocating_Test_001 end");
}

HWTEST_F(LocatorAbilityTest, COMMON_Test_001, TestSize.Level1)
{
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCurrentWifiBssidForLocating_Test_001 start");
    // Arrange
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ASSERT_TRUE(locatorAbility != nullptr);
    std::string bssid = "0112:110:123";
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();

    // Act
    ErrCode result = ERRCODE_SUCCESS;
    result = locatorAbility->CheckPreciseLocationPermissions(123123, 123123); // INVALID token-id test

    LBSLOGI(LOCATOR, "[LocatorAbilityTest] GetCurrentWifiBssidForLocating_Test_001 end");
}
}  // namespace Location
}  // namespace OHOS