/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "common_utils_test.h"

#include "string_ex.h"

#include "accesstoken_kit.h"
#include "hilog/log.h"
#include "nativetoken_kit.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "permission_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 5;
const int32_t APPOXI_LOCATION_PERM_NUM = 3;
const int32_t ACC_LOCATION_PERM_NUM = 3;
const int UNKNOWN_SA_ID = -1;
const uint32_t CAPABILITY = 0x102;
const double NUM_ACC_E6 = 1.000001;
const double NUM_ACC_E7 = 1.0000001;
void CommonUtilsTest::SetUp()
{
    MockNativeAccurateLocation();
}

void CommonUtilsTest::TearDown()
{
}

void CommonUtilsTest::MockNativePermission()
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
        .processName = "CommonTest1",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void CommonUtilsTest::MockNativeApproxiPermission()
{
    const char *perms[] = {
        ACCESS_APPROXIMATELY_LOCATION.c_str(), ACCESS_BACKGROUND_LOCATION.c_str(),
        MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = APPOXI_LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "CommonTest2",
        .aplStr = "system_basic",
    };
    tokenIdForApproxi_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenIdForApproxi_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void CommonUtilsTest::MockNativeAccurateLocation()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_BACKGROUND_LOCATION.c_str(),
        MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = ACC_LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "CommonTest3",
        .aplStr = "system_basic",
    };
    tokenIdForAcc_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenIdForAcc_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void CommonUtilsTest::LoadSystemAbility()
{
    auto locationSaLoadManager = LocationSaLoadManager::GetInstance();
    if (locationSaLoadManager == nullptr) {
        return;
    }
    locationSaLoadManager->LoadLocationSa(LOCATION_LOCATOR_SA_ID);
#ifdef FEATURE_GNSS_SUPPORT
    locationSaLoadManager->LoadLocationSa(LOCATION_GNSS_SA_ID);
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    locationSaLoadManager->LoadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    locationSaLoadManager->LoadLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
    locationSaLoadManager->LoadLocationSa(LOCATION_GEO_CONVERT_SA_ID);
#endif
}

HWTEST_F(CommonUtilsTest, AbilityConvertToIdTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, AbilityConvertToIdTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] AbilityConvertToIdTest001 begin");
    EXPECT_EQ(LOCATION_GNSS_SA_ID, CommonUtils::AbilityConvertToId(GNSS_ABILITY));
    EXPECT_EQ(LOCATION_NETWORK_LOCATING_SA_ID, CommonUtils::AbilityConvertToId(NETWORK_ABILITY));
    EXPECT_EQ(LOCATION_NOPOWER_LOCATING_SA_ID, CommonUtils::AbilityConvertToId(PASSIVE_ABILITY));
    EXPECT_EQ(LOCATION_GEO_CONVERT_SA_ID, CommonUtils::AbilityConvertToId(GEO_ABILITY));
    EXPECT_EQ(UNKNOWN_SA_ID, CommonUtils::AbilityConvertToId("unknown_ability"));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] AbilityConvertToIdTest001 end");
}

HWTEST_F(CommonUtilsTest, GetCapabilityTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, GetCapabilityTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetCapabilityTest001 begin");
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapabilityToString("unknown_ability", CAPABILITY));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapabilityToString(GNSS_ABILITY, CAPABILITY));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapabilityToString(NETWORK_ABILITY, CAPABILITY));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapabilityToString(PASSIVE_ABILITY, CAPABILITY));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapabilityToString(GEO_ABILITY, CAPABILITY));

    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapability("unknown_ability"));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapability(GNSS_ABILITY));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapability(NETWORK_ABILITY));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapability(PASSIVE_ABILITY));
    EXPECT_NE(Str8ToStr16(""), CommonUtils::GetCapability(GEO_ABILITY));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetCapabilityTest001 end");
}

HWTEST_F(CommonUtilsTest, GetLabelTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, GetLabelTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetLabelTest001 begin");
    EXPECT_NE("", CommonUtils::GetLabel(GNSS_ABILITY).tag);
    EXPECT_NE("", CommonUtils::GetLabel(NETWORK_ABILITY).tag);
    EXPECT_NE("", CommonUtils::GetLabel(PASSIVE_ABILITY).tag);
    EXPECT_NE("", CommonUtils::GetLabel(GEO_ABILITY).tag);
    EXPECT_NE("", CommonUtils::GetLabel("unknown_ability").tag);
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetLabelTest001 end");
}

HWTEST_F(CommonUtilsTest, GetRemoteObjectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, GetRemoteObjectTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetRemoteObjectTest001 begin");
    // inert to map
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NOPOWER_LOCATING_SA_ID));
    CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID);
    EXPECT_EQ(nullptr, CommonUtils::GetRemoteObject(UNKNOWN_SA_ID));

    // read from map
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NOPOWER_LOCATING_SA_ID));
    CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID);
    EXPECT_EQ(nullptr, CommonUtils::GetRemoteObject(UNKNOWN_SA_ID));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetRemoteObjectTest001 end");
}

HWTEST_F(CommonUtilsTest, GetCurrentUserIdTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, GetCurrentUserIdTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetCurrentUserIdTest001 begin");
    int32_t userId = 0;
    EXPECT_EQ(true, CommonUtils::GetCurrentUserId(userId));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetCurrentUserIdTest001 end");
}

HWTEST_F(CommonUtilsTest, Str16ToStr8Test001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, Str16ToStr8Test001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] Str16ToStr8Test001 begin");
    EXPECT_EQ(DEFAULT_STRING, CommonUtils::Str16ToStr8(DEFAULT_USTRING));
    EXPECT_EQ("string16", CommonUtils::Str16ToStr8(u"string16"));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] Str16ToStr8Test001 end");
}

HWTEST_F(CommonUtilsTest, DoubleEqualTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, DoubleEqualTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] DoubleEqualTest001 begin");
    EXPECT_EQ(true, CommonUtils::DoubleEqual(1.0, 1.0));
    EXPECT_EQ(true, CommonUtils::DoubleEqual(1.0, NUM_ACC_E7));
    EXPECT_EQ(true, CommonUtils::DoubleEqual(1.0, NUM_ACC_E6));
    EXPECT_EQ(false, CommonUtils::DoubleEqual(1.0, 2.0));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] DoubleEqualTest001 end");
}

HWTEST_F(CommonUtilsTest, CalculationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, CalculationTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] CalculationTest001 begin");
    EXPECT_EQ(0, CommonUtils::CalDistance(1.0, 1.0, 1.0, 1.0));
    EXPECT_NE(0, CommonUtils::CalDistance(1.0, 1.0, 1.0, NUM_ACC_E6));
    EXPECT_NE(0, CommonUtils::CalDistance(1.0, 1.0, NUM_ACC_E6, 1.0));
    EXPECT_NE(-1, CommonUtils::DoubleRandom(0.0, 1.0));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] CalculationTest001 end");
}

HWTEST_F(CommonUtilsTest, GetBundleNameByUidTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CommonUtilsTest, GetBundleNameByUidTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetBundleNameByUidTest001 begin");
    std::string bundleName;
    EXPECT_EQ(false, CommonUtils::GetBundleNameByUid(SYSTEM_UID, bundleName));
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetBundleNameByUidTest001 end");
}

HWTEST_F(CommonUtilsTest, CountDownLatchWaitTest001, TestSize.Level1)
{
    auto latch = std::make_shared<CountDownLatch>();
    latch->SetCount(0);
    latch->Wait(0); // count is 0
    EXPECT_EQ(0, latch->GetCount());
}

HWTEST_F(CommonUtilsTest, CountDownLatchWaitTest002, TestSize.Level1)
{
    auto latch = std::make_shared<CountDownLatch>();
    latch->SetCount(1);
    latch->Wait(0); // wait 0ms
    EXPECT_EQ(1, latch->GetCount());
}

HWTEST_F(CommonUtilsTest, CountDownLatchCountDownTest001, TestSize.Level1)
{
    auto latch = std::make_shared<CountDownLatch>();
    latch->SetCount(1);
    latch->CountDown();
    EXPECT_EQ(0, latch->GetCount());
}

HWTEST_F(CommonUtilsTest, CountDownLatchCountDownTest002, TestSize.Level1)
{
    auto latch = std::make_shared<CountDownLatch>();
    latch->CountDown();
    EXPECT_EQ(0, latch->GetCount());
}

HWTEST_F(CommonUtilsTest, CountDownLatchCountDownTest003, TestSize.Level1)
{
    auto latch = std::make_shared<CountDownLatch>();
    latch->SetCount(5);
    latch->CountDown();
    EXPECT_EQ(4, latch->GetCount());
}

HWTEST_F(CommonUtilsTest, GetMacArrayTest001, TestSize.Level1)
{
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetMacArrayTest001 begin");
    uint8_t macArray[MAC_LEN];
    if (CommonUtils::GetMacArray("20:28:3e:74:34:70", macArray) != EOK) {
        LBSLOGE(COMMON_UTILS, "GetMacArray failed.");
    }
    EXPECT_EQ(32, macArray[0]);
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetMacArrayTest001 end");
}

HWTEST_F(CommonUtilsTest, SplitTest001, TestSize.Level1)
{
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] SplitTest001 begin");
    std::vector<std::string> strVec = CommonUtils::Split("aa:bb:cc:dd:ee:ff", ":");
    EXPECT_EQ(6, strVec.size());
}

HWTEST_F(CommonUtilsTest, GetStringParameter001, TestSize.Level1)
{
    std::string name = "";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetStringParameter001 begin");
    bool ret = CommonUtils::GetStringParameter("test", name);
    EXPECT_EQ(false, ret);
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetStringParameter001 rnd");
}

HWTEST_F(CommonUtilsTest, GetStringParameter002, TestSize.Level1)
{
    std::string name = "";
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetStringParameter002 begin");
    CommonUtils::GetStringParameter(SUPL_MODE_NAME, name);
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetStringParameter002 end");
}

HWTEST_F(CommonUtilsTest, GetCurrentTime001, TestSize.Level1)
{
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetCurrentTime001 begin");
    int64_t timeStamp = 0;
    timeStamp = CommonUtils::GetCurrentTime();
    EXPECT_NE(0, timeStamp);
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GetCurrentTime001 end");
}

HWTEST_F(CommonUtilsTest, GenerateUuid001, TestSize.Level1)
{
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GenerateUuid001 begin");
    std::string uuid = CommonUtils::GenerateUuid();
    EXPECT_LT(0, uuid.size());
    LBSLOGI(COMMON_UTILS, "[CommonUtilsTest] GenerateUuid001 end");
}
} // namespace Location
} // namespace OHOS
