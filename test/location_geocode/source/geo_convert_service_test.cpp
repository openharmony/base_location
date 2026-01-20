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

#ifdef FEATURE_GEOCODE_SUPPORT
#define private public
#include "geo_convert_service_test.h"
#include "geo_convert_service.h"
#undef private

#include "parameters.h"
#include <string>
#include "string_ex.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include <file_ex.h>
#include <thread>
#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "geo_address.h"
#include "location_config_manager.h"
#include "location_sa_load_manager.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "geocoding_mock_info.h"
#include "geo_convert_service.h"
#include "geo_convert_skeleton.h"
#include "location_dumper.h"
#include "location_log.h"
#include "permission_manager.h"
#include <gtest/gtest.h>
#include "mock_geo_convert_service.h"
#include "mock_i_remote_object.h"

#include "locationhub_ipc_interface_code.h"
#include "location_sa_load_manager.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "location_log.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 5;
const int32_t LOOP_COUNT = 11;
const std::string ARGS_HELP = "-h";
void GeoConvertServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    service_ = new (std::nothrow) GeoConvertService();
    proxy_ = std::make_shared<GeoConvertProxy>(service_);
}

void GeoConvertServiceTest::TearDown()
{
}

void GeoConvertServiceTest::MockNativePermission()
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
        .processName = "GeoCodeServiceTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

bool GeoConvertServiceTest::Available()
{
    MessageParcel replyParcel;
    proxy_->IsGeoConvertAvailable(replyParcel);
    return replyParcel.ReadInt32() == ERRCODE_SUCCESS;
}

HWTEST_F(GeoConvertServiceTest, GeoConvertAvailable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertAvailable001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertAvailable001 begin");
    EXPECT_EQ(true, proxy_->EnableReverseGeocodingMock());

    /*
     * @tc.steps: step1. Call system ability and check whether available.
     * @tc.expected: step1. system ability is available.
     */
    Available();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertAvailable001 end");
}

/*
 * @tc.name: GeoAddressByCoordinate001
 * @tc.desc: Test get address from system ability by coordinate.
 * @tc.type: FUNC
 */
HWTEST_F(GeoConvertServiceTest, GetAddressByCoordinate001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GetAddressByCoordinate001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByCoordinate001 begin");
    /*
     * @tc.steps: step1.read test data.
     */
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel.WriteDouble(39.92879); // latitude
    dataParcel.WriteDouble(116.3709); // longitude
    dataParcel.WriteInt32(5); // maxItem
    dataParcel.WriteInt32(1); // geocoder param object tag
    dataParcel.WriteString16(Str8ToStr16("ZH")); // language
    dataParcel.WriteString16(Str8ToStr16("cn")); // country
    dataParcel.WriteString16(Str8ToStr16("")); // description
    dataParcel.WriteString16(Str8ToStr16("test")); // package name

    /*
     * @tc.steps: step2. test get address by coordinate.
     * @tc.expected: step2. no exception head info.
     */
    proxy_->GetAddressByCoordinate(dataParcel, replyParcel);
    EXPECT_EQ(ERRCODE_REVERSE_GEOCODING_FAIL, replyParcel.ReadInt32());
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByCoordinate001 end");
}

HWTEST_F(GeoConvertServiceTest, GetAddressByCoordinate002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GetAddressByCoordinate002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByCoordinate002 begin");
    /*
     * @tc.steps: step1.read test data.
     */
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel.WriteDouble(39.92879); // latitude
    dataParcel.WriteDouble(116.3709); // longitude
    dataParcel.WriteInt32(5); // maxItem
    dataParcel.WriteInt32(1); // geocoder param object tag
    dataParcel.WriteString16(Str8ToStr16("ZH")); // language
    dataParcel.WriteString16(Str8ToStr16("cn")); // country
    dataParcel.WriteString16(Str8ToStr16("")); // description
    dataParcel.WriteString16(Str8ToStr16("test")); // package name

    /*
     * @tc.steps: step2. test get address by coordinate.
     * @tc.expected: step2. no exception head info.
     */
    service_->mockEnabled_ = false;
    sptr<IRemoteObject> serviceProxy =
        CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID, CommonUtils::InitDeviceId());
    service_->serviceProxy_ = serviceProxy;
    service_->GetAddressByCoordinate(dataParcel, replyParcel);
    EXPECT_EQ(ERRCODE_REVERSE_GEOCODING_FAIL, replyParcel.ReadInt32());
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByCoordinate002 end");
}

