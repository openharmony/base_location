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
#include "fusion_fence_event_callback_test.h"

#include <cstdlib>

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "permission_manager.h"

#include "fusion_fence_event_callback.h"
#include "fusion_fence_ability.h"
#include "common_utils.h"
#include "constant_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 6;
const std::string MANAGER_SETTINGS = "ohos.permission.MANAGE_SETTINGS";

void FusionFenceEventCallbackTest::SetUp()
{
    MockNativePermission();
}

void FusionFenceEventCallbackTest::TearDown()
{}

void FusionFenceEventCallbackTest::MockNativePermission()
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
        .processName = "FusionFenceEventCallbackTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackConstructor001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackConstructor001, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackConstructor001 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackConstructor001 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange001, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange001 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    FusionFenceTransition transition;
    transition.identifier = "test_identifier_001";
    transition.scene = FusionFenceScene::AIRPORT;
    transition.transitionEvent = GEOFENCE_TRANSITION_EVENT_ENTER;
    fusionFenceCallback->OnTransitionStatusChange(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange001 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange002, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange002 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    FusionFenceTransition transition;
    transition.identifier = "test_identifier_002";
    transition.scene = FusionFenceScene::TRAIN_STATION;
    transition.transitionEvent = GEOFENCE_TRANSITION_EVENT_EXIT;
    fusionFenceCallback->OnTransitionStatusChange(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange002 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange003, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange003 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    FusionFenceTransition transition;
    transition.identifier = "test_identifier_003";
    transition.scene = FusionFenceScene::SUBWAY;
    transition.transitionEvent = GEOFENCE_TRANSITION_EVENT_DWELL;
    fusionFenceCallback->OnTransitionStatusChange(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange003 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange004, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange004, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange004 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    FusionFenceTransition transition;
    transition.identifier = "test_identifier_004";
    transition.scene = FusionFenceScene::SHOP;
    transition.transitionEvent = GEOFENCE_TRANSITION_EVENT_LEAVING_GEOFENCE;
    fusionFenceCallback->OnTransitionStatusChange(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange004 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange005, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange005, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange005 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    FusionFenceTransition transition;
    transition.identifier = "";
    transition.scene = FusionFenceScene::AIRPORT;
    transition.transitionEvent = GEOFENCE_TRANSITION_EVENT_ENTER;
    fusionFenceCallback->OnTransitionStatusChange(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange005 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange006, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange006, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange006 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    FusionFenceTransition transition;
    transition.identifier = "test_identifier_006";
    transition.scene = FusionFenceScene::TRAIN_STATION;
    transition.transitionEvent = GEOFENCE_TRANSITION_EVENT_APPROACHING_GEOFENCE;
    fusionFenceCallback->OnTransitionStatusChange(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange006 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange007, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnTransitionStatusChange007, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange007 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    FusionFenceTransition transition;
    transition.identifier = "test_identifier_007";
    transition.scene = FusionFenceScene::SUBWAY;
    transition.transitionEvent = GEOFENCE_TRANSITION_EVENT_NEAR_WANDER;
    fusionFenceCallback->OnTransitionStatusChange(transition);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnTransitionStatusChange007 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest001, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest001 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(FusionFenceInterfaceCode::ON_TRANSITION_STATUS_CHANGE);
    fusionFenceCallback->OnRemoteRequest(code, data, reply, option);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest001 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest002, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest002 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"invalid_token");
    uint32_t code = static_cast<uint32_t>(FusionFenceInterfaceCode::ON_TRANSITION_STATUS_CHANGE);
    int ret = fusionFenceCallback->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(-1, ret);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest002 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest003, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest003 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(FusionFenceInterfaceCode::ON_TRANSITION_STATUS_CHANGE);
    fusionFenceCallback->OnRemoteRequest(code, data, reply, option);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest003 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackOnRemoteRequest004, TestSize.Level1";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest004 begin");
    sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
    EXPECT_NE(nullptr, fusionFenceCallback);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t unknownCode = 9999;
    fusionFenceCallback->OnRemoteRequest(unknownCode, data, reply, option);
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackOnRemoteRequest004 end");
}

HWTEST_F(FusionFenceEventCallbackTest, FusionFenceEventCallbackDestructor001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "FusionFenceEventCallbackTest, FusionFenceEventCallbackDestructor001, TestSize.Level0";
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackDestructor001 begin");
    {
        sptr<FusionFenceEventCallback> fusionFenceCallback = new (std::nothrow) FusionFenceEventCallback();
        EXPECT_NE(nullptr, fusionFenceCallback);
    }
    LBSLOGI(FUSION_FENCE, "[FusionFenceEventCallbackTest] FusionFenceEventCallbackDestructor001 end");
}
}  // namespace Location
}  // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT