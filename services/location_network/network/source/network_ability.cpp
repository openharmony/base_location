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

#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability.h"
#include <file_ex.h>
#include <thread>
#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "want_agent_helper.h"

#include "common_utils.h"
#include "location_config_manager.h"
#include "location_dumper.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "network_callback_host.h"

namespace OHOS {
namespace Location {
const uint32_t EVENT_REPORT_LOCATION = 0x0100;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
constexpr uint32_t WAIT_MS = 100;
const int MAX_RETRY_COUNT = 5;
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

class AbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {
        std::string uri = element.GetURI();
        LBSLOGD(NETWORK, "Connected uri is %{public}s, result is %{public}d.", uri.c_str(), resultCode);
        if (resultCode != ERR_OK) {
            return;
        }
        DelayedSingleton<NetworkAbility>::GetInstance().get()->NotifyConnected(remoteObject);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int) override
    {
        std::string uri = element.GetURI();
        LBSLOGD(NETWORK, "Disconnected uri is %{public}s.", uri.c_str());
        DelayedSingleton<NetworkAbility>::GetInstance().get()->NotifyDisConnected();
    }
};

bool NetworkAbility::ConnectNlpService()
{
    LBSLOGD(NETWORK, "start ConnectNlpService");
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    if (!nlpServiceReady_) {
        AAFwk::Want connectionWant;
        std::string name;
        bool result = LocationConfigManager::GetInstance().GetNlpServiceName(SERVICE_CONFIG_FILE, name);
        if (!result || name.empty()) {
            LBSLOGE(NETWORK, "get service name failed!");
            return false;
        }
        connectionWant.SetElementName(name, ABILITY_NAME);
        sptr<AAFwk::IAbilityConnection> conn = new (std::nothrow) AbilityConnection();
        if (conn == nullptr) {
            LBSLOGE(NETWORK, "get connection failed!");
            return false;
        }
        int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(connectionWant, conn, -1);
        if (ret != ERR_OK) {
            LBSLOGE(NETWORK, "Connect cloud service failed!");
            return false;
        }

        auto waitStatus = connectCondition_.wait_for(
            uniqueLock, std::chrono::seconds(CONNECT_TIME_OUT), [this]() { return nlpServiceReady_; });
        if (!waitStatus) {
            LBSLOGE(NETWORK, "Connect cloudService timeout!");
            return false;
        }
    }
    return true;
}

bool NetworkAbility::ReConnectNlpService()
{
    int retryCount = 0;
    if (nlpServiceReady_) {
        LBSLOGI(NETWORK, "Connect success!");
        return true;
    }
    while (retryCount < MAX_RETRY_COUNT) {
        retryCount++;
        bool ret = ConnectNlpService();
        if (ret) {
            LBSLOGI(NETWORK, "Connect success!");
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }
    return false;
}

void NetworkAbility::NotifyConnected(const sptr<IRemoteObject>& remoteObject)
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    nlpServiceReady_ = true;
    nlpServiceProxy_ = remoteObject;
    connectCondition_.notify_all();
}

void NetworkAbility::NotifyDisConnected()
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    nlpServiceReady_ = false;
    nlpServiceProxy_ = nullptr;
    connectCondition_.notify_all();
}

LocationErrCode NetworkAbility::SendLocationRequest(WorkRecord &workrecord)
{
    LocationRequest(workrecord);
    return ERRCODE_SUCCESS;
}

LocationErrCode NetworkAbility::SetEnable(bool state)
{
    if (state) {
        Enable(true, AsObject());
        return ERRCODE_SUCCESS;
    }
    if (!CheckIfNetworkConnecting()) {
        Enable(false, AsObject());
    }
    return ERRCODE_SUCCESS;
}

void NetworkAbility::UnloadNetworkSystemAbility()
{
    if (!CheckIfNetworkConnecting()) {
        LocationSaLoadManager::GetInstance().UnloadLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
    }
}

bool NetworkAbility::CheckIfNetworkConnecting()
{
    return IsMockEnabled() || !GetLocationMock().empty() || GetRequestNum() != 0;
}

LocationErrCode NetworkAbility::SelfRequest(bool state)
{
    LBSLOGI(NETWORK, "SelfRequest %{public}d", state);
    HandleSelfRequest(IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), state);
    return ERRCODE_SUCCESS;
}

void NetworkAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    if (!nlpServiceReady_) {
        std::string name;
        bool result = LocationConfigManager::GetInstance().GetNlpServiceName(SERVICE_CONFIG_FILE, name);
        if (!result || name.empty()) {
            LBSLOGE(NETWORK, "get service name failed!");
            return;
        }
        if (!CommonUtils::CheckAppInstalled(name)) { // app is not installed
            LBSLOGE(NETWORK, "nlp service is not available.");
            return;
        } else if (!ReConnectNlpService()) {
            LBSLOGE(NETWORK, "nlp service is not ready.");
            return;
        }
    }
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    if (nlpServiceProxy_ == nullptr) {
        LBSLOGE(NETWORK, "nlpProxy is nullptr.");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (isAdded) {
        LBSLOGD(NETWORK, "start network location");
        sptr<NetworkCallbackHost> callback = new (std::nothrow) NetworkCallbackHost();
        if (callback == nullptr) {
            LBSLOGE(NETWORK, "can not get valid callback.");
            return;
        }
        data.WriteString16(Str8ToStr16(workRecord.GetUuid(0)));
        data.WriteInt64(workRecord.GetTimeInterval(0) * SEC_TO_MILLI_SEC);
        data.WriteInt32(LocationRequestType::PRIORITY_TYPE_BALANCED_POWER_ACCURACY);
        data.WriteRemoteObject(callback->AsObject());
        int error = nlpServiceProxy_->SendRequest(REQUEST_NETWORK_LOCATION, data, reply, option);
        if (error != ERR_OK) {
            LBSLOGE(NETWORK, "SendRequest to cloud service failed.");
            return;
        }
    } else {
        LBSLOGD(NETWORK, "stop network location");
        data.WriteString16(Str8ToStr16(workRecord.GetUuid(0)));
        int error = nlpServiceProxy_->SendRequest(REMOVE_NETWORK_LOCATION, data, reply, option);
        if (error != ERR_OK) {
            LBSLOGE(NETWORK, "SendRequest to cloud service failed.");
            return;
        }
    }
}

LocationErrCode NetworkAbility::EnableMock()
{
    if (!EnableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode NetworkAbility::DisableMock()
{
    if (!DisableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

bool NetworkAbility::IsMockEnabled()
{
    return IsLocationMocked();
}

LocationErrCode NetworkAbility::SetMocked(const int timeInterval,
    const std::vector<std::shared_ptr<Location>> &location)
{
    if (!SetMockedLocations(timeInterval, location)) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

void NetworkAbility::ProcessReportLocationMock()
{
    std::vector<std::shared_ptr<Location>> mockLocationArray = GetLocationMock();
    if (mockLocationIndex_ < mockLocationArray.size()) {
        ReportMockedLocation(mockLocationArray[mockLocationIndex_++]);
        if (networkHandler_ != nullptr) {
            networkHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION,
                0, GetTimeIntervalMock() * EVENT_INTERVAL_UNITE);
        }
    } else {
        ClearLocationMock();
        mockLocationIndex_ = 0;
    }
}

void NetworkAbility::SendReportMockLocationEvent()
{
    if (networkHandler_ == nullptr) {
        return;
    }
    networkHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION, 0, 0);
}

int32_t NetworkAbility::ReportMockedLocation(const std::shared_ptr<Location> location)
{
    if ((IsLocationMocked() && !location->GetIsFromMock()) ||
        (!IsLocationMocked() && location->GetIsFromMock())) {
        LBSLOGE(NETWORK, "location mock is enabled, do not report gnss location!");
        return ERR_OK;
    }
    ReportLocationInfo(NETWORK_ABILITY, location);
#ifdef FEATURE_PASSIVE_SUPPORT
    ReportLocationInfo(PASSIVE_ABILITY, location);
#endif
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
    if (networkHandler_ == nullptr) {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return;
    }
    switch (code) {
        case SEND_LOCATION_REQUEST: {
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, workrecord);
            if (networkHandler_->SendEvent(event)) {
                reply.WriteInt32(ERRCODE_SUCCESS);
            } else {
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            }
            break;
        }
        case SET_MOCKED_LOCATIONS: {
            if (!IsMockEnabled()) {
                reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
                break;
            }
            int timeInterval = data.ReadInt32();
            int locationSize = data.ReadInt32();
            locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
                locationSize;
            std::shared_ptr<std::vector<std::shared_ptr<Location>>> vcLoc =
                std::make_shared<std::vector<std::shared_ptr<Location>>>();
            for (int i = 0; i < locationSize; i++) {
                vcLoc->push_back(Location::UnmarshallingShared(data));
            }
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, vcLoc, timeInterval);
            if (networkHandler_->SendEvent(event)) {
                reply.WriteInt32(ERRCODE_SUCCESS);
            } else {
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            }
            break;
        }
        case SELF_REQUEST: {
            int64_t param = data.ReadBool() ? 1 : 0;
            networkHandler_->SendEvent(code, param, 0) ? reply.WriteInt32(ERRCODE_SUCCESS) :
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
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
    auto networkAbility = DelayedSingleton<NetworkAbility>::GetInstance();
    if (networkAbility == nullptr) {
        LBSLOGE(NETWORK, "ProcessEvent: NetworkAbility is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(NETWORK, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_REPORT_LOCATION: {
            networkAbility->ProcessReportLocationMock();
            break;
        }
        case ISubAbility::SEND_LOCATION_REQUEST: {
            std::unique_ptr<WorkRecord> workrecord = event->GetUniqueObject<WorkRecord>();
            if (workrecord != nullptr) {
                networkAbility->LocationRequest(*workrecord);
            }
            break;
        }
        case ISubAbility::SET_MOCKED_LOCATIONS: {
            int timeInterval = event->GetParam();
            auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
            if (vcLoc != nullptr) {
                std::vector<std::shared_ptr<Location>> mockLocations;
                for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
                    mockLocations.push_back(*it);
                }
                networkAbility->SetMocked(timeInterval, mockLocations);
            }
            break;
        }
        case ISubAbility::SELF_REQUEST: {
            bool state = event->GetParam();
            networkAbility->SelfRequest(state);
            // no need unload sa, return
            return;
        }
        default:
            break;
    }
    networkAbility->UnloadNetworkSystemAbility();
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_NETWORK_SUPPORT
