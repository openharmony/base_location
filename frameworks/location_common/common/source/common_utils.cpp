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

#include <map>
#include <random>

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"

#include "common_utils.h"

namespace OHOS {
namespace Location {
static std::shared_ptr<std::map<int, sptr<IRemoteObject>>> g_proxyMap =
    std::make_shared<std::map<int, sptr<IRemoteObject>>>();
std::mutex g_proxyMutex;

bool CommonUtils::CheckLocationPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId);
}

bool CommonUtils::CheckPermission(const std::string &permission, uint32_t callerToken, uint32_t tokenFirstCaller)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    int result = Security::AccessToken::PERMISSION_DENIED;
    if (tokenFirstCaller == 0) {
        if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_INVALID) {
            LBSLOGD(COMMON_UTILS, "has no permission.permission name=%{public}s", permission.c_str());
            return false;
        } else {
            result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permission);
        }
    } else {
        result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, tokenFirstCaller, permission);
    }
    if (result == Security::AccessToken::PERMISSION_GRANTED) {
        return true;
    } else {
        LBSLOGD(COMMON_UTILS, "has no permission.permission name=%{public}s", permission.c_str());
        return false;
    }
}

bool CommonUtils::CheckBackgroundPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_BACKGROUND_LOCATION, tokenId, firstTokenId);
}

bool CommonUtils::CheckApproximatelyPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId);
}

bool CommonUtils::CheckSecureSettings(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(MANAGE_SECURE_SETTINGS, tokenId, firstTokenId);
}

int CommonUtils::GetPermissionLevel(uint32_t tokenId, uint32_t firstTokenId)
{
    int ret = PERMISSION_INVALID;
    if (CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId) &&
        CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId)) {
        ret = PERMISSION_ACCURATE;
    } else if (CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId) &&
        !CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId)) {
        ret = PERMISSION_APPROXIMATELY;
    } else if (!CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId) &&
        CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId)) {
        ret = PERMISSION_ACCURATE;
    }  else {
        ret = PERMISSION_INVALID;
    }
    return ret;
}

int CommonUtils::AbilityConvertToId(const std::string ability)
{
    if (GNSS_ABILITY.compare(ability) == 0) {
        return LOCATION_GNSS_SA_ID;
    }
    if (NETWORK_ABILITY.compare(ability) == 0) {
        return LOCATION_NETWORK_LOCATING_SA_ID;
    }
    if (PASSIVE_ABILITY.compare(ability) == 0) {
        return LOCATION_NOPOWER_LOCATING_SA_ID;
    }
    if (GEO_ABILITY.compare(ability) == 0) {
        return LOCATION_GEO_CONVERT_SA_ID;
    }
    return -1;
}

std::u16string CommonUtils::GetCapabilityToString(std::string ability, uint32_t capability)
{
    std::string value = "{\"Capabilities\":{\"" + ability + "\":" + std::to_string(capability) + "}}";
    return Str8ToStr16(value);
}

std::u16string CommonUtils::GetCapability(std::string ability)
{
    uint32_t capability = 0x102;
    return GetCapabilityToString(ability, capability);
}

OHOS::HiviewDFX::HiLogLabel CommonUtils::GetLabel(std::string name)
{
    if (GNSS_ABILITY.compare(name) == 0) {
        return GNSS;
    }
    if (NETWORK_ABILITY.compare(name) == 0) {
        return NETWORK;
    }
    if (PASSIVE_ABILITY.compare(name) == 0) {
        return PASSIVE;
    }
    if (GEO_ABILITY.compare(name) == 0) {
        return GEO_CONVERT;
    }
    OHOS::HiviewDFX::HiLogLabel label = { LOG_CORE, LOCATOR_LOG_ID, "unknown" };
    return label;
}

sptr<IRemoteObject> CommonUtils::GetRemoteObject(int abilityId)
{
    return GetRemoteObject(abilityId, InitDeviceId());
}

sptr<IRemoteObject> CommonUtils::GetRemoteObject(int abilityId, std::string deviceId)
{
    std::lock_guard<std::mutex> lock(g_proxyMutex);
    auto objectGnss = g_proxyMap->find(abilityId);
    if (objectGnss == g_proxyMap->end()) {
        auto manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (manager == nullptr) {
            LBSLOGE(COMMON_UTILS, "GetSystemAbilityManager is null.");
            return nullptr;
        }
        sptr<IRemoteObject> object = manager->GetSystemAbility(abilityId, deviceId);
        if (object == nullptr) {
            LBSLOGE(COMMON_UTILS, "GetSystemAbility is null.");
            return nullptr;
        }
        g_proxyMap->insert(std::make_pair(abilityId, object));
        return object;
    } else {
        sptr<IRemoteObject> remoteObject = objectGnss->second;
        return remoteObject;
    }
}

std::string CommonUtils::InitDeviceId()
{
    std::string deviceId;
    return deviceId;
}

