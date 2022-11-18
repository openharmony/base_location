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

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const int32_t APPOXI_LOCATION_PERM_NUM = 4;
const int32_t ACC_LOCATION_PERM_NUM = 4;
const int UNKNOWN_SA_ID = -1;
const uint32_t CAPABILITY = 0x102;
const double NUM_ACC_E6 = 1.000001;
const double NUM_ACC_E7 = 1.0000001;
void CommonUtilsTest::SetUp()
{
}

void CommonUtilsTest::TearDown()
{
}

void CommonUtilsTest::MockNativePermission()
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
        .processName = "CommonTest",
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
        .processName = "CommonTest",
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
        .processName = "CommonTest",
        .aplStr = "system_basic",
    };
    tokenIdForAcc_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenIdForAcc_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(CommonUtilsTest, AbilityConvertToIdTest001, TestSize.Level1)
{
    EXPECT_EQ(LOCATION_GNSS_SA_ID, CommonUtils::AbilityConvertToId(GNSS_ABILITY));
    EXPECT_EQ(LOCATION_NETWORK_LOCATING_SA_ID, CommonUtils::AbilityConvertToId(NETWORK_ABILITY));
    EXPECT_EQ(LOCATION_NOPOWER_LOCATING_SA_ID, CommonUtils::AbilityConvertToId(PASSIVE_ABILITY));
    EXPECT_EQ(LOCATION_GEO_CONVERT_SA_ID, CommonUtils::AbilityConvertToId(GEO_ABILITY));
    EXPECT_EQ(UNKNOWN_SA_ID, CommonUtils::AbilityConvertToId("unknown_ability"));
}

HWTEST_F(CommonUtilsTest, GetCapabilityTest001, TestSize.Level1)
{
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
}

HWTEST_F(CommonUtilsTest, GetLabelTest001, TestSize.Level1)
{
    EXPECT_EQ(GNSS, CommonUtils::GetLabel(GNSS_ABILITY));
    EXPECT_EQ(NETWORK, CommonUtils::GetLabel(NETWORK_ABILITY));
    EXPECT_EQ(PASSIVE, CommonUtils::GetLabel(PASSIVE_ABILITY));
    EXPECT_EQ(GEO_CONVERT, CommonUtils::GetLabel(GEO_ABILITY));
    OHOS::HiviewDFX::HiLogLabel label = { LOG_CORE, LOCATOR_LOG_ID, "unknown" };
    EXPECT_EQ(label, CommonUtils::GetLabel("unknown_ability"));
}

HWTEST_F(CommonUtilsTest, GetRemoteObjectTest001, TestSize.Level1)
{
    // inert to map
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NOPOWER_LOCATING_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(UNKNOWN_SA_ID));

    // read from map
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_NOPOWER_LOCATING_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID));
    EXPECT_NE(nullptr, CommonUtils::GetRemoteObject(UNKNOWN_SA_ID));
}

HWTEST_F(CommonUtilsTest, GetRemoteObjectTest001, TestSize.Level1)
{
    uint32_t invalidTokenId = 0;
    uint32_t firstTokenId = 0;
    // invalid type
    EXPECT_EQ(false, CheckLocationPermission(invalidTokenId, firstTokenId));
    EXPECT_EQ(false, CheckApproximatelyPermission(invalidTokenId, firstTokenId));
    EXPECT_EQ(false, CheckBackgroundPermission(invalidTokenId, firstTokenId));
    EXPECT_EQ(false, CheckSecureSettings(invalidTokenId, firstTokenId));

    // shell type
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t callingFirstTokenid = IPCSkeleton::GetFirstTokenID();
    EXPECT_EQ(false, CheckLocationPermission(callingTokenId, callingFirstTokenid));
    EXPECT_EQ(false, CheckApproximatelyPermission(callingTokenId, callingFirstTokenid));
    EXPECT_EQ(false, CheckBackgroundPermission(callingTokenId, callingFirstTokenid));
    EXPECT_EQ(false, CheckSecureSettings(callingTokenId, callingFirstTokenid));

    MockNativePermission(); // grant the location permissions
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    EXPECT_EQ(true, CheckLocationPermission(tokenId, 0));
    EXPECT_EQ(true, CheckApproximatelyPermission(tokenId, 0));
    EXPECT_EQ(true, CheckBackgroundPermission(tokenId, 0));
    EXPECT_EQ(true, CheckSecureSettings(tokenId, 0));

    // invalid first token id
    EXPECT_EQ(false, CheckLocationPermission(tokenId, 1));
    EXPECT_EQ(false, CheckApproximatelyPermission(tokenId, 1));
    EXPECT_EQ(false, CheckBackgroundPermission(tokenId, 1));
    EXPECT_EQ(false, CheckSecureSettings(tokenId, 1));

    // valid token id and first token id
    EXPECT_EQ(true, CheckLocationPermission(tokenId, tokenId));
    EXPECT_EQ(true, CheckApproximatelyPermission(tokenId, tokenId));
    EXPECT_EQ(true, CheckBackgroundPermission(tokenId, tokenId));
    EXPECT_EQ(true, CheckSecureSettings(tokenId, tokenId));
}

