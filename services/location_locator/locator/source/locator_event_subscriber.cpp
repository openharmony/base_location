/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "locator_event_subscriber.h"

#include "hook_utils.h"
#include "location_log.h"
#include "locator_ability.h"
#include "location_config_manager.h"
#include "beacon_fence_manager.h"

namespace OHOS {
namespace Location {
constexpr const char* LOCATOR_STANDBY_NAP = "napped";
constexpr const char* LOCATOR_STANDBY_SLEEPING = "sleeping";
const uint32_t SIM_STATE_READY = 4; // Telephony::SimState::SIM_STATE_READY

LocatorEventSubscriber::LocatorEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info) {}

LocatorEventSubscriber::~LocatorEventSubscriber() {}

void LocatorEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    std::string action = event.GetWant().GetAction();
    LBSLOGI(LOCATOR_EVENT, "received action = %{public}s", action.c_str());
    if (OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED.compare(action) == 0) {
        auto locatorAbility = LocatorAbility::GetInstance();
        const bool napped = event.GetWant().GetBoolParam(LOCATOR_STANDBY_NAP, 0);
        const bool sleeping = event.GetWant().GetBoolParam(LOCATOR_STANDBY_SLEEPING, 0);
        LBSLOGD(LOCATOR_EVENT, "device idle napped: %{public}d, sleeping: %{public}d", napped, sleeping);
        locatorAbility->SyncIdleState(sleeping);
        return;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    if (std::string(MODE_CHANGED_EVENT).compare(action) == 0) {
        locatorAbility->UpdateSaAbility();
    } else if (std::string(LOCATION_PRIVACY_ACCEPT_EVENT).compare(action) == 0) {
        LocationConfigManager::GetInstance()->SetPrivacyTypeState(PRIVACY_TYPE_STARTUP, true);
        locatorAbility->SetSwitchState(true);
    } else if (std::string(LOCATION_PRIVACY_REJECT_EVENT).compare(action) == 0) {
        locatorAbility->SetSwitchState(false);
    } else if (OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED.compare(action) == 0) {
        int stateCode = event.GetCode();
        if (stateCode == static_cast<int>(SIM_STATE_READY)) {
            HookUtils::ExecuteHookWhenSimStateChange(action);
        }
    } else if (std::string(LOCATION_CUST_CONFIG_POLICY_CHANGE).compare(action) == 0) {
        HookUtils::ExecuteHookWhenCustConfigPolicyChange();
    } else if (std::string(PACKAGE_REMOVED_EVENT).compare(action) == 0) {
        std::string packageName = event.GetWant().GetStringParam(AppExecFwk::Constants::BUNDLE_NAME);
        BeaconFenceManager::GetInstance()->RemoveBeaconFenceByPackageName(packageName);
    }
}
} // namespace Location
} // namespace OHOS
