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

#ifdef FEATURE_PASSIVE_SUPPORT
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
#include "location_sa_load_manager.h"
#include "work_record.h"
#include "locationhub_ipc_interface_code.h"

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

LocationErrCode PassiveAbility::SendLocationRequest(WorkRecord &workrecord)
{
    LocationRequest(workrecord);
    return ERRCODE_SUCCESS;
}

LocationErrCode PassiveAbility::SetEnable(bool state)
{
    if (state) {
        Enable(true, AsObject());
        return ERRCODE_SUCCESS;
    }
    if (!CheckIfPassiveConnecting()) {
        Enable(false, AsObject());
    }
    return ERRCODE_SUCCESS;
}

void PassiveAbility::UnloadPassiveSystemAbility()
{
    auto locationSaLoadManager = DelayedSingleton<LocationSaLoadManager>::GetInstance();
    if (locationSaLoadManager == nullptr) {
        return;
    }

    if (!CheckIfPassiveConnecting()) {
        locationSaLoadManager->UnloadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
    }
}

bool PassiveAbility::CheckIfPassiveConnecting()
{
    return IsMockEnabled() || !GetLocationMock().empty() || GetRequestNum() != 0;
}

void PassiveAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    LBSLOGE(PASSIVE, "enter RequestRecord");
}

LocationErrCode PassiveAbility::EnableMock()
{
    if (!EnableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode PassiveAbility::DisableMock()
{
    if (!DisableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

bool PassiveAbility::IsMockEnabled()
{
    return IsLocationMocked();
}

LocationErrCode PassiveAbility::SetMocked(const int timeInterval,
    const std::vector<std::shared_ptr<Location>> &location)
{
    if (!SetMockedLocations(timeInterval, location)) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

void PassiveAbility::SendReportMockLocationEvent()
{
    ClearLocationMock();
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
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return;
    }
    switch (code) {
        case static_cast<uint32_t>(PassiveInterfaceCode::SET_MOCKED_LOCATIONS): {
            if (!IsMockEnabled()) {
                reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
                break;
            }
            int timeInterval = data.ReadInt32();
            int locationSize = data.ReadInt32();
            timeInterval = timeInterval < 0 ? 1 : timeInterval;
            locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
                locationSize;
            std::shared_ptr<std::vector<std::shared_ptr<Location>>> vcLoc =
                std::make_shared<std::vector<std::shared_ptr<Location>>>();
            for (int i = 0; i < locationSize; i++) {
                vcLoc->push_back(Location::UnmarshallingShared(data));
            }
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, vcLoc, timeInterval);
            if (passiveHandler_->SendEvent(event)) {
                reply.WriteInt32(ERRCODE_SUCCESS);
            } else {
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            }
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
    auto passiveAbility = DelayedSingleton<PassiveAbility>::GetInstance();
    if (passiveAbility == nullptr) {
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(PASSIVE, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case static_cast<uint32_t>(PassiveInterfaceCode::SET_MOCKED_LOCATIONS): {
            int timeInterval = event->GetParam();
            auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
            if (vcLoc == nullptr) {
                break;
            }
            std::vector<std::shared_ptr<Location>> mockLocations;
            for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
                mockLocations.push_back(*it);
            }
            if (passiveAbility != nullptr) {
                passiveAbility->SetMocked(timeInterval, mockLocations);
            }
            break;
        }
        default:
            break;
    }
    passiveAbility->UnloadPassiveSystemAbility();
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_PASSIVE_SUPPORT