bool CommonUtils::GetCurrentUserId(int &userId)
{
    std::vector<int> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        LBSLOGE(COMMON_UTILS, "QueryActiveOsAccountIds failed ret:%{public}d", ret);
        return false;
    }
    if (activeIds.empty()) {
        LBSLOGE(COMMON_UTILS, "QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    userId = activeIds[0];
    return true;
}

void CountDownLatch::Wait(int time)
{
    LBSLOGD(LOCATOR_STANDARD, "enter wait, time = %{public}d", time);
    std::unique_lock<std::mutex> lock(mutex_);
    if (count_ == 0) {
        LBSLOGE(LOCATOR_STANDARD, "count_ = 0");
        return;
    }
    condition_.wait_for(lock, std::chrono::seconds(time / SEC_TO_MILLI_SEC), [&]() {return count_ == 0;});
}

void CountDownLatch::CountDown()
{
    LBSLOGD(LOCATOR_STANDARD, "enter CountDown");
    std::unique_lock<std::mutex> lock(mutex_);
    int oldC = count_.load();
    while (oldC > 0) {
        if (count_.compare_exchange_strong(oldC, oldC - 1)) {
            if (oldC == 1) {
                LBSLOGD(LOCATOR_STANDARD, "notify_all");
                condition_.notify_all();
            }
            break;
        }
        oldC = count_.load();
    }
}

int CountDownLatch::GetCount()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return count_;
}

void CountDownLatch::SetCount(int count)
{
    std::unique_lock<std::mutex> lock(mutex_);
    count_ = count;
}

std::string CommonUtils::Str16ToStr8(std::u16string str)
{
    if (str == DEFAULT_USTRING) {
        return DEFAULT_STRING;
    }
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert(DEFAULT_STRING);
    std::string result = convert.to_bytes(str);
    return result == DEFAULT_STRING ? "" : result;
}

bool CommonUtils::DoubleEqual(double a, double b)
{
    if (fabs(a - b) < 1e-6) {
        return true;
    } else {
        return false;
    }
}

double CommonUtils::CalDistance(const double lat1, const double lon1, const double lat2, const double lon2)
{
    double radLat1 = lat1 * PI / DEGREE_PI;
    double radLat2 = lat2 * PI / DEGREE_PI;
    double radLon1 = lon1 * PI / DEGREE_PI;
    double radLon2 = lon2 * PI / DEGREE_PI;

    double latDiff = radLat1 - radLat2;
    double lonDiff = radLon1 - radLon2;
    double temp = sqrt(pow(sin(latDiff / DIS_FROMLL_PARAMETER), DIS_FROMLL_PARAMETER) +
        cos(radLat1) * cos(radLat2) * pow(sin(lonDiff / DIS_FROMLL_PARAMETER), DIS_FROMLL_PARAMETER));
    double disRad = asin(temp) * DIS_FROMLL_PARAMETER;
    double dis = disRad * EARTH_RADIUS;
    return dis;
}

double CommonUtils::DoubleRandom(double min, double max)
{
    double param = 0.0;
    std::random_device rd;
    static std::uniform_real_distribution<double> u(min, max);
    static std::default_random_engine e(rd());
    param = u(e);
    return param;
}

int CommonUtils::IntRandom(int min, int max)
{
    int param = 0;
    std::random_device rd;
    static std::uniform_int_distribution<int> u(min, max);
    static std::default_random_engine e(rd());
    param = u(e);
    return param;
}

bool CommonUtils::CheckSystemPermission(pid_t uid, uint32_t callerTokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL ||
        tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_INVALID) {
        return false;
    }
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        LBSLOGE(COMMON_UTILS, "Get system ability manager failed!");
        return false;
    }
    auto bundleMgrSa = systemManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleMgrSa == nullptr) {
        LBSLOGE(COMMON_UTILS, "GetSystemAbility return nullptr!");
        return false;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleMgrSa);
    if (bundleMgr == nullptr) {
        LBSLOGE(COMMON_UTILS, "iface_cast return nullptr!");
        return false;
    }
    bool isSysApp = bundleMgr->CheckIsSystemAppByUid(uid);
    LBSLOGD(COMMON_UTILS, "Is system App uid[%{public}d]: %{public}d", uid, isSysApp);
    return isSysApp;
}

bool CommonUtils::GetBundleNameByUid(int32_t uid, std::string& bundleName)
{
    sptr<ISystemAbilityManager> smgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smgr == nullptr) {
        LBSLOGE(COMMON_UTILS, "GetBundleNameByUid Fail to get system ability manager.");
        return false;
    }
    sptr<IRemoteObject> remoteObject = smgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        LBSLOGE(COMMON_UTILS, "GetBundleNameByUid Fail to get system ability manager.");
        return false;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy(new AppExecFwk::BundleMgrProxy(remoteObject));
    if (bundleMgrProxy == nullptr) {
        LBSLOGE(COMMON_UTILS, "GetBundleNameByUid Bundle mgr proxy is nullptr.");
        return false;
    }
    return bundleMgrProxy->GetBundleNameForUid(uid, bundleName);
}

bool CommonUtils::GetApplicationInfo(const std::string& bundleName)
{
    int userId = 0;
    bool ret = GetCurrentUserId(userId);
    if (!ret) {
        LBSLOGE(COMMON_UTILS, "GetCurrentUserId failed");
        return false;
    }
    AppExecFwk::ApplicationInfo info;
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        LBSLOGE(COMMON_UTILS, "fail to get system ability manager!");
        return false;
    }
    auto bundleMgrSa = systemManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleMgrSa == nullptr) {
        LBSLOGE(COMMON_UTILS, "fail to get system ability!");
        return false;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleMgrSa);
    if (bundleMgr == nullptr) {
        LBSLOGE(COMMON_UTILS, "Bundle mgr is nullptr.");
        return false;
    }
    bundleMgr->GetApplicationInfoV9(bundleName, 0, userId, info);
    LBSLOGI(COMMON_UTILS, "GetApplicationInfo, name=%{public}s, bundleName=%{public}s", info.name.c_str(), info.bundleName.c_str());
    if (info.name.empty() || info.bundleName.empty()) {
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS
