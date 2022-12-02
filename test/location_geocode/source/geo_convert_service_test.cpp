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

#include "geo_convert_service_test.h"

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

#include "common_utils.h"
#include "geo_coding_mock_info.h"
#include "geo_convert_service.h"
#include "geo_convert_skeleton.h"
#include "location_dumper.h"
#include "location_log.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const std::string ARGS_HELP = "-h";
void GeoConvertServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    MockNativePermission();
    service_ = new (std::nothrow) GeoConvertService();
    EXPECT_NE(nullptr, service_);
    proxy_ = new (std::nothrow) GeoConvertProxy(service_);
    EXPECT_NE(nullptr, proxy_);
    available_ = Available();
}

void GeoConvertServiceTest::TearDown()
{
    /*
     * @tc.teardown: release memory.
     */
    proxy_ = nullptr;
}

void GeoConvertServiceTest::MockNativePermission()
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
    if (proxy_ != nullptr) {
        proxy_->IsGeoConvertAvailable(replyParcel);
    }
    replyParcel.ReadInt32();
    int temp = replyParcel.ReadInt32();
    bool result = (temp != 0);
    return result;
}

/*
 * @tc.name: GeoConvertAvailable001
 * @tc.desc: Check location system ability whether available.
 * @tc.type: FUNC
 */
HWTEST_F(GeoConvertServiceTest, GeoConvertAvailable001, TestSize.Level1)
{
    if (!available_) {
        return;
    }

    /*
     * @tc.steps: step1. Call system ability and check whether available.
     * @tc.expected: step1. system ability is available.
     */
    bool result = Available();
    EXPECT_EQ(true, result);
}

/*
 * @tc.name: GeoAddressByCoordinate001
 * @tc.desc: Test get address from system ability by coordinate.
 * @tc.type: FUNC
 */
HWTEST_F(GeoConvertServiceTest, GetAddressByCoordinate001, TestSize.Level1)
{
    if (!available_) {
        return;
    }

    /*
     * @tc.steps: step1.read test data.
     */
    MessageParcel dataParcel;
    MessageParcel replyParcel;
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
    bool ret = false;
    int exceptionHeader = replyParcel.ReadInt32();
    if (exceptionHeader == REPLY_CODE_NO_EXCEPTION) {
        ret = true;
    }
    EXPECT_TRUE(ret);
}

/*
 * @tc.name: GetAddressByLocationName001
 * @tc.desc: Test get address from system ability by location name.
 * @tc.type: FUNC
 */
HWTEST_F(GeoConvertServiceTest, GetAddressByLocationName001, TestSize.Level1)
{
    if (!available_) {
        return;
    }

    /*
     * @tc.steps: step1.read test data.
     */
    MessageParcel dataParcel;
    MessageParcel replyParcel;
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
    bool ret = false;
    int exceptionHeader = replyParcel.ReadInt32();
    if (exceptionHeader == REPLY_CODE_NO_EXCEPTION) {
        ret = true;
    }
    EXPECT_TRUE(ret);
}

HWTEST_F(GeoConvertServiceTest, ReverseGeocodingMock001, TestSize.Level1)
{
    EXPECT_EQ(true, proxy_->EnableReverseGeocodingMock());
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    EXPECT_EQ(true, proxy_->SetReverseGeocodingMockInfo(mockInfo));

    EXPECT_EQ(true, proxy_->DisableReverseGeocodingMock());
    EXPECT_EQ(true, proxy_->SetReverseGeocodingMockInfo(mockInfo));
}

HWTEST_F(GeoConvertServiceTest, GeoConvertServiceOnStartAndOnStop001, TestSize.Level1)
{
    service_->OnStart(); // start ability
    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, service_->QueryServiceState());
    service_->OnStop(); // stop ability
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, service_->QueryServiceState());
}

HWTEST_F(GeoConvertServiceTest, GeoConvertServiceDump001, TestSize.Level1)
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
    EXPECT_EQ(ERR_OK, service_->Dump(fd, args));

    std::vector<std::u16string> emptyArgs;
    EXPECT_EQ(ERR_OK, service_->Dump(fd, emptyArgs));

    std::vector<std::u16string> helpArgs;
    std::u16string helpArg1 = Str8ToStr16(ARGS_HELP);
    helpArgs.emplace_back(helpArg1);
    EXPECT_EQ(ERR_OK, service_->Dump(fd, helpArgs));
}

HWTEST_F(GeoConvertServiceTest, GeoConvertProxyGetAddressByCoordinate001, TestSize.Level1)
{
    MessageParcel parcel1;
    MessageParcel reply1;
    EXPECT_EQ(true, proxy_->EnableReverseGeocodingMock());
    EXPECT_EQ(REPLY_CODE_NO_EXCEPTION, proxy_->GetAddressByCoordinate(parcel1, reply1));

    MessageParcel parcel2;
    MessageParcel reply2;
    EXPECT_EQ(true, proxy_->DisableReverseGeocodingMock());
    proxy_->GetAddressByCoordinate(parcel2, reply2);
    EXPECT_EQ(REPLY_CODE_UNSUPPORT, reply2.ReadInt32());
}

HWTEST_F(GeoConvertServiceTest, GeoConvertProxyGetAddressByLocationName001, TestSize.Level1)
{
    MessageParcel parcel;
    MessageParcel reply;
    proxy_->GetAddressByLocationName(parcel, reply);
    EXPECT_EQ(REPLY_CODE_UNSUPPORT, reply.ReadInt32());
}
}  // namespace Location
} // namespace OHOS