HWTEST_F(CommonUtilsTest, GetCurrentUserIdTest001, TestSize.Level1)
{
    int32_t userId = 0;
    EXPECT_EQ(true, CommonUtils::GetCurrentUserId(userId));
}

HWTEST_F(CommonUtilsTest, Str16ToStr8Test001, TestSize.Level1)
{
    EXPECT_EQ(DEFAULT_STRING, CommonUtils::Str16ToStr8(DEFAULT_USTRING));
    EXPECT_EQ("string16", CommonUtils::Str16ToStr8(u"string16"));
}

HWTEST_F(CommonUtilsTest, DoubleEqualTest001, TestSize.Level1)
{
    EXPECT_EQ(true, CommonUtils::DoubleEqual(1.0, 1.0));
    EXPECT_EQ(true, CommonUtils::DoubleEqual(1.0, NUM_ACC_E7));
    EXPECT_EQ(false, CommonUtils::DoubleEqual(1.0, NUM_ACC_E6));
    EXPECT_EQ(false, CommonUtils::DoubleEqual(1.0, 2.0));
}

HWTEST_F(CommonUtilsTest, CalculationTest001, TestSize.Level1)
{
    EXPECT_EQ(0, CommonUtils::CalDistance(1.0, 1.0, 1.0, 1.0));
    EXPECT_NE(0, CommonUtils::CalDistance(1.0, 1.0, 1.0, NUM_ACC_E6));
    EXPECT_NE(0, CommonUtils::CalDistance(1.0, 1.0, NUM_ACC_E6, 1.0));
    EXPECT_NE(-1, CommonUtils::DoubleRandom(0.0, 1.0));
}

HWTEST_F(CommonUtilsTest, GetPermissionLevelTest001, TestSize.Level1)
{
    EXPECT_EQ(PERMISSION_INVALID, CommonUtils::GetPermissionLevel(0, 0));
}

HWTEST_F(CommonUtilsTest, GetPermissionLevelTest002, TestSize.Level1)
{
    MockNativePermission();
    EXPECT_EQ(PERMISSION_ACCURATE, CommonUtils::GetPermissionLevel(tokenId_, 0));
}

HWTEST_F(CommonUtilsTest, GetPermissionLevelTest003, TestSize.Level1)
{
    MockNativeAccurateLocation();
    EXPECT_EQ(PERMISSION_ACCURATE, CommonUtils::GetPermissionLevel(tokenIdForAcc_, 0));
}

HWTEST_F(CommonUtilsTest, GetPermissionLevelTest004, TestSize.Level1)
{
    MockNativeApproxiPermission();
    EXPECT_EQ(PERMISSION_APPROXIMATELY, CommonUtils::GetPermissionLevel(tokenIdForApproxi_, 0));
}

HWTEST_F(CommonUtilsTest, CheckSystemPermissionTest001, TestSize.Level1)
{
    EXPECT_EQ(false, CommonUtils::CheckSystemPermission(SYSTEM_UID, 0));
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    EXPECT_EQ(false, CommonUtils::CheckSystemPermission(SYSTEM_UID, callingTokenId));
    MockNativePermission();
    EXPECT_EQ(true, CommonUtils::CheckSystemPermission(SYSTEM_UID, tokenId_));
}

HWTEST_F(CommonUtilsTest, GetBundleNameByUidTest001, TestSize.Level1)
{
    EXPECT_EQ(false, CommonUtils::GetBundleNameByUid(SYSTEM_UID, "bundleName"));
}