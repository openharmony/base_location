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

#include "common_utils.h"
#include <map>
#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
static std::shared_ptr<std::map<int, sptr<IRemoteObject>>> g_proxyMap =
    std::make_shared<std::map<int, sptr<IRemoteObject>>>();
std::mutex g_proxyMutex;

bool CommonUtils::CheckSystemCalling(pid_t uid)
{
    return true;
}

bool CommonUtils::CheckLocationPermission()
{
    return CheckPermission(ACCESS_LOCATION);
}

bool CommonUtils::CheckPermission(const std::string &permission)
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    auto tokenFirstCaller = IPCSkeleton::GetFirstTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    int result = Security::AccessToken::PERMISSION_DENIED;
    if (tokenFirstCaller == 0) {
        if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
            result = Security::AccessToken::AccessTokenKit::VerifyNativeToken(callerToken, permission);
        } else if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
            result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permission);
        } else {
            LBSLOGE(COMMON_UTILS, "invalid callerToken");
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

bool CommonUtils::CheckBackgroundPermission()
{
    return CheckPermission(ACCESS_BACKGROUND_LOCATION);
}

bool CommonUtils::CheckSecureSettings()
{
    return CheckPermission(MANAGE_SECURE_SETTINGS);
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
	LBSLOGD(LOCATOR_STANDARD, "enter wait, time = %{public}ld", time);
	std::unique_lock<std::mutex> lock(mutex_);
	if (count_ == 0)
		LBSLOGE(LOCATOR_STANDARD, "count_ = 0");
		return;
	}
	condition_.wait_for(lock, std::chrono::seconds(time / 1000), [&](){return count_ == 0;});
}

void CountDownLatch::CountDown()
{
	LBSLOGD(LOCATOR_STANDARD, "enter CountDown");
	std::unique_lock<std::mutex> lock(mutex_);
	int old_c = count_.load();
	while (old_c > 0) {
		if (count_.compare_exchange_strong(old_c, old_c - 1)) {
			if (old_c == 1) {
				LBSLOGD(LOCATOR_STANDARD, "notify_all");
				condition_.notify_all();
			}
			break;
		}
		old_c = count_.load();
	}
}

int CountDownLatch::GetCount() const
{
	std::unique_lock<std::mutex> lock(mutex_);
	return count_;
}

void CountDownLatch::SetCount(int count)
{
	std::unique_lock<std::mutex> lock(mutex_);
	count_ = count;
}
} // namespace Location
} // namespace OHOS
