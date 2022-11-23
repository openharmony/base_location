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
#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "want_agent_helper.h"

#include "common_utils.h"
#include "location_log.h"
#include "location_dumper.h"
#include "locator_ability.h"
#include "network_callback_host.h"

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

class CloudConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {
        std::string uri = element.GetURI();
        LBSLOGI(NETWORK, "Connected uri is %{public}s, result is %{public}d.", uri.c_str(), resultCode);
        if (resultCode != ERR_OK) {
            return;
        }
        DelayedSingleton<NetworkAbility>::GetInstance().get()->notifyConnected(remoteObject);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int) override
    {
        std::string uri = element.GetURI();
        LBSLOGI(NETWORK, "Disconnected uri is %{public}s.", uri.c_str());
        DelayedSingleton<NetworkAbility>::GetInstance().get()->notifyDisConnected();
    }
};

bool NetworkAbility::ConnectHms()
{
    LBSLOGI(NETWORK, "start ConnectHms");
    std::unique_lock<std::mutex> uniqueLock(connectMutex_);
    if (!connectServiceReady_) {
        AAFwk::Want connectionWant;
        connectionWant.SetElementName(SERVICE_NAME, ABILITY_NAME);
        sptr<AAFwk::IAbilityConnection> cloudConnection = new CloudConnection();
        int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(connectionWant, cloudConnection, -1);
        if (ret != ERR_OK) {
            LBSLOGE(NETWORK, "Connect cloud service failed!");
            return false;
        }

        auto waitStatus = connectCondition_.wait_for(
            uniqueLock, std::chrono::seconds(CONNECT_TIME_OUT), [this]() { return connectServiceReady_; });
        if (!waitStatus) {
            LBSLOGE(NETWORK, "Connect cloudService timeout!");
            return false;
        }
    }
    return true;
}

void NetworkAbility::notifyConnected(const sptr<IRemoteObject>& remoteObject)
{
    std::unique_lock<std::mutex> uniqueLock(connectMutex_);
    connectServiceReady_ = true;
    cloudServiceProxy_ = remoteObject;
    connectCondition_.notify_all();
}

void NetworkAbility::notifyDisConnected()
{
    std::unique_lock<std::mutex> uniqueLock(connectMutex_);
    connectServiceReady_ = false;
    cloudServiceProxy_ = nullptr;
    connectCondition_.notify_all();
}

void NetworkAbility::SendLocationRequest(WorkRecord &workrecord)
{
    LocationRequest(workrecord);
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
    LBSLOGI(NETWORK, "enter RequestRecord. %{public}d", isAdded ? 1 : 0);
    if (!connectServiceReady_ && !ConnectHms()) {
        return;
    }
    LBSLOGI(NETWORK, "test enter RequestRecord, [%{public}s]", workRecord.ToString().c_str());
    if (cloudServiceProxy_ != nullptr) {
        MessageParcel data, reply;
        MessageOption option;
        if (isAdded) {
            sptr<NetworkCallbackHost> callback = new (std::nothrow) NetworkCallbackHost();
            LBSLOGI(NETWORK, "test1126, uuid:%{public}s, timeInterval:%{public}d", workRecord.GetUUid(0).c_str(), workRecord.GetTimeInterval(0));
            data.WriteString16(Str8ToStr16(workRecord.GetUUid(0)));
            data.WriteInt64(workRecord.GetTimeInterval(0) * SEC_TO_MILLI_SEC);
            data.WriteInt32(PRIORITY_TYPE_BALANCED_POWER_ACCURACY);
            data.WriteRemoteObject(callback->AsObject());
            int error = cloudServiceProxy_->SendRequest(REQUEST_NETWORK_LOCATION, data, reply, option);
            if (error != ERR_OK) {
                LBSLOGE(NETWORK, "SendRequest to cloud service failed.");
                return;
            }
            LBSLOGE(NETWORK, "start network location.");
        } else {
            data.WriteString16(Str8ToStr16(workRecord.GetUUid(0)));
            int error = cloudServiceProxy_->SendRequest(REMOVE_NETWORK_LOCATION, data, reply, option);
            if (error != ERR_OK) {
                LBSLOGE(NETWORK, "SendRequest to cloud service failed.");
                return;
            }
            LBSLOGE(NETWORK, "stop network location.");
        } 
    }
}

bool NetworkAbility::EnableMock(const LocationMockConfig& config)
{
    return EnableLocationMock(config);
}

bool NetworkAbility::DisableMock(const LocationMockConfig& config)
{
    return DisableLocationMock(config);
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
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocation(locationNew, NETWORK_ABILITY);
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocation(locationNew, PASSIVE_ABILITY);
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
        default:
            break;
    }
}
} // namespace Location
} // namespace OHOS