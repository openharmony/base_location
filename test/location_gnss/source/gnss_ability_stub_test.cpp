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

#include "gnss_ability_stub_test.h"

#include "ipc_types.h"
#include "message_option.h"
#include "message_parcel.h"

#include "common_utils.h"
#include "gnss_ability_proxy.h"
#include "location_log.h"
#include "subability_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int UNKNOWN_CODE = -1;
void GnssAbilityStubTest::SetUp()
{
}

void GnssAbilityStubTest::TearDown()
{
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest001, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest001 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, SendMessage(_, _, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::SEND_LOCATION_REQUEST, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest001 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest002, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest002 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, SetEnable(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::SET_ENABLE, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest002 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest003, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest003 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, RefrashRequirements()).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::REFRESH_REQUESTS, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest003 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest004, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest004 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, RegisterGnssStatusCallback(_, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::REG_GNSS_STATUS, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest004 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest005, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest005 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, UnregisterGnssStatusCallback(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::UNREG_GNSS_STATUS, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest005 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest006, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest006 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, RegisterNmeaMessageCallback(_, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::REG_NMEA, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest006 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest007, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest007 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, RegisterCachedCallback(_, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::REG_CACHED, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest007 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest008, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest008 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, UnregisterCachedCallback(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::UNREG_CACHED, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest008 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest009, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest009 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, SendCommand(_)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::SEND_COMMANDS, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest009 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest010, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest010 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, EnableMock()).WillOnce(DoAll(Return(true)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::ENABLE_LOCATION_MOCK, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest010 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest011, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest011 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, DisableMock()).WillOnce(DoAll(Return(true)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::DISABLE_LOCATION_MOCK, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest011 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest012, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest012 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, SendMessage(_, _, _)).WillOnce(DoAll(Return()));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::SET_MOCKED_LOCATIONS, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest012 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest013, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest013 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, FlushCachedGnssLocations()).WillOnce(DoAll(Return(REPLY_CODE_UNSUPPORT)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_UNSUPPORT,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::FLUSH_CACHED, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest013 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest014, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest014 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    EXPECT_CALL(*gnssAbilityStub, GetCachedGnssLocationsSize()).WillOnce(DoAll(Return(-1)));
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(ISubAbility::GET_CACHED_SIZE, parcel, reply, option));
    EXPECT_EQ(-1, reply.ReadInt32());
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest014 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest015, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest015 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    MessageParcel parcel;
    parcel.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(IPC_STUB_UNKNOW_TRANS_ERR,
        gnssAbilityStub->OnRemoteRequest(UNKNOWN_CODE, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest015 end");
}

HWTEST_F(GnssAbilityStubTest, GnssAbilityStubTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GnssAbilityStubTest, GnssAbilityStubTest016, TestSize.Level1";
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest016 begin");
    auto gnssAbilityStub = sptr<MockGnssAbilityStub>(new (std::nothrow) MockGnssAbilityStub());
    MessageParcel parcel;
    parcel.WriteInterfaceToken(u"UNKNOWN_DESCRIPTOR");
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(REPLY_CODE_EXCEPTION,
        gnssAbilityStub->OnRemoteRequest(UNKNOWN_CODE, parcel, reply, option));
    LBSLOGI(GNSS, "[GnssAbilityStubTest] GnssAbilityStubTest016 end");
}
}  // namespace Location
}  // namespace OHOS