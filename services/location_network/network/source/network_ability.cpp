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

#include "network_ability.h"
#include <file_ex.h>
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "location_log.h"
#include "location_dumper.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
const uint32_t EVENT_REPORT_LOCATION = 0x0001;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
const bool REGISTER_RESULT = NetworkAbility::MakeAndRegisterAbility(
    DelayedSingleton<NetworkAbility>::GetInstance().get());

NetworkAbility::NetworkAbility() : SystemAbility(LOCATION_NETWORK_LOCATING_SA_ID, true)
{
    SetAbility(NETWORK_ABILITY);
    networkHandler_ = std::make_shared<NetworkHandler>(AppExecFwk::EventRunner::Create(true));
    LBSLOGI(NETWORK, "ability constructed.");
}

NetworkAbility::~NetworkAbility() {}

void NetworkAbility::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(NETWORK, "ability has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(NETWORK, "failed to init ability");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    LBSLOGI(NETWORK, "OnStart start ability success.");
}

void NetworkAbility::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToAbility_ = false;
    LBSLOGI(NETWORK, "OnStop ability stopped.");
}

bool NetworkAbility::Init()
{
    if (!registerToAbility_) {
        if (!Publish(AsObject())) {
            LBSLOGE(NETWORK, "Init Publish failed!");
            return false;
        }
        registerToAbility_ = true;
    }
    return true;
}

void NetworkAbility::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    LocationRequest(interval, workrecord);
}

void NetworkAbility::SetEnable(bool state)
{
    Enable(state, AsObject());
}

void NetworkAbility::SelfRequest(bool state)
{
    LBSLOGE(NETWORK, "SelfRequest %{public}d", state);
    HandleSelfRequest(IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), state);
}

void NetworkAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    LBSLOGE(NETWORK, "enter RequestRecord");
}

bool NetworkAbility::EnableMock(const LocationMockConfig& config)
{
    return EnableLocationMock(config);
}

bool NetworkAbility::DisableMock(const LocationMockConfig& config)
{
    return DisableLocationMock(config);
}

bool NetworkAbility::IsMockEnabled()
{
    return IsLocationMocked();
}

bool NetworkAbility::SetMocked(const LocationMockConfig& config,
    const std::vector<std::shared_ptr<Location>> &location)
{
    return SetMockedLocations(config, location);
}

void NetworkAbility::ProcessReportLocationMock()
{
    std::vector<std::shared_ptr<Location>> mockLocationArray = GetLocationMock();
    if (mockLocationIndex_ < mockLocationArray.size()) {
        ReportMockedLocation(mockLocationArray[mockLocationIndex_++]);
        networkHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION, 0, GetTimeIntervalMock() * EVENT_INTERVAL_UNITE);
    } else {
        ClearLocationMock();
        mockLocationIndex_ = 0;
    }
}

void NetworkAbility::SendReportMockLocationEvent()
{
    networkHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION, 0, 0);
}

int32_t NetworkAbility::ReportMockedLocation(const std::shared_ptr<Location> location)
{
    std::unique_ptr<Location> locationNew = std::make_unique<Location>();
    locationNew->SetLatitude(location->GetLatitude());
    locationNew->SetLongitude(location->GetLongitude());
    locationNew->SetAltitude(location->GetAltitude());
    locationNew->SetAccuracy(location->GetAccuracy());
    locationNew->SetSpeed(location->GetSpeed());
    locationNew->SetDirection(location->GetDirection());
    locationNew->SetTimeStamp(location->GetTimeStamp());
    locationNew->SetTimeSinceBoot(location->GetTimeSinceBoot());
    locationNew->SetAdditions(location->GetAdditions());
    locationNew->SetAdditionSize(location->GetAdditionSize());
    locationNew->SetIsFromMock(location->GetIsFromMock());
    if ((IsLocationMocked() && !location->GetIsFromMock()) ||
        (!IsLocationMocked() && location->GetIsFromMock())) {
        LBSLOGE(NETWORK, "location mock is enabled, do not report gnss location!");
        return ERR_OK;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if(locatorAbility != nullptr) {
        locatorAbility.get()->ReportLocation(locationNew, NETWORK_ABILITY);
        locatorAbility.get()->ReportLocation(locationNew, PASSIVE_ABILITY);
    }
    return ERR_OK;
}

void NetworkAbility::SaDumpInfo(std::string& result)
{
    result += "Network Location enable status: false";
    result += "\n";
}

int32_t NetworkAbility::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.NetWorkDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(NETWORK, "Network save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}

void NetworkAbility::SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
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
            bool result = networkHandler_->SendEvent(event);
            reply.WriteBool(result);
            break;
        }
        default:
            break;
    }
}

NetworkHandler::NetworkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

NetworkHandler::~NetworkHandler() {}

void NetworkHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(NETWORK, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_REPORT_LOCATION: {
            DelayedSingleton<NetworkAbility>::GetInstance()->ProcessReportLocationMock();
            break;
        }
        case ISubAbility::SET_MOCKED_LOCATIONS: {
            int timeInterval = event->GetParam();
            LocationMockConfig mockConfig;
            mockConfig.SetTimeInterval(timeInterval);
            auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
            if (vcLoc != nullptr) {
                std::vector<std::shared_ptr<Location>> mockLocations;
                for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
                    mockLocations.push_back(*it);
                }
                DelayedSingleton<NetworkAbility>::GetInstance()->SetMocked(
                    mockConfig, mockLocations);
            }
            break;
        }
        default:
            break;
    }
}
} // namespace Location
} // namespace OHOS