/*
 * @tc.name: GetAddressByLocationName001
 * @tc.desc: Test get address from system ability by location name.
 * @tc.type: FUNC
 */
HWTEST_F(GeoConvertServiceTest, GetAddressByLocationName001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GetAddressByLocationName001, TestSize.Level0";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByLocationName001 begin");

    /*
     * @tc.steps: step1.read test data.
     */
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel.WriteString16(Str8ToStr16("北京")); // input description of a location
    dataParcel.WriteDouble(0.0); // minLatitude
    dataParcel.WriteDouble(0.0); // minLongitude
    dataParcel.WriteDouble(0.0); // maxLatitude
    dataParcel.WriteDouble(0.0); // maxLongitude
    dataParcel.WriteInt32(5); // maxItem
    dataParcel.WriteInt32(1); // description
    dataParcel.WriteString16(Str8ToStr16("ZH")); // language
    dataParcel.WriteString16(Str8ToStr16("cn")); // country
    dataParcel.WriteString16(Str8ToStr16("")); // description
    dataParcel.WriteString16(u"ohos"); // package name

    /*
     * @tc.steps: step2. test get address by location's name.
     * @tc.expected: step2. no exception head info.
     */
    sptr<IRemoteObject> serviceProxy =
        CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID, CommonUtils::InitDeviceId());
    service_->serviceProxy_ = serviceProxy;
    proxy_->GetAddressByLocationName(dataParcel, replyParcel);
    EXPECT_EQ(ERRCODE_GEOCODING_FAIL, replyParcel.ReadInt32());
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByLocationName001 end");
}

HWTEST_F(GeoConvertServiceTest, ReverseGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, ReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] ReverseGeocodingMock001 begin");
    EXPECT_EQ(true, proxy_->EnableReverseGeocodingMock());
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetReverseGeocodingMockInfo(mockInfo));

    EXPECT_EQ(true, proxy_->DisableReverseGeocodingMock());
    EXPECT_EQ(ERRCODE_SUCCESS, proxy_->SetReverseGeocodingMockInfo(mockInfo));
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] ReverseGeocodingMock001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertServiceDump001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertServiceDump001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertServiceDump001 begin");
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
    EXPECT_EQ(ERR_OK, service_->Dump(fd, args));

    std::vector<std::u16string> emptyArgs;
    EXPECT_EQ(ERR_OK, service_->Dump(fd, emptyArgs));

    std::vector<std::u16string> helpArgs;
    std::u16string helpArg1 = Str8ToStr16(ARGS_HELP);
    helpArgs.emplace_back(helpArg1);
    EXPECT_EQ(ERR_OK, service_->Dump(fd, helpArgs));
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertServiceDump001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertProxyGetAddressByCoordinate001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertProxyGetAddressByCoordinate001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertProxyGetAddressByCoordinate001 begin");
    MessageParcel parcel1;
    MessageParcel reply1;
    EXPECT_EQ(true, proxy_->EnableReverseGeocodingMock());
    proxy_->GetAddressByCoordinate(parcel1, reply1);
    EXPECT_EQ(ERRCODE_SUCCESS, reply1.ReadInt32());
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertProxyGetAddressByCoordinate001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertProxyGetAddressByCoordinate002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertProxyGetAddressByCoordinate002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertProxyGetAddressByCoordinate002 begin");
    MessageParcel parcel2;
    MessageParcel reply2;
    EXPECT_EQ(true, proxy_->DisableReverseGeocodingMock());
    proxy_->GetAddressByCoordinate(parcel2, reply2);
    EXPECT_EQ(ERRCODE_REVERSE_GEOCODING_FAIL, reply2.ReadInt32());
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertProxyGetAddressByCoordinate002 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertInit001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertInit001, TestSize.Level0";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertInit001 begin");
    service_->registerToService_  = true;
    service_->Init();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnInit001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertOnRemoteRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertOnRemoteRequest001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnRemoteRequest001 begin");
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
    requestParcel.WriteBuffer("data", 4);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    service_->OnRemoteRequest(0, requestParcel, reply, option);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnRemoteRequest001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertOnStop001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertOnStop001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnStop001 begin");
    service_->OnStop();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnStop001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertNotifyDisConnected001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertNotifyDisConnected001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertNotifyDisConnected001 begin");
    service_->NotifyDisConnected();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertNotifyDisConnected001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertSaDumpInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertSaDumpInfo001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertSaDumpInfo001 begin");
    string result = "";
    service_->SaDumpInfo(result);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertSaDumpInfo001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertResetServiceProxy001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertResetServiceProxy001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertResetServiceProxy001 begin");
    EXPECT_EQ(true, service_->ResetServiceProxy());
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertResetServiceProxy001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertOnStart001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertOnStart001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnStart001 begin");
    service_->state_ = ServiceRunningState::STATE_RUNNING;
    service_->OnStart();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnStart001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertOnStart002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertOnStart002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnStart002 begin");
    service_->OnStart();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertOnStart002 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertConnectService001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertConnectService001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertConnectService001 begin");
    sptr<IRemoteObject> serviceProxy =
        CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID, CommonUtils::InitDeviceId());
    service_->serviceProxy_ = serviceProxy;
    service_->ConnectService();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertConnectService001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertReConnectService001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertReConnectService001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertReConnectService001 begin");
    service_->serviceProxy_ = nullptr;
    service_->ConnectService();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertConnectService002 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertReConnectService002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertReConnectService002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertReConnectService002 begin");
    sptr<IRemoteObject> serviceProxy =
        CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID, CommonUtils::InitDeviceId());
    service_->serviceProxy_ = serviceProxy;
    service_->ConnectService();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertConnectService003 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertNotifyConnected001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertNotifyConnected001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertNotifyConnected001 begin");
    sptr<IRemoteObject> remoteObject;
    service_->NotifyConnected(remoteObject);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertNotifyConnected001 end");
}

