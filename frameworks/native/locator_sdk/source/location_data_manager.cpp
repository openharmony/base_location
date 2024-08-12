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
#include "location_data_rdb_manager.h"
#include "location_log.h"
#include "common_hisysevent.h"
#include "parameter.h"
namespace OHOS {
namespace Location {
LocationDataManager* LocationDataManager::GetInstance()
{
    static LocationDataManager data;
    return &data;
}

LocationDataManager::LocationDataManager()
{
}

LocationDataManager::~LocationDataManager()
{
}

LocationErrCode LocationDataManager::ReportSwitchState(bool isEnabled)
{
    int state = isEnabled ? ENABLED : DISABLED;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto switchCallback : switchCallbacks_) {
        sptr<IRemoteObject> remoteObject = switchCallback->AsObject();
        auto callback = std::make_unique<SwitchCallbackProxy>(remoteObject);
        callback->OnSwitchChange(state);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataManager::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "register an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto item : switchCallbacks_) {
        if (item && item->AsObject() == callback) {
            LBSLOGE(LOCATOR, "callback has registered");
            return ERRCODE_SUCCESS;
        }
    }
    switchCallbacks_.push_back(switchCallback);
    LBSLOGD(LOCATOR, "after uid:%{public}d register, switch callback size:%{public}s",
        uid, std::to_string(switchCallbacks_.size()).c_str());
    if (IsSwitchStateReg()) {
        RegisterLocationSwitchObserver();
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationDataManager::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "unregister an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    if (switchCallbacks_.size() <= 0) {
        LBSLOGE(COUNTRY_CODE, "switchCallbacks_ size <= 0");
        return ERRCODE_SUCCESS;
    }
    size_t i = 0;
    for (; i < switchCallbacks_.size(); i++) {
        if (switchCallbacks_[i] == nullptr) {
            continue;
        }
        sptr<IRemoteObject> remoteObject = switchCallbacks_[i]->AsObject();
        if (remoteObject == callback) {
            break;
        }
    }
    if (i >= switchCallbacks_.size()) {
        LBSLOGD(GNSS, "switch callback is not in vector");
        return ERRCODE_SUCCESS;
    }
    switchCallbacks_.erase(switchCallbacks_.begin() + i);
    LBSLOGD(LOCATOR, "after unregister, switch callback size:%{public}s",
        std::to_string(switchCallbacks_.size()).c_str());
    return ERRCODE_SUCCESS;
}

bool LocationDataManager::IsSwitchStateReg()
{
    std::unique_lock<std::mutex> lock(isSwitchObserverRegMutex_);
    return isSwitchObserverReg_;
}

void LocationDataManager::RegisterLocationSwitchObserver()
{
    auto eventCallback = [](const char *key, const char *value, void *context) {
        int32_t state = DEFAULT_SWITCH_STATE;
        state = LocationDataRdbManager::QuerySwitchState();
        auto manager = LocationDataManager::GetInstance();
        if (manager == nullptr) {
            LBSLOGE(LOCATOR, "SubscribeLocaleConfigEvent LocationDataRdbManager is nullptr");
            return;
        }
        if (state == DEFAULT_SWITCH_STATE) {
            LBSLOGE(COUNTRY_CODE, "LOCATION_SWITCH_MODE changed. state %{public}d. do not report", state);
            return;
        }
        bool switchState = (state == ENABLED);
        LBSLOGI(COUNTRY_CODE, "LOCATION_SWITCH_MODE changed. switchState %{public}d", switchState);
        manager->ReportSwitchState(switchState);
    };

    int ret = WatchParameter(LOCATION_SWITCH_MODE, eventCallback, nullptr);
    if (ret != SUCCESS) {
        LBSLOGE(LOCATOR, "WatchParameter fail");
        return;
    }
    std::unique_lock<std::mutex> lock(isSwitchObserverRegMutex_);
    isSwitchObserverReg_ = true;
    return;
}
}  // namespace Location
}  // namespace OHOS
