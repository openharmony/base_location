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

#include "permission_manager_test.h"

#include "string_ex.h"

#include "accesstoken_kit.h"
#include "hilog/log.h"
#include "nativetoken_kit.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "location_log.h"
#include "location_sa_load_manager.h"
#include "permission_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const int32_t APPOXI_LOCATION_PERM_NUM = 3;
const int32_t ACC_LOCATION_PERM_NUM = 3;
void PermissionManagerTest::SetUp()
{
}

void PermissionManagerTest::TearDown()
{
}

void PermissionManagerTest::MockNativePermission()
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
        .processName = "PermissionManagerTest1",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void PermissionManagerTest::MockNativeApproxiPermission()
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
        .processName = "PermissionManagerTest2",
        .aplStr = "system_basic",
    };
    tokenIdForApproxi_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenIdForApproxi_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void PermissionManagerTest::MockNativeAccurateLocation()
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
        .processName = "PermissionManagerTest3",
        .aplStr = "system_basic",
    };
    tokenIdForAcc_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenIdForAcc_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(PermissionManagerTest, GetRemoteObjectTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, GetRemoteObjectTest002, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetRemoteObjectTest002 begin");
    uint32_t invalidTokenId = 0;
    uint32_t firstTokenId = 0;
    // invalid type
    EXPECT_EQ(false, PermissionManager::CheckLocationPermission(invalidTokenId, firstTokenId));
    EXPECT_EQ(false, PermissionManager::CheckApproximatelyPermission(invalidTokenId, firstTokenId));
    EXPECT_EQ(false, PermissionManager::CheckBackgroundPermission(invalidTokenId, firstTokenId));
    EXPECT_EQ(false, PermissionManager::CheckSecureSettings(invalidTokenId, firstTokenId));

    MockNativePermission(); // grant the location permissions
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    PermissionManager::CheckLocationPermission(tokenId, 0);
    EXPECT_EQ(true, PermissionManager::CheckApproximatelyPermission(tokenId, 0));
    EXPECT_EQ(true, PermissionManager::CheckBackgroundPermission(tokenId, 0));
    EXPECT_EQ(true, PermissionManager::CheckSecureSettings(tokenId, 0));

    // invalid first token id
    EXPECT_EQ(false, PermissionManager::CheckLocationPermission(tokenId, 1));
    EXPECT_EQ(false, PermissionManager::CheckApproximatelyPermission(tokenId, 1));
    EXPECT_EQ(false, PermissionManager::CheckBackgroundPermission(tokenId, 1));
    EXPECT_EQ(false, PermissionManager::CheckSecureSettings(tokenId, 1));

    // valid token id and first token id
    EXPECT_EQ(true, PermissionManager::CheckLocationPermission(tokenId, tokenId));
    EXPECT_EQ(true, PermissionManager::CheckApproximatelyPermission(tokenId, tokenId));
    EXPECT_EQ(true, PermissionManager::CheckBackgroundPermission(tokenId, tokenId));
    EXPECT_EQ(true, PermissionManager::CheckSecureSettings(tokenId, tokenId));
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetRemoteObjectTest002 end");
}

HWTEST_F(PermissionManagerTest, GetPermissionLevelTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, GetPermissionLevelTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest001 begin");
    EXPECT_EQ(PERMISSION_INVALID, PermissionManager::GetPermissionLevel(0, 0));
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest001 end");
}

HWTEST_F(PermissionManagerTest, GetPermissionLevelTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, GetPermissionLevelTest002, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest002 begin");
    MockNativePermission();
    PermissionManager::GetPermissionLevel(tokenId_, 0);
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest002 end");
}

HWTEST_F(PermissionManagerTest, GetPermissionLevelTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, GetPermissionLevelTest003, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest003 begin");
    MockNativeAccurateLocation();
    PermissionManager::GetPermissionLevel(tokenIdForAcc_, 0);
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest003 end");
}

HWTEST_F(PermissionManagerTest, GetPermissionLevelTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, GetPermissionLevelTest004, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest004 begin");
    MockNativeApproxiPermission();
    PermissionManager::GetPermissionLevel(tokenIdForApproxi_, 0);
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] GetPermissionLevelTest004 end");
}

HWTEST_F(PermissionManagerTest, CheckSystemPermissionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, CheckSystemPermissionTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckSystemPermissionTest001 begin");
    EXPECT_EQ(false, PermissionManager::CheckSystemPermission(0, 1));
    MockNativePermission();
    EXPECT_EQ(true, PermissionManager::CheckSystemPermission(tokenId_, 1));
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckSystemPermissionTest001 end");
}

HWTEST_F(PermissionManagerTest, CheckCallingPermissionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, CheckCallingPermissionTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckCallingPermissionTest001 begin");
    pid_t uid = 8888;
    pid_t pid = 8888;
    MessageParcel reply;
    EXPECT_EQ(false, PermissionManager::CheckCallingPermission(uid, pid, reply));
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckCallingPermissionTest001 end");
}

HWTEST_F(PermissionManagerTest, CheckRssProcessName001, TestSize.Level1)
{
    uint32_t invalidTokenId = 0;
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckRssProcessName001 begin");
    EXPECT_EQ(false, PermissionManager::CheckRssProcessName(invalidTokenId));
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckRssProcessName001 end");
}

HWTEST_F(PermissionManagerTest, CheckMockLocationPermissionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PermissionManagerTest, CheckMockLocationPermissionTest001, TestSize.Level1";
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckMockLocationPermissionTest001 begin");
    uint32_t invalidTokenId = 0;
    uint32_t firstTokenId = 0;
    EXPECT_EQ(false, PermissionManager::CheckMockLocationPermission(invalidTokenId, firstTokenId));
    LBSLOGI(COMMON_UTILS, "[PermissionManagerTest] CheckMockLocationPermissionTest001 end");
}
} // namespace Location
} // namespace OHOS
