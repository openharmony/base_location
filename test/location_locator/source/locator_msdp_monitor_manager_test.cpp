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

#include "string_ex.h"

#include "locator_msdp_monitor_manager_test.h"


using namespace testing::ext;
namespace OHOS {
namespace Location {

void LocatorMsdpMonitorManagerTest::SetUp()
{
}

void LocatorMsdpMonitorManagerTest::TearDown()
{
}

HWTEST_F(LocatorMsdpMonitorManagerTest, RegisterMovementCallBack001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, RegisterMovementCallBack001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] RegisterMovementCallBack001 begin");
    auto locatorMsdpMonitorManager = LocatorMsdpMonitorManager::GetInstance();
    locatorMsdpMonitorManager->UnRegisterMovementCallBack();
   
    LBSLOGI(LOCATOR, "[WorkRecordTest] RegisterMovementCallBack001 end");
}

HWTEST_F(LocatorMsdpMonitorManagerTest, UpdateStillMovementState001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, UpdateStillMovementState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] UpdateStillMovementState001 begin");
    auto locatorMsdpMonitorManager = LocatorMsdpMonitorManager::GetInstance();
    locatorMsdpMonitorManager->UpdateStillMovementState(true);
   
    LBSLOGI(LOCATOR, "[WorkRecordTest] UpdateStillMovementState001 end");
}

HWTEST_F(LocatorMsdpMonitorManagerTest, GetStillMovementState001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, GetStillMovementState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] GetStillMovementState001 begin");
    auto MsdpMotionServiceStatusChange = LocatorMsdpMonitorManager::GetInstance();
    MsdpMotionServiceStatusChange->GetStillMovementState();
   
    LBSLOGI(LOCATOR, "[WorkRecordTest] GetStillMovementState001 end");
}

HWTEST_F(LocatorMsdpMonitorManagerTest, OnRemoveSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "WorkRecordTest, OnRemoveSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[WorkRecordTest] OnRemoveSystemAbility001 begin");
    auto locatorMsdpMonitorManager = LocatorMsdpMonitorManager::GetInstance();
    locatorMsdpMonitorManager->OnRemoveSystemAbility();
   
    LBSLOGI(LOCATOR, "[WorkRecordTest] OnRemoveSystemAbility001 end");
}
} // namespace Location
} // namespace OHOS