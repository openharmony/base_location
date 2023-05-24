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
#include "common_utils.h"

namespace OHOS {
namespace Location {
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

LocationErrCode LocationDataManager::QuerySwitchState(bool &isEnabled)
{
    int32_t state = DISABLED;
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    LocationErrCode errCode = DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR, "%{public}s: can not query state, reset state.", __func__);
        errCode = DelayedSingleton<LocationDataRdbHelper>::GetInstance()->
            SetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    }
    isEnabled = (state == ENABLED);
    return errCode;
}
}  // namespace Location
}  // namespace OHOS
