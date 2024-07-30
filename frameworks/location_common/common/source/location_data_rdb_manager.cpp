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
#include "parameter.h"

namespace OHOS {
namespace Location {
const int DEFAULT_USERID = 100;
const int UNKNOW_ERROR = -1;
const int MAX_SIZE = 100;
const char* LOCATION_SWITCH_MODE = "persist.location.switch_mode";
const std::string LOCATION_ENHANCE_STATUS = "location_enhance_status";

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

std::string LocationDataRdbManager::GetLocationDataSecureUri(std::string key)
{
    int userId = 0;
    if (!CommonUtils::GetCurrentUserId(userId)) {
        userId = DEFAULT_USERID;
    }
    std::string uri = "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_SECURE_" +
        std::to_string(userId) +
        "?Proxy=true&key=" + key;
    return uri;
}

int LocationDataRdbManager::QuerySwitchState()
{
    int32_t state = DISABLED;
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: query state failed, errcode = %{public}d", __func__, errCode);
        return DEFAULT_STATE;
    }
    SetSwitchMode(state);
    return state;
}

LocationErrCode LocationDataRdbManager::SetSwitchState(int modeValue)
{
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    return LocationDataRdbHelper::GetInstance()->
        SetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, modeValue);
}

bool LocationDataRdbManager::SetLocationWorkingState(int32_t state)
{
    Uri locationWorkingStateUri(GetLocationDataUri(LOCATION_WORKING_STATE));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
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
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationWorkingStateUri, LOCATION_WORKING_STATE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not get value, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}

int LocationDataRdbManager::GetSwitchMode()
{
    char result[MAX_SIZE] = {0};
    std::string value = "";
    auto res = GetParameter(LOCATION_SWITCH_MODE, "", result, MAX_SIZE);
    if (res < 0 || strlen(result) == 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d", __func__, res);
        return UNKNOW_ERROR;
    }
    value = result;
    for (auto ch : value) {
        if (std::isdigit(ch) == 0) {
            LBSLOGE(COMMON_UTILS, "wrong para");
            return UNKNOW_ERROR;
        }
    }
    if (value.size() == 0) {
        return UNKNOW_ERROR;
    }
    return std::stoi(value);
}

bool LocationDataRdbManager::SetSwitchMode(int value)
{
    char valueArray[MAX_SIZE] = {0};
    (void)sprintf_s(valueArray, sizeof(valueArray), "%d", value);
    int res = SetParameter(LOCATION_SWITCH_MODE, valueArray);
    if (res < 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s failed, res: %{public}d", __func__, res);
        return false;
    }
    return true;
}

bool LocationDataRdbManager::SetLocationEnhanceStatus(int32_t state)
{
    Uri locationWorkingStateUri(GetLocationDataSecureUri(LOCATION_ENHANCE_STATUS));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        SetValue(locationWorkingStateUri, LOCATION_ENHANCE_STATUS, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS,
            "can not set value, key = %{public}s, errcode = %{public}d", LOCATION_ENHANCE_STATUS.c_str(), errCode);
        return false;
    }
    return true;
}

bool LocationDataRdbManager::GetLocationEnhanceStatus(int32_t& state)
{
    Uri locationWorkingStateUri(GetLocationDataSecureUri(LOCATION_ENHANCE_STATUS));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationWorkingStateUri, LOCATION_ENHANCE_STATUS, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS,
            "can not get value, key = %{public}s, errcode = %{public}d", LOCATION_ENHANCE_STATUS.c_str(), errCode);
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS