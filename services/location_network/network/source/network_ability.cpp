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
#include "locationhub_ipc_interface_code.h"
#include "location_log_event_ids.h"
#include "common_hisysevent.h"
#include "location_data_rdb_manager.h"

namespace OHOS {
namespace Location {
const uint32_t EVENT_REPORT_MOCK_LOCATION = 0x0100;
const uint32_t EVENT_RESTART_ALL_LOCATION_REQUEST = 0x0200;
const uint32_t EVENT_STOP_ALL_LOCATION_REQUEST = 0x0300;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
constexpr uint32_t WAIT_MS = 100;
const int MAX_RETRY_COUNT = 5;
const std::string UNLOAD_NETWORK_TASK = "network_sa_unload";
const uint32_t RETRY_INTERVAL_OF_UNLOAD_SA = 4 * 60 * EVENT_INTERVAL_UNITE;
const bool REGISTER_RESULT = NetworkAbility::MakeAndRegisterAbility(
    NetworkAbility::GetInstance());

NetworkAbility::NetworkAbility() : SystemAbility(LOCATION_NETWORK_LOCATING_SA_ID, true)
{
    SetAbility(NETWORK_ABILITY);
    networkHandler_ =
        std::make_shared<NetworkHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    LBSLOGI(NETWORK, "ability constructed.");
}

NetworkAbility::~NetworkAbility()
{
    conn_ = nullptr;
}

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

NetworkAbility* NetworkAbility::GetInstance()
{
    static NetworkAbility data;
    return &data;
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
        NetworkAbility::GetInstance()->NotifyConnected(remoteObject);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int) override
    {
        std::string uri = element.GetURI();
        LBSLOGD(NETWORK, "Disconnected uri is %{public}s.", uri.c_str());
        NetworkAbility::GetInstance()->NotifyDisConnected();
    }
};

bool NetworkAbility::ConnectNlpService()
{
    LBSLOGD(NETWORK, "start ConnectNlpService");
    if (!IsConnect()) {
        AAFwk::Want connectionWant;
        std::string serviceName;
        bool result = LocationConfigManager::GetInstance()->GetNlpServiceName(serviceName);
        if (!result || serviceName.empty()) {
            LBSLOGE(NETWORK, "get service name failed!");
            return false;
        }
        std::string abilityName;
        bool res = LocationConfigManager::GetInstance()->GetNlpAbilityName(abilityName);
        if (!res || abilityName.empty()) {
            LBSLOGE(NETWORK, "get service name failed!");
            return false;
        }
        connectionWant.SetElementName(serviceName, abilityName);
        conn_ = sptr<AAFwk::IAbilityConnection>(new (std::nothrow) AbilityConnection());
        if (conn_ == nullptr) {
            LBSLOGE(NETWORK, "get connection failed!");
            return false;
        }
        int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(connectionWant, conn_, -1);
        if (ret != ERR_OK) {
            LBSLOGE(NETWORK, "Connect cloud service failed, ret = %{public}d", ret);
            return false;
        }
        std::unique_lock<ffrt::mutex> uniqueLock(mutex_);
        auto waitStatus = connectCondition_.wait_for(
            uniqueLock, std::chrono::seconds(CONNECT_TIME_OUT), [this]() { return nlpServiceProxy_ != nullptr; });
        if (!waitStatus) {
            WriteLocationInnerEvent(NLP_SERVICE_TIMEOUT, {});
            LBSLOGE(NETWORK, "Connect cloudService timeout!");
            return false;
        }
    }
    RegisterNLPServiceDeathRecipient();
    return true;
}

bool NetworkAbility::ReConnectNlpService()
{
    int retryCount = 0;
    if (IsConnect()) {
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

bool NetworkAbility::ResetServiceProxy()
{
    std::unique_lock<ffrt::mutex> uniqueLock(mutex_);
    nlpServiceProxy_ = nullptr;
    return true;
}

void NetworkAbility::NotifyConnected(const sptr<IRemoteObject>& remoteObject)
{
    std::unique_lock<ffrt::mutex> uniqueLock(mutex_);
    nlpServiceProxy_ = remoteObject;
    connectCondition_.notify_all();
}

void NetworkAbility::NotifyDisConnected()
{
    std::unique_lock<ffrt::mutex> uniqueLock(mutex_);
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
    LBSLOGD(NETWORK, "SetEnable: %{public}d", state);
    if (networkHandler_ == nullptr) {
        LBSLOGE(NETWORK, "%{public}s networkHandler is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto event = state ? AppExecFwk::InnerEvent::Get(EVENT_RESTART_ALL_LOCATION_REQUEST, 0) :
        AppExecFwk::InnerEvent::Get(EVENT_STOP_ALL_LOCATION_REQUEST, 0);
    networkHandler_->SendHighPriorityEvent(event);
    return ERRCODE_SUCCESS;
}

void NetworkAbility::UnloadNetworkSystemAbility()
{
    if (networkHandler_ == nullptr) {
        LBSLOGE(NETWORK, "%{public}s networkHandler is nullptr", __func__);
        return;
    }
    networkHandler_->RemoveTask(UNLOAD_NETWORK_TASK);
    if (CheckIfNetworkConnecting()) {
        return;
    }
    auto task = [this]() {
        LocationSaLoadManager::UnInitLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
    };
    if (networkHandler_ != nullptr) {
        networkHandler_->PostTask(task, UNLOAD_NETWORK_TASK, RETRY_INTERVAL_OF_UNLOAD_SA);
    }
}

bool NetworkAbility::CheckIfNetworkConnecting()
{
    return IsMockEnabled() || !GetLocationMock().empty() || GetRequestNum() != 0;
}

LocationErrCode NetworkAbility::SelfRequest(bool state)
{
    LBSLOGD(NETWORK, "SelfRequest %{public}d", state);
    HandleSelfRequest(IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid(), state);
    return ERRCODE_SUCCESS;
}

void NetworkAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    if (!IsConnect()) {
        std::string serviceName;
        bool result = LocationConfigManager::GetInstance()->GetNlpServiceName(serviceName);
        if (!result || serviceName.empty()) {
            LBSLOGE(NETWORK, "get service name failed!");
            return;
        }
        if (!CommonUtils::CheckAppInstalled(serviceName)) { // app is not installed
            LBSLOGE(NETWORK, "nlp service is not available.");
            return;
        } else if (!ReConnectNlpService()) {
            LBSLOGE(NETWORK, "nlp service is not ready.");
            return;
        }
    }
    std::unique_lock<ffrt::mutex> uniqueLock(mutex_);
    if (isAdded) {
        RequestNetworkLocation(workRecord);
    } else {
        RemoveNetworkLocation(workRecord);
        if (GetRequestNum() == 0 && conn_ != nullptr) {
            LBSLOGD(NETWORK, "RequestRecord disconnect");
            AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn_);
        }
    }
}

bool NetworkAbility::RequestNetworkLocation(WorkRecord &workRecord)
{
    if (nlpServiceProxy_ == nullptr) {
        LBSLOGE(NETWORK, "nlpProxy is nullptr.");
        return false;
    }
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED) {
        LBSLOGE(NETWORK, "QuerySwitchState is DISABLED");
        return false;
    }
    LBSLOGD(NETWORK, "start network location");
    sptr<NetworkCallbackHost> callback = new (std::nothrow) NetworkCallbackHost();
    if (callback == nullptr) {
        LBSLOGE(NETWORK, "can not get valid callback.");
        return false;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteString16(Str8ToStr16(workRecord.GetUuid(0)));
    data.WriteInt64(workRecord.GetTimeInterval(0) * MILLI_PER_SEC);
    data.WriteInt32(workRecord.GetNlpRequestType(0));
    data.WriteRemoteObject(callback->AsObject());
    if (workRecord.GetName(0).size() == 0) {
        data.WriteString16(Str8ToStr16(std::to_string(workRecord.GetUid(0)))); // uid
    } else {
        data.WriteString16(Str8ToStr16(workRecord.GetName(0))); // bundleName
    }
    int error = nlpServiceProxy_->SendRequest(REQUEST_NETWORK_LOCATION, data, reply, option);
    if (error != ERR_OK) {
        LBSLOGE(NETWORK, "SendRequest to cloud service failed. error = %{public}d", error);
        return false;
    }
    return true;
}

bool NetworkAbility::RemoveNetworkLocation(WorkRecord &workRecord)
{
    if (nlpServiceProxy_ == nullptr) {
        LBSLOGE(NETWORK, "nlpProxy is nullptr.");
        return false;
    }
    LBSLOGD(NETWORK, "stop network location");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteString16(Str8ToStr16(workRecord.GetUuid(0)));
    data.WriteString16(Str8ToStr16(workRecord.GetName(0))); // bundleName
    int error = nlpServiceProxy_->SendRequest(REMOVE_NETWORK_LOCATION, data, reply, option);
    if (error != ERR_OK) {
        LBSLOGE(NETWORK, "SendRequest to cloud service failed.");
        return false;
    }
    return true;
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
            networkHandler_->SendHighPriorityEvent(EVENT_REPORT_MOCK_LOCATION,
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
    networkHandler_->SendHighPriorityEvent(EVENT_REPORT_MOCK_LOCATION, 0, 0);
}

int32_t NetworkAbility::ReportMockedLocation(const std::shared_ptr<Location> location)
{
    if ((IsLocationMocked() && !location->GetIsFromMock()) ||
        (!IsLocationMocked() && location->GetIsFromMock())) {
        LBSLOGE(NETWORK, "location mock is enabled, do not report network location!");
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
        case static_cast<uint32_t>(NetworkInterfaceCode::SEND_LOCATION_REQUEST): {
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, workrecord);
            if (networkHandler_->SendEvent(event)) {
                reply.WriteInt32(ERRCODE_SUCCESS);
            } else {
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            }
            break;
        }
        case static_cast<uint32_t>(NetworkInterfaceCode::SET_MOCKED_LOCATIONS): {
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
            if (networkHandler_->SendEvent(event)) {
                reply.WriteInt32(ERRCODE_SUCCESS);
            } else {
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            }
            break;
        }
        case static_cast<uint32_t>(NetworkInterfaceCode::SELF_REQUEST): {
            int64_t param = data.ReadBool() ? 1 : 0;
            networkHandler_->SendEvent(code, param, 0) ? reply.WriteInt32(ERRCODE_SUCCESS) :
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            break;
        }
        default:
            break;
    }
}

void NetworkAbility::RegisterNLPServiceDeathRecipient()
{
    std::unique_lock<ffrt::mutex> uniqueLock(mutex_);
    if (nlpServiceProxy_ == nullptr) {
        LBSLOGE(NETWORK, "%{public}s: nlpServiceProxy_ is nullptr", __func__);
        return;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) NLPServiceDeathRecipient());
    nlpServiceProxy_->AddDeathRecipient(death);
}

bool NetworkAbility::IsConnect()
{
    std::unique_lock<ffrt::mutex> uniqueLock(mutex_);
    return nlpServiceProxy_ != nullptr;
}

void NetworkAbility::ReportLocationError(int32_t errCode, std::string errMsg, std::string uuid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"location.ILocator");
    data.WriteInt32(LOCATING_FAILED_INTERNET_ACCESS_FAILURE);
    data.WriteString(errMsg);
    data.WriteString(uuid);
    sptr<IRemoteObject> objectLocator =
        CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID, CommonUtils::InitDeviceId());
    if (objectLocator == nullptr) {
        LBSLOGE(NETWORK, "%{public}s get locator sa failed", __func__);
        return;
    }
    objectLocator->SendRequest(static_cast<int>(LocatorInterfaceCode::REPORT_LOCATION_ERROR), data, reply, option);
}

