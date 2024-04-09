/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "location_data_rdb_manager.h"

#include "common_utils.h"
#include "location_data_rdb_helper.h"


namespace OHOS {
namespace Location {
const int DEFAULT_USERID = 100;

std::string LocationDataRdbManager::GetLocationDataUri(std::string key)
{
    int userId = 0;
    if (!CommonUtils::GetCurrentUserId(userId)) {
        userId = DEFAULT_USERID;
    }
    std::string uri = "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_" +
        std::to_string(userId) +
        "?Proxy=true&key=" + key;
    return uri;
}

int LocationDataRdbManager::QuerySwitchState()
{
    int32_t state = DISABLED;
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    LocationErrCode errCode = DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: query state failed, errcode = %{public}d", __func__, errCode);
    }
    return state;
}

LocationErrCode LocationDataRdbManager::SetSwitchState(int modeValue)
{
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    return DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
        SetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, modeValue);
}

bool LocationDataRdbManager::SetLocationWorkingState(int32_t state)
{
    Uri locationWorkingStateUri(GetLocationDataUri(LOCATION_WORKING_STATE));
    LocationErrCode errCode = DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
        SetValue(locationWorkingStateUri, LOCATION_WORKING_STATE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not set value to db, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}

bool LocationDataRdbManager::GetLocationWorkingState(int32_t& state)
{
    Uri locationWorkingStateUri(GetLocationDataUri(LOCATION_WORKING_STATE));
    LocationErrCode errCode = DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
        GetValue(locationWorkingStateUri, LOCATION_WORKING_STATE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not get value, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS