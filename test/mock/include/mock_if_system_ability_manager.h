/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef BASE_LOCATION_MOCK_IF_SYSTEM_ABILITY_MANAGER_H
#define BASE_LOCATION_MOCK_IF_SYSTEM_ABILITY_MANAGER_H

#include "gmock/gmock.h"

#include "if_system_ability_manager.h"

namespace OHOS {
namespace Location {
class MockIfSystemAbilityManager : public ISystemAbilityManager {
public:
    MockIfSystemAbilityManager() {}
    ~MockIfSystemAbilityManager() {}
    MOCK_METHOD(sptr<IRemoteObject>, GetSystemAbility, (int32_t systemAbilityId));
    MOCK_METHOD(sptr<IRemoteObject>, CheckSystemAbility, (int32_t systemAbilityId));
    MOCK_METHOD(int32_t, RemoveSystemAbility, (int32_t systemAbilityId));
    MOCK_METHOD(int32_t, SubscribeSystemAbility, (int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener));
    MOCK_METHOD(int32_t, UnSubscribeSystemAbility, (int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener));
    MOCK_METHOD(sptr<IRemoteObject>, GetSystemAbility, (int32_t systemAbilityId, const std::string& deviceId));
    MOCK_METHOD(sptr<IRemoteObject>, CheckSystemAbility, (int32_t systemAbilityId, const std::string& deviceId));
    MOCK_METHOD(int32_t, AddOnDemandSystemAbilityInfo, (int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName));

    MOCK_METHOD(sptr<IRemoteObject>, CheckSystemAbility, (int32_t systemAbilityId, bool& isExist));
    MOCK_METHOD(int32_t, AddSystemAbility, (int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const ISystemAbilityManager::SAExtraProp& extraProp));
    MOCK_METHOD(int32_t, AddSystemProcess, (const std::u16string& procName, const sptr<IRemoteObject>& procObject));
    MOCK_METHOD(int32_t, LoadSystemAbility, (int32_t systemAbilityId,
        const sptr<ISystemAbilityLoadCallback>& callback));
    MOCK_METHOD(int32_t, LoadSystemAbility, (int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback));
    MOCK_METHOD(int32_t, UnloadSystemAbility, (int32_t systemAbilityId));
    MOCK_METHOD(int32_t, CancelUnloadSystemAbility, (int32_t systemAbilityId));
    MOCK_METHOD(int32_t, GetRunningSystemProcess, (std::list<SystemProcessInfo>& systemProcessInfos));
    MOCK_METHOD(int32_t, SubscribeSystemProcess, (const sptr<ISystemProcessStatusChange>& listener));
    MOCK_METHOD(int32_t, UnSubscribeSystemProcess, (const sptr<ISystemProcessStatusChange>& listener));
};
} // namespace Location
} // namespace OHOS
#endif