/*
* Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifdef FEATURE_GNSS_SUPPORT
#include "fusion_fence_ability_test.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "fusion_fence_request.h"
#include "location_log.h"

#include <cstdlib>

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "permission_manager.h"

#include "common_utils.h"
#include "constant_definition.h"

#define private public
#include "fusion_fence_ability.h"
#include "fusion_fence_event_callback.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_IDENTIFIER = "test_identifier_001";
const std::string TEST_IDENTIFIER_2 = "test_identifier_002";
const std::string MANAGER_SETTINGS = "ohos.permission.MANAGE_SETTINGS";
const int32_t WAIT_EVENT_TIME = 3;
const int32_t LOCATION_PERM_NUM = 6;

void FusionFenceAbilityTest::SetUp()
{
    MockNativePermission();
}

void FusionFenceAbilityTest::TearDown()
{
}

void FusionFenceAbilityTest::TearDownTestCase()
{
    sleep(WAIT_EVENT_TIME);
}

void FusionFenceAbilityTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        MANAGER_SETTINGS.c_str(), ACCESS_CONTROL_LOCATION_SWITCH.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "FusionFenceAbilityTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

std::shared_ptr<FusionFenceRequest> CreateFusionFenceRequest(
    const std::string& identifier, const std::string& bundleName, int32_t fenceType)
{
    auto request = std::make_shared<FusionFenceRequest>();
    request->SetIdentifier(identifier);
    request->SetBundleName(bundleName);
    request->SetFenceType(fenceType);
    request->SetScene(FusionFenceScene::AIRPORT);
    request->SetMonitorTransitionEvents(1);
    request->SetExpirationMs(1000);
    return request;
}

std::shared_ptr<FusionFenceRequest> CreateFusionFenceRequestWithGnss(
    const std::string& identifier, const std::string& bundleName, size_t gnssCount)
{
    auto request = CreateFusionFenceRequest(identifier, bundleName, FUSION_FENCE_GNSS);
    std::vector<std::shared_ptr<FusionFenceGnss>> gnssFences;
    for (size_t i = 0; i < gnssCount; i++) {
        auto gnssFence = std::make_shared<FusionFenceGnss>();
        gnssFence->gnssFenceType = GnssFenceType::GNSS_FENCE_CIRCULAR;
        gnssFences.push_back(gnssFence);
    }
    request->SetGnssFences(gnssFences);
    return request;
}

HWTEST_F(FusionFenceAbilityTest, AddFusionFence_NullRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, AddFusionFence_NullRequest, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFusionFence_NullRequest begin");
    std::shared_ptr<FusionFenceRequest> request = nullptr;
    auto errCode = FusionFenceAbility::GetInstance()->AddFusionFence(request);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, errCode);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFusionFence_NullRequest end");
}

HWTEST_F(FusionFenceAbilityTest, AddFusionFence_NotSupported, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, AddFusionFence_NotSupported, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFusionFence_NotSupported begin");
    auto request = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto errCode = FusionFenceAbility::GetInstance()->AddFusionFence(request);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFusionFence_NotSupported end");
}

HWTEST_F(FusionFenceAbilityTest, AddFusionFence_DuplicateRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, AddFusionFence_DuplicateRequest, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFusionFence_DuplicateRequest begin");
    auto request1 = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto request2 = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto hasDuplicate1 = FusionFenceAbility::GetInstance()->HasDuplicateAddFusionFenceRequest(request1);
    EXPECT_FALSE(hasDuplicate1);
    auto hasDuplicate2 = FusionFenceAbility::GetInstance()->HasDuplicateAddFusionFenceRequest(request2);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFusionFence_DuplicateRequest end");
}

HWTEST_F(FusionFenceAbilityTest, HasDuplicateAddFusionFenceRequest_Nullptr, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, HasDuplicateAddFusionFenceRequest_Nullptr, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] HasDuplicateAddFusionFenceRequest_Nullptr begin");
    std::shared_ptr<FusionFenceRequest> request = nullptr;
    auto result = FusionFenceAbility::GetInstance()->HasDuplicateAddFusionFenceRequest(request);
    EXPECT_FALSE(result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] HasDuplicateAddFusionFenceRequest_Nullptr end");
}

HWTEST_F(FusionFenceAbilityTest, HasDuplicateAddFusionFenceRequest_NoDuplicate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, HasDuplicateAddFusionFenceRequest_NoDuplicate, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] HasDuplicateAddFusionFenceRequest_NoDuplicate begin");
    auto request1 = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto request2 = CreateFusionFenceRequest(TEST_IDENTIFIER_2, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto result = FusionFenceAbility::GetInstance()->HasDuplicateAddFusionFenceRequest(request2);
    EXPECT_FALSE(result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] HasDuplicateAddFusionFenceRequest_NoDuplicate end");
}

HWTEST_F(FusionFenceAbilityTest, IsFusionFenceExists_EmptyIdentifier, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, IsFusionFenceExists_EmptyIdentifier, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] IsFusionFenceExists_EmptyIdentifier begin");
    auto result = FusionFenceAbility::GetInstance()->IsFusionFenceExists("");
    EXPECT_FALSE(result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] IsFusionFenceExists_EmptyIdentifier end");
}

HWTEST_F(FusionFenceAbilityTest, IsFusionFenceExists_NotFound, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, IsFusionFenceExists_NotFound, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] IsFusionFenceExists_NotFound begin");
    auto result = FusionFenceAbility::GetInstance()->IsFusionFenceExists("nonexistent_identifier");
    EXPECT_FALSE(result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] IsFusionFenceExists_NotFound end");
}

HWTEST_F(FusionFenceAbilityTest, GetGnssFenceCount, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, GetGnssFenceCount, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCount begin");
    auto count = FusionFenceAbility::GetInstance()->GetGnssFenceCount();
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCount count=%{public}zu", count);
    EXPECT_GE(count, 0);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCount end");
}

HWTEST_F(FusionFenceAbilityTest, GetGnssFenceCountForOneApp, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, GetGnssFenceCountForOneApp, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCountForOneApp begin");
    auto count = FusionFenceAbility::GetInstance()->GetGnssFenceCountForOneApp(TEST_BUNDLE_NAME);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCountForOneApp count=%{public}d", count);
    EXPECT_GE(count, 0);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCountForOneApp end");
}

HWTEST_F(FusionFenceAbilityTest, GetGnssFenceCountForOneApp_UnknownBundle, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, GetGnssFenceCountForOneApp_UnknownBundle, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCountForOneApp_UnknownBundle begin");
    auto count = FusionFenceAbility::GetInstance()->GetGnssFenceCountForOneApp("unknown.bundle.name");
    EXPECT_EQ(0, count);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetGnssFenceCountForOneApp_UnknownBundle end");
}

HWTEST_F(FusionFenceAbilityTest, CheckFenceLimit_NonGnssType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, CheckFenceLimit_NonGnssType, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] CheckFenceLimit_NonGnssType begin");
    auto request = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_CELLULAR);
    auto errCode = FusionFenceAbility::GetInstance()->CheckFenceLimit(request);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] CheckFenceLimit_NonGnssType end");
}

HWTEST_F(FusionFenceAbilityTest, CheckFenceLimit_GnssType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, CheckFenceLimit_GnssType, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] CheckFenceLimit_GnssType begin");
    auto request = CreateFusionFenceRequestWithGnss(TEST_IDENTIFIER, TEST_BUNDLE_NAME, 1);
    auto errCode = FusionFenceAbility::GetInstance()->CheckFenceLimit(request);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] CheckFenceLimit_GnssType errCode=%{public}d", errCode);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] CheckFenceLimit_GnssType end");
}

HWTEST_F(FusionFenceAbilityTest, AddFenceCount_GnssType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, AddFenceCount_GnssType, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFenceCount_GnssType begin");
    auto request = CreateFusionFenceRequestWithGnss(TEST_IDENTIFIER, TEST_BUNDLE_NAME, 1);
    FusionFenceAbility::GetInstance()->AddFenceCount(request);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFenceCount_GnssType end");
}

HWTEST_F(FusionFenceAbilityTest, AddFenceCount_NonGnssType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, AddFenceCount_NonGnssType, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFenceCount_NonGnssType begin");
    auto request = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_CELLULAR);
    FusionFenceAbility::GetInstance()->AddFenceCount(request);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] AddFenceCount_NonGnssType end");
}

HWTEST_F(FusionFenceAbilityTest, RemoveFenceCount_GnssType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, RemoveFenceCount_GnssType, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFenceCount_GnssType begin");
    auto request = CreateFusionFenceRequestWithGnss(TEST_IDENTIFIER, TEST_BUNDLE_NAME, 1);
    FusionFenceAbility::GetInstance()->RemoveFenceCount(request);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFenceCount_GnssType end");
}

HWTEST_F(FusionFenceAbilityTest, RemoveFenceCount_NonGnssType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, RemoveFenceCount_NonGnssType, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFenceCount_NonGnssType begin");
    auto request = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_CELLULAR);
    FusionFenceAbility::GetInstance()->RemoveFenceCount(request);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFenceCount_NonGnssType end");
}

HWTEST_F(FusionFenceAbilityTest, RemoveFusionFence_NullRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, RemoveFusionFence_NullRequest, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFusionFence_NullRequest begin");
    std::shared_ptr<FusionFenceRequest> request = nullptr;
    auto errCode = FusionFenceAbility::GetInstance()->RemoveFusionFence(request);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, errCode);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFusionFence_NullRequest end");
}

HWTEST_F(FusionFenceAbilityTest, RemoveFusionFence_NotSupported, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, RemoveFusionFence_NotSupported, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFusionFence_NotSupported begin");
    auto request = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto errCode = FusionFenceAbility::GetInstance()->RemoveFusionFence(request);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFusionFence_NotSupported end");
}

HWTEST_F(FusionFenceAbilityTest, GetFusionFenceRequest_Empty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, GetFusionFenceRequest_Empty, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetFusionFenceRequest_Empty begin");
    auto result = FusionFenceAbility::GetInstance()->GetFusionFenceRequest("nonexistent");
    EXPECT_EQ(nullptr, result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] GetFusionFenceRequest_Empty end");
}

HWTEST_F(FusionFenceAbilityTest, ReportFusionFenceEvent_NotFound, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, ReportFusionFenceEvent_NotFound, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ReportFusionFenceEvent_NotFound begin");
    FusionFenceTransition transition;
    transition.identifier = "nonexistent_id";
    transition.scene = FusionFenceScene::AIRPORT;
    transition.transitionEvent = GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER;
    FusionFenceAbility::GetInstance()->ReportFusionFenceEvent(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ReportFusionFenceEvent_NotFound end");
}

HWTEST_F(FusionFenceAbilityTest, ReportOperateResult_NullRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, ReportOperateResult_NullRequest, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ReportOperateResult_NullRequest begin");
    std::shared_ptr<FusionFenceRequest> request = nullptr;
    FusionFenceAbility::GetInstance()->ReportOperateResult(
        request, GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD, ERRCODE_SUCCESS);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ReportOperateResult_NullRequest end");
}

HWTEST_F(FusionFenceAbilityTest, RemoveFusionFenceByCallbackWhenAppDie_NullCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, RemoveFusionFenceByCallbackWhenAppDie_NullCallback, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFusionFenceByCallbackWhenAppDie_NullCallback begin");
    sptr<IRemoteObject> callback = nullptr;
    FusionFenceAbility::GetInstance()->RemoveFusionFenceByCallbackWhenAppDie(callback);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] RemoveFusionFenceByCallbackWhenAppDie_NullCallback end");
}

HWTEST_F(FusionFenceAbilityTest, ExecuteFusionFenceProcess_NullRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, ExecuteFusionFenceProcess_NullRequest, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_NullRequest begin");
    std::shared_ptr<FusionFenceRequest> request = nullptr;
    auto result = FusionFenceAbility::GetInstance()->ExecuteFusionFenceProcess(
        request, static_cast<int>(GnssInterfaceCode::ADD_FUSION_FENCE));
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_NullRequest result=%{public}d", result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_NullRequest end");
}

HWTEST_F(FusionFenceAbilityTest, ExecuteFusionFenceProcess_AddFusionFence, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, ExecuteFusionFenceProcess_AddFusionFence, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_AddFusionFence begin");
    auto request = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto result = FusionFenceAbility::GetInstance()->ExecuteFusionFenceProcess(
        request, static_cast<int>(GnssInterfaceCode::ADD_FUSION_FENCE));
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_AddFusionFence result=%{public}d", result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_AddFusionFence end");
}

HWTEST_F(FusionFenceAbilityTest, ExecuteFusionFenceProcess_RemoveFusionFence, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, ExecuteFusionFenceProcess_RemoveFusionFence, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_RemoveFusionFence begin");
    auto request = CreateFusionFenceRequest(TEST_IDENTIFIER, TEST_BUNDLE_NAME, FUSION_FENCE_GNSS);
    auto result = FusionFenceAbility::GetInstance()->ExecuteFusionFenceProcess(
        request, static_cast<int>(GnssInterfaceCode::REMOVE_FUSION_FENCE));
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_RemoveFusionFence result=%{public}d", result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_RemoveFusionFence end");
}

HWTEST_F(FusionFenceAbilityTest, ExecuteFusionFenceProcess_IsSupported, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, ExecuteFusionFenceProcess_IsSupported, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_IsSupported begin");
    std::shared_ptr<FusionFenceRequest> request;
    auto result = FusionFenceAbility::GetInstance()->ExecuteFusionFenceProcess(
        request, static_cast<int>(GnssInterfaceCode::IS_FUSION_FENCE_SUPPORTED));
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_IsSupported result=%{public}d", result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] ExecuteFusionFenceProcess_IsSupported end");
}

HWTEST_F(FusionFenceAbilityTest, IsFusionFenceSupported, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, IsFusionFenceSupported, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] IsFusionFenceSupported begin");
    auto result = FusionFenceAbility::GetInstance()->IsFusionFenceSupported();
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] IsFusionFenceSupported result=%{public}d", result);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] IsFusionFenceSupported end");
}

HWTEST_F(FusionFenceAbilityTest, FusionFenceCallbackDeathRecipient_OnRemoteDied, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, FusionFenceCallbackDeathRecipient_OnRemoteDied, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] FusionFenceCallbackDeathRecipient_OnRemoteDied begin");
    auto deathRecipient = std::make_shared<FusionFenceCallbackDeathRecipient>();
    wptr<IRemoteObject> remote;
    deathRecipient->OnRemoteDied(remote);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] FusionFenceCallbackDeathRecipient_OnRemoteDied end");
}

HWTEST_F(FusionFenceAbilityTest, FusionFenceAbility_GetInstance, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FusionFenceAbilityTest, FusionFenceAbility_GetInstance, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] FusionFenceAbility_GetInstance begin");
    auto instance1 = FusionFenceAbility::GetInstance();
    auto instance2 = FusionFenceAbility::GetInstance();
    EXPECT_EQ(instance1, instance2);
    LBSLOGI(FUSION_FENCE, "[FusionFenceAbilityTest] FusionFenceAbility_GetInstance end");
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT