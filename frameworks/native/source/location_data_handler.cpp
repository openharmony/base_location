/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "location_data_handler.h"

#include "uri.h"

#include "common_utils.h"
#include "location_data_rdb_helper.h"
#include "location_data_manager.h"

namespace OHOS {
namespace Location {
const uint32_t SWITCH_STATE_CHANGED = 1;

LocationDataHandler::LocationDataHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : EventHandler(runner) {}

LocationDataHandler::~LocationDataHandler() {}

void LocationDataHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case SWITCH_STATE_CHANGED: {
            HandleSwitchStateChanged(event);
            break;
        }
        default: {
            break;
        }
    }
}

void LocationDataHandler::HandleSwitchStateChanged(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    int32_t state = DISABLED;
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    LocationDataRdbHelper::GetInstance().GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    int64_t value = event->GetParam();
    if (state != value) {
        LocationDataRdbHelper::GetInstance().SetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    }
    LocationDataManager::GetInstance().ReportSwitchState(state);
}
} // namespace Location
} // namespace OHOS