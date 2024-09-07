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


using namespace testing::ext;
namespace OHOS {
namespace Location {
const uint32_t EVENT_SEND_SWITCHSTATE_TO_HIFENCE = 0x0006;
const int32_t LOCATION_PERM_NUM = 4;
const std::string ARGS_HELP = "-h";
void LocatorAbilityTest::SetUp()
{
    LoadSystemAbility();
    MockNativePermission();
}

void LocatorAbilityTest::TearDown()
{
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

HWTEST_F(LocatorAbilityTest, LocatorAbilityApplyRequestsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityApplyRequestsTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityApplyRequestsTest001 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    int delay = 1;
    locatorAbility->ApplyRequests(delay);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityApplyRequestsTest001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityUpdateSaAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityUpdateSaAbilityTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUpdateSaAbilityTest001 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->UpdateSaAbility();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUpdateSaAbilityTest001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityRemoveUnloadTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRemoveUnloadTaskTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveUnloadTask001 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    int code = 1;
    locatorAbility->RemoveUnloadTask(code);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveUnloadTaskTest001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilityAddGnssGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityAddGnssGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddGnssGeofence001 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    locatorAbility->AddGnssGeofence(request);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddGnssGeofence001 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityAddGnssGeofence002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityAddGnssGeofence002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddGnssGeofence002 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    locatorAbility->AddGnssGeofence(request);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityAddGnssGeofence002 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilityRemoveGnssGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRemoveGnssGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveGnssGeofence001 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    locatorAbility->RemoveGnssGeofence(request);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveGnssGeofence001 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityRemoveGnssGeofence002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRemoveGnssGeofence002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveGnssGeofence002 begin");
    auto locatorAbility = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    locatorAbility->AddGnssGeofence(request);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRemoveGnssGeofence002 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityStartLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityStartLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStartLocating001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    AppIdentity identity;
    locatorAbility->StartLocating(requestConfig, callbackStub, identity);
    sptr<IRemoteObject> objectGnss = CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    locatorAbility->proxyMap_->insert(make_pair(GNSS_ABILITY, objectGnss));
    locatorAbility->reportManager_ = ReportManager::GetInstance();
    locatorAbility->requestManager_ = RequestManager::GetInstance();
    locatorAbility->StartLocating(requestConfig, callbackStub, identity);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStartLocating001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityIsCacheVaildScenario001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityIsCacheVaildScenario001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityIsCacheVaildScenario001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::shared_ptr<Request> request = std::make_shared<Request>();
    sptr<LocatorCallbackHost> locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());
    sptr<ILocatorCallback> callback = sptr<ILocatorCallback>(locatorCallbackHost);
    request->SetLocatorCallBack(callback);
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->HandleStartLocating(request, callback);
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->HandleStartLocating(request, callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityHandleStartLocating001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityStopLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityStopLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStopLocating001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    sptr<ILocatorCallback> callback;
    locatorAbility->requestManager_ = nullptr;
    locatorAbility->StopLocating(callback);
    locatorAbility->requestManager_ = RequestManager::GetInstance();
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->StopLocating(callback);
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->StopLocating(callback);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityStopLocating001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityGetCacheLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityGetCacheLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityGetCacheLocation001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::unique_ptr<Location> loc;
    AppIdentity identity;
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->GetCacheLocation(loc, identity);
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->GetCacheLocation(loc, identity);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityGetCacheLocation001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->EnableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityEnableReverseGeocodingMock002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock002 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->EnableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityEnableReverseGeocodingMock002 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->DisableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityDisableReverseGeocodingMock002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock002 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->DisableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityDisableReverseGeocodingMock002 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    locatorAbility->SetReverseGeocodingMockInfo(mockInfo);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySetReverseGeocodingMockInfo002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo002 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    locatorAbility->SetReverseGeocodingMockInfo(mockInfo);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySetReverseGeocodingMockInfo002 end");
}
#endif

HWTEST_F(LocatorAbilityTest, LocatorAbilityRegisterPermissionCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRegisterPermissionCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRegisterPermissionCallback001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
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
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::string state("state");
    locatorAbility->ReportDataToResSched(state);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityReportDataToResSched001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityUpdateLastLocationRequestNum001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityUpdateLastLocationRequestNum001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUpdateLastLocationRequestNum001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->UpdateLastLocationRequestNum();
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->UpdateLastLocationRequestNum();
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUpdateLastLocationRequestNum001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilitySendNetworkLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySendNetworkLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySendNetworkLocation001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::unique_ptr<Location> location = std::make_unique<Location>();
    locatorAbility->SendNetworkLocation(location);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySendNetworkLocation001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityRegisterLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityRegisterLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRegisterLocationError001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    sptr<ILocatorCallback> callback;
    AppIdentity identity;
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->RegisterLocationError(callback, identity);
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->RegisterLocationError(callback, identity);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityRegisterLocationError001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityUnRegisterLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityUnRegisterLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUnRegisterLocationError001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    sptr<ILocatorCallback> callback;
    AppIdentity identity;
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->UnregisterLocationError(callback, identity);
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->UnregisterLocationError(callback, identity);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUnRegisterLocationError001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilityReportLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilityReportLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityReportLocationError001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    std::string uuid;
    int32_t errCode = 10;
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->ReportLocationError(uuid, errCode);
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    locatorAbility->ReportLocationError(uuid, errCode);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilityUnRegisterLocationError001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorAbilitySyncIdleState001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorAbilitySyncIdleState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorAbilitySyncIdleState001 begin");
    auto locatorAbility =
        sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    bool state = true;
    locatorAbility->locatorHandler_ = nullptr;
    locatorAbility->SyncIdleState(state);
    state = false;
    locatorAbility->locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
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
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    locatorHandler->GetCachedLocationSuccess(event);
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerGetCachedLocationSuccess001 end");
}

HWTEST_F(LocatorAbilityTest, LocatorHandlerGetCachedLocationFailed001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAbilityTest, LocatorHandlerGetCachedLocationFailed001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorAbilityTest] LocatorHandlerGetCachedLocationFailed001 begin");
    std::shared_ptr<AppExecFwk::EventRunner> runner;
    auto locatorHandler =
        new (std::nothrow) LocatorHandler(runner);
    int state = 1;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
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
    locatorHandler->UpdateLastLocationRequestNum(event);
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
}  // namespace Location
}  // namespace OHOS