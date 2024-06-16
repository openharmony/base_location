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

#ifndef LOCATOR_MSDP_MONITOR_MANAGER_H
#define LOCATOR_MSDP_MONITOR_MANAGER_H
#ifdef MOVEMENT_CLIENT_ENABLE
#include "locator_msdp_state_change_cb.h"
#include "movement_client.h"
#include "iservice_registry.h"
#include "location_log.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"
#include "locator_msdp_state_change_cb.h"

namespace OHOS {
namespace Location {
class LocatorMsdpMonitorManager {
public:
    static LocatorMsdpMonitorManager* GetInstance();
    LocatorMsdpMonitorManager();
    ~LocatorMsdpMonitorManager();
    void RegisterMovementCallBack();
    void UnRegisterMovementCallBack();
    void UpdateStillMovementState(bool stillState);
    bool GetStillMovementState();

    std::atomic_bool isDeviceStillState_;

private:
    void Init();
    sptr<DeviceMovementCallback> deviceMovementCallback_;
    std::mutex deviceMovementEventMutex;
    sptr<ISystemAbilityStatusChange> saStatusListener_;
};

class MsdpMotionServiceStatusChange : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};
}
}
#endif // MOVEMENT_CLIENT_ENABLE
#endif // LOCATOR_MSDP_MONITOR_MANAGER_H