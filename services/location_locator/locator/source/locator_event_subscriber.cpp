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
#include "location_log.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
LocatorEventSubscriber::LocatorEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info) {}

LocatorEventSubscriber::~LocatorEventSubscriber() {}

void LocatorEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(LOCATOR_EVENT, "OnReceiveEvent: LocatorAbility is nullptr.");
        return;
    }
    std::string action = event.GetWant().GetAction();
    LBSLOGI(LOCATOR_EVENT, "received action = %{public}s", action.c_str());
    if (MODE_CHANGED_EVENT.compare(action) == 0) {
        locatorAbility.get()->UpdateSaAbility();
    }
}
} // namespace Location
} // namespace OHOS
