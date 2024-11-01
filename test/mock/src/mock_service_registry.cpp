/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef BASE_LOCATION_MOCK_SERVICE_REGISTRY_H
#define BASE_LOCATION_MOCK_SERVICE_REGISTRY_H

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_manager_proxy.h"

#include "location_log.h"

namespace OHOS {
SystemAbilityManagerClient& SystemAbilityManagerClient::GetInstance()
{
    LBSLOGI(Location::LOCATOR, "%{public}s return instance", __func__);
    static auto instance = new SystemAbilityManagerClient();
    return *instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::GetSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return GetSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return nullptr;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return GetSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return GetSystemAbility(systemAbilityId);
}

sptr<IRemoteObject> SystemAbilityManagerProxy::CheckSystemAbility(int32_t systemAbilityId, bool& isExist)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return GetSystemAbility(systemAbilityId);
}

int32_t SystemAbilityManagerProxy::AddOnDemandSystemAbilityInfo(int32_t systemAbilityId,
    const std::u16string& localAbilityManagerName)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::RemoveSystemAbility(int32_t systemAbilityId)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

std::vector<std::u16string> SystemAbilityManagerProxy::ListSystemAbilities(unsigned int dumpFlags)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return empty vector", __func__);
    std::vector<std::u16string> saNames;
    return saNames;
}

int32_t SystemAbilityManagerProxy::SubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::UnSubscribeSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityStatusChange>& listener)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::UnloadSystemAbility(int32_t systemAbilityId)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::CancelUnloadSystemAbility(int32_t systemAbilityId)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::LoadSystemAbility(int32_t systemAbilityId, const std::string& deviceId,
    const sptr<ISystemAbilityLoadCallback>& callback)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::GetRunningSystemProcess(std::list<SystemProcessInfo>& systemProcessInfos)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::SubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::UnSubscribeSystemProcess(const sptr<ISystemProcessStatusChange>& listener)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::AddSystemAbility(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
    const SAExtraProp& extraProp)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::AddSystemAbilityWrapper(int32_t code, MessageParcel& data)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::AddSystemProcess(
    const std::u16string& procName, const sptr<IRemoteObject>& procObject)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

int32_t SystemAbilityManagerProxy::GetOnDemandSystemAbilityIds(std::vector<int32_t>& systemAbilityIds)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return ERR_OK", __func__);
    return ERR_OK;
}

sptr<IRemoteObject> SystemAbilityManagerProxy::Recompute(int32_t systemAbilityId, int32_t code)
{
    LBSLOGI(Location::LOCATOR, "%{public}s return nullptr", __func__);
    return nullptr;
}
} // namespace OHOS
#endif