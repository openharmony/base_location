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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability_test.h"

#include <cstdlib>

#include "accesstoken_kit.h"
#include "cell_information.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "gnss_event_callback.h"
#include "location_dumper.h"

#include "mock_i_cellular_data_manager.h"
#include "permission_manager.h"
#include "geofence_request.h"

#include "agnss_ni_manager.h"
#include "call_manager_client.h"
#include "string_utils.h"

#include "gnss_interface_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
using HDI::Location::Agnss::V2_0::IAGnssCallback;
using HDI::Location::Agnss::V2_0::AGnssRefInfo;
using HDI::Location::Gnss::V2_0::IGnssCallback;
using HDI::Location::Gnss::V2_0::LocationInfo;
using HDI::Location::Gnss::V2_0::ConstellationCategory;
using HDI::Location::Agnss::V2_0::AGnssRefInfoType;
const uint32_t EVENT_SEND_SWITCHSTATE_TO_HIFENCE = 0x0006;
const int32_t LOCATION_PERM_NUM = 5;
const std::string ARGS_HELP = "-h";
const std::string MANAGER_SETTINGS = "ohos.permission.MANAGE_SETTINGS";
constexpr const char *UNLOAD_GNSS_TASK = "gnss_sa_unload";
constexpr int32_t FENCE_MAX_ID = 1000000;
const int32_t WAIT_EVENT_TIME = 3;
void GnssAbilityTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    ability_ = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, ability_);
    ability_->ConnectHdi();
    callbackStub_ = new (std::nothrow) GnssStatusCallbackNapi();
    EXPECT_NE(nullptr, callbackStub_);
    nemaCallbackStub_ = new (std::nothrow) NmeaMessageCallbackNapi();
    EXPECT_NE(nullptr, nemaCallbackStub_);
    cachedLocationCallbackStub_ = new (std::nothrow) CachedLocationsCallbackNapi();
    EXPECT_NE(nullptr, cachedLocationCallbackStub_);
    proxy_ = new (std::nothrow) GnssAbilityProxy(ability_);
    EXPECT_NE(nullptr, proxy_);
    agnssCallback_ = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback_);
}

void GnssAbilityTest::TearDown()
{
    /*
     * @tc.teardown: release memory.
     */
    ability_->RemoveHdi();
    proxy_ = nullptr;
    callbackStub_ = nullptr;
    nemaCallbackStub_ = nullptr;
    cachedLocationCallbackStub_ = nullptr;
    ability_->gnssHandler_->RemoveTask(UNLOAD_GNSS_TASK);
    agnssCallback_ = nullptr;
    ability_ = nullptr;
}

void GnssAbilityTest::TearDownTestCase()
{
    sleep(WAIT_EVENT_TIME);
}

void GnssAbilityTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        MANAGER_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "GnssAbilityTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

/*
 * @tc.name: SendLocationRequest001
 * @tc.desc: Build Request, marshall and unmarshall data Then Send it
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SendLocationRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendLocationRequest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendLocationRequest001 begin");
    /*
     * @tc.steps: step1. build location request data.
     */
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    int num = 2;
    for (int i = 0; i < num; i++) {
        std::shared_ptr<Request> request = std::make_shared<Request>();
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetTimeInterval(i);
        request->SetUid(i + 1);
        request->SetPid(i + 2);
        request->SetPackageName("nameForTest");
        request->SetRequestConfig(*requestConfig);
        request->SetUuid(std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM)));
        request->SetNlpRequestType(i + 1);
        workRecord->Add(request);
    }
    /*
     * @tc.steps: step2. send location request
     * @tc.expected: step2. no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SendLocationRequest(*workRecord));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendLocationRequest001 end");
}

/*
 * @tc.name: SetEnableAndDisable001
 * @tc.desc: Test disable and enable system ability
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SetEnableAndDisable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetEnableAndDisable001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SetEnableAndDisable001 begin");

    /*
     * @tc.steps: step1.remove SA
     * @tc.expected: step1. object1 is null.
     */
    MessageParcel data1;
    data1.WriteBool(false); // if the state is false
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(data1.ReadBool()));

    /*
     * @tc.steps: step2. test enable SA
     * @tc.expected: step2. object2 is not null.
     */
    MessageParcel data2;
    data2.WriteBool(true); // if the state is true
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(data2.ReadBool()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SetEnableAndDisable001 end");
}

/*
 * @tc.name: RefrashRequirements001
 * @tc.desc: Test refrash requirements
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RefrashRequirements001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RefrashRequirements001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RefrashRequirements001 begin");
    /*
     * @tc.steps: step1. test refrash requirements
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->RefrashRequirements());
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RefrashRequirements001 end");
}

/*
 * @tc.name: RegisterGnssStatusCallback001
 * @tc.desc: Test register gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterGnssStatusCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    pid_t lastCallingUid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: log info : "SendRegisterMsgToRemote callback is nullptr".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->RegisterGnssStatusCallback(client, lastCallingUid));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterGnssStatusCallback001 end");
}

/*
 * @tc.name: RegisterAndUnregisterGnssStatusCallback001
 * @tc.desc: Test register and unregister gnss status callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterGnssStatusCallback001 begin");
    /*
     * @tc.steps: step1. give the last calling uid
     */
    pid_t lastCallingUid = 1;

    /*
     * @tc.steps: step2. test register gnss status callback
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->RegisterGnssStatusCallback(callbackStub_->AsObject(), lastCallingUid));

    /*
     * @tc.steps: step3. test unregister gnss status callback
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->UnregisterGnssStatusCallback(callbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterGnssStatusCallback001 end");
}

/*
 * @tc.name: UnregisterGnssStatusCallback001
 * @tc.desc: Test unregister gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnregisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterGnssStatusCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister gnss status callback
     * @tc.expected: log info : "unregister an invalid gnssStatus callback".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->UnregisterGnssStatusCallback(client));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterGnssStatusCallback001 end");
}

/*
 * @tc.name: RegisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterNmeaMessageCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    pid_t uid = 1;
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: log info : "register an invalid nmea callback".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->RegisterNmeaMessageCallback(client, uid));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterNmeaMessageCallback001 end");
}

/*
 * @tc.name: RegisterAndUnregisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterNmeaMessageCallback001 begin");
    /*
     * @tc.steps: step1.the client is not null.
     */
    pid_t uid = 1;

    /*
     * @tc.steps: step2. test register nmea message callback
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->RegisterNmeaMessageCallback(nemaCallbackStub_->AsObject(), uid));

    /*
     * @tc.steps: step3. test unregister nmea message callback
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->UnregisterNmeaMessageCallback(nemaCallbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterNmeaMessageCallback001 end");
}

/*
 * @tc.name: UnregisterNmeaMessageCallback001
 * @tc.desc: Test unregister nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnregisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterNmeaMessageCallback001 begin");
    /*
     * @tc.steps: step1.the client is null.
     */
    sptr<IRemoteObject> client = nullptr;

    /*
     * @tc.steps: step2. test unregister nmea message callback
     * @tc.expected: log info : "unregister an invalid nmea callback".
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->UnregisterNmeaMessageCallback(client));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterNmeaMessageCallback001 end");
}

/*
 * @tc.name: RegisterCachedCallback001
 * @tc.desc: Test register cache call back
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterCachedCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterCachedCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback001 begin");
    /*
     * @tc.steps: step1.prepare request config and the call back is null.
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = 100;
    requestConfig->wakeUpCacheQueueFull = true;

    sptr<IRemoteObject> callback = nullptr;

    /*
     * @tc.steps: step2. test register cached call back if call back is null.
     * @tc.expected: log info : "register an invalid cached location callback"
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->RegisterCachedCallback(requestConfig, callback));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback001 end");
}

/*
 * @tc.name: RegisterCachedCallback002
 * @tc.desc: Test register cache call back and the queue is not full.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterCachedCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterCachedCallback003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback003 begin");
    /*
     * @tc.steps: step1.prepare request config and the call back is not null.
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = 100;
    requestConfig->wakeUpCacheQueueFull = false;

    /*
     * @tc.steps: step2. test register cached call back if call back is not null.
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED,
        proxy_->RegisterCachedCallback(requestConfig, cachedLocationCallbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterCachedCallback003 end");
}

/*
 * @tc.name: RegisterAndUnregisterCachedCallback001
 * @tc.desc: Test register and unregister cache call back and the queue is full.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterCachedCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterAndUnregisterCachedCallback002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterCachedCallback002 begin");
    /*
     * @tc.steps: step1.prepare request config and the call back is not null.
     */
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = 100;
    requestConfig->wakeUpCacheQueueFull = true;

    /*
     * @tc.steps: step2. test register cached call back if call back is not null.
     * @tc.expected: no exception happens
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED,
        proxy_->RegisterCachedCallback(requestConfig, cachedLocationCallbackStub_->AsObject()));

    /*
     * @tc.steps: step3. test unregister cached call back if call back is not null.
     * @tc.expected: no exception happens.
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->UnregisterCachedCallback(cachedLocationCallbackStub_->AsObject()));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] RegisterAndUnregisterCachedCallback002 end");
}

/*
 * @tc.name: UnregisterCachedCallback001
 * @tc.desc: Test unregister cache call back and call back is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterCachedCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnregisterCachedCallback001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterCachedCallback001 begin");
    /*
     * @tc.steps: step1. the call back is null.
     */
    sptr<IRemoteObject> callback = nullptr;

    /*
     * @tc.steps: step2. test unregister cached call back if call back is null.
     * @tc.expected: log info : "register an invalid cached location callback"
     */
    EXPECT_EQ(ERRCODE_INVALID_PARAM, proxy_->UnregisterCachedCallback(callback));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] UnregisterCachedCallback001 end");
}

