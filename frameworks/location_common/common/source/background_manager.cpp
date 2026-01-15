/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "background_manager.h"
#include <vector>
#include <algorithm>
#include <mutex>
#include "common_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"


namespace OHOS {
namespace Location {
const int FOREGROUPAPP_STATUS = 2;
std::mutex BackgroundManager::foregroundAppMutex_;
BackgroundManager* BackgroundManager::GetInstance()
{
    static BackgroundManager manager;
    return &manager;
}

BackgroundManager::BackgroundManager()
{
}

BackgroundManager::~BackgroundManager()
{
}

bool BackgroundManager::IsAppBackground(std::string bundleName)
{
    sptr<ISystemAbilityManager> samgrClient = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrClient == nullptr) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "Get system ability manager failed.");
        return false;
    }
    sptr<AppExecFwk::IAppMgr> iAppManager =
        iface_cast<AppExecFwk::IAppMgr>(samgrClient->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (iAppManager == nullptr) {
        LBSLOGE(LOCATOR_BACKGROUND_PROXY, "Failed to get ability manager service.");
        return false;
    }
    std::vector<AppExecFwk::AppStateData> foregroundAppList;
    iAppManager->GetForegroundApplications(foregroundAppList);
    auto it = std::find_if(foregroundAppList.begin(), foregroundAppList.end(), [bundleName] (auto foregroundApp) {
        return bundleName.compare(foregroundApp.bundleName) == 0;
    });
    if (it != foregroundAppList.end()) {
        LBSLOGD(LOCATOR_BACKGROUND_PROXY, "app : %{public}s is foreground.", bundleName.c_str());
        return false;
    }
    return true;
}

bool BackgroundManager::IsAppBackground(int uid, std::string bundleName)
{
    std::unique_lock lock(foregroundAppMutex_);
    auto iter = foregroundAppMap_.find(uid);
    if (iter == foregroundAppMap_.end()) {
        return IsAppBackground(bundleName);
    }
    return false;
}

void BackgroundManager::UpdateBackgroundAppStatues(int32_t uid, int32_t status)
{
    std::unique_lock lock(foregroundAppMutex_);
    if (status == FOREGROUPAPP_STATUS) {
        foregroundAppMap_[uid] = status;
    } else {
        auto iter = foregroundAppMap_.find(uid);
        if (iter != foregroundAppMap_.end()) {
            foregroundAppMap_.erase(iter);
        }
    }
    LBSLOGD(REQUEST_MANAGER, "UpdateBackgroundApp uid = %{public}d, state = %{public}d", uid, status);
}

bool BackgroundManager::IsAppInLocationContinuousTasks(pid_t uid, pid_t pid)
{
#ifdef BGTASKMGR_SUPPORT
    std::vector<std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>> continuousTasks;
    ErrCode result = BackgroundTaskMgr::BackgroundTaskMgrHelper::GetContinuousTaskApps(continuousTasks);
    if (result != ERR_OK) {
        return false;
    }
    for (auto iter = continuousTasks.begin(); iter != continuousTasks.end(); iter++) {
        auto continuousTask = *iter;
        if (continuousTask == nullptr) {
            continue;
        }
        if (continuousTask->GetCreatorUid() != uid || continuousTask->GetCreatorPid() != pid) {
            continue;
        }
        auto typeIds = continuousTask->GetTypeIds();
        for (auto typeId : typeIds) {
            if (typeId == BackgroundTaskMgr::BackgroundMode::Type::LOCATION) {
                return true;
            }
        }
    }
#endif
    return false;
}

bool BackgroundManager::IsAppHasFormVisible(uint32_t tokenId, uint64_t tokenIdEx)
{
    bool ret = false;
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        return ret;
    }
#ifdef FMSKIT_NATIVE_SUPPORT
    ret = OHOS::AppExecFwk::FormMgr::GetInstance().HasFormVisible(tokenId);
#endif
    return ret;
}

} // namespace Location
} // namespace OHOS