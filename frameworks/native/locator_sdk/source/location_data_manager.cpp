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
#include "if_system_ability_manager.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "location_data_rdb_observer.h"
#include "location_data_rdb_manager.h"

namespace OHOS {
namespace Location {
LocationDataManager* LocationDataManager::GetInstance()
{
    static LocationDataManager data;
    return &data;
}

LocationDataManager::LocationDataManager()
{
    switchCallbacks_ = std::make_unique<std::map<pid_t, sptr<ISwitchCallback>>>();
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
        return;
    }
    if (saStatusListener_ == nullptr) {
        saStatusListener_ = sptr<DataShareSystemAbilityListener>(new DataShareSystemAbilityListener());
    }
    int32_t result = sam->SubscribeSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, saStatusListener_);
    if (result != ERR_OK) {
        LBSLOGE(LOCATOR, "%{public}s SubcribeSystemAbility result is %{public}d!", __func__, result);
    }
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

void LocationDataManager::ResetIsObserverReg()
{
    isObserverReg_ = false;
}

void LocationDataManager::RegisterDatashareObserver()
{
    auto locationDataRdbHelper = DelayedSingleton<LocationDataRdbHelper>::GetInstance();
    auto dataRdbObserver = sptr<LocationDataRdbObserver>(new (std::nothrow) LocationDataRdbObserver());
    if (locationDataRdbHelper == nullptr || dataRdbObserver == nullptr) {
        return;
    }
    if (!isObserverReg_) {
        Uri locationDataEnableUri(LOCATION_DATA_URI);
        locationDataRdbHelper->RegisterDataObserver(locationDataEnableUri, dataRdbObserver);
        isObserverReg_ = true;
    }
}

void DataShareSystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s enter", __func__);
    auto locationDataManager = DelayedSingleton<LocationDataManager>::GetInstance();
    if (locationDataManager == nullptr) {
        return;
    }
    locationDataManager->RegisterDatashareObserver();
}

void DataShareSystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s enter", __func__);
    auto locationDataManager = DelayedSingleton<LocationDataManager>::GetInstance();
    if (locationDataManager == nullptr) {
        return;
    }
    locationDataManager->ResetIsObserverReg();
}
}  // namespace Location
}  // namespace OHOS
