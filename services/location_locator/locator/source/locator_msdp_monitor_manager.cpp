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
#include "locator_msdp_monitor_manager.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
LocatorMsdpMonitorManager::LocatorMsdpMonitorManager()
{
    Init();
}

LocatorMsdpMonitorManager* LocatorMsdpMonitorManager::GetInstance()
{
    static LocatorMsdpMonitorManager data;
    return &data;
}

LocatorMsdpMonitorManager::~LocatorMsdpMonitorManager()
{
    saStatusListener_ = nullptr;
}

void LocatorMsdpMonitorManager::Init()
{
    int32_t result;
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s samgrProxy is nullptr!", __func__);
        return;
    }
    saStatusListener_ = sptr<MsdpMotionServiceStatusChange>(new MsdpMotionServiceStatusChange());
    if (saStatusListener_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s saStatusListener_ is nullptr!", __func__);
        return;
    }
    result = samgrProxy->SubscribeSystemAbility(static_cast<int32_t>(MSDP_MOTION_SERVICE_ID),
        saStatusListener_);
    LBSLOGI(LOCATOR, "%{public}s SubcribeSystemAbility result is %{public}d!", __func__, result);
}

void LocatorMsdpMonitorManager::RegisterMovementCallBack()
{
    LBSLOGI(LOCATOR, "RegisterMovementCallBack");
    std::unique_lock<std::mutex> lock(deviceMovementEventMutex);
    if (deviceMovementCallback_ == nullptr) {
        deviceMovementCallback_ = sptr<DeviceMovementCallback>(new DeviceMovementCallback());
    }
    if (Msdp::MovementClient::GetInstance().SubscribeCallback(
        Msdp::MovementDataUtils::MovementType::TYPE_STILL, deviceMovementCallback_) != ERR_OK) {
        LBSLOGI(LOCATOR, "Register a device movement observer failed!");
        deviceMovementCallback_ = nullptr;
    }
}
 
void LocatorMsdpMonitorManager::UnRegisterMovementCallBack()
{
    LBSLOGI(LOCATOR, "UnRegisterMovementCallBack");
    std::unique_lock<std::mutex> lock(deviceMovementEventMutex);
    if (deviceMovementCallback_ == nullptr) {
        return;
    }
    Msdp::MovementClient::GetInstance().UnSubscribeCallback(
        Msdp::MovementDataUtils::MovementType::TYPE_STILL, deviceMovementCallback_);
    deviceMovementCallback_ = nullptr;
}

void LocatorMsdpMonitorManager::UpdateStillMovementState(bool stillState)
{
    isDeviceStillState_.store(stillState);
    LBSLOGI(LOCATOR, "device movement state change, isDeviceStillState_ %{public}d",
        isDeviceStillState_.load());
    auto locatorAbility = LocatorAbility::GetInstance();
    locatorAbility->SyncStillMovementState(stillState);
}

bool LocatorMsdpMonitorManager::GetStillMovementState()
{
    return isDeviceStillState_.load();
}

void MsdpMotionServiceStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGI(LOCATOR, "MsdpMotionServiceStatusChange::OnAddSystemAbility");
    LocatorMsdpMonitorManager::GetInstance()->RegisterMovementCallBack();
}

void MsdpMotionServiceStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGI(LOCATOR, "MsdpMotionServiceStatusChange::OnRemoveSystemAbility");
    LocatorMsdpMonitorManager::GetInstance()->UnRegisterMovementCallBack();
}
} // namespace Location
} // namespace OHOS
#endif // MOVEMENT_CLIENT_ENABLE