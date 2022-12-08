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

#include "network_ability_stub_test.h"

#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"

#include "common_utils.h"
#include "network_ability_proxy.h"
#include "location_log.h"
#include "subability_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int UNKNOWN_CODE = -1;
void NetworkAbilityStubTest::SetUp()
{
}

void NetworkAbilityStubTest::TearDown()
{
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest001 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, SendLocationRequest(_, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        networkAbilityStub->OnRemoteRequest(ISubAbility::SEND_LOCATION_REQUEST, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest001 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest002, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest002 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, SetEnable(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        networkAbilityStub->OnRemoteRequest(ISubAbility::SET_ENABLE, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest002 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest003, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest003 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, SelfRequest(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        networkAbilityStub->OnRemoteRequest(ISubAbility::SELF_REQUEST, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest003 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest004, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest004 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, EnableMock(_)).WillOnce(DoAll(Return(true)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        networkAbilityStub->OnRemoteRequest(ISubAbility::ENABLE_LOCATION_MOCK, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest004 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest005, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest005 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, DisableMock(_)).WillOnce(DoAll(Return(true)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        networkAbilityStub->OnRemoteRequest(ISubAbility::DISABLE_LOCATION_MOCK, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest005 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest006, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest006 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, SendMessage(_, _, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        networkAbilityStub->OnRemoteRequest(ISubAbility::SET_MOCKED_LOCATIONS, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest006 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest007, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest007 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(IPC_STUB_UNKNOW_TRANS_ERR,
        networkAbilityStub->OnRemoteRequest(UNKNOWN_CODE, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest007 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest008, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest008 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    MessageParcel parcel;
    parcel.WriteInterfaceToken(u"UNKNOWN_DESCRIPTOR");
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_EXCEPTION,
        networkAbilityStub->OnRemoteRequest(UNKNOWN_CODE, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest008 end");
}
}  // namespace Location
}  // namespace OHOS