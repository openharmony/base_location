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

#include "passive_ability.h"

#include <file_ex.h>

#include "singleton.h"
#include "string_ex.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "location.h"
#include "location_dumper.h"
#include "location_log.h"
#include "location_mock_config.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = PassiveAbility::MakeAndRegisterAbility(
    DelayedSingleton<PassiveAbility>::GetInstance().get());

PassiveAbility::PassiveAbility() : SystemAbility(LOCATION_NOPOWER_LOCATING_SA_ID, true)
{
    SetAbility(PASSIVE_ABILITY);
    LBSLOGI(PASSIVE, "ability constructed.");
}

PassiveAbility::~PassiveAbility() {}

void PassiveAbility::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(PASSIVE, "ability has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(PASSIVE, "failed to init ability");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    LBSLOGI(PASSIVE, "OnStart start ability success.");
}

void PassiveAbility::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToAbility_ = false;
    LBSLOGI(PASSIVE, "OnStop ability stopped.");
}

bool PassiveAbility::Init()
{
    if (!registerToAbility_) {
        bool ret = Publish(AsObject());
        if (!ret) {
            LBSLOGE(PASSIVE, "Init Publish failed!");
            return false;
        }
        registerToAbility_ = true;
    }
    return true;
}

void PassiveAbility::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    LocationRequest(interval, workrecord);
}

void PassiveAbility::SetEnable(bool state)
{
    Enable(state, AsObject());
}

void PassiveAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    LBSLOGE(PASSIVE, "enter RequestRecord");
}

bool PassiveAbility::EnableMock(const LocationMockConfig& config)
{
    return EnableLocationMock(config);
}

bool PassiveAbility::DisableMock(const LocationMockConfig& config)
{
    return DisableLocationMock(config);
}

bool PassiveAbility::SetMocked(const LocationMockConfig& config,
    const std::vector<std::shared_ptr<Location>> &location)
{
    return SetMockedLocations(config, location);
}

void PassiveAbility::SendReportMockLocationEvent()
{
}

void PassiveAbility::SaDumpInfo(std::string& result)
{
    result += "Passive Location enable status: true";
    result += "\n";
}

int32_t PassiveAbility::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.PassiveDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(PASSIVE, "Passive save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}
} // namespace Location
} // namespace OHOS