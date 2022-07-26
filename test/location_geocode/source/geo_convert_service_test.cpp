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
#include <string>
#include "common_utils.h"
#include "geo_convert_service.h"
#include "geo_convert_skeleton.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "location_log.h"
#include "parameters.h"
#include "string_ex.h"
#include "system_ability_definition.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Location;

void GeoConvertServiceTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, systemAbilityManager);
    sptr<IRemoteObject> object = systemAbilityManager->GetSystemAbility(LOCATION_GEO_CONVERT_SA_ID);
    EXPECT_NE(nullptr, object);
    proxy_ = new (std::nothrow) GeoConvertProxy(object);
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