HWTEST_F(GeoConvertServiceTest, RegisterGeoServiceDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, RegisterGeoServiceDeathRecipient001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] RegisterGeoServiceDeathRecipient001 begin");
    service_->RegisterGeoServiceDeathRecipient();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] RegisterGeoServiceDeathRecipient001 end");
}

HWTEST_F(GeoConvertServiceTest, RegisterGeoServiceSendGeocodeRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, RegisterGeoServiceSendGeocodeRequest001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] RegisterGeoServiceSendGeocodeRequest001 begin");
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel.WriteString16(Str8ToStr16("北京")); // input description of a location
    dataParcel.WriteDouble(0.0); // minLatitude
    dataParcel.WriteDouble(0.0); // minLongitude
    dataParcel.WriteDouble(0.0); // maxLatitude
    dataParcel.WriteDouble(0.0); // maxLongitude
    dataParcel.WriteInt32(5); // maxItem
    dataParcel.WriteInt32(1); // description
    dataParcel.WriteString16(Str8ToStr16("ZH")); // language
    dataParcel.WriteString16(Str8ToStr16("cn")); // country
    dataParcel.WriteString16(Str8ToStr16("")); // description
    dataParcel.WriteString16(u"ohos"); // package name
    int code = 1;
    service_->SendGeocodeRequest(code, dataParcel, replyParcel, option);
    sptr<IRemoteObject> serviceProxy =
        CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID, CommonUtils::InitDeviceId());
    service_->serviceProxy_ = serviceProxy;
    service_->SendGeocodeRequest(code, dataParcel, replyParcel, option);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] RegisterGeoServiceSendGeocodeRequest001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertReportAddressMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertReportAddressMock001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertReportAddressMock001 begin");
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel.WriteString16(Str8ToStr16("北京")); // input description of a location
    dataParcel.WriteDouble(0.0); // minLatitude
    dataParcel.WriteDouble(0.0); // minLongitude
    dataParcel.WriteDouble(0.0); // maxLatitude
    dataParcel.WriteDouble(0.0); // maxLongitude
    dataParcel.WriteInt32(5); // maxItem
    dataParcel.WriteInt32(1); // description
    dataParcel.WriteString16(Str8ToStr16("ZH")); // language
    dataParcel.WriteString16(Str8ToStr16("cn")); // country
    dataParcel.WriteString16(Str8ToStr16("")); // description
    dataParcel.WriteString16(u"ohos"); // package name
    service_->ReportAddressMock(dataParcel, replyParcel);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertReportAddressMock001 end");
}

