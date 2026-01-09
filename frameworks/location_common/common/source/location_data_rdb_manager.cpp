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
#include <nlohmann/json.hpp>
#include "hook_utils.h"

namespace OHOS {
namespace Location {
const int DEFAULT_USERID = 100;
const int MAX_SIZE = 100;

const std::string LOCATION_ENHANCE_STATUS = "location_enhance_status";
std::mutex LocationDataRdbManager::locationSwitchModeMutex_;
std::mutex LocationDataRdbManager::locationWorkingStateMutex_;
std::mutex LocationDataRdbManager::gnssSessionStateMutex_;
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


int LocationDataRdbManager::QuerySwitchStateForUser(int32_t userId)
{
    if (userId == 0) {
        CommonUtils::GetCurrentUserId(userId);
    }
    int res = LocationDataRdbManager::GetSwitchStateFromSysparaForUser(userId);
    if (res == DISABLED || res == ENABLED) {
        return res;
    }
    int32_t state = DEFAULT_SWITCH_STATE;
    Uri locationDataEnableUri(GetLocationDataUriForUser("location_enable", userId));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: query state failed, errcode = %{public}d", __func__, errCode);
        return DEFAULT_SWITCH_STATE;
    }
    if (res == DEFAULT_SWITCH_STATE && state != DEFAULT_SWITCH_STATE) {
        LocationDataRdbManager::SetSwitchStateToSysparaForUser(state, userId);
    }
    return state;
}


int LocationDataRdbManager::QuerySwitchStateWithUid(int32_t uid)
{
    int userId = CommonUtils::GetUserIdByUid(uid);
    int res = LocationDataRdbManager::QuerySwitchStateForUser(userId);
    return res;
}


int LocationDataRdbManager::QuerySystemSwitchState()
{
    std::vector<int> activeIds;
    CommonUtils::GetActiveUserIds(activeIds);
    for (int activeId : activeIds) {
        int res = QuerySwitchStateForUser(activeId);
        if (res == ENABLED) {
            return ENABLED;
        }
    }
    return DISABLED;
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
    std::unique_lock<std::mutex> lock(locationWorkingStateMutex_);
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
    std::unique_lock<std::mutex> lock(locationWorkingStateMutex_);
    Uri locationWorkingStateUri(GetLocationDataUriByCurrentUserId(LOCATION_WORKING_STATE));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationWorkingStateUri, LOCATION_WORKING_STATE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not get value, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}

bool LocationDataRdbManager::SetGnssSessionState(int32_t state, std::string uri, std::string colName)
{
    std::unique_lock<std::mutex> lock(gnssSessionStateMutex_);
    Uri gnssSessionStateUri(uri);
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        SetValue(gnssSessionStateUri, colName, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not set value to db, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}

int LocationDataRdbManager::GetSwitchStateForUser(int userId)
{
    return GetSwitchStateFromSysparaForUser(userId);
}

int LocationDataRdbManager::GetSwitchStateFromSysparaForCurrentUser()
{
    int32_t userId = 0;
    if (!CommonUtils::GetCurrentUserId(userId)) {
        userId = DEFAULT_USERID;
    }
    return GetSwitchStateFromSysparaForUser(userId);
}

bool LocationDataRdbManager::SetSwitchStateToSysparaForCurrentUser(int value)
{
    int32_t userId = 0;
    if (!CommonUtils::GetCurrentUserId(userId)) {
        userId = DEFAULT_USERID;
    }
    return SetSwitchStateToSysparaForUser(value, userId);
}

int LocationDataRdbManager::GetSwitchStateFromSysparaForUser(int32_t userId)
{
    char result[MAX_SIZE] = {0};
    std::string value = "";
    std::unique_lock<std::mutex> lock(locationSwitchModeMutex_);
    auto res = GetParameter(LOCATION_SWITCH_MODE, "", result, MAX_SIZE);
    if (res < 0 || strlen(result) == 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d", __func__, res);
        return DEFAULT_SWITCH_STATE;
    }
    value = result;
    nlohmann::json switchInfo = nlohmann::json::parse(value, nullptr, false);
    if (switchInfo.is_discarded()) {
        LBSLOGE(COMMON_UTILS, "switchInfo parse failed");
        return DEFAULT_SWITCH_STATE;
    }
    if (!switchInfo.contains(std::to_string(userId))) {
        LBSLOGE(COMMON_UTILS, "userId switch %{public}d is not exist", userId);
        return DEFAULT_SWITCH_STATE;
    }
    auto jsonItem = switchInfo.at(std::to_string(userId));
    if (!jsonItem.is_number()) {
        LBSLOGE(COMMON_UTILS, "switch state is invalid");
        return DEFAULT_SWITCH_STATE;
    }
    auto state = jsonItem.get<int>();
    return state;
}

bool LocationDataRdbManager::SetSwitchStateToSysparaForUser(int value, int32_t userId)
{
    char result[MAX_SIZE] = {0};
    std::unique_lock<std::mutex> lock(locationSwitchModeMutex_);
    nlohmann::json oldSwitchInfo;
    auto res = GetParameter(LOCATION_SWITCH_MODE, "", result, MAX_SIZE);
    if (res < 0 || strlen(result) == 0) {
        // If there is no value in sysparam, go on and write it.
        LBSLOGI(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d", __func__, res);
    } else {
        std::string SwitchStr = result;
        oldSwitchInfo = nlohmann::json::parse(SwitchStr, nullptr, false);
        if (oldSwitchInfo.is_discarded()) {
            LBSLOGI(COMMON_UTILS, "switchInfo parse failed");
            // If there is no valid value in sysparam, go on and overwrite it.
        }
    }
    nlohmann::json newSwitchInfo;
    std::vector<int> activeIds;
    // copy oldSwitchInfo to newSwitchInfo
    if (CommonUtils::GetAllUserId(activeIds)) {
        for (auto && [key, state] : oldSwitchInfo.items()) {
            if (IsUserIdInActiveIds(activeIds, key)) {
                newSwitchInfo[key] = state;
            }
        }
    }
    newSwitchInfo[std::to_string(userId)] = value;
    std::string newSwitchStr = newSwitchInfo.dump();
    char valueArray[MAX_SIZE] = {0};
    auto ret = sprintf_s(valueArray, sizeof(valueArray), "%s", newSwitchStr.c_str());
    if (ret <= 0) {
        LBSLOGE(COMMON_UTILS, "sprintf_s failed, ret: %{public}d", ret);
        return false;
    }
    res = SetParameter(LOCATION_SWITCH_MODE, valueArray);
    if (res < 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s failed, res: %{public}d", __func__, res);
        return false;
    }
    return true;
}

int LocationDataRdbManager::QueryAndSyncSwitchStatusWithUserId(int userId)
{
    if (userId == 0) {
        CommonUtils::GetCurrentUserId(userId);
    }
    int dbState = DEFAULT_SWITCH_STATE;
    Uri locationDataEnableUri(GetLocationDataUriForUser("location_enable", userId));
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, dbState);
    if (errCode != ERRCODE_SUCCESS) {
        // It needs to be updated when it is the default, and there is no need to return.
        LBSLOGE(COMMON_UTILS, "%{public}s: query state failed, errcode = %{public}d", __func__, errCode);
    }
    int sysparaState = LocationDataRdbManager::GetSwitchStateFromSysparaForUser(userId);
    if (sysparaState == DEFAULT_SWITCH_STATE && dbState != DEFAULT_SWITCH_STATE) {
        LocationDataRdbManager::SetSwitchStateToSysparaForUser(dbState, userId);
    } else if (sysparaState != DEFAULT_SWITCH_STATE && dbState != sysparaState) {
        LocationDataRdbManager::SetSwitchStateToDb(sysparaState);
    }
    HookUtils::ExecuteHookWhenSyncSwitchStates(sysparaState);
    return sysparaState;
}

void LocationDataRdbManager::SyncSwitchStatus()
{
    std::vector<int> activeIds;
    CommonUtils::GetActiveUserIds(activeIds);
    int sysparaState = DISABLED;
    for (int activeId : activeIds) {
        if (QueryAndSyncSwitchStatusWithUserId(activeId) == ENABLED) {
            sysparaState = ENABLED;
        }
    }
    HookUtils::ExecuteHookWhenSyncSwitchStates(sysparaState);
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

bool LocationDataRdbManager::IsUserIdInActiveIds(std::vector<int> activeIds, std::string userId)
{
    for (auto id : activeIds) {
        if (std::to_string(id).compare(userId) == 0) {
            return true;
        }
    }
    return false;
}

bool LocationDataRdbManager::GetIntelligentStatus(
    std::string createHelperUri, std::string queryUri, std::string colName, std::string& state)
{
    Uri locationIntelligentUri(queryUri);
    LocationErrCode errCode = LocationDataRdbHelper::GetInstance()->
        GetIntelligentValue(locationIntelligentUri, createHelperUri, colName, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS,
            "can not get value, errcode = %{public}d", errCode);
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS