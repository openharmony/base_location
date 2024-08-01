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

#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability_stub_test.h"

#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location_log.h"
#include "network_ability_proxy.h"
#include "network_callback_host.h"
#include "subability_common.h"
#include "locationhub_ipc_interface_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int UNKNOWN_CODE = -1;
const int ERR_CODE = 100;
const int DEFAULT_STATUS = 0;
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
    EXPECT_CALL(*networkAbilityStub, SendMessage(_, _, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(ERRCODE_SUCCESS,
        networkAbilityStub->OnRemoteRequest(static_cast<uint32_t>(NetworkInterfaceCode::SEND_LOCATION_REQUEST),
        parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest001 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest002, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest002 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, SetEnable(_)).WillOnce(DoAll(Return(ERRCODE_SUCCESS)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(ERRCODE_SUCCESS,
        networkAbilityStub->OnRemoteRequest(static_cast<uint32_t>(NetworkInterfaceCode::SET_ENABLE),
        parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest002 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest004, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest004 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, EnableMock()).WillOnce(DoAll(Return(ERRCODE_SUCCESS)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(ERRCODE_SUCCESS,
        networkAbilityStub->OnRemoteRequest(static_cast<uint32_t>(NetworkInterfaceCode::ENABLE_LOCATION_MOCK),
        parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest004 end");
}

HWTEST_F(NetworkAbilityStubTest, NetworkAbilityStubTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkAbilityStubTest005, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest005 begin");
    auto networkAbilityStub = sptr<MockNetworkAbilityStub>(new (std::nothrow) MockNetworkAbilityStub());
    EXPECT_CALL(*networkAbilityStub, DisableMock()).WillOnce(DoAll(Return(ERRCODE_SUCCESS)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(ERRCODE_SUCCESS,
        networkAbilityStub->OnRemoteRequest(static_cast<uint32_t>(NetworkInterfaceCode::DISABLE_LOCATION_MOCK),
        parcel, reply, option));
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
    EXPECT_EQ(ERRCODE_SUCCESS,
        networkAbilityStub->OnRemoteRequest(static_cast<uint32_t>(NetworkInterfaceCode::SET_MOCKED_LOCATIONS),
        parcel, reply, option));
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
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE,
        networkAbilityStub->OnRemoteRequest(UNKNOWN_CODE, parcel, reply, option));
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkAbilityStubTest008 end");
}

/*
 * @tc.name: NetworkCallback001
 * @tc.desc: network callback
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityStubTest, NetworkCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkCallback001, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkCallback001 begin");
    auto networkCallbackHost =
        sptr<NetworkCallbackHost>(new (std::nothrow) NetworkCallbackHost());
    EXPECT_NE(nullptr, networkCallbackHost);
    std::unique_ptr<Location> location = std::make_unique<Location>();
    networkCallbackHost->OnLocationReport(location);
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkCallback001 end");
}

/*
 * @tc.name: NetworkCallback002
 * @tc.desc: network callback
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityStubTest, NetworkCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkCallback002, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkCallback002 begin");
    auto networkCallbackHost =
        sptr<NetworkCallbackHost>(new (std::nothrow) NetworkCallbackHost());
    EXPECT_NE(nullptr, networkCallbackHost);
    networkCallbackHost->OnLocatingStatusChange(DEFAULT_STATUS); // nullptr error
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkCallback002 end");
}

/*
 * @tc.name: NetworkCallback003
 * @tc.desc: network callback
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityStubTest, NetworkCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "NetworkAbilityStubTest, NetworkCallback003, TestSize.Level1";
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkCallback003 begin");
    auto networkCallbackHost =
        sptr<NetworkCallbackHost>(new (std::nothrow) NetworkCallbackHost());
    EXPECT_NE(nullptr, networkCallbackHost);
    networkCallbackHost->OnErrorReport(ERR_CODE); // nullptr error
    LBSLOGI(NETWORK, "[NetworkAbilityStubTest] NetworkCallback003 end");
}
}  // namespace Location
}  // namespace OHOS
#endif // FEATURE_NETWORK_SUPPORT
