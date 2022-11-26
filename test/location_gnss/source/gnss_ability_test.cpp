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

#include "gnss_ability_test.h"

#include <cstdlib>

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "agnss_event_callback.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "gnss_event_callback.h"
#include "location_dumper.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
using HDI::Location::Agnss::V1_0::IAGnssCallback;
using HDI::Location::Agnss::V1_0::AGnssRefInfo;
using HDI::Location::Gnss::V1_0::IGnssCallback;
using HDI::Location::Gnss::V1_0::LocationInfo;
const int32_t LOCATION_PERM_NUM = 4;
const std::string ARGS_HELP = "-h";
void GnssAbilityTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    ability_ = new (std::nothrow) GnssAbility();
    EXPECT_NE(nullptr, ability_);
    callbackStub_ = new (std::nothrow) GnssStatusCallbackHost();
    EXPECT_NE(nullptr, callbackStub_);
    proxy_ = new (std::nothrow) GnssAbilityProxy(ability_);
    EXPECT_NE(nullptr, proxy_);
}

void GnssAbilityTest::TearDown()
{
    /*
     * @tc.teardown: release memory.
     */
    proxy_ = nullptr;
    callbackStub_ = nullptr;
}

void GnssAbilityTest::MockNativePermission()
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
    /*
     * @tc.steps: step1. build location request data.
     */
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    int num = 2;
    for (int i = 0; i < num; i++) {
        int uid = i + 1;
        int pid = i + 2;
        int timeInterval = i;
        std::string name = "nameForTest";
        std::string uuid = std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM));
        workRecord->Add(uid, pid, name, timeInterval, uuid);
    }
    /*
     * @tc.steps: step2. send location request
     * @tc.expected: step2. no exception happens.
     */
    proxy_->SendLocationRequest(*workRecord);
}

/*
 * @tc.name: SetEnableAndDisable001
 * @tc.desc: Test disable and enable system ability
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SetEnableAndDisable001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.remove SA
     * @tc.expected: step1. object1 is null.
     */
    MessageParcel data1;
    data1.WriteBool(false); // if the state is false
    proxy_->SetEnable(data1.ReadBool());
    sptr<ISystemAbilityManager> systemAbilityManager1 =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object1 = systemAbilityManager1->GetSystemAbility(LOCATION_GNSS_SA_ID);
    EXPECT_EQ(nullptr, object1); // no SA can be given

    /*
     * @tc.steps: step2. test enable SA
     * @tc.expected: step2. object2 is not null.
     */
    MessageParcel data2;
    data2.WriteBool(true); // if the state is true
    proxy_->SetEnable(data2.ReadBool());
    sptr<ISystemAbilityManager> systemAbilityManager2 =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object2 = systemAbilityManager2->GetSystemAbility(LOCATION_GNSS_SA_ID);
    EXPECT_NE(nullptr, object2); // SA can be given
}

/*
 * @tc.name: RefrashRequirements001
 * @tc.desc: Test refrash requirements
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RefrashRequirements001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test refrash requirements
     * @tc.expected: no exception happens.
     */
    proxy_->RefrashRequirements();
}

/*
 * @tc.name: RegisterGnssStatusCallback001
 * @tc.desc: Test register gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterGnssStatusCallback001, TestSize.Level1)
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
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterGnssStatusCallback001, TestSize.Level1)
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
     * @tc.expected: no exception happens
     */
    proxy_->UnregisterGnssStatusCallback(callbackStub_->AsObject());
}

/*
 * @tc.name: UnregisterGnssStatusCallback001
 * @tc.desc: Test unregister gnss status callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterGnssStatusCallback001, TestSize.Level1)
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
 * @tc.name: RegisterNmeaMessageCallback001
 * @tc.desc: Test register nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterNmeaMessageCallback001, TestSize.Level1)
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
 * @tc.desc: Test register nmea message callback if client is not null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterNmeaMessageCallback001, TestSize.Level1)
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
     * @tc.expected: no exception happens.
     */
    proxy_->UnregisterNmeaMessageCallback(callbackStub_->AsObject());
}

/*
 * @tc.name: UnregisterNmeaMessageCallback001
 * @tc.desc: Test unregister nmea message callback if client is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterNmeaMessageCallback001, TestSize.Level1)
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
 * @tc.name: RegisterCachedCallback001
 * @tc.desc: Test register cache call back
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterCachedCallback001, TestSize.Level1)
{
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
    proxy_->RegisterCachedCallback(requestConfig, callback);
}

/*
 * @tc.name: RegisterCachedCallback002
 * @tc.desc: Test register cache call back and the queue is not full.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterCachedCallback003, TestSize.Level1)
{
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
    proxy_->RegisterCachedCallback(requestConfig, callbackStub_->AsObject());
}

/*
 * @tc.name: RegisterAndUnregisterCachedCallback001
 * @tc.desc: Test register and unregister cache call back and the queue is full.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, RegisterAndUnregisterCachedCallback002, TestSize.Level1)
{
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
    proxy_->RegisterCachedCallback(requestConfig, callbackStub_->AsObject());

    /*
     * @tc.steps: step3. test unregister cached call back if call back is not null.
     * @tc.expected: no exception happens.
     */
    proxy_->UnregisterCachedCallback(callbackStub_->AsObject());
}

/*
 * @tc.name: UnregisterCachedCallback001
 * @tc.desc: Test unregister cache call back and call back is null
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, UnregisterCachedCallback001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. the call back is null.
     */
    sptr<IRemoteObject> callback = nullptr;

    /*
     * @tc.steps: step2. test unregister cached call back if call back is null.
     * @tc.expected: log info : "register an invalid cached location callback"
     */
    proxy_->UnregisterCachedCallback(callback);
}

/*
 * @tc.name: GetCachedGnssLocationsSize001
 * @tc.desc: Test get cached gnss locations size
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, GetCachedGnssLocationsSize001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test get cached gnss locations size.
     * @tc.expected: size equals -1.
     */
    int size = proxy_->GetCachedGnssLocationsSize();
    EXPECT_EQ(-1, size);
}

/*
 * @tc.name: FlushCachedGnssLocations001
 * @tc.desc: Test unregister country code call back
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, FlushCachedGnssLocations001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test flush cached gnss locations.
     * @tc.expected: reply code unsupport
     */
    int ret = proxy_->FlushCachedGnssLocations();
    EXPECT_EQ(REPLY_CODE_UNSUPPORT, ret);
}

/*
 * @tc.name: SendCommand001
 * @tc.desc: Test send
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, SendCommand001, TestSize.Level1)
{
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
    proxy_->SendCommand(locationCommand);
}

/*
 * @tc.name: AddFence001
 * @tc.desc: Test add fence
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, AddFence001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build geo fence request
     */
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = 2;
    request->geofence.latitude = 35.1;
    request->geofence.longitude = 40.2;
    request->geofence.radius = 2.2;
    request->geofence.expiration = 12.2;

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
HWTEST_F(GnssAbilityTest, RemoveFence001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build geo fence request
     */
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = 2;
    request->geofence.latitude = 35.1;
    request->geofence.longitude = 40.2;
    request->geofence.radius = 2.2;
    request->geofence.expiration = 12.2;

    /*
     * @tc.steps: step2. test remove fence
     * @tc.expected: no exception happens
     */
    proxy_->RemoveFence(request);
}

HWTEST_F(GnssAbilityTest, GnssLocationMock001, TestSize.Level1)
{
    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    EXPECT_EQ(true, proxy_->EnableMock());
    EXPECT_EQ(true, ability_->IsMockEnabled());
    EXPECT_EQ(true, proxy_->SetMocked(timeInterval, locations));
    
    EXPECT_EQ(true, proxy_->DisableMock());
    EXPECT_EQ(false, ability_->IsMockEnabled());
    EXPECT_EQ(false, proxy_->SetMocked(timeInterval, locations));
}

HWTEST_F(GnssAbilityTest, GnssOnStartAndOnStop001, TestSize.Level1)
{
    ability_->OnStart(); // start ability
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, ability_->QueryServiceState());
    ability_->OnStart(); // start ability again
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, ability_->QueryServiceState());

    ability_->OnStop(); // stop ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, ability_->QueryServiceState());
    ability_->OnStart(); // restart ability
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, ability_->QueryServiceState());
}

HWTEST_F(GnssAbilityTest, GnssDump001, TestSize.Level1)
{
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
    ability_->Dump(fd, args);

    std::vector<std::u16string> emptyArgs;
    ability_->Dump(fd, emptyArgs);

    std::vector<std::u16string> helpArgs;
    std::u16string helpArg1 = Str8ToStr16(ARGS_HELP);
    helpArgs.emplace_back(helpArg1);
    ability_->Dump(fd, emptyArgs);
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent001, TestSize.Level1)
{
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 2;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
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
    parcel.WriteBool(true); // isFromMock is true
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(true, proxy_->EnableMock());
    EXPECT_EQ(true, proxy_->SetMocked(timeInterval, locations));

    EXPECT_EQ(true, proxy_->DisableMock()); // disable mock
    ability_->SendReportMockLocationEvent(); // report mocked location
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent002, TestSize.Level1)
{
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 2;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
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
    parcel.WriteBool(true); // isFromMock is true
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(true, proxy_->EnableMock());
    EXPECT_EQ(true, proxy_->SetMocked(timeInterval, locations));

    EXPECT_EQ(true, proxy_->DisableMock()); // disable mock
    ability_->SendReportMockLocationEvent(); // do not report mocked location
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent003, TestSize.Level1)
{
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 2;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
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
    parcel.WriteBool(false); // isFromMock is false
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(true, proxy_->EnableMock());
    EXPECT_EQ(true, proxy_->SetMocked(timeInterval, locations));

    EXPECT_EQ(true, proxy_->EnableMock()); // enable mock
    ability_->SendReportMockLocationEvent(); // do not report mocked location
}

HWTEST_F(GnssAbilityTest, GnssSendReportMockLocationEvent004, TestSize.Level1)
{
    ability_->SendReportMockLocationEvent(); // clear location mock

    int timeInterval = 2;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
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
    parcel.WriteBool(false); // isFromMock is false
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(true, proxy_->EnableMock());
    EXPECT_EQ(true, proxy_->SetMocked(timeInterval, locations));

    EXPECT_EQ(true, proxy_->DisableMock()); // disable mock
    ability_->SendReportMockLocationEvent(); // do not report mocked location
}

HWTEST_F(GnssAbilityTest, AddFenceAndRemoveFenceTest001, TestSize.Level1)
{
    std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
    ability_->AddFence(fence);
    ability_->RemoveFence(fence);
}

HWTEST_F(GnssAbilityTest, GnssAbilityReportSv001, TestSize.Level1)
{
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
    }
    status->ReadFromParcel(parcel);
    ability_->ReportSv(status);
}
}  // namespace Location
}  // namespace OHOS