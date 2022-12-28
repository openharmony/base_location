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

#include "passive_ability_stub_test.h"

#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"

#include "common_utils.h"
#include "passive_ability_proxy.h"
#include "location_log.h"
#include "subability_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int UNKNOWN_CODE = -1;
void PassiveAbilityStubTest::SetUp()
{
}

void PassiveAbilityStubTest::TearDown()
{
}

HWTEST_F(PassiveAbilityStubTest, PassiveAbilityStubTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveAbilityStubTest001, TestSize.Level1";
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest001 begin");
    auto passiveAbilityStub = sptr<MockPassiveAbilityStub>(new (std::nothrow) MockPassiveAbilityStub());
    EXPECT_CALL(*passiveAbilityStub, SendLocationRequest(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        passiveAbilityStub->OnRemoteRequest(ISubAbility::SEND_LOCATION_REQUEST, parcel, reply, option));
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest001 end");
}

HWTEST_F(PassiveAbilityStubTest, PassiveAbilityStubTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveAbilityStubTest002, TestSize.Level1";
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest002 begin");
    auto passiveAbilityStub = sptr<MockPassiveAbilityStub>(new (std::nothrow) MockPassiveAbilityStub());
    EXPECT_CALL(*passiveAbilityStub, SetEnable(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        passiveAbilityStub->OnRemoteRequest(ISubAbility::SET_ENABLE, parcel, reply, option));
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest002 end");
}

HWTEST_F(PassiveAbilityStubTest, PassiveAbilityStubTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveAbilityStubTest003, TestSize.Level1";
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest003 begin");
    auto passiveAbilityStub = sptr<MockPassiveAbilityStub>(new (std::nothrow) MockPassiveAbilityStub());
    EXPECT_CALL(*passiveAbilityStub, EnableMock(_)).WillOnce(DoAll(Return(true)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        passiveAbilityStub->OnRemoteRequest(ISubAbility::ENABLE_LOCATION_MOCK, parcel, reply, option));
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest003 end");
}

HWTEST_F(PassiveAbilityStubTest, PassiveAbilityStubTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveAbilityStubTest004, TestSize.Level1";
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest004 begin");
    auto passiveAbilityStub = sptr<MockPassiveAbilityStub>(new (std::nothrow) MockPassiveAbilityStub());
    EXPECT_CALL(*passiveAbilityStub, DisableMock(_)).WillOnce(DoAll(Return(true)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        passiveAbilityStub->OnRemoteRequest(ISubAbility::DISABLE_LOCATION_MOCK, parcel, reply, option));
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest004 end");
}

HWTEST_F(PassiveAbilityStubTest, PassiveAbilityStubTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveAbilityStubTest005, TestSize.Level1";
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest005 begin");
    auto passiveAbilityStub = sptr<MockPassiveAbilityStub>(new (std::nothrow) MockPassiveAbilityStub());
    EXPECT_CALL(*passiveAbilityStub, SendMessage(_, _, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        passiveAbilityStub->OnRemoteRequest(ISubAbility::SET_MOCKED_LOCATIONS, parcel, reply, option));
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest005 end");
}

HWTEST_F(PassiveAbilityStubTest, PassiveAbilityStubTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveAbilityStubTest006, TestSize.Level1";
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest006 begin");
    auto passiveAbilityStub = sptr<MockPassiveAbilityStub>(new (std::nothrow) MockPassiveAbilityStub());
    MessageParcel parcel;
    parcel.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(IPC_STUB_UNKNOW_TRANS_ERR,
        passiveAbilityStub->OnRemoteRequest(UNKNOWN_CODE, parcel, reply, option));
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest006 end");
}

HWTEST_F(PassiveAbilityStubTest, PassiveAbilityStubTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PassiveAbilityStubTest, PassiveAbilityStubTest007, TestSize.Level1";
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest007 begin");
    auto passiveAbilityStub = sptr<MockPassiveAbilityStub>(new (std::nothrow) MockPassiveAbilityStub());
    MessageParcel parcel;
    parcel.WriteInterfaceToken(u"UNKNOWN_DESCRIPTOR");
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_EXCEPTION,
        passiveAbilityStub->OnRemoteRequest(UNKNOWN_CODE, parcel, reply, option));
    LBSLOGI(PASSIVE, "[PassiveAbilityStubTest] PassiveAbilityStubTest007 end");
}
}  // namespace Location
}  // namespace OHOS