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

#include "location_data_rdb_observer.h"

#include <singleton.h>
#include "common_utils.h"
#include "location_data_rdb_helper.h"
#include "location_log.h"
#include "uri.h"
#include "common_utils.h"
#include "location_data_manager.h"
#include "location_data_rdb_manager.h"

namespace OHOS {
namespace Location {
using namespace AppExecFwk;

LocationDataRdbObserver::LocationDataRdbObserver()
{
}

LocationDataRdbObserver::~LocationDataRdbObserver() = default;

void LocationDataRdbObserver::OnChange()
{
    LBSLOGD(LOCATOR, "LocationDataRdbObserver::%{public}s enter", __func__);
    HandleSwitchStateChanged();
}

void LocationDataRdbObserver::HandleSwitchStateChanged()
{
    auto rdbHelper = LocationDataRdbHelper::GetInstance();
    auto locationDataManager = LocationDataManager::GetInstance();
    if (rdbHelper == nullptr || locationDataManager == nullptr ||
        !locationDataManager->IsSwitchStateReg()) {
        LBSLOGE(LOCATOR, "%{public}s: param is nullptr", __func__);
        return;
    }

    int32_t state = DEFAULT_STATE;
    state = LocationDataRdbManager::QuerySwitchState();
    locationDataManager->ReportSwitchState(state);
}
} // namespace Location
} // namespace OHOS