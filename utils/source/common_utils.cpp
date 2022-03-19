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
#include <thread>

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
sptr<IRemoteObject> CommonUtils::GetLocationService()
{
    return nullptr;
}

bool CommonUtils::RemoteToLocationService(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option;
    return RemoteToLocationService(code, data, reply, option);
}

bool CommonUtils::RemoteToLocationService(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    return true;
}

bool CommonUtils::EnforceInterface(const std::u16string &descriptor, MessageParcel &data)
{
    data.ReadInt32(); // strictPolicy
    data.ReadInt32(); // workSource
    std::u16string result = data.ReadString16();
    return descriptor.compare(result) == 0;
}

bool CommonUtils::CheckSystemCalling(pid_t uid)
{
    return true;
}

bool CommonUtils::CheckLocatorInterfaceToken(std::u16string descripter, MessageParcel &data)
{
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    LBSLOGD(COMMON_UTILS, "local des %{public}s, remote des: %{public}s", Str16ToStr8(descripter).c_str(),
        Str16ToStr8(remoteDescripter).c_str());
    if (descripter.compare(remoteDescripter) != 0) {
        LBSLOGE(COMMON_UTILS, "this remote request token is invalid");
        return false;
    }
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

uint32_t CommonUtils::GetCapabilityValue(std::string ability)
{
    std::string device = OHOS::system::GetParameter(BUILD_INFO, DEVICE_UNKOWN);
    std::string simulateDevice = OHOS::system::GetParameter(TEST_SIMULATE_DEVICE, DEVICE_UNKOWN);
    uint32_t capability = CAP_DEFAULT;
    if (DEVICE_WEAR.compare(simulateDevice) == 0) {
        capability = CAP_GNSS_WEAR;
    } else if (DEVICE_PHONE.compare(device) == 0) {
        if (GNSS_ABILITY.compare(ability) == 0) {
            capability = CAP_GNSS_PHONE;
        }
    } else if (DEVICE_WEAR.compare(device) == 0) {
        if (GNSS_ABILITY.compare(ability) == 0) {
            capability = CAP_GNSS_WEAR;
        }
    }
    return capability;
}

std::u16string CommonUtils::GetCapabilityToString(std::string ability, uint32_t capability)
{
    std::string value = "{\"Capabilities\":{\"" + ability + "\":" + std::to_string(capability) + "}}";
    return Str8ToStr16(value);
}

std::u16string CommonUtils::GetCapability(std::string ability)
{
    uint32_t capability = GetCapabilityValue(ability);
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
    sptr<ISystemAbilityManager> manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (manager == nullptr) {
        return nullptr;
    }
    sptr<IRemoteObject> object = manager->GetSystemAbility(abilityId, deviceId);
    return object;
}

std::string CommonUtils::InitDeviceId()
{
    std::string deviceId;
    sptr<ISystemAbilityManager> manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (manager != nullptr) {
    }
    return deviceId;
}

int CommonUtils::GetPrivacyType(LocationPrivacyType type)
{
    int ret;
    switch (type) {
        case LocationPrivacyType::OTHERS: {
            ret = 0;
            break;
        }
        case LocationPrivacyType::STARTUP: {
            ret = 1;
            break;
        }
        case LocationPrivacyType::CORE_LOCATION: {
            ret = 2;
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }
    return ret;
}

LocationPrivacyType CommonUtils::GetPrivacyTypeByInt(int type)
{
    LocationPrivacyType ret;
    switch (type) {
        case 0: {
            ret = LocationPrivacyType::OTHERS;
            break;
        }
        case 1: {
            ret = LocationPrivacyType::STARTUP;
            break;
        }
        case 2: {
            ret = LocationPrivacyType::CORE_LOCATION;
            break;
        }
        default: {
            ret = LocationPrivacyType::OTHERS;
            break;
        }
    }
    return ret;
}
} // namespace Location
} // namespace OHOS
