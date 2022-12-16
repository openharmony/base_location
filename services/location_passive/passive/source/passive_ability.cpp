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
    passiveHandler_ = std::make_shared<PassiveHandler>(AppExecFwk::EventRunner::Create(true));
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

void PassiveAbility::SendLocationRequest(WorkRecord &workrecord)
{
    LocationRequest(workrecord);
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

bool PassiveAbility::IsMockEnabled()
{
    return IsLocationMocked();
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

void PassiveAbility::SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    if (passiveHandler_ == nullptr) {
        return;
    }
    switch (code) {
        case SET_MOCKED_LOCATIONS: {
            if (!IsMockEnabled()) {
                reply.WriteBool(false);
                break;
            }
            std::unique_ptr<LocationMockConfig> mockConfig = LocationMockConfig::Unmarshalling(data);
            int locationSize = data.ReadInt32();
            locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
                locationSize;
            std::shared_ptr<std::vector<std::shared_ptr<Location>>> vcLoc =
                std::make_shared<std::vector<std::shared_ptr<Location>>>();
            for (int i = 0; i < locationSize; i++) {
                vcLoc->push_back(Location::UnmarshallingShared(data));
            }
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, vcLoc, mockConfig->GetTimeInterval());
            bool result = passiveHandler_->SendEvent(event);
            reply.WriteBool(result);
            break;
        }
        default:
            break;
    }
}

PassiveHandler::PassiveHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

PassiveHandler::~PassiveHandler() {}

void PassiveHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(PASSIVE, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case ISubAbility::SET_MOCKED_LOCATIONS: {
            int timeInterval = event->GetParam();
            LocationMockConfig mockConfig;
            mockConfig.SetTimeInterval(timeInterval);
            auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
            if (vcLoc == nullptr) {
                break;
            }
            std::vector<std::shared_ptr<Location>> mockLocations;
            for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
                mockLocations.push_back(*it);
            }
            auto passiveAbility = DelayedSingleton<PassiveAbility>::GetInstance();
            if (passiveAbility != nullptr) {
                passiveAbility->SetMocked(mockConfig, mockLocations);
            }
            break;
        }
        default:
            break;
    }
}
} // namespace Location
} // namespace OHOS