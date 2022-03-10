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

#include "ipc_skeleton.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "lbs_log.h"

namespace OHOS {
namespace Location {
LocatorEventSubscriber::LocatorEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info) {}

LocatorEventSubscriber::~LocatorEventSubscriber() {}

void LocatorEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    proxyLocator_ = std::make_unique<LocatorProxy>(CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID,
        CommonUtils::InitDeviceId()));

    std::string action = event.GetWant().GetAction();
    LBSLOGI(LOCATOR_EVENT, "received action = %{public}s", action.c_str());
    if (MODE_CHANGED_EVENT.compare(action) == 0 && proxyLocator_ != nullptr) {
        proxyLocator_->UpdateSaAbility();
    }
}
} // namespace Location
} // namespace OHOS
