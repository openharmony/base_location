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

#ifdef FEATURE_PASSIVE_SUPPORT
#define private public
#include "passive_ability_test.h"
#include "passive_ability.h"
#undef private

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location_dumper.h"
#include "permission_manager.h"
#include "locationhub_ipc_interface_code.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 5;
const std::string ARGS_HELP = "-h";
const std::string UNLOAD_PASSIVE_TASK = "passive_sa_unload";
const int32_t WAIT_EVENT_TIME = 1;

sptr<PassiveAbility> PassiveAbilityTest::ability_;
sptr<PassiveAbilityProxy> PassiveAbilityTest::proxy_;

void PassiveAbilityTest::SetUp()
{
}

void PassiveAbilityTest::TearDown()
{
}

void PassiveAbilityTest::SetUpTestCase()
{
    MockNativePermission();
    ability_ = new (std::nothrow) PassiveAbility();
    proxy_ = new (std::nothrow) PassiveAbilityProxy(ability_);
    EXPECT_NE(nullptr, proxy_);
}

void PassiveAbilityTest::TearDownTestCase()
{
    ability_->passiveHandler_->RemoveAllEvents();
    sleep(WAIT_EVENT_TIME);
    proxy_ = nullptr;
    ability_ = nullptr;
}

void PassiveAbilityTest::MockNativePermission()
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
        .processName = "PassiveAbilityTest",
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
HWTEST_F(PassiveAbilityTest, SendLocationRequest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, SendLocationRequest001, TestSize.Level0";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] SendLocationRequest001 begin");
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
        request->SetNlpRequestType(0);
        workRecord->Add(request);
    }
    /*
     * @tc.steps: step2. send location request
     * @tc.expected: step2. no exception happens.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SendLocationRequest(*workRecord));
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] SendLocationRequest001 begin");
}

/*
 * @tc.name: SetEnableAndDisable001
 * @tc.desc: Test disable and enable system ability
 * @tc.type: FUNC
 */
HWTEST_F(PassiveAbilityTest, SetEnableAndDisable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, SetEnableAndDisable001, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] SetEnableAndDisable001 begin");
    /*
     * @tc.steps: step1.remove SA
     * @tc.expected: step1. object1 is null.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(false)); // after mock, sa obj is nullptr

    /*
     * @tc.steps: step2. test enable SA
     * @tc.expected: step2. object2 is not null.
     */
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetEnable(true)); // after mock, sa obj is nullptr
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] SetEnableAndDisable001 end");
}

HWTEST_F(PassiveAbilityTest, PassiveLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveLocationMock001, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveLocationMock001 begin");
    int timeInterval = 1;
    std::vector<std::shared_ptr<Location>> locations;
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
        parcel.WriteInt32(1); // isFromMock
        locations.push_back(Location::UnmarshallingShared(parcel));
    }
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->EnableMock());
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetMocked(timeInterval, locations));
    ability_->passiveHandler_->RemoveEvent(static_cast<uint32_t>(PassiveInterfaceCode::SET_MOCKED_LOCATIONS));
    sleep(WAIT_EVENT_TIME);

    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->DisableMock());
    EXPECT_EQ(LOCATION_ERRCODE_NOT_SUPPORTED, proxy_->SetMocked(timeInterval, locations));
    ability_->passiveHandler_->RemoveEvent(static_cast<uint32_t>(PassiveInterfaceCode::SET_MOCKED_LOCATIONS));
    sleep(WAIT_EVENT_TIME);
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveLocationMock001 end");
}

HWTEST_F(PassiveAbilityTest, PassiveOnStartAndOnStop001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveOnStartAndOnStop001, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveOnStartAndOnStop001 begin");
    ability_->OnStart(); // start ability
    ability_->OnStart(); // start ability again
    ability_->state_ = ServiceRunningState::STATE_RUNNING;
    ability_->OnStart(); // start ability again
    ability_->OnStop(); // stop ability
    ability_->OnStart(); // restart ability
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveOnStartAndOnStop001 end");
}

HWTEST_F(PassiveAbilityTest, PassiveDump001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveDump001, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveDump001 begin");
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
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveDump001 end");
}

HWTEST_F(PassiveAbilityTest, PassiveSendReportMockLocationEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityTest, PassiveSendReportMockLocationEvent001, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportMockLocationEvent001 begin");
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
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportMockLocationEvent001 end");
}

HWTEST_F(PassiveAbilityTest, PassiveSendReportMockLocationEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityTest, PassiveSendReportMockLocationEvent002, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportMockLocationEvent002 begin");
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
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportMockLocationEvent002 end");
}

HWTEST_F(PassiveAbilityTest, PassiveSendReportUnloadPassiveSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityTest, PassiveSendReportUnloadPassiveSystemAbility001, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportUnloadPassiveSystemAbility001 begin");
    ability_->UnloadPassiveSystemAbility();
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportUnloadPassiveSystemAbility001 end");
}

HWTEST_F(PassiveAbilityTest, PassiveSendReportSendMessage001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityTest, PassiveSendReportUnloadSendMessage001, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportUnloadSendMessage001 begin");
    MessageParcel parcel;
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
    MessageParcel reply;
    ability_->SendMessage(LOCATION_ERRCODE_NOT_SUPPORTED, parcel, reply);
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityTest] PassiveSendReportSendMessage001 end");
}

HWTEST_F(PassiveAbilityTest, PassiveOnStartAndOnStop002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveOnStartAndOnStop002, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveOnStartAndOnStop002 begin");
    ability_->state_ = ServiceRunningState::STATE_RUNNING;
    ability_->OnStart(); // start ability again
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveOnStartAndOnStop002 end");
}

HWTEST_F(PassiveAbilityTest, PassiveInit002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveInit002, TestSize.Level1";
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveInit002 begin");
    ability_->registerToAbility_ = true;
    ability_->Init(); // start ability again
    LBSLOGI(PASSIVE_TEST, "[PassiveAbilityStubTest] PassiveInit002 end");
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_PASSIVE_SUPPORT
