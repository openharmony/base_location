/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "bundle_mgr_helper.h"
#include "location_log.h"

namespace OHOS {
namespace Location {

const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
constexpr int32_t DEFAULT_USERID = 100;

/*
 * Check whether the application is installed by bundleName
 * @param bundleName
 * @return true if app is installed
 * @return false if app is not installed
 */
bool BundleMgrHelper::CheckAppInstalled(const std::string& bundleName)
{
    AppExecFwk::ApplicationInfo info;
    GetAppInfo(bundleName, info);
    if (info.name.empty() || info.bundleName.empty()) {
        return false;
    }
    return true;
}
 
bool BundleMgrHelper::GetCurrentUserId(int &userId)
{
    std::vector<int> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        userId = DEFAULT_USERID;
        LBSLOGI(COMMON_UTILS, "GetCurrentUserId failed ret:%{public}d", ret);
        return false;
    }
    if (activeIds.empty()) {
        userId = DEFAULT_USERID;
        LBSLOGE(COMMON_UTILS, "QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    userId = activeIds[0];
    return true;
}
 
bool BundleMgrHelper::GetBundleNameByUid(int32_t uid, std::string& bundleName)
{
    AppExecFwk::BundleMgrClient bundleMgrClient;
    int32_t error = bundleMgrClient.GetNameForUid(uid, bundleName);
    if (error != ERR_OK) {
        return false;
    }
    return true;
}

bool BundleMgrHelper::CheckAppDebug(const std::string& bundleName)
{
    AppExecFwk::ApplicationInfo info;
    GetAppInfo(bundleName, info);
    if (!info.debug) {
        LBSLOGI(COMMON_UTILS, "CheckAppDebug debug ret:%{public}d", info.debug);
        return false;
    }
    return true;
}

bool BundleMgrHelper::GetAppInfo(const std::string& bundleName, AppExecFwk::ApplicationInfo& info)
{
    int userId = 0;
    bool ret = GetCurrentUserId(userId);
    if (!ret) {
        LBSLOGE(COMMON_UTILS, "GetCurrentUserId failed");
        return false;
    }
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        LBSLOGE(COMMON_UTILS, "fail to get system ability manager!");
        return false;
    }
    auto bundleMgrSa = systemManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleMgrSa == nullptr) {
        LBSLOGE(COMMON_UTILS, "fail to get bundle manager system ability!");
        return false;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleMgrSa);
    if (bundleMgr == nullptr) {
        LBSLOGE(COMMON_UTILS, "Bundle mgr is nullptr.");
        return false;
    }
    bundleMgr->GetApplicationInfoV9(bundleName, 0, userId, info);
    return true;
}
}
}