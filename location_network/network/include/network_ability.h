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

#ifndef NETWORK_ABILITY_H
#define NETWORK_ABILITY_H

#include <mutex>
#include <singleton.h>
#include "if_system_ability_manager.h"
#include "system_ability.h"
#include "common_utils.h"
#include "network_ability_skeleton.h"
#include "subability_common.h"

namespace OHOS {
namespace Location {
class NetworkAbility : public SystemAbility, public NetworkAbilityStub, public SubAbility,
    DelayedSingleton<NetworkAbility> {
DECLEAR_SYSTEM_ABILITY(NetworkAbility);

public:
    DISALLOW_COPY_AND_MOVE(NetworkAbility);
    NetworkAbility();
    ~NetworkAbility();
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    void SendLocationRequest(uint64_t interval, WorkRecord &workrecord) override;
    std::unique_ptr<Location> GetCachedLocation() override;
    void SetEnable(bool state) override;
    void SelfRequest(bool state) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void RequestRecord(sptr<LocationCallbackStub> addCallback, WorkRecord &workRecord, bool isAdded) override;
private:
    bool Init();
    static void SaDumpInfo(std::string& result);
    bool registerToAbility_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
};
} // namespace Location
} // namespace OHOS
#endif // NETWORK_ABILITY_H