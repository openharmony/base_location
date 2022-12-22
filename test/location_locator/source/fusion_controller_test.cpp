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

#include "fusion_controller_test.h"

#include <singleton.h>

#include "common_utils.h"
#include "constant_definition.h"
#include "fusion_controller.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int UNKNOWN_TYPE = 0;
void FusionControllerTest::SetUp()
{
    fusionController_ = DelayedSingleton<FusionController>::GetInstance();
    EXPECT_NE(nullptr, fusionController_);
}

void FusionControllerTest::TearDown()
{
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies001 begin");
    fusionController_->ActiveFusionStrategies(SCENE_NAVIGATION);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies001 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies002 begin");
    fusionController_->ActiveFusionStrategies(SCENE_TRAJECTORY_TRACKING);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies002 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies003, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies003 begin");
    fusionController_->ActiveFusionStrategies(PRIORITY_FAST_FIRST_FIX);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies003 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies004, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies004 begin");
    fusionController_->ActiveFusionStrategies(UNKNOWN_TYPE);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies004 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies005, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies005 begin");
    fusionController_->ActiveFusionStrategies(SCENE_NAVIGATION); // when needReset is true
    fusionController_->ActiveFusionStrategies(SCENE_NAVIGATION); // when needReset is false
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies005 end");
}

HWTEST_F(FusionControllerTest, Process001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, Process001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process001 begin");
    fusionController_->Process(GNSS_ABILITY);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process001 end");
}

HWTEST_F(FusionControllerTest, Process002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, Process002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process002 begin");
    fusionController_->Process(PASSIVE_ABILITY); // is not gnss ability
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process002 end");
}

HWTEST_F(FusionControllerTest, FuseResult001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, FuseResult001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult001 begin");
    auto location = std::make_unique<Location>();
    fusionController_->FuseResult(GNSS_ABILITY, location);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult001 end");
}

HWTEST_F(FusionControllerTest, FuseResult002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, FuseResult002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult002 begin");
    auto location = std::make_unique<Location>();
    fusionController_->FuseResult(NETWORK_ABILITY, location); // is not gnss ability
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult002 end");
}
} // namespace Location
} // namespace OHOS