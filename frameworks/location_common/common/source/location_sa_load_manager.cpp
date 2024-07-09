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

#include "location_sa_load_manager.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "location_log.h"
#include "common_hisysevent.h"
#include "location_log_event_ids.h"

namespace OHOS {
namespace Location {
LocationSaLoadManager* LocationSaLoadManager::GetInstance()
{
    static LocationSaLoadManager data;
    return &data;
}

LocationSaLoadManager::LocationSaLoadManager()
{
}

LocationSaLoadManager::~LocationSaLoadManager()
{
}

LocationErrCode LocationSaLoadManager::LoadLocationSa(int32_t systemAbilityId)
{
    LBSLOGD(LOCATOR, "%{public}s enter, systemAbilityId = [%{public}d] loading", __func__, systemAbilityId);
    InitLoadState();
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: get system ability manager failed!", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }

    auto locationSaLoadCallback = sptr<LocationSaLoadCallback>(new LocationSaLoadCallback());
    int32_t ret = samgr->LoadSystemAbility(systemAbilityId, locationSaLoadCallback);
    if (ret != ERR_OK) {
        LBSLOGE(LOCATOR, "%{public}s: Failed to load system ability, SA Id = [%{public}d], ret = [%{public}d].",
            __func__, systemAbilityId, ret);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return WaitLoadStateChange(systemAbilityId);
}

void LocationSaLoadManager::InitLoadState()
{
    std::unique_lock<std::mutex> lock(locatorMutex_);
    state_ = false;
}

LocationErrCode LocationSaLoadManager::WaitLoadStateChange(int32_t systemAbilityId)
{
    std::unique_lock<std::mutex> lock(locatorMutex_);
    auto wait = locatorCon_.wait_for(lock, std::chrono::milliseconds(LOCATION_LOADSA_TIMEOUT_MS), [this] {
        return state_ == true;
    });
    if (!wait) {
        LBSLOGE(LOCATOR_STANDARD, "locator sa [%{public}d] start time out.", systemAbilityId);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocationSaLoadManager::UnloadLocationSa(int32_t systemAbilityId)
{
    LBSLOGD(LOCATOR, "%{public}s enter, systemAbilityId = [%{public}d] unloading", __func__, systemAbilityId);
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s: get system ability manager failed!", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int32_t ret = samgr->UnloadSystemAbility(systemAbilityId);
    if (ret != ERR_OK) {
        LBSLOGE(LOCATOR, "%{public}s: Failed to unload system ability, SA Id = [%{public}d], ret = [%{public}d].",
            __func__, systemAbilityId, ret);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return ERRCODE_SUCCESS;
}

void LocationSaLoadManager::LoadSystemAbilitySuccess()
{
    std::unique_lock<std::mutex> lock(locatorMutex_);
    state_ = true;
    locatorCon_.notify_one();
}

void LocationSaLoadManager::LoadSystemAbilityFail()
{
    std::unique_lock<std::mutex> lock(locatorMutex_);
    state_ = false;
    locatorCon_.notify_one();
}


bool LocationSaLoadManager::CheckIfSystemAbilityAvailable(int32_t systemAbilityId)
{
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LBSLOGE(COMMON_UTILS, "%{public}s: get system ability manager failed!", __func__);
        return false;
    }
    return (samgr->CheckSystemAbility(systemAbilityId) != nullptr);
}

bool LocationSaLoadManager::InitLocationSa(int32_t systemAbilityId)
{
    if (LocationSaLoadManager::CheckIfSystemAbilityAvailable(systemAbilityId)) {
        LBSLOGD(COMMON_UTILS, "sa has been loaded");
        return true;
    }
    bool ret = true;
    auto startTime = CommonUtils::GetCurrentTimeStamp();
    auto instance = LocationSaLoadManager::GetInstance();
    if (instance == nullptr || instance->LoadLocationSa(systemAbilityId) != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR, "sa load failed.");
        ret = false;
    }
    auto endTime = CommonUtils::GetCurrentTimeStamp();
    WriteLocationInnerEvent(SA_LOAD, {"saId", std::to_string(systemAbilityId), "type", "load",
        "ret", std::to_string(ret), "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
    return true;
}

bool LocationSaLoadManager::UnInitLocationSa(int32_t systemAbilityId)
{
    if (!LocationSaLoadManager::CheckIfSystemAbilityAvailable(systemAbilityId)) {
        LBSLOGD(LOCATOR, "sa has been unloaded");
        return true;
    }
    bool ret = true;
    auto startTime = CommonUtils::GetCurrentTimeStamp();
    auto instance = LocationSaLoadManager::GetInstance();
    if (instance == nullptr || instance->UnloadLocationSa(systemAbilityId) != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR, "sa unload failed.");
        ret = false;
    }
    auto endTime = CommonUtils::GetCurrentTimeStamp();
    WriteLocationInnerEvent(SA_LOAD, {"saId", std::to_string(systemAbilityId), "type", "unload",
        "ret", std::to_string(ret), "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
    return true;
}

void LocationSaLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    LBSLOGD(LOCATOR, "LocationSaLoadManager Load SA success, systemAbilityId = [%{public}d]", systemAbilityId);
    auto instance = LocationSaLoadManager::GetInstance();
    if (instance == nullptr) {
        LBSLOGE(LOCATOR, "LocationSaLoadManager GetInstance return null");
        return;
    }
    instance->LoadSystemAbilitySuccess();
}

void LocationSaLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LBSLOGD(LOCATOR, "LocationSaLoadManager Load SA failed, systemAbilityId = [%{public}d]", systemAbilityId);
    auto instance = LocationSaLoadManager::GetInstance();
    if (instance == nullptr) {
        LBSLOGE(LOCATOR, "LocationSaLoadManager GetInstance return null");
        return;
    }
    instance->LoadSystemAbilityFail();
}
}; // namespace Location
}; // namespace OHOS