/*
 * @tc.name: GetCachedGnssLocationsSize001
 * @tc.desc: Test get cached gnss locations size
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, GetCachedGnssLocationsSize001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCachedGnssLocationsSize001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GetCachedGnssLocationsSize001 begin");
    /*
     * @tc.steps: step1. test get cached gnss locations size.
     * @tc.expected: size equals -1.
     */
    int size = -1;
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->GetCachedGnssLocationsSize(size));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GetCachedGnssLocationsSize001 end");
}

/*
 * @tc.name: FlushCachedGnssLocations001
 * @tc.desc: Test unregister country code call back
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, FlushCachedGnssLocations001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, FlushCachedGnssLocations001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] FlushCachedGnssLocations001 begin");
    /*
     * @tc.steps: step1. test flush cached gnss locations.
     * @tc.expected: reply code unsupport
     */
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->FlushCachedGnssLocations());
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] FlushCachedGnssLocations001 end");
}

/*
 * @tc.name: SendCommand001
 * @tc.desc: Test send
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SendCommand001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendCommand001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendCommand001 begin");
    /*
     * @tc.steps: step1. build location command
     */
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario = 1;
    locationCommand->command = true;

    /*
     * @tc.steps: step2. test send command.
     * @tc.expected: current function is empty, nothing happens
     */
    ability_->SendCommand(locationCommand);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendCommand001 end");
}

HWTEST_F(GnssAbilityTest, SendCommand003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendCommand003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendCommand003 begin");
    std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
    locationCommand->scenario = 1;
    locationCommand->command = true;
    proxy_->SendCommand(locationCommand);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] SendCommand003 end");
}

HWTEST_F(GnssAbilityTest, AddFence003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AddFence003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddFence003 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    proxy_->AddFence(request);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddFence003 end");
}

HWTEST_F(GnssAbilityTest, RemoveFence003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveFence003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveFence003 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    proxy_->RemoveFence(request);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveFence003 end");
}

HWTEST_F(GnssAbilityTest, AddGnssGeofence003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AddGnssGeofence003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddGnssGeofence003 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    proxy_->AddGnssGeofence(request);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddGnssGeofence003 end");
}

HWTEST_F(GnssAbilityTest, RemoveGnssGeofence003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveGnssGeofence003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofence003 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    proxy_->RemoveGnssGeofence(request);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofence003 end");
}

HWTEST_F(GnssAbilityTest, QuerySupportCoordinateSystemType002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, QuerySupportCoordinateSystemType002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] QuerySupportCoordinateSystemType002 begin");
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    proxy_->QuerySupportCoordinateSystemType(coordinateSystemTypes);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] QuerySupportCoordinateSystemType002 end");
}

HWTEST_F(GnssAbilityTest, SendNetworkLocation003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendNetworkLocation003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendNetworkLocation003 begin");
    std::unique_ptr<Location> location = std::make_unique<Location>();
    proxy_->SendNetworkLocation(location);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendNetworkLocation003 end");
}

HWTEST_F(GnssAbilityTest, GnssLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssLocationMock001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssLocationMock001 begin");
    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->EnableMock());
    EXPECT_EQ(true, ability_->IsMockEnabled());
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetMocked(timeInterval, locations));

    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->DisableMock());
    EXPECT_EQ(false, ability_->IsMockEnabled());
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, proxy_->SetMocked(timeInterval, locations));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssLocationMock001 begin");
}

HWTEST_F(GnssAbilityTest, GnssOnStartAndOnStop001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssOnStartAndOnStop001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStartAndOnStop001 begin");
    ability_->OnStart(); // start ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, ability_->QueryServiceState()); // mock will return nullptr

    ability_->OnStop(); // stop ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, ability_->QueryServiceState()); // mock will return nullptr
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStartAndOnStop001 end");
}

HWTEST_F(GnssAbilityTest, GnssDump001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssDump001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDump001 begin");
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
    EXPECT_EQ(ERR_OK, ability_->Dump(fd, args));

    std::vector<std::u16string> emptyArgs;
    EXPECT_EQ(ERR_OK, ability_->Dump(fd, emptyArgs));

    std::vector<std::u16string> helpArgs;
    std::u16string helpArg1 = Str8ToStr16(ARGS_HELP);
    helpArgs.emplace_back(helpArg1);
    EXPECT_EQ(ERR_OK, ability_->Dump(fd, helpArgs));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDump001 end");
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendReportMockLocationEvent001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent001 begin");
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
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
    parcel.WriteInt32(1); // isFromMock is true
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->EnableMock());
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->SetMocked(timeInterval, locations));
    sleep(2);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent001 end");
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendReportMockLocationEvent002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent002 begin");
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
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
    parcel.WriteInt32(0); // isFromMock is false
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->EnableMock());
    EXPECT_EQ(ERRCODE_SUCCESS, ability_->SetMocked(timeInterval, locations));
    sleep(2);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent002 end");
}

