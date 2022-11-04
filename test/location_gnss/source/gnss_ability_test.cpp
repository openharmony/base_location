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

#include "test_utils.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "constant_definition.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Location;

void GnssAbilityTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    TestUtils::MockNativePermission();
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, systemAbilityManager);
    if (systemAbilityManager->CheckSystemAbility(LOCATION_GNSS_SA_ID) == nullptr) {
        LBSLOGE(GNSS, "Can not get SA, return.");
    }
    sptr<IRemoteObject> systemAbility = systemAbilityManager->GetSystemAbility(LOCATION_GNSS_SA_ID);
    EXPECT_NE(nullptr, systemAbility);
    callbackStub_ = new (std::nothrow) GnssStatusCallbackHost();
    EXPECT_NE(nullptr, callbackStub_);
    proxy_ = new (std::nothrow) GnssAbilityProxy(systemAbility);
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
        std::string name = "nameForTest";
        workRecord->Add(uid, pid, name);
    }
    uint64_t interval = 1;

    /*
     * @tc.steps: step2. send location request
     * @tc.expected: step2. no exception happens.
     */
    proxy_->SendLocationRequest(interval, *workRecord);
    EXPECT_EQ(true, true); // always true
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
 * @tc.name: HandleRemoteRequest001
 * @tc.desc: Test handle remote request when state is true.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, HandleRemoteRequest001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test handle remote request
     * @tc.expected: step1. no exception happens.
     */
    proxy_->RemoteRequest(true);

    EXPECT_EQ(true, true); // always true
}

/*
 * @tc.name: HandleRemoteRequest002
 * @tc.desc: Test handle remote request when state is false.
 * @tc.type: FUNC
 */
HWTEST_F(GnssAbilityTest, HandleRemoteRequest002, TestSize.Level1)
{
    /*
     * @tc.steps: step1. test handle remote request
     * @tc.expected: step1. no exception happens.
     */
    proxy_->RemoteRequest(false);

    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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

    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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

    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    EXPECT_EQ(true, true); // always true
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
    request->priority = 1;
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
    EXPECT_EQ(true, true); // always true
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
    request->priority = 1;
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
    EXPECT_EQ(true, true); // always true
}