NetworkHandler::NetworkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

NetworkHandler::~NetworkHandler() {}

void NetworkHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto networkAbility = NetworkAbility::GetInstance();
    if (networkAbility == nullptr) {
        LBSLOGE(NETWORK, "ProcessEvent: NetworkAbility is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(NETWORK, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_REPORT_MOCK_LOCATION: {
            networkAbility->ProcessReportLocationMock();
            break;
        }
        case EVENT_RESTART_ALL_LOCATION_REQUEST: {
            networkAbility->RestartAllLocationRequests();
            break;
        }
        case EVENT_STOP_ALL_LOCATION_REQUEST: {
            networkAbility->StopAllLocationRequests();
            break;
        }
        case static_cast<uint32_t>(NetworkInterfaceCode::SEND_LOCATION_REQUEST): {
            std::unique_ptr<WorkRecord> workrecord = event->GetUniqueObject<WorkRecord>();
            if (workrecord != nullptr) {
                networkAbility->LocationRequest(*workrecord);
            }
            break;
        }
        case static_cast<uint32_t>(NetworkInterfaceCode::SET_MOCKED_LOCATIONS): {
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
        case static_cast<uint32_t>(NetworkInterfaceCode::SELF_REQUEST): {
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

NLPServiceDeathRecipient::NLPServiceDeathRecipient()
{
}

NLPServiceDeathRecipient::~NLPServiceDeathRecipient()
{
}

void NLPServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto networkAbility = NetworkAbility::GetInstance();
    if (networkAbility != nullptr) {
        LBSLOGI(NETWORK, "nlp ResetServiceProxy");
        networkAbility->ResetServiceProxy();
    }
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_NETWORK_SUPPORT