HWTEST_F(GnssAbilityTest, GnssAbilityReportSv001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssAbilityReportSv001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssAbilityReportSv001 begin");
    std::unique_ptr<SatelliteStatus> status = std::make_unique<SatelliteStatus>();
    MessageParcel parcel;
    int sateNum = 2;
    parcel.WriteInt64(2); // satellitesNumber
    for (int i = 0; i < sateNum; i++) {
        parcel.WriteInt64(i); // satelliteId
        parcel.WriteDouble(i + 1.0); // carrierToNoiseDensity
        parcel.WriteDouble(i + 2.0); // altitude
        parcel.WriteDouble(i + 3.0); // azimuth
        parcel.WriteDouble(i + 4.0); // carrierFrequency
        parcel.WriteInt64(i + 5.0); // constellation type
    }
    ASSERT_TRUE(status != nullptr);
    status->ReadFromParcel(parcel);
    ASSERT_TRUE(ability_ != nullptr);
    ability_->ReportSv(status);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssAbilityReportSv001 end");
}

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestSetUpAgnssDataLink001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestSetUpAgnssDataLink001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSetUpAgnssDataLink001 begin");
    AGnssDataLinkRequest request;
    request.agnssType = HDI::Location::Agnss::V2_0::AGNSS_TYPE_SUPL;
    request.setUpType = HDI::Location::Agnss::V2_0::ESTABLISH_DATA_CONNECTION;
    EXPECT_EQ(ERR_OK, agnssCallback_->RequestSetUpAgnssDataLink(request));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSetUpAgnssDataLink001 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestSubscriberSetId001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestSubscriberSetId001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSubscriberSetId001 begin");
    sptr<IAGnssCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_NE(nullptr, agnssCallback);
    SubscriberSetIdType type = HDI::Location::Agnss::V2_0::AGNSS_SETID_TYPE_IMSI;
    EXPECT_EQ(ERR_OK, agnssCallback->RequestSubscriberSetId(type));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestSubscriberSetId001 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo001 begin");
    sptr<IAGnssCallback> agnssCallback = new (std::nothrow) AGnssEventCallback();
    EXPECT_CALL(MockICellularDataManager::GetInstance(), GetDefaultCellularDataSlotId).WillRepeatedly(Return(-1));
    EXPECT_NE(nullptr, agnssCallback);
    AGnssRefInfoType type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    EXPECT_EQ(ERR_OK, agnssCallback->RequestAgnssRefInfo(type));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo001 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo002 begin");
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::GsmCellInformation> gsmCellInformation = new Telephony::GsmCellInformation();
    gsmCellInformation->Init(0, 0, 0);
    agnssCallback_->JudgmentDataGsm(refInfo, gsmCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_GSM, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo002 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo003 begin");
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::GsmCellInformation> gsmCellInformation = nullptr;
    agnssCallback_->JudgmentDataGsm(refInfo, gsmCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_GSM, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo003 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo004, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo004 begin");
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::LteCellInformation> lteCellInformation = new Telephony::LteCellInformation();
    lteCellInformation->Init(0, 0, 0);
    agnssCallback_->JudgmentDataLte(refInfo, lteCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_LTE, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo004 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo005, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo005 begin");
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::LteCellInformation> lteCellInformation = nullptr;
    agnssCallback_->JudgmentDataLte(refInfo, lteCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_LTE, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo005 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo006, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo006 begin");

    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::WcdmaCellInformation> umtsCellInformation = new Telephony::WcdmaCellInformation();
    umtsCellInformation->Init(0, 0, 0);
    agnssCallback_->JudgmentDataUmts(refInfo, umtsCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_UMTS, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo006 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo007, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo007 begin");

    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::WcdmaCellInformation> umtsCellInformation = nullptr;
    agnssCallback_->JudgmentDataUmts(refInfo, umtsCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_UMTS, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo007 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo008, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo008 begin");

    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::NrCellInformation> nrCellInformation = new Telephony::NrCellInformation();
    nrCellInformation->Init(0, 0, 0);
    agnssCallback_->JudgmentDataNr(refInfo, nrCellInformation);
    EXPECT_EQ(HDI::Location::Agnss::V2_0::CELLID_TYPE_NR, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo008 end");
}

HWTEST_F(GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AGnssEventCallbackRequestAgnssRefInfo009, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo009 begin");
    AGnssRefInfo refInfo;
    refInfo.type = HDI::Location::Agnss::V2_0::ANSS_REF_INFO_TYPE_CELLID;
    sptr<Telephony::NrCellInformation> nrCellInformation = nullptr;
    agnssCallback_->JudgmentDataNr(refInfo, nrCellInformation);
    EXPECT_NE(HDI::Location::Agnss::V2_0::CELLID_TYPE_NR, refInfo.cellId.type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] AGnssEventCallbackRequestAgnssRefInfo009 end");
}
#endif

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportLocation001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    gnssCallback->ReportLocation(locationInfo);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportLocation002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportLocation002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    proxy_->EnableMock();
    auto ret = gnssCallback->ReportLocation(locationInfo);
    EXPECT_EQ(ERR_OK, ret);
    proxy_->DisableMock();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportLocation002 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssWorkingStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssWorkingStatus001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssWorkingStatus001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssWorkingStatus status = HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_NONE;
    gnssCallback->ReportGnssWorkingStatus(status);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssWorkingStatus001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssWorkingStatus002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssWorkingStatus002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssWorkingStatus002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssWorkingStatus status = HDI::Location::Gnss::V2_0::GNSS_WORKING_STATUS_NONE;
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility = nullptr;
    gnssCallback->ReportGnssWorkingStatus(status);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssWorkingStatus002 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportNmea001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportNmea001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportNmea001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    gnssCallback->ReportNmea(0, "nmea", 0);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportNmea001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportNmea002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportNmea002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportNmea002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility = nullptr;
    gnssCallback->ReportNmea(0, "nmea", 0);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportNmea002 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssCapabilities001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssCapabilities001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssCapabilities001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssCapabilities capabilities = HDI::Location::Gnss::V2_0::GNSS_CAP_SUPPORT_MSB;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportGnssCapabilities(capabilities));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssCapabilities001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssCapabilities002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssCapabilities002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssCapabilities002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssCapabilities capabilities = HDI::Location::Gnss::V2_0::GNSS_CAP_SUPPORT_MSB;
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility = nullptr;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportGnssCapabilities(capabilities));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssCapabilities002 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    SatelliteStatusInfo statusInfo;
    statusInfo.satellitesNumber = 0;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportSatelliteStatusInfo(statusInfo));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo002 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportSatelliteStatusInfo003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo003 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    SatelliteStatusInfo statusInfo;
    statusInfo.satellitesNumber = 1;
    statusInfo.elevation.push_back(12);
    statusInfo.azimuths.push_back(30);
    statusInfo.carrierFrequencies.push_back(40);
    statusInfo.carrierToNoiseDensitys.push_back(40);
    statusInfo.satelliteIds.push_back(1);
    statusInfo.constellation.push_back(static_cast<ConstellationCategory>(1));
    statusInfo.additionalInfo.push_back(
        HDI::Location::Gnss::V2_0::SATELLITES_ADDITIONAL_INFO_EPHEMERIS_DATA_EXIST);
    EXPECT_EQ(ERR_OK, gnssCallback->ReportSatelliteStatusInfo(statusInfo));
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    EXPECT_EQ(ERR_OK, gnssCallback->ReportLocation(locationInfo));
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportSatelliteStatusInfo003 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackRequestGnssReferenceInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackRequestGnssReferenceInfo001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestGnssReferenceInfo001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssRefInfoType type = HDI::Location::Gnss::V2_0::GNSS_REF_INFO_TIME;
    gnssCallback->RequestGnssReferenceInfo(type);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestGnssReferenceInfo001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackRequestPredictGnssData001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackRequestPredictGnssData001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestPredictGnssData001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    gnssCallback->RequestPredictGnssData();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackRequestPredictGnssData001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportCachedLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportCachedLocation001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportCachedLocation001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    std::vector<LocationInfo> gnssLocations;
    LocationInfo locationInfo;
    locationInfo.latitude = 1.0;
    locationInfo.longitude = 2.0;
    locationInfo.altitude = 1.0;
    locationInfo.horizontalAccuracy = 1.0;
    locationInfo.speed = 1.0;
    locationInfo.bearing= 1.0;
    locationInfo.timeForFix = 1000000000;
    locationInfo.timeSinceBoot = 1000000000;
    gnssLocations.push_back(locationInfo);
    gnssCallback->ReportCachedLocation(gnssLocations);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportCachedLocation001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssNiNotification001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssNiNotification001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssNiNotification001 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssNiNotificationRequest notif;
    gnssCallback->ReportGnssNiNotification(notif);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssNiNotification001 end");
}

HWTEST_F(GnssAbilityTest, GnssEventCallbackReportGnssNiNotification002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEventCallbackReportGnssNiNotification002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssNiNotification002 begin");
    sptr<IGnssCallback> gnssCallback = new (std::nothrow) GnssEventCallback();
    EXPECT_NE(nullptr, gnssCallback);
    GnssNiNotificationRequest notif;
    notif.notificationCategory = GNSS_NI_NOTIFICATION_REQUIRE_NOTIFY;
    gnssCallback->ReportGnssNiNotification(notif);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEventCallbackReportGnssNiNotification002 end");
}

HWTEST_F(GnssAbilityTest, GeofenceEventCallbackReportGeofenceAvailability001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GeofenceEventCallbackReportGeofenceAvailability001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceAvailability001 begin");
    sptr<IGeofenceCallback> geofenceEventCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceEventCallback);
    geofenceEventCallback->ReportGeofenceAvailability(true);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceAvailability001 end");
}

HWTEST_F(GnssAbilityTest, GeofenceEventCallbackReportGeofenceEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GeofenceEventCallbackReportGeofenceEvent001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceEvent001 begin");
    sptr<IGeofenceCallback> geofenceEventCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceEventCallback);
    int32_t fenceIndex = 0;
    HDI::Location::Geofence::V2_0::LocationInfo location;
    GeofenceEvent event = GeofenceEvent::GEOFENCE_EVENT_ENTERED;
    int64_t timestamp = 0;
    geofenceEventCallback->ReportGeofenceEvent(fenceIndex, location, event, timestamp);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceEvent001 end");
}

