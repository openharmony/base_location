/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "common_event_helper.h"

#include "common_event.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "want.h"
#include "location_log.h"

namespace OHOS {
namespace Location {

bool CommonEventHelper::PublishLocationModeChangeCommonEventAsUser(const int &modeValue, const int32_t &userId)
{
    AAFwk::Want want;
    want.SetAction(COMMON_EVENT_LOCATION_MODE_STATE_CHANGED);
    EventFwk::CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(modeValue);
    if (!EventFwk::CommonEventManager::PublishCommonEventAsUser(commonData, userId)) {
        LBSLOGE(COMMON_EVENT_HELPER, "failed to send common event as user!");
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS
