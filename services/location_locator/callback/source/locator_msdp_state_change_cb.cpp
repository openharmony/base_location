/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifdef MOVEMENT_CLIENT_ENABLE
#include "locator_msdp_state_change_cb.h"
#include "locator_msdp_monitor_manager.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
void DeviceMovementCallback::OnMovementChanged(const Msdp::MovementDataUtils::MovementData &movementData)
{
    LBSLOGI(LOCATOR, "OnMovementChanged type=%{public}d, value=%{public}d, time=%{public}s",
        movementData.type, movementData.value, std::to_string(CommonUtils::GetCurrentTimeMilSec()).c_str());
    if (movementData.type == Msdp::MovementDataUtils::MovementType::TYPE_STILL) {
        auto locatorMsdpMonitorManager = LocatorMsdpMonitorManager::GetInstance();
        if (movementData.value == Msdp::MovementDataUtils::MovementValue::VALUE_ENTER) {
            locatorMsdpMonitorManager->UpdateStillMovementState(true);
        } else {
            locatorMsdpMonitorManager->UpdateStillMovementState(false);
        }
    }
}
}
}
#endif // MOVEMENT_CLIENT_ENABLE