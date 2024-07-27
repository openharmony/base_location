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

#ifndef PASSIVE_ABILITY_H
#define PASSIVE_ABILITY_H
#ifdef FEATURE_PASSIVE_SUPPORT

#include <mutex>
#include <singleton.h>

#include "event_handler.h"
#include "event_runner.h"
#include "if_system_ability_manager.h"
#include "system_ability.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "passive_ability_skeleton.h"
#include "subability_common.h"

namespace OHOS {
namespace Location {
class PassiveHandler : public AppExecFwk::EventHandler {
public:
    explicit PassiveHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~PassiveHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
};

class PassiveAbility : public SystemAbility, public PassiveAbilityStub, public SubAbility {
DECLEAR_SYSTEM_ABILITY(PassiveAbility);

public:
    DISALLOW_COPY_AND_MOVE(PassiveAbility);

    static PassiveAbility* GetInstance();
    PassiveAbility();
    ~PassiveAbility() override;
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    LocationErrCode SendLocationRequest(WorkRecord &workrecord) override;
    LocationErrCode SetEnable(bool state) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void RequestRecord(WorkRecord &workRecord, bool isAdded) override;
    LocationErrCode EnableMock() override;
    LocationErrCode DisableMock() override;
    LocationErrCode SetMocked(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
    void SendReportMockLocationEvent() override;
    bool IsMockEnabled();
    void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) override;
    bool CancelIdleState() override;
    void UnloadPassiveSystemAbility() override;
private:
    bool Init();
    static void SaDumpInfo(std::string& result);
    bool CheckIfPassiveConnecting();

    std::shared_ptr<PassiveHandler> passiveHandler_;
    bool registerToAbility_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_PASSIVE_SUPPORT
#endif // PASSIVE_ABILITY_H