HWTEST_F(GnssAbilityTest, GeofenceEventCallbackReportGeofenceEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GeofenceEventCallbackReportGeofenceEvent002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceEvent002 begin");
    sptr<IGeofenceCallback> geofenceEventCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceEventCallback);
    int32_t fenceIndex = 0;
    HDI::Location::Geofence::V2_0::LocationInfo location;
    GeofenceEvent event = GeofenceEvent::GEOFENCE_EVENT_ENTERED;
    int64_t timestamp = 0;
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility = nullptr;
    geofenceEventCallback->ReportGeofenceEvent(fenceIndex, location, event, timestamp);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceEvent002 end");
}

HWTEST_F(GnssAbilityTest, GeofenceEventCallbackReportGeofenceOperateResult001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GeofenceEventCallbackReportGeofenceOperateResult001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceOperateResult001 begin");
    sptr<IGeofenceCallback> geofenceEventCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceEventCallback);
    int32_t fenceIndex = 0;
    GeofenceOperateType type = GeofenceOperateType::TYPE_ADD;
    GeofenceOperateResult result = GeofenceOperateResult::GEOFENCE_OPERATION_SUCCESS;
    geofenceEventCallback->ReportGeofenceOperateResult(fenceIndex, type, result);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceOperateResult001 end");
}

HWTEST_F(GnssAbilityTest, GeofenceEventCallbackReportGeofenceOperateResult002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GeofenceEventCallbackReportGeofenceOperateResult002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceOperateResult002 begin");
    sptr<IGeofenceCallback> geofenceEventCallback = new (std::nothrow) GeofenceEventCallback();
    EXPECT_NE(nullptr, geofenceEventCallback);
    int32_t fenceIndex = 0;
    GeofenceOperateType type = GeofenceOperateType::TYPE_ADD;
    GeofenceOperateResult result = GeofenceOperateResult::GEOFENCE_OPERATION_SUCCESS;
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility = nullptr;
    geofenceEventCallback->ReportGeofenceOperateResult(fenceIndex, type, result);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GeofenceEventCallbackReportGeofenceOperateResult002 end");
}

HWTEST_F(GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent001 begin");
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(AGNSS_NI_ACCEPT_EVENT);
    matchingSkills.AddEvent(AGNSS_NI_REJECT_EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<GnssCommonEventSubscriber> subscriber =
        std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);
    OHOS::EventFwk::CommonEventData eventData = OHOS::EventFwk::CommonEventData();
    OHOS::AAFwk::Want want = OHOS::AAFwk::Want();
    want.SetAction(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    eventData.SetWant(want);
    subscriber->OnReceiveEvent(eventData);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent001 end");
}

HWTEST_F(GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent002 begin");
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(AGNSS_NI_ACCEPT_EVENT);
    matchingSkills.AddEvent(AGNSS_NI_REJECT_EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<GnssCommonEventSubscriber> subscriber =
        std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);
    OHOS::EventFwk::CommonEventData eventData = OHOS::EventFwk::CommonEventData();
    OHOS::AAFwk::Want want = OHOS::AAFwk::Want();
    want.SetAction(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    eventData.SetWant(want);
    subscriber->OnReceiveEvent(eventData);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent002 end");
}

HWTEST_F(GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent003 begin");
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(AGNSS_NI_ACCEPT_EVENT);
    matchingSkills.AddEvent(AGNSS_NI_REJECT_EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<GnssCommonEventSubscriber> subscriber =
        std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);
    OHOS::EventFwk::CommonEventData eventData = OHOS::EventFwk::CommonEventData();
    OHOS::AAFwk::Want want = OHOS::AAFwk::Want();
    want.SetAction(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    eventData.SetWant(want);
    subscriber->OnReceiveEvent(eventData);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent003 end");
}

HWTEST_F(GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent004 begin");
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(AGNSS_NI_ACCEPT_EVENT);
    matchingSkills.AddEvent(AGNSS_NI_REJECT_EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<GnssCommonEventSubscriber> subscriber =
        std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);
    OHOS::EventFwk::CommonEventData eventData = OHOS::EventFwk::CommonEventData();
    OHOS::AAFwk::Want want = OHOS::AAFwk::Want();
    want.SetAction(AGNSS_NI_ACCEPT_EVENT);
    eventData.SetWant(want);
    subscriber->OnReceiveEvent(eventData);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent004 end");
}

HWTEST_F(GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssCommonEventSubscriberOnReceiveEvent005, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent005 begin");
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(AGNSS_NI_ACCEPT_EVENT);
    matchingSkills.AddEvent(AGNSS_NI_REJECT_EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<GnssCommonEventSubscriber> subscriber =
        std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);
    OHOS::EventFwk::CommonEventData eventData = OHOS::EventFwk::CommonEventData();
    OHOS::AAFwk::Want want = OHOS::AAFwk::Want();
    want.SetAction(AGNSS_NI_REJECT_EVENT);
    eventData.SetWant(want);
    subscriber->OnReceiveEvent(eventData);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssCommonEventSubscriberOnReceiveEvent005 end");
}

HWTEST_F(GnssAbilityTest, GnssOnStart001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssOnStart001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStart001 begin");
    ability_->state_ = ServiceRunningState::STATE_RUNNING;
    ability_->OnStart();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssOnStart001 end");
}

HWTEST_F(GnssAbilityTest, GnssSendLocationRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendLocationRequest001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendLocationRequest001 begin");
    WorkRecord workrecord;
    ability_->SendLocationRequest(workrecord);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendLocationRequest001 end");
}

HWTEST_F(GnssAbilityTest, GnssDisableGnss001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssDisableGnss001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDisableGnss001 begin");
    sptr<GnssAbility> gnssAbility1 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility1 != nullptr);
    gnssAbility1->gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_BEGIN;
    gnssAbility1->DisableGnss();

    sptr<GnssAbility> gnssAbility2 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility2 != nullptr);
    gnssAbility2->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    gnssAbility2->DisableGnss();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssDisableGnss001 end");
}

HWTEST_F(GnssAbilityTest, GnssStartGnss001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssStartGnss001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssStartGnss001 begin");
    sptr<GnssAbility> gnssAbility1 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility1 != nullptr);
    gnssAbility1->gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_BEGIN;
    gnssAbility1->StartGnss();

    sptr<GnssAbility> gnssAbility2 = new (std::nothrow) GnssAbility();
    ASSERT_TRUE(gnssAbility2 != nullptr);
    gnssAbility2->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    gnssAbility2->StartGnss();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssStartGnss001 end");
}

HWTEST_F(GnssAbilityTest, GnssEnableGnss001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEnableGnss001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss001 begin");
    ability_->gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
    bool ret = ability_->EnableGnss();
    EXPECT_EQ(false, ret);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss001 end");
}

HWTEST_F(GnssAbilityTest, GnssEnableGnss002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssEnableGnss002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss002 begin");
    ability_->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_ON;
    bool ret = ability_->EnableGnss();
    EXPECT_EQ(false, ret);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssEnableGnss002 end");
}

HWTEST_F(GnssAbilityTest, GnssInit001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssInit001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssInit001 begin");
    ability_->registerToAbility_ = true;
    EXPECT_EQ(true, ability_->Init());
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssInit001 end");
}

HWTEST_F(GnssAbilityTest, GnssRequestRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssRequestRecord001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRequestRecord001 begin");
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    ability_->RequestRecord(*workRecord, false);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRequestRecord001 end");
}

HWTEST_F(GnssAbilityTest, GnssRequestRecord002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssRequestRecord002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRequestRecord002 begin");
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    ability_->RequestRecord(*workRecord, true);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssRequestRecord002 end");
}

HWTEST_F(GnssAbilityTest, GnssReConnectHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssReConnectHdi001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssReConnectHdi001 begin");
    ability_->ReConnectHdi();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssReConnectHdi001 end");
}

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
HWTEST_F(GnssAbilityTest, GnssSetRefInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSetRefInfo001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSetRefInfo001 begin");
    AGnssRefInfo refInfo;
    ability_->SetRefInfo(refInfo);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSetRefInfo001 end");
}
#endif

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendReportMockLocationEvent003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent003 begin");
    ability_->SendReportMockLocationEvent();
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendReportMockLocationEvent003 end");
}

HWTEST_F(GnssAbilityTest, GnssSendMessage001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendMessage001, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage001 begin");
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.INetworkAbility");
    requestParcel.WriteBuffer("data", 4);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    ability_->SendMessage(0, requestParcel, reply);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage001 end");
}

HWTEST_F(GnssAbilityTest, GnssSendMessage002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendMessage002, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage002 begin");
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.INetworkAbility");
    requestParcel.WriteBuffer("data", 4);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    ability_->SendMessage(static_cast<uint32_t>(GnssAbilityInterfaceCode::ADD_FENCE), requestParcel, reply);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage002 end");
}

HWTEST_F(GnssAbilityTest, GnssSendMessage003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendMessage003, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage003 begin");
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.INetworkAbility");
    requestParcel.WriteBuffer("data", 4);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    ability_->SendMessage(0, requestParcel, reply);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage003 end");
}

HWTEST_F(GnssAbilityTest, GnssSendMessage004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssSendMessage004, TestSize.Level1";
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage004 begin");
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.INetworkAbility");
    requestParcel.WriteBuffer("data", 4);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    ability_->SendMessage(0, requestParcel, reply);
    LBSLOGI(GNSS_TEST, "[GnssAbilityTest] GnssSendMessage004 end");
}

HWTEST_F(GnssAbilityTest, SubAbilityCommonGetRequestNum001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "SubAbilityCommonTest, GetRequestNum001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[SubAbilityCommonTest] GetRequestNum001 begin");
    sptr<GnssAbility> gnssAbility = new (std::nothrow) GnssAbility();
    ability_->newRecord_ = nullptr;
    ability_->GetRequestNum();

    ability_->newRecord_ = std::make_unique<WorkRecord>();
    std::shared_ptr<WorkRecord> workRecord = std::make_shared<WorkRecord>();
    int num = 2;
    std::shared_ptr<Request> request = std::make_shared<Request>();
    for (int i = 0; i < num; i++) {
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetTimeInterval(i);
        request->SetUid(i + 1);
        request->SetPid(i + 2);
        request->SetPackageName("nameForTest");
        request->SetRequestConfig(*requestConfig);
        request->SetUuid(std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM)));
        request->SetNlpRequestType(0);
        workRecord->Add(request);
    }
    ability_->newRecord_->Set(*workRecord);
    ability_->GetRequestNum();

    ability_->newRecord_ = nullptr;
    ability_->GetRequestNum();

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetTimeInterval(0);
    request->SetUid(0);
    request->SetPid(0);
    request->SetRequestConfig(*requestConfig);
    request->SetUuid(std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM)));
    request->SetNlpRequestType(0);
    ability_->lastRecord_->Add(request);
    ability_->HandleRemoveRecord(*workRecord);
    ability_->lastRecord_->Clear();
    ability_->lastRecord_->Set(*workRecord);
    ability_->HandleAddRecord(*workRecord);
    LBSLOGI(LOCATOR, "[SubAbilityCommonTest] GetRequestNum001 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags001 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_ephemeris";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_EPHEMERIS, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags001 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags002 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_almanac";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALMANAC, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags002 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags003 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_position";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_POSITION, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags003 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags004, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags004 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_time";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_TIME, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags004 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags005, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags005 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_iono";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_IONO, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags005 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags006, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags006 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_utc";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_UTC, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags006 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags007, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags007 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_health";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_HEALTH, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags007 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags008, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags008 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_svdir";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVDIR, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags008 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags009, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags009 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_svsteer";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVSTEER, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags009 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags010, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags010, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags010 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_sadata";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SADATA, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags010 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags011, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags011, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags011 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_rti";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_RTI, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags011 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags012, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags012, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags012 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_celldb_info";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_CELLDB_INFO, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags012 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags013, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags013, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags013 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_all";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(true, result);
    EXPECT_EQ(GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALL, flags);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags013 end");
}

HWTEST_F(GnssAbilityTest, GetCommandFlags014, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetCommandFlags014, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags014 begin");
    GnssAuxiliaryDataType flags;
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "unknow";
    bool result = ability_->GetCommandFlags(cmd, flags);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetCommandFlags014 end");
}

HWTEST_F(GnssAbilityTest, SetEnable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetEnable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetEnable001 begin");
    LocationErrCode result = ability_->SetEnable(true);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetEnable001 end");
}

HWTEST_F(GnssAbilityTest, SetEnable002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetEnable002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetEnable002 begin");
    LocationErrCode result = ability_->SetEnable(false);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetEnable002 end");
}

HWTEST_F(GnssAbilityTest, SendCommand002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendCommand002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendCommand002 begin");
    std::unique_ptr<LocationCommand> cmd = std::make_unique<LocationCommand>();
    cmd->command = "delete_auxiliary_data_all";
    LocationErrCode result = ability_->SendCommand(cmd);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendCommand002 end");
}

HWTEST_F(GnssAbilityTest, SetPositionMode001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetPositionMode001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetPositionMode001 begin");
    LocationErrCode result = ability_->SetPositionMode();
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetPositionMode001 end");
}

HWTEST_F(GnssAbilityTest, AddFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AddFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddFence001 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    LocationErrCode result = ability_->AddFence(request);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddFence001 end");
}

HWTEST_F(GnssAbilityTest, RemoveFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveFence001 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    LocationErrCode result = ability_->RemoveFence(request);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveFence001 end");
}

HWTEST_F(GnssAbilityTest, RemoveFence002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveFence002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveFence002 begin");
    std::shared_ptr<GeofenceRequest> request = nullptr;
    ability_->RemoveFence(request);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveFence002 end");
}

HWTEST_F(GnssAbilityTest, AddGnssGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AddGnssGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddGnssGeofence001 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    LocationErrCode result = ability_->AddGnssGeofence(request);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddGnssGeofence001 end");
}

HWTEST_F(GnssAbilityTest, RemoveGnssGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveGnssGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofence001 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    LocationErrCode result = ability_->RemoveGnssGeofence(request);
    EXPECT_EQ(ERRCODE_GEOFENCE_INCORRECT_ID, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofence001 end");
}

HWTEST_F(GnssAbilityTest, RemoveGnssGeofence002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveGnssGeofence002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofence002 begin");
    std::shared_ptr<GeofenceRequest> request = nullptr;
    LocationErrCode result = ability_->RemoveGnssGeofence(request);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofence002 end");
}

HWTEST_F(GnssAbilityTest, RegisterGnssGeofenceCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterGnssGeofenceCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RegisterGnssGeofenceCallback001 begin");
    std::shared_ptr<GeofenceRequest> request = nullptr;
    sptr<IRemoteObject> callback = nullptr;
    bool result = ability_->RegisterGnssGeofenceCallback(request, callback);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RegisterGnssGeofenceCallback001 end");
}

HWTEST_F(GnssAbilityTest, UnregisterGnssGeofenceCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnregisterGnssGeofenceCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnregisterGnssGeofenceCallback001 begin");
    int fenceId = 1;
    bool result = ability_->UnregisterGnssGeofenceCallback(fenceId);
    EXPECT_EQ(true, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnregisterGnssGeofenceCallback001 end");
}

HWTEST_F(GnssAbilityTest, RemoveGnssGeofenceRequestByCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveGnssGeofenceRequestByCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofenceRequestByCallback001 begin");
    sptr<IRemoteObject> callbackObj = nullptr;
    bool result = ability_->RemoveGnssGeofenceRequestByCallback(callbackObj);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofenceRequestByCallback001 end");
}

#ifdef HDF_DRIVERS_INTERFACE_GEOFENCE_ENABLE
HWTEST_F(GnssAbilityTest, ReportGeofenceOperationResult001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ReportGeofenceOperationResult001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReportGeofenceOperationResult001 begin");
    int fenceId = 0;
    GeofenceOperateType type = GeofenceOperateType::TYPE_DELETE;
    GeofenceOperateResult result = GeofenceOperateResult::GEOFENCE_OPERATION_SUCCESS;
    ability_->ReportGeofenceOperationResult(fenceId, type, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReportGeofenceOperationResult001 end");
}

HWTEST_F(GnssAbilityTest, ReportGeofenceEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ReportGeofenceEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReportGeofenceEvent001 begin");
    int fenceId = 0;
    GeofenceEvent event = GeofenceEvent::GEOFENCE_EVENT_ENTERED;
    ability_->ReportGeofenceEvent(fenceId, event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReportGeofenceEvent001 end");
}

HWTEST_F(GnssAbilityTest, GetGeofenceRequestByFenceId001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GetGeofenceRequestByFenceId001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetGeofenceRequestByFenceId001 begin");
    int fenceId = 0;
    ability_->GetGeofenceRequestByFenceId(fenceId);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GetGeofenceRequestByFenceId001 end");
}

HWTEST_F(GnssAbilityTest, ExecuteFenceProcess001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ExecuteFenceProcess001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ExecuteFenceProcess001 begin");
    GnssInterfaceCode code= GnssInterfaceCode::ADD_FENCE_INFO;
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    ability_->ExecuteFenceProcess(code, request);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ExecuteFenceProcess001 end");
}

HWTEST_F(GnssAbilityTest, SetGeofenceCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetGeofenceCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetGeofenceCallback001 begin");
    ability_->SetGeofenceCallback();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetGeofenceCallback001 end");
}

HWTEST_F(GnssAbilityTest, StopGnss001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StopGnss001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StopGnss001 begin");
    ability_->StopGnss();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StopGnss001 end");
}

HWTEST_F(GnssAbilityTest, RemoveHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveHdi001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveHdi001 begin");
    ability_->RemoveHdi();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveHdi001 end");
}

HWTEST_F(GnssAbilityTest, QuerySupportCoordinateSystemType001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, QuerySupportCoordinateSystemType001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] QuerySupportCoordinateSystemType001 begin");
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    ability_->QuerySupportCoordinateSystemType(coordinateSystemTypes);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] QuerySupportCoordinateSystemType001 end");
}

#endif

HWTEST_F(GnssAbilityTest, ReConnectHdiImpl001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ReConnectHdiImpl001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReConnectHdiImpl001 begin");
    ability_->ReConnectHdiImpl();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReConnectHdiImpl001 end");
}

HWTEST_F(GnssAbilityTest, InjectTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, InjectTime001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectTime001 begin");
    ability_->InjectTime();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectTime001 end");
}

HWTEST_F(GnssAbilityTest, SendNetworkLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendNetworkLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendNetworkLocation001 begin");
    std::unique_ptr<Location> location = nullptr;
    LocationErrCode result = ability_->SendNetworkLocation(location);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendNetworkLocation001 end");
}

HWTEST_F(GnssAbilityTest, SendNetworkLocation002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendNetworkLocation002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendNetworkLocation002 begin");
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->SetLatitude(31.2568);
    LocationErrCode result = ability_->SendNetworkLocation(location);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendNetworkLocation002 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerRun001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerRun001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerRun001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->Run();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerRun001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerUnRegisterAgnssNiEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerUnRegisterAgnssNiEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerUnRegisterAgnssNiEvent001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->UnRegisterAgnssNiEvent();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerUnRegisterAgnssNiEvent001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerCheckSmsSuplInit001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerCheckSmsSuplInit001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerCheckSmsSuplInit001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    EventFwk::Want want;
    want.SetParam("slotId", 0);
    std::vector<std::string> newPdus = {"0891683108200075F4240D91688129562983F600001240800102142302C130"};
    want.SetParam("pdus", newPdus);
    want.SetParam("isCdma", true);
    agnssNiManager->CheckSmsSuplInit(want);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerCheckSmsSuplInit001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerCheckWapSuplInit001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerCheckWapSuplInit001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerCheckWapSuplInit001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    EventFwk::Want want;
    want.SetParam("slotId", 0);
    std::string appId = "16";
    want.SetParam("applicationId", appId);
    agnssNiManager->CheckWapSuplInit(want);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerCheckWapSuplInit001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerCheckWapSuplInit002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerCheckWapSuplInit002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerCheckWapSuplInit002 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    EventFwk::Want want;
    want.SetParam("slotId", 0);
    std::string appId = "0";
    want.SetParam("applicationId", appId);
    std::string rawData = "0891683108200075F4240D91688129562983F600001240800102142302C130";
    want.SetParam("rawData", rawData);
    agnssNiManager->CheckWapSuplInit(want);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerCheckWapSuplInit002 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerOnCallStateChanged001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerOnCallStateChanged001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnCallStateChanged001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    EventFwk::Want want;
    want.SetParam("state", (int32_t)Telephony::TelCallState::CALL_STATUS_DIALING);
    want.SetParam("slotId", 0);
    std::string number = "110";
    want.SetParam("number", number);
    agnssNiManager->OnCallStateChanged(want);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnCallStateChanged001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerSendUserResponse001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerSendUserResponse001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerSendUserResponse001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->SendUserResponse(GNSS_NI_RESPONSE_CMD_ACCEPT);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerSendUserResponse001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerSendUserResponse002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerSendUserResponse002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerSendUserResponse002 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->SendUserResponse(GNSS_NI_RESPONSE_CMD_ACCEPT);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerSendUserResponse002 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerOnAddSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerOnAddSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnAddSystemAbility001 begin");
    auto statusChangeListener = new SystemAbilityStatusChangeListener();
    int32_t systemAbilityId = -1;
    const std::string deviceId = "test";
    statusChangeListener->OnAddSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnAddSystemAbility001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerOnAddSystemAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerOnAddSystemAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnAddSystemAbility002 begin");
    auto statusChangeListener = new SystemAbilityStatusChangeListener();
    int32_t systemAbilityId = COMMON_EVENT_SERVICE_ID;
    const std::string deviceId = "test";
    statusChangeListener->OnAddSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnAddSystemAbility002 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerOnRemoveSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerOnRemoveSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnRemoveSystemAbility001 begin");
    auto statusChangeListener = new SystemAbilityStatusChangeListener();
    int32_t systemAbilityId = -1;
    const std::string deviceId = "test";
    statusChangeListener->OnRemoveSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnRemoveSystemAbility001 end");
}

HWTEST_F(GnssAbilityTest, AgnssNiManagerOnRemoveSystemAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AgnssNiManagerOnRemoveSystemAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnRemoveSystemAbility002 begin");
    auto statusChangeListener = new SystemAbilityStatusChangeListener();
    int32_t systemAbilityId = COMMON_EVENT_SERVICE_ID;
    const std::string deviceId = "test";
    statusChangeListener->OnRemoveSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AgnssNiManagerOnRemoveSystemAbility002 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsStringToHex001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsStringToHex001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsStringToHex001 begin");
    std::string str = "0314";
    EXPECT_NE("", StringUtils::StringToHex(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsStringToHex001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsStringToHex002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsStringToHex002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsStringToHex002 begin");
    const char *data = "0314";
    int byteLength = 4;
    EXPECT_NE("", StringUtils::StringToHex(data, byteLength));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsStringToHex002 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsStringToHex003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsStringToHex003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsStringToHex003 begin");
    std::vector<uint8_t> data = {0, 3, 1, 4};
    EXPECT_NE("", StringUtils::StringToHex(data));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsStringToHex003 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsHexToString001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsHexToString001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToString001 begin");
    std::string str = "0314";
    EXPECT_NE("", StringUtils::HexToString(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToString001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsHexToString002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsHexToString002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToString002 begin");
    std::string str = "";
    EXPECT_EQ("", StringUtils::HexToString(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToString002 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsToUtf8001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsToUtf8001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf8001 begin");
    std::u16string str = u"hello, world!";
    EXPECT_NE("", StringUtils::ToUtf8(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf8001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsToUtf8002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsToUtf8002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf8002 begin");
    std::u16string str = u"";
    EXPECT_EQ("", StringUtils::ToUtf8(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf8002 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsToUtf16001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsToUtf16001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf16001 begin");
    std::string str = "0314";
    EXPECT_NE(u"", StringUtils::ToUtf16(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf16001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsToUtf16002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsToUtf16002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf16002 begin");
    std::string str = "";
    EXPECT_EQ(u"", StringUtils::ToUtf16(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsToUtf16002 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsUtf8ToWstring001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsUtf8ToWstring001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsUtf8ToWstring001 begin");
    std::string str = "hello world";
    EXPECT_NE(L"", StringUtils::Utf8ToWstring(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsUtf8ToWstring001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsUcs2ToWstring001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsUcs2ToWstring001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsUcs2ToWstring001 begin");
    std::string str = "0xD869";
    EXPECT_NE(L"", StringUtils::Ucs2ToWstring(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsUcs2ToWstring001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsWstringToString001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsWstringToString001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsWstringToString001 begin");
    std::wstring str = L"中文";
    EXPECT_NE("", StringUtils::WstringToString(str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsWstringToString001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsHexToByteVector001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsHexToByteVector001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector001 begin");
    std::string str = "0314";
    std::vector<uint8_t> ret = StringUtils::HexToByteVector(str);
    EXPECT_NE(0, ret.size());
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsHexToByteVector002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsHexToByteVector002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector002 begin");
    std::string str = "ABCDEF";
    std::vector<uint8_t> ret = StringUtils::HexToByteVector(str);
    EXPECT_NE(0, ret.size());
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector002 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsHexToByteVector003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsHexToByteVector003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector003 begin");
    std::string str = "abcdef";
    std::vector<uint8_t> ret = StringUtils::HexToByteVector(str);
    EXPECT_NE(0, ret.size());
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector003 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsHexToByteVector004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsHexToByteVector003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector003 begin");
    std::string str = "";
    std::vector<uint8_t> ret = StringUtils::HexToByteVector(str);
    EXPECT_EQ(0, ret.size());
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector003 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsHexToByteVector005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsHexToByteVector005, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector005 begin");
    std::string str = "     ";
    std::vector<uint8_t> ret = StringUtils::HexToByteVector(str);
    EXPECT_EQ(2, ret.size());
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsHexToByteVector005 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsGsm7Decode001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsGsm7Decode001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsGsm7Decode001 begin");
    std::string gsm7Str = "ABCDEFGHI";
    EXPECT_NE("", StringUtils::Gsm7Decode(gsm7Str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsGsm7Decode001 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsGsm7Decode002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsGsm7Decode002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsGsm7Decode002 begin");
    std::string gsm7Str = "abcdefg";
    EXPECT_NE("", StringUtils::Gsm7Decode(gsm7Str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsGsm7Decode002 end");
}

HWTEST_F(GnssAbilityTest, StringUtilsGsm7Decode003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StringUtilsGsm7Decode003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsGsm7Decode003 begin");
    std::string gsm7Str = "   ";
    EXPECT_NE("", StringUtils::Gsm7Decode(gsm7Str));
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StringUtilsGsm7Decode003 end");
}

HWTEST_F(GnssAbilityTest, GnssStatusCallbackDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssStatusCallbackDeathRecipient001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GnssStatusCallbackDeathRecipient001 begin");
    auto gnssStatusCallbackDeathRecipient = new (std::nothrow) GnssStatusCallbackDeathRecipient();
    EXPECT_NE(nullptr, gnssStatusCallbackDeathRecipient);
    wptr<IRemoteObject> remote;
    gnssStatusCallbackDeathRecipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GnssStatusCallbackDeathRecipient001 end");
}

HWTEST_F(GnssAbilityTest, NmeaCallbackDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, NmeaCallbackDeathRecipient001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] NmeaCallbackDeathRecipient001 begin");
    auto nmeaCallbackDeathRecipient = new (std::nothrow) NmeaCallbackDeathRecipient();
    EXPECT_NE(nullptr, nmeaCallbackDeathRecipient);
    wptr<IRemoteObject> remote;
    nmeaCallbackDeathRecipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] NmeaCallbackDeathRecipient001 end");
}

HWTEST_F(GnssAbilityTest, CachedLocationCallbackDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, CachedLocationCallbackDeathRecipient001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] CachedLocationCallbackDeathRecipient001 begin");
    auto cachedLocationCallbackDeathRecipient = new (std::nothrow) CachedLocationCallbackDeathRecipient();
    EXPECT_NE(nullptr, cachedLocationCallbackDeathRecipient);
    wptr<IRemoteObject> remote;
    cachedLocationCallbackDeathRecipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] CachedLocationCallbackDeathRecipient001 end");
}

HWTEST_F(GnssAbilityTest, GnssGeofenceCallbackDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, GnssGeofenceCallbackDeathRecipient001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GnssGeofenceCallbackDeathRecipient001 begin");
    auto gnssGeofenceCallbackDeathRecipient = new (std::nothrow) GnssGeofenceCallbackDeathRecipient();
    EXPECT_NE(nullptr, gnssGeofenceCallbackDeathRecipient);
    wptr<IRemoteObject> remote;
    gnssGeofenceCallbackDeathRecipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] GnssGeofenceCallbackDeathRecipient001 end");
}

HWTEST_F(GnssAbilityTest, ReConnectHdiImpl002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ReConnectHdiImpl002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReConnectHdiImpl002 begin");
    ability_->gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_BEGIN;
    ability_->ReConnectHdiImpl();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReConnectHdiImpl002 end");
}

HWTEST_F(GnssAbilityTest, UnloadGnssSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnloadGnssSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnloadGnssSystemAbility001 begin");
    ability_->UnloadGnssSystemAbility();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnloadGnssSystemAbility001 end");
}

HWTEST_F(GnssAbilityTest, UnloadGnssSystemAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnloadGnssSystemAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnloadGnssSystemAbility002 begin");
    ability_->UnloadGnssSystemAbility();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnloadGnssSystemAbility002 end");
}

HWTEST_F(GnssAbilityTest, UnloadGnssSystemAbility003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, UnloadGnssSystemAbility003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnloadGnssSystemAbility003 begin");
    ability_->gnssHandler_ == std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    ability_->UnloadGnssSystemAbility();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] UnloadGnssSystemAbility003 end");
}

HWTEST_F(GnssAbilityTest, SetPositionMode002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetPositionMode002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetPositionMode002 begin");
    ability_->SetPositionMode();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetPositionMode002 end");
}

HWTEST_F(GnssAbilityTest, InjectTime002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, InjectTime002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectTime002 begin");
    ability_->InjectTime();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectTime002 end");
}

HWTEST_F(GnssAbilityTest, InjectLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, InjectLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectLocation001 begin");
    auto location = std::make_unique<Location>();
    ability_->nlpLocation_ = *location;
    ability_->nlpLocation_.SetAccuracy(1e-9);
    ability_->nlpLocation_.SetTimeStamp(0);
    ability_->InjectLocation();
    ability_->nlpLocation_.SetTimeStamp(1);
    ability_->InjectLocation();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectLocation001 end");
}

HWTEST_F(GnssAbilityTest, InjectLocation002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, InjectLocation002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectLocation002 begin");
    auto location = std::make_unique<Location>();
    ability_->nlpLocation_ = *location;
    ability_->nlpLocation_.SetAccuracy(1e-10);
    ability_->nlpLocation_.SetTimeStamp(0);
    ability_->InjectLocation();
    ability_->nlpLocation_.SetTimeStamp(1);
    ability_->InjectLocation();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] InjectLocation002 end");
}

HWTEST_F(GnssAbilityTest, AddGnssGeofence002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, AddGnssGeofence002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddGnssGeofence002 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    LocationErrCode result = ability_->AddGnssGeofence(request);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] AddGnssGeofence002 end");
}

HWTEST_F(GnssAbilityTest, RegisterGnssGeofenceCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RegisterGnssGeofenceCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RegisterGnssGeofenceCallback002 begin");
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    sptr<IRemoteObject> callback = CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    EXPECT_EQ(nullptr, callback);
    ability_->RegisterGnssGeofenceCallback(request, callback);
    callback = nullptr;
    ability_->RegisterGnssGeofenceCallback(request, callback);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RegisterGnssGeofenceCallback002 end");
}

HWTEST_F(GnssAbilityTest, RemoveGnssGeofenceRequestByCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, RemoveGnssGeofenceRequestByCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofenceRequestByCallback002 begin");
    sptr<IRemoteObject> callback = CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    bool result = ability_->RemoveGnssGeofenceRequestByCallback(callback);
    EXPECT_EQ(false, result);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] RemoveGnssGeofenceRequestByCallback002 end");
}

HWTEST_F(GnssAbilityTest, DisableGnss002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, DisableGnss002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] DisableGnss002 begin");
    ability_->DisableGnss();
    ability_->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    ability_->DisableGnss();
    ability_->gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
    ability_->DisableGnss();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] DisableGnss002 end");
}

HWTEST_F(GnssAbilityTest, StopGnss002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, StopGnss002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StopGnss002 begin");
    ability_->StopGnss();
    ability_->gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    ability_->StopGnss();
    ability_->gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
    ability_->StopGnss();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] StopGnss002 end");
}

HWTEST_F(GnssAbilityTest, IsDeviceLoaded001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, IsDeviceLoaded001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] IsDeviceLoaded001 begin");
    std::string servName = "servName";
    ability_->IsDeviceLoaded(servName);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] IsDeviceLoaded001 end");
}

HWTEST_F(GnssAbilityTest, ConnectGnssHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ConnectGnssHdi001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectGnssHdi001 begin");
    ability_->ConnectGnssHdi();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectGnssHdi001 end");
}

HWTEST_F(GnssAbilityTest, ConnectAgnssHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ConnectAgnssHdi001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectAgnssHdi001 begin");
    ability_->ConnectAgnssHdi();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectAgnssHdi001 end");
}

HWTEST_F(GnssAbilityTest, ConnectGeofenceHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ConnectGeofenceHdi001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectGeofenceHdi001 begin");
    ability_->ConnectGeofenceHdi();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectGeofenceHdi001 end");
}

HWTEST_F(GnssAbilityTest, ConnectHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ConnectHdi001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectHdi001 begin");
    ability_->ConnectHdi();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ConnectHdi001 end");
}

HWTEST_F(GnssAbilityTest, SetAgnssServer001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SetAgnssServer001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetAgnssServer001 begin");
    ability_->SetAgnssServer();
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SetAgnssServer001 end");
}

HWTEST_F(GnssAbilityTest, ReportMockedLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, ReportMockedLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReportMockedLocation001 begin");
    std::shared_ptr<Location> location = std::make_shared<Location>();
    ability_->mockEnabled_ = false;
    location->isFromMock_ = true;
    ability_->ReportMockedLocation(location);
    location->isFromMock_ = false;
    ability_->ReportMockedLocation(location);
    ability_->mockEnabled_ = true;
    location->isFromMock_ = true;
    ability_->ReportMockedLocation(location);
    location->isFromMock_ = false;
    ability_->ReportMockedLocation(location);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] ReportMockedLocation001 end");
}

HWTEST_F(GnssAbilityTest, SendEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, SendEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendEvent001 begin");
    ability_->gnssHandler_= std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    MessageParcel reply;
    ability_->SendEvent(event, reply);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] SendEvent001 end");
}

HWTEST_F(GnssAbilityTest, HandleSendLocationRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleSendLocationRequest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSendLocationRequest001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleSendLocationRequest(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSendLocationRequest001 end");
}

HWTEST_F(GnssAbilityTest, HandleSetMockedLocations001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleSetMockedLocations001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSetMockedLocations001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleSendLocationRequest(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSetMockedLocations001 end");
}

HWTEST_F(GnssAbilityTest, HandleSendCommands001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleSendCommands001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSendCommands001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleSendLocationRequest(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSendCommands001 end");
}

HWTEST_F(GnssAbilityTest, HandleReconnectHdi001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleReconnectHdi001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleReconnectHdi001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleReconnectHdi(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleReconnectHdi001 end");
}

HWTEST_F(GnssAbilityTest, HandleSetEnable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleSetEnable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSetEnable001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleSetEnable(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSetEnable001 end");
}

HWTEST_F(GnssAbilityTest, HandleAddFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleAddFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleAddFence001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleAddFence(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleAddFence001 end");
}

HWTEST_F(GnssAbilityTest, HandleRemoveFence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleRemoveFence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleRemoveFence001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleRemoveFence(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleRemoveFence001 end");
}

HWTEST_F(GnssAbilityTest, HandleAddGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleAddGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleAddGeofence001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleAddGeofence(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleAddGeofence001 end");
}

HWTEST_F(GnssAbilityTest, HandleRemoveGeofence001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleRemoveGeofence001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleRemoveGeofence001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleRemoveGeofence(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleRemoveGeofence001 end");
}

HWTEST_F(GnssAbilityTest, HandleSendNetworkLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, HandleSendNetworkLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSendNetworkLocation001 begin");
    int state = DISABLED;
    AppExecFwk::InnerEvent::Pointer event  =
        AppExecFwk::InnerEvent::Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    auto gnssHandler = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    gnssHandler->HandleSendNetworkLocation(event);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] HandleSendNetworkLocation001 end");
}

HWTEST_F(GnssAbilityTest, OnRemoteDied001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityTest, OnRemoteDied001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[GnssAbilityTest] OnRemoteDied001 begin");
    auto deathRecipient = new (std::nothrow) LocationHdiDeathRecipient();
    const wptr<IRemoteObject> object;
    deathRecipient->OnRemoteDied(object);
    LBSLOGI(LOCATOR, "[GnssAbilityTest] OnRemoteDied001 end");
}
}  // namespace Location
}  // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
