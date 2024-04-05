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

#include "location_data_manager.h"

#include "uri.h"

#include "switch_callback_proxy.h"
#include "constant_definition.h"
#include "location_data_rdb_helper.h"
#include "location_log.h"
#include "common_hisysevent.h"

namespace OHOS {
namespace Location {
const std::string LOCATION_DATA_ABILITY_PREFIX = "datashare://";
const std::string LOCATION_DATA_URI_ID =
    "/com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=location_enable";
const std::string LOCATION_DATA_URI = LOCATION_DATA_ABILITY_PREFIX + LOCATION_DATA_URI_ID;
const std::string LOCATION_DATA_COLUMN_KEYWORD = "KEYWORD";
const std::string LOCATION_DATA_COLUMN_VALUE = "VALUE";
const std::string LOCATION_DATA_COLUMN_ENABLE = "location_switch_enable";
const std::string LOCATION_WORKING_STATE = "location_working_state";

LocationDataManager::LocationDataManager()
{
    switchCallbacks_ = std::make_unique<std::map<pid_t, sptr<ISwitchCallback>>>();
}

LocationDataManager::~LocationDataManager()
{
}

LocationErrCode LocationDataManager::ReportSwitchState(bool isEnabled)
{
    if (switchCallbacks_ == nullptr) {
        LBSLOGE(LOCATOR, "switchCallbacks_ is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    int state = isEnabled ? ENABLED : DISABLED;

    std::unique_lock<std::mutex> lock(mutex_);
    for (auto iter = switchCallbacks_->begin(); iter != switchCallbacks_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        auto callback = std::make_unique<SwitchCallbackProxy>(remoteObject);
        callback->OnSwitchChange(state);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataManager::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr || switchCallbacks_ == nullptr) {
        LBSLOGE(LOCATOR, "register an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    switchCallbacks_->erase(uid);
    switchCallbacks_->insert(std::make_pair(uid, switchCallback));
    LBSLOGD(LOCATOR, "after uid:%{public}d register, switch callback size:%{public}s",
        uid, std::to_string(switchCallbacks_->size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataManager::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr || switchCallbacks_ == nullptr) {
        LBSLOGE(LOCATOR, "unregister an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }

    pid_t uid = -1;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto iter = switchCallbacks_->begin(); iter != switchCallbacks_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        if (remoteObject == callback) {
            uid = iter->first;
            break;
        }
    }
    switchCallbacks_->erase(uid);
    LBSLOGD(LOCATOR, "after uid:%{public}d unregister, switch callback size:%{public}s",
        uid, std::to_string(switchCallbacks_->size()).c_str());
    return ERRCODE_SUCCESS;
}

void LocationDataManager::SetCachedSwitchState(int state)
{
    std::unique_lock<std::mutex> lock(switchStateMutex_);
    isStateCached_ = true;
    cachedSwitchState_ = state;
}

bool LocationDataManager::IsSwitchStateReg()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return (switchCallbacks_->size() > 0);
}

bool LocationDataManager::IsSwitchStateReg()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return (switchCallbacks_->size() > 0);
}

int LocationDataManager::QuerySwitchState()
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

std::string LocationDataManager::GetLocationDataUri(std::string key)
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

bool LocationDataManager::SetLocationWorkingState(int32_t state)
{
    std::unique_lock<std::mutex> lock(locationWorkingState_);
    Uri locationWorkingStateUri(LocationDataManager::GetLocationDataUri(LOCATION_WORKING_STATE));
    LocationErrCode errCode = DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
        SetValue(locationWorkingStateUri, LOCATION_WORKING_STATE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not set value to db, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}

bool LocationDataManager::GetLocationWorkingState(int32_t& state)
{
    std::unique_lock<std::mutex> lock(locationWorkingState_);
    Uri locationWorkingStateUri(LocationDataManager::GetLocationDataUri(LOCATION_WORKING_STATE));
    LocationErrCode errCode = DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
        GetValue(locationWorkingStateUri, LOCATION_WORKING_STATE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(COMMON_UTILS, "%{public}s: can not get value, errcode = %{public}d", __func__, errCode);
        return false;
    }
    return true;
}
}  // namespace Location
}  // namespace OHOS
