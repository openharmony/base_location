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
const int MAX_SIZE = 100;
std::mutex LocationDataRdbManager::mutex_;
const std::string LOCATION_ENHANCE_STATUS = "location_enhance_status";

std::string LocationDataRdbManager::GetLocationDataUriByCurrentUserId(std::string key)
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

std::string LocationDataRdbManager::GetLocationDataUriForUser(std::string key, int32_t userId)
{
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
    int32_t state = DEFAULT_SWITCH_STATE;
    int res = LocationDataRdbManager::GetSwitchStateFromSyspara();
    if (res == DISABLED || res == ENABLED) {
        return res;
    }
    Uri locationDataEnableUri(GetLocationDataUriByCurrentUserId("location_enable"));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: query state failed, errcode = %{public}d", __func__, errCode);
        return DEFAULT_SWITCH_STATE;
    }
    if (res == DEFAULT_SWITCH_STATE && state != DEFAULT_SWITCH_STATE) {
        LocationDataRdbManager::SetSwitchStateToSyspara(state);
    }
    return state;
}

LocationErrCode LocationDataRdbManager::SetSwitchStateToDb(int modeValue)
{
    Uri locationDataEnableUri(GetLocationDataUriByCurrentUserId("location_enable"));
    return LocationDataRdbHelper::GetInstance()->
        SetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, modeValue);
}

LocationErrCode LocationDataRdbManager::SetSwitchStateToDbForUser(int modeValue, int32_t userId)
{
    Uri locationDataEnableUri(GetLocationDataUriForUser("location_enable", userId));
    return LocationDataRdbHelper::GetInstance()->
        SetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, modeValue);
}

LocationErrCode LocationDataRdbManager::GetSwitchStateFromDbForUser(int32_t& state, int32_t userId)
{
    Uri locationDataEnableUri(GetLocationDataUriForUser("location_enable", userId));
    return LocationDataRdbHelper::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
}

bool LocationDataRdbManager::SetLocationWorkingState(int32_t state)
{
    Uri locationWorkingStateUri(GetLocationDataUriByCurrentUserId(LOCATION_WORKING_STATE));
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
    Uri locationWorkingStateUri(GetLocationDataUriByCurrentUserId(LOCATION_WORKING_STATE));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationWorkingStateUri, LOCATION_WORKING_STATE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not get value, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}

int LocationDataRdbManager::GetSwitchStateFromSyspara()
{
    char result[MAX_SIZE] = {0};
    std::string value = "";
    std::unique_lock<std::mutex> lock(mutex_);
    auto res = GetParameter(LOCATION_SWITCH_MODE, "", result, MAX_SIZE);
    if (res < 0 || strlen(result) == 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d", __func__, res);
        return DEFAULT_SWITCH_STATE;
    }
    int userId = 0;
    if (!CommonUtils::GetCurrentUserId(userId)) {
        userId = DEFAULT_USERID;
    }
    value = result;
    auto switchVec = CommonUtils::split(value, ",");
    for (auto item : switchVec) {
        auto switchStateForUser = CommonUtils::split(item, "_")
        if (switchStateForUser.size() > 1 && switchStateForUser[0] == std::to_string(userId) &&
            std::isdigit(switchStateForUser[1][0]) == 0) {
            return std::stoi(switchStateForUser[1]);
        }
    }
    return DEFAULT_SWITCH_STATE;
}

bool LocationDataRdbManager::SetSwitchStateToSyspara(int value)
{
    char valueArray[MAX_SIZE] = {0};
    char result[MAX_SIZE] = {0};
    std::unique_lock<std::mutex> lock(mutex_);
    auto res = GetParameter(LOCATION_SWITCH_MODE, "", result, MAX_SIZE);
    if (res < 0 || strlen(result) == 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d", __func__, res);
    }
    std::string oldSwitchStr = "";
    std::string newSwitchStr = "";
    oldSwitchStr = result;
    int userId = 0;
    if (!CommonUtils::GetCurrentUserId(userId)) {
        userId = DEFAULT_USERID;
    }
    auto switchVec = CommonUtils::split(value, ",");
    bool isFind = false;
    if (oldSwitchStr.size() > 1) {
        for (auto item : switchVec) {
            auto switchStateForUser = CommonUtils::split(item, "_")
            if (switchStateForUser.size() > 1 && switchStateForUser[0] == std::to_string(userId) &&
                std::isdigit(switchStateForUser[1][0]) == 0) {
                isFind = true;
                newSwitchStr += std::to_string(userId) + "_" + std::to_string(value) + ",";
            } else if (item.size() > 0) {
                newSwitchStr += item + ",";
            } 
        }
    } 
    if (!isFind) {
        newSwitchStr += std::to_string(userId) + "_" + std::to_string(value);
    }
    
    auto ret = sprintf_s(valueArray, sizeof(valueArray), "%s", newSwitchStr.c_str());
    if (ret < 0) {
        LBSLOGE(COMMON_UTILS, "sprintf_s failed, ret: %{public}d", ret);
    }
    
    res = SetParameter(LOCATION_SWITCH_MODE, valueArray);
    if (res < 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s failed, res: %{public}d", __func__, res);
        return false;
    }
    return true;
}

int LocationDataRdbManager::GetSwitchStateFromSysparaForUser(int32_t userId)
{
    char result[MAX_SIZE] = {0};
    std::string value = "";
    std::unique_lock<std::mutex> lock(mutex_);
    auto res = GetParameter(LOCATION_SWITCH_MODE, "", result, MAX_SIZE);
    if (res < 0 || strlen(result) == 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d", __func__, res);
        return DEFAULT_SWITCH_STATE;
    }
    value = result;
    auto switchVec = CommonUtils::split(value, ",");
    for (auto item : switchVec) {
        auto switchStateForUser = CommonUtils::split(item, "_")
        if (switchStateForUser.size() > 1 && switchStateForUser[0] == std::to_string(userId) &&
            std::isdigit(switchStateForUser[1][0]) == 0) {
            return std::stoi(switchStateForUser[1]);
        }
    }
    return DEFAULT_SWITCH_STATE;
}

bool LocationDataRdbManager::SetSwitchStateToSysparaForUser(int value, int32_t userId)
{
    char valueArray[MAX_SIZE] = {0};
    char result[MAX_SIZE] = {0};
    std::unique_lock<std::mutex> lock(mutex_);
    auto res = GetParameter(LOCATION_SWITCH_MODE, "", result, MAX_SIZE);
    if (res < 0 || strlen(result) == 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d", __func__, res);
    }
    std::string oldSwitchStr = "";
    std::string newSwitchStr = "";
    oldSwitchStr = result;
    auto switchVec = CommonUtils::split(value, ",");
    bool isFind = false;
    int currentUserId = 0;
    if (oldSwitchStr.size() <= 1) {
        if (!CommonUtils::GetCurrentUserId(currentUserId)) {
            currentUserId = DEFAULT_USERID;
        }
        newSwitchStr += std::to_string(currentUserId) + "_" + std::to_string(value) + ",";
    } else {
        for (auto item : switchVec) {
            auto switchStateForUser = CommonUtils::split(item, "_")
            if (switchStateForUser.size() > 1 && switchStateForUser[0] == std::to_string(userId) &&
                std::isdigit(switchStateForUser[1][0]) == 0) {
                isFind = true;
                newSwitchStr += std::to_string(userId) + "_" + std::to_string(value) + ",";
            } else if (item.size() > 0) {
                newSwitchStr += item + ",";
            } 
        }
    }
    if (!isFind && (currentUserId != userId)) {
        newSwitchStr += std::to_string(userId) + "_" + std::to_string(value);
    }
    
    auto ret = sprintf_s(valueArray, sizeof(valueArray), "%s", newSwitchStr.c_str());
    if (ret < 0) {
        LBSLOGE(COMMON_UTILS, "sprintf_s failed, ret: %{public}d", ret);
    }
    
    res = SetParameter(LOCATION_SWITCH_MODE, valueArray);
    if (res < 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s failed, res: %{public}d", __func__, res);
        return false;
    }
    return true;
}

void LocationDataRdbManager::SyncSwitchStatus()
{
    int dbState = DEFAULT_SWITCH_STATE;
    Uri locationDataEnableUri(GetLocationDataUriByCurrentUserId("location_enable"));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, dbState);
    if (errCode != ERRCODE_SUCCESS) {
        // It needs to be updated when it is the default, and there is no need to return.
        LBSLOGE(COMMON_UTILS, "%{public}s: query state failed, errcode = %{public}d", __func__, errCode);
    }
    int sysparaState = LocationDataRdbManager::GetSwitchStateFromSyspara();
    if (sysparaState == DEFAULT_SWITCH_STATE && dbState != DEFAULT_SWITCH_STATE) {
        LocationDataRdbManager::SetSwitchStateToSyspara(dbState);
    } else if (sysparaState != DEFAULT_SWITCH_STATE && dbState != sysparaState) {
        LocationDataRdbManager::SetSwitchStateToDb(sysparaState);
    }
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