HWTEST_F(GeoConvertServiceTest, GeoConvertReportAddressMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoConvertReportAddressMock002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertReportAddressMock002 begin");
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel.WriteString16(Str8ToStr16("北京")); // input description of a location
    dataParcel.WriteDouble(0.0); // minLatitude
    dataParcel.WriteDouble(0.0); // minLongitude
    dataParcel.WriteDouble(0.0); // maxLatitude
    dataParcel.WriteDouble(0.0); // maxLongitude
    dataParcel.WriteInt32(5); // maxItem
    dataParcel.WriteInt32(1); // description
    dataParcel.WriteString16(Str8ToStr16("ZH")); // language
    dataParcel.WriteString16(Str8ToStr16("cn")); // country
    dataParcel.WriteString16(Str8ToStr16("")); // description
    dataParcel.WriteString16(u"ohos"); // package name
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    std::shared_ptr<GeocodingMockInfo> info = std::make_shared<GeocodingMockInfo>();
    mockInfo.push_back(info);
    info->ReadFromParcel(dataParcel);
    service_->mockInfo_ = mockInfo;
    service_->ReportAddressMock(dataParcel, replyParcel);
    MessageParcel dataParcel2;
    dataParcel2.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel2.WriteDouble(1.0); // minLatitude
    dataParcel2.WriteDouble(0.0); // minLongitude
    info->ReadFromParcel(dataParcel2);
    mockInfo.push_back(info);
    service_->mockInfo_ = mockInfo;
    service_->ReportAddressMock(dataParcel, replyParcel);
    MessageParcel dataParcel3;
    dataParcel3.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel3.WriteDouble(1.0); // minLatitude
    dataParcel3.WriteDouble(1.0); // minLongitude
    info->ReadFromParcel(dataParcel3);
    mockInfo.push_back(info);
    service_->mockInfo_ = mockInfo;
    service_->ReportAddressMock(dataParcel, replyParcel);
    MessageParcel dataParcel4;
    dataParcel4.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel4.WriteDouble(0.0); // minLatitude
    dataParcel4.WriteDouble(1.0); // minLongitude
    info->ReadFromParcel(dataParcel4);
    mockInfo.push_back(info);
    service_->mockInfo_ = mockInfo;
    service_->ReportAddressMock(dataParcel, replyParcel);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoConvertReportAddressMock002 end");
}

HWTEST_F(GeoConvertServiceTest, GetAddressByLocationName002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GetAddressByLocationName002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByLocationName002 begin");

    /*
     * @tc.steps: step1.read test data.
     */
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor());
    dataParcel.WriteString16(Str8ToStr16("北京")); // input description of a location
    dataParcel.WriteDouble(0.0); // minLatitude
    dataParcel.WriteDouble(0.0); // minLongitude
    dataParcel.WriteDouble(0.0); // maxLatitude
    dataParcel.WriteDouble(0.0); // maxLongitude
    dataParcel.WriteInt32(5); // maxItem
    dataParcel.WriteInt32(1); // description
    dataParcel.WriteString16(Str8ToStr16("ZH")); // language
    dataParcel.WriteString16(Str8ToStr16("cn")); // country
    dataParcel.WriteString16(Str8ToStr16("")); // description
    dataParcel.WriteString16(u"ohos"); // package name

    /*
     * @tc.steps: step2. test get address by location's name.
     * @tc.expected: step2. no exception head info.
     */
    proxy_->GetAddressByLocationName(dataParcel, replyParcel);
    EXPECT_EQ(ERRCODE_GEOCODING_FAIL, replyParcel.ReadInt32());
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetAddressByLocationName002 end");
}

HWTEST_F(GeoConvertServiceTest, GeoServiceDeathRecipientOnRemoteDied001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GeoServiceDeathRecipientOnRemoteDied001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoServiceDeathRecipientOnRemoteDied001 begin");
    auto recipient = new (std::nothrow) GeoServiceDeathRecipient();
    wptr<IRemoteObject> remote;
    recipient->OnRemoteDied(remote);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GeoServiceDeathRecipientOnRemoteDied001 end");
}

HWTEST_F(GeoConvertServiceTest, UnRegisterGeoServiceDeathRecipient001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, UnRegisterGeoServiceDeathRecipient001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] UnRegisterGeoServiceDeathRecipient001 begin");
    ASSERT_TRUE(service_ != nullptr);
    service_->UnRegisterGeoServiceDeathRecipient();
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] UnRegisterGeoServiceDeathRecipient001 end");
}

HWTEST_F(GeoConvertServiceTest, SendGeocodeRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, SendGeocodeRequest001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] SendGeocodeRequest001 begin");
    auto mockGeoConvertRequest = std::make_unique<GeoConvertRequest>();
    mockGeoConvertRequest->SetLocale("test");
    mockGeoConvertRequest->GetLocale();
    mockGeoConvertRequest->SetLatitude(30);
    mockGeoConvertRequest->GetLatitude();
    mockGeoConvertRequest->SetLongitude(40);
    mockGeoConvertRequest->GetLongitude();
    mockGeoConvertRequest->SetMaxItems(99);
    mockGeoConvertRequest->GetMaxItems();
    mockGeoConvertRequest->SetDescription("test");
    mockGeoConvertRequest->GetDescription();
    mockGeoConvertRequest->SetMaxLatitude(180);
    mockGeoConvertRequest->GetMaxLatitude();
    mockGeoConvertRequest->SetMaxLongitude(90);
    mockGeoConvertRequest->GetMaxLongitude();
    mockGeoConvertRequest->SetMinLongitude(0);
    mockGeoConvertRequest->GetMinLongitude();
    mockGeoConvertRequest->SetBundleName("test");
    mockGeoConvertRequest->GetBundleName();
    sptr<IRemoteObject> callback = nullptr;
    mockGeoConvertRequest->SetCallback(callback);
    mockGeoConvertRequest->GetCallback();
    mockGeoConvertRequest->SetCountry("Shanghai");
    mockGeoConvertRequest->GetCountry();
    mockGeoConvertRequest->SetMinLatitude(0);
    mockGeoConvertRequest->GetMinLatitude();
    mockGeoConvertRequest->SetTransId("Shanghai");
    mockGeoConvertRequest->GetTransId();
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(0x0100, mockGeoConvertRequest);
    ASSERT_TRUE(service_ != nullptr);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] SendGeocodeRequest001 end");
}

HWTEST_F(GeoConvertServiceTest, Marshalling001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, Marshalling001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] Marshalling001 begin");
    MessageParcel parcel;
    auto mockGeoConvertRequest = std::make_unique<GeoConvertRequest>();
    mockGeoConvertRequest->SetRequestType(GeoCodeType::REQUEST_GEOCODE);
    mockGeoConvertRequest->Marshalling(parcel);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] Marshalling001 end");
}

HWTEST_F(GeoConvertServiceTest, ReadFromParcel001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, ReadFromParcel001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] ReadFromParcel001 begin");
    MessageParcel parcel;
    auto mockGeoConvertRequest = std::make_unique<GeoConvertRequest>();
    mockGeoConvertRequest->SetRequestType(GeoCodeType::REQUEST_GEOCODE);
    mockGeoConvertRequest->ReadFromParcel(parcel);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] ReadFromParcel001 end");
}

HWTEST_F(GeoConvertServiceTest, OrderParcel001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, OrderParcel001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] OrderParcel001 begin");
    MessageParcel parcelIn;
    MessageParcel parcelOut;
    auto mockGeoConvertRequest = std::make_unique<GeoConvertRequest>();
    auto cb = sptr<MockIRemoteObject>(new (std::nothrow) MockIRemoteObject());
    std::string bundleName = "test";
    GeoCodeType requestType = GeoCodeType::REQUEST_GEOCODE;
    mockGeoConvertRequest->SetRequestType(GeoCodeType::REQUEST_GEOCODE);
    mockGeoConvertRequest->OrderParcel(parcelIn, parcelOut, cb, requestType, bundleName);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] OrderParcel001 end");
}

HWTEST_F(GeoConvertServiceTest, AddCahedGeoAddress001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, AddCahedGeoAddress001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] AddCahedGeoAddress001 begin");
    auto geoConvertRequest = std::make_unique<GeoConvertRequest>();
    geoConvertRequest->SetLocale("test");
    geoConvertRequest->SetLatitude(30);
    geoConvertRequest->SetLongitude(40);
    sptr<IRemoteObject> callback = nullptr;
    geoConvertRequest->SetCallback(callback);
    geoConvertRequest->SetCountry("Shanghai");
    geoConvertRequest->SetMinLatitude(0);
    geoConvertRequest->SetTransId("Shanghai");
    std::list<std::shared_ptr<GeoAddress>> result;
    std::shared_ptr<GeoAddress> geoAddress = std::make_shared<GeoAddress>();
    result.push_back(geoAddress);
    MessageParcel dataParcel;
    service_->WriteResultToParcel(result, dataParcel);
    service_->AddCahedGeoAddress(*geoConvertRequest, dataParcel);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] OrderParcel001 end");
}

HWTEST_F(GeoConvertServiceTest, AddCahedGeoAddress002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, AddCahedGeoAddress002, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] AddCahedGeoAddress002 begin");
    auto geoConvertRequest = std::make_unique<GeoConvertRequest>();
    sptr<IRemoteObject> callback = nullptr;
    geoConvertRequest->SetCallback(callback);
    geoConvertRequest->SetCountry("Shanghai");
    geoConvertRequest->SetMinLatitude(0);
    geoConvertRequest->SetTransId("Shanghai");
    std::list<std::shared_ptr<GeoAddress>> result;
    for (int i = 0; i < LOOP_COUNT; i++) {
        geoConvertRequest->SetLocale(std::to_string(i));
        geoConvertRequest->SetLatitude(i);
        geoConvertRequest->SetLongitude(i + 1);
        std::shared_ptr<GeoAddress> geoAddress = std::make_shared<GeoAddress>();
        if (i != 0) {
            geoAddress->placeName_ = std::to_string(i);
        }
        result.push_back(geoAddress);
        MessageParcel dataParcel;
        service_->WriteResultToParcel(result, dataParcel);
        service_->AddCahedGeoAddress(*geoConvertRequest, dataParcel);
    }
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] OrderParcel001 end");
}

HWTEST_F(GeoConvertServiceTest, GetCahedGeoAddress001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, GetCahedGeoAddress001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] GetCahedGeoAddress001 begin");
    auto geoConvertRequest = std::make_unique<GeoConvertRequest>();
    geoConvertRequest->SetLocale("1");
    geoConvertRequest->SetLatitude(30);
    geoConvertRequest->SetLongitude(40);
    sptr<IRemoteObject> callback = nullptr;
    geoConvertRequest->SetCallback(callback);
    geoConvertRequest->SetCountry("Shanghai");
    geoConvertRequest->SetMinLatitude(0);
    geoConvertRequest->SetTransId("Shanghai");
    service_->GetCahedGeoAddress(std::make_unique<GeoConvertRequest>(*geoConvertRequest));
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] OrderParcel001 end");
}

HWTEST_F(GeoConvertServiceTest, SendCacheAddressToRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeoConvertServiceTest, SendCacheAddressToRequest001, TestSize.Level1";
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] SendCacheAddressToRequest001 begin");
    auto geoConvertRequest = std::make_unique<GeoConvertRequest>();
    geoConvertRequest->SetLocale("1");
    geoConvertRequest->SetLatitude(30);
    geoConvertRequest->SetLongitude(40);
    sptr<IRemoteObject> callback = nullptr;
    geoConvertRequest->SetCallback(callback);
    geoConvertRequest->SetCountry("Shanghai");
    geoConvertRequest->SetMinLatitude(0);
    geoConvertRequest->SetTransId("Shanghai");
    std::list<std::shared_ptr<GeoAddress>> result;
    std::shared_ptr<GeoAddress> geoAddress = std::make_shared<GeoAddress>();
    result.push_back(geoAddress);
    MessageParcel dataParcel;
    service_->WriteResultToParcel(result, dataParcel);
    service_->SendCacheAddressToRequest(std::make_unique<GeoConvertRequest>(*geoConvertRequest), result);
    LBSLOGI(GEO_CONVERT, "[GeoConvertServiceTest] OrderParcel001 end");
}
}  // namespace Location
} // namespace OHOS
#endif // FEATURE_GEOCODE_SUPPORT
