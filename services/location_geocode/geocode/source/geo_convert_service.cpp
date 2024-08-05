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

#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_convert_service.h"
#include <file_ex.h>
#include <thread>
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "geo_address.h"
#include "geo_convert_request.h"
#include "common_utils.h"
#include "location_config_manager.h"
#include "location_dumper.h"
#include "location_sa_load_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    GeoConvertService::GetInstance());
const uint32_t EVENT_SEND_GEOREQUEST = 0x0100;
const char* UNLOAD_GEOCONVERT_TASK = "geoconvert_sa_unload";
const int GEOCONVERT_CONNECT_TIME_OUT = 1;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
const int UNLOAD_GEOCONVERT_DELAY_TIME = 5 * EVENT_INTERVAL_UNITE;
GeoConvertService* GeoConvertService::GetInstance()
{
    static GeoConvertService data;
    return &data;
}

GeoConvertService::GeoConvertService() : SystemAbility(LOCATION_GEO_CONVERT_SA_ID, true)
{
    geoConvertHandler_ =
        std::make_shared<GeoConvertHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    LBSLOGI(GEO_CONVERT, "GeoConvertService constructed.");
}

GeoConvertService::~GeoConvertService()
{
    if (geoConvertHandler_ != nullptr) {
        geoConvertHandler_->RemoveTask(UNLOAD_GEOCONVERT_TASK);
    }
    conn_ = nullptr;
}

void GeoConvertService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(GEO_CONVERT, "GeoConvertService has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(GEO_CONVERT, "failed to init GeoConvertService");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    LBSLOGI(GEO_CONVERT, "GeoConvertService::OnStart start service success.");
}

void GeoConvertService::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    if (conn_ != nullptr) {
        AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn_);
        LBSLOGD(GEO_CONVERT, "GeoConvertService::OnStop and disconnect");
        UnRegisterGeoServiceDeathRecipient();
        SetServiceConnectState(ServiceConnectState::STATE_DISCONNECT);
        conn_ = nullptr;
    }
    LBSLOGI(GEO_CONVERT, "GeoConvertService::OnStop service stopped.");
}

bool GeoConvertService::Init()
{
    if (!registerToService_) {
        bool ret = Publish(AsObject());
        if (!ret) {
            LBSLOGE(GEO_CONVERT, "GeoConvertService::Init Publish failed!");
            return false;
        }
        registerToService_ = true;
    }
    return true;
}

class AbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {
        std::string uri = element.GetURI();
        LBSLOGD(GEO_CONVERT, "Connected uri is %{public}s, result is %{public}d.", uri.c_str(), resultCode);
        if (resultCode != ERR_OK) {
            return;
        }
        GeoConvertService::GetInstance()->NotifyConnected(remoteObject);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int) override
    {
        std::string uri = element.GetURI();
        LBSLOGD(GEO_CONVERT, "Disconnected uri is %{public}s.", uri.c_str());
        GeoConvertService::GetInstance()->NotifyDisConnected();
    }
};

bool GeoConvertService::ConnectService()
{
    LBSLOGD(GEO_CONVERT, "start ConnectService");
    AAFwk::Want connectionWant;
    std::string serviceName;
    bool result = LocationConfigManager::GetInstance()->GetGeocodeServiceName(serviceName);
    if (!result || serviceName.empty()) {
        LBSLOGE(GEO_CONVERT, "get service name failed!");
        return false;
    }
    std::string abilityName;
    bool res = LocationConfigManager::GetInstance()->GetGeocodeAbilityName(abilityName);
    if (!res || abilityName.empty()) {
        LBSLOGE(GEO_CONVERT, "get service name failed!");
        return false;
    }
    connectionWant.SetElementName(serviceName, abilityName);
    conn_ = sptr<AAFwk::IAbilityConnection>(new (std::nothrow) AbilityConnection());
    if (conn_ == nullptr) {
        LBSLOGE(GEO_CONVERT, "get connection failed!");
        return false;
    }
    SetServiceConnectState(ServiceConnectState::STATE_CONNECTTING);
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(connectionWant, conn_, -1);
    if (ret != ERR_OK) {
        LBSLOGE(GEO_CONVERT, "Connect cloud service failed!");
        return false;
    }
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    auto waitStatus =
        connectCondition_.wait_for(uniqueLock,
        std::chrono::seconds(GEOCONVERT_CONNECT_TIME_OUT), [this]() { return serviceProxy_ != nullptr; });
    if (!waitStatus) {
        LBSLOGE(GEO_CONVERT, "Connect cloudService timeout!");
        SetServiceConnectState(ServiceConnectState::STATE_DISCONNECT);
        return false;
    }
    SetServiceConnectState(ServiceConnectState::STATE_CONNECTTED);
    RegisterGeoServiceDeathRecipient();
    return true;
}

void GeoConvertService::NotifyConnected(const sptr<IRemoteObject>& remoteObject)
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    serviceProxy_ = remoteObject;
    connectCondition_.notify_all();
}

void GeoConvertService::NotifyDisConnected()
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    serviceProxy_ = nullptr;
    connectCondition_.notify_all();
}

int GeoConvertService::IsGeoConvertAvailable(MessageParcel &reply)
{
    std::string serviceName;
    bool result = LocationConfigManager::GetInstance()->GetGeocodeServiceName(serviceName);
    if (!result || serviceName.empty()) {
        LBSLOGE(GEO_CONVERT, "get service name failed!");
        reply.WriteInt32(ERRCODE_SUCCESS);
        reply.WriteBool(false);
        return ERRCODE_SUCCESS;
    }
    reply.WriteInt32(ERRCODE_SUCCESS);
    if (!CommonUtils::CheckAppInstalled(serviceName)) { // app is not installed
        reply.WriteBool(false);
    } else {
        reply.WriteBool(true);
    }
    return ERRCODE_SUCCESS;
}

int GeoConvertService::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    if (mockEnabled_) {
        ReportAddressMock(data, reply);
        return ERRCODE_SUCCESS;
    }
    if (!GetService()) {
        reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
        return ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    GeoCodeType requestType = GeoCodeType::REQUEST_REVERSE_GEOCODE;
    auto geoConvertRequest = GeoConvertRequest::Unmarshalling(data, requestType);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_SEND_GEOREQUEST, geoConvertRequest);
    if (geoConvertHandler_ != nullptr) {
        geoConvertHandler_->SendEvent(event);
    }
    return ERRCODE_SUCCESS;
}

void GeoConvertService::ReportAddressMock(MessageParcel &data, MessageParcel &reply)
{
    int arraySize = 0;
    std::vector<std::shared_ptr<GeoAddress>> array;
    ReverseGeocodeRequest request;
    request.latitude = data.ReadDouble();
    request.longitude = data.ReadDouble();
    request.maxItems = data.ReadInt32();
    data.ReadInt32(); // locale size
    request.locale = Str16ToStr8(data.ReadString16());
    std::unique_lock<std::mutex> lock(mockInfoMutex_, std::defer_lock);
    lock.lock();
    for (size_t i = 0; i < mockInfo_.size(); i++) {
        std::shared_ptr<GeocodingMockInfo> info = mockInfo_[i];
        if (!CommonUtils::DoubleEqual(request.latitude, info->GetLocation()->latitude) ||
            !CommonUtils::DoubleEqual(request.longitude, info->GetLocation()->longitude)) {
            continue;
        }
        arraySize++;
        array.push_back(info->GetGeoAddressInfo());
    }
    lock.unlock();
    reply.WriteInt32(ERRCODE_SUCCESS);
    if (arraySize > 0) {
        reply.WriteInt32(arraySize);
        for (size_t i = 0; i < array.size(); i++) {
            array[i]->Marshalling(reply);
        }
    } else {
        reply.WriteInt32(0);
    }
}

int GeoConvertService::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    if (!GetService()) {
        reply.WriteInt32(ERRCODE_GEOCODING_FAIL);
        return ERRCODE_GEOCODING_FAIL;
    }
    GeoCodeType requestType = GeoCodeType::REQUEST_GEOCODE;
    auto geoConvertRequest = GeoConvertRequest::Unmarshalling(data, requestType);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_SEND_GEOREQUEST, geoConvertRequest);
    if (geoConvertHandler_ != nullptr) {
        geoConvertHandler_->SendEvent(event);
    }
    return ERRCODE_SUCCESS;
}

bool GeoConvertService::GetService()
{
    if (!IsConnect() && !IsConnecting()) {
        std::string serviceName;
        bool result = LocationConfigManager::GetInstance()->GetGeocodeServiceName(serviceName);
        if (!result || serviceName.empty()) {
            LBSLOGE(GEO_CONVERT, "get service name failed!");
            return false;
        }
        if (!CommonUtils::CheckAppInstalled(serviceName)) { // app is not installed
            LBSLOGE(GEO_CONVERT, "service is not available.");
            return false;
        } else if (!ConnectService()) {
            return false;
        }
    }
    return true;
}

bool GeoConvertService::IsConnect()
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    return serviceProxy_ != nullptr &&
        GetServiceConnectState() == ServiceConnectState::STATE_CONNECTTED;
}

bool GeoConvertService::EnableReverseGeocodingMock()
{
    LBSLOGD(GEO_CONVERT, "EnableReverseGeocodingMock");
    mockEnabled_ = true;
    return true;
}

bool GeoConvertService::DisableReverseGeocodingMock()
{
    LBSLOGD(GEO_CONVERT, "DisableReverseGeocodingMock");
    mockEnabled_ = false;
    return true;
}

LocationErrCode GeoConvertService::SetReverseGeocodingMockInfo(
    std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    LBSLOGD(GEO_CONVERT, "SetReverseGeocodingMockInfo");
    std::unique_lock<std::mutex> lock(mockInfoMutex_, std::defer_lock);
    lock.lock();
    mockInfo_.assign(mockInfo.begin(), mockInfo.end());
    lock.unlock();
    return ERRCODE_SUCCESS;
}

bool GeoConvertService::CancelIdleState()
{
    bool ret = CancelIdle();
    if (!ret) {
        LBSLOGE(GEO_CONVERT, "%{public}s cancel idle failed!", __func__);
        return false;
    }
    return true;
}

void GeoConvertService::UnloadGeoConvertSystemAbility()
{
    if (geoConvertHandler_ == nullptr) {
        LBSLOGE(GEO_CONVERT, "%{public}s geoConvertHandler_ is nullptr", __func__);
        return;
    }
    geoConvertHandler_->RemoveTask(UNLOAD_GEOCONVERT_TASK);
    if (CheckIfGeoConvertConnecting()) {
        return;
    }
    auto task = [this]() {
        LocationSaLoadManager::UnInitLocationSa(LOCATION_GEO_CONVERT_SA_ID);
        GeoConvertService::GetInstance()->DisconnectAbilityConnect();
    };
    geoConvertHandler_->PostTask(task, UNLOAD_GEOCONVERT_TASK, UNLOAD_GEOCONVERT_DELAY_TIME);
}

void GeoConvertService::DisconnectAbilityConnect()
{
    if (conn_ != nullptr) {
        AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn_);
        UnRegisterGeoServiceDeathRecipient();
        SetServiceConnectState(ServiceConnectState::STATE_DISCONNECT);
        conn_ = nullptr;
        LBSLOGI(GEO_CONVERT, "UnloadGeoConvert OnStop and disconnect");
    }
}

bool GeoConvertService::CheckIfGeoConvertConnecting()
{
    return mockEnabled_;
}

void GeoConvertService::SaDumpInfo(std::string& result)
{
    result += "GeoConvert enable status: false";
    result += "\n";
}

int32_t GeoConvertService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.GeocodeDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(GEO_CONVERT, "Geocode save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}

bool GeoConvertService::ResetServiceProxy()
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    serviceProxy_ = nullptr;
    return true;
}

void GeoConvertService::RegisterGeoServiceDeathRecipient()
{
    if (serviceProxy_ == nullptr) {
        LBSLOGE(GEO_CONVERT, "%{public}s: geoServiceProxy_ is nullptr", __func__);
        return;
    }
    if (geoServiceRecipient_ != nullptr) {
        serviceProxy_->AddDeathRecipient(geoServiceRecipient_);
    }
}

void GeoConvertService::UnRegisterGeoServiceDeathRecipient()
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    if (serviceProxy_ == nullptr) {
        LBSLOGE(GEO_CONVERT, "%{public}s: geoServiceProxy_ is nullptr", __func__);
        return;
    }
    if (geoServiceRecipient_ != nullptr) {
        serviceProxy_->RemoveDeathRecipient(geoServiceRecipient_);
        geoServiceRecipient_ = nullptr;
    }
}

bool GeoConvertService::SendGeocodeRequest(int code, MessageParcel& dataParcel, MessageParcel& replyParcel,
    MessageOption& option)
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    if (serviceProxy_ == nullptr) {
        LBSLOGE(GEO_CONVERT, "serviceProxy is nullptr!");
        return false;
    }
    int error = serviceProxy_->SendRequest(code, dataParcel, replyParcel, option);
    if (error != ERR_OK) {
        LBSLOGE(GEO_CONVERT, "SendRequest to cloud service failed. error = %{public}d", error);
        return false;
    }
    return true;
}

ServiceConnectState GeoConvertService::GetServiceConnectState()
{
    std::unique_lock<std::mutex> uniqueLock(connectStateMutex_);
    return connectState_;
}

void GeoConvertService::SetServiceConnectState(ServiceConnectState connectState)
{
    std::unique_lock<std::mutex> uniqueLock(connectStateMutex_);
    connectState_ = connectState;
}

bool GeoConvertService::IsConnecting()
{
    std::unique_lock<std::mutex> uniqueLock(connectStateMutex_);
    return connectState_ == ServiceConnectState::STATE_CONNECTTING;
}

GeoServiceDeathRecipient::GeoServiceDeathRecipient()
{
}

GeoServiceDeathRecipient::~GeoServiceDeathRecipient()
{
}

void GeoServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto geoConvertService = GeoConvertService::GetInstance();
    if (geoConvertService != nullptr) {
        LBSLOGI(GEO_CONVERT, "geo OnRemoteDied");
        geoConvertService->ResetServiceProxy();
        geoConvertService->SetServiceConnectState(ServiceConnectState::STATE_DISCONNECT);
    }
}

GeoConvertHandler::GeoConvertHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

GeoConvertHandler::~GeoConvertHandler() {}

void GeoConvertHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto geoConvertService = GeoConvertService::GetInstance();
    if (geoConvertService == nullptr) {
        LBSLOGE(NETWORK, "ProcessEvent: GeoConvertService is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(GEO_CONVERT, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_SEND_GEOREQUEST: {
            std::unique_ptr<GeoConvertRequest> geoConvertRequest = event->GetUniqueObject<GeoConvertRequest>();
            if (geoConvertRequest == nullptr) {
                return;
            }
            MessageParcel dataParcel;
            MessageParcel replyParcel;
            MessageOption option;
            geoConvertRequest->Marshalling(dataParcel);
            bool ret = geoConvertService->SendGeocodeRequest(static_cast<int>(geoConvertRequest->GetRequestType()),
                dataParcel, replyParcel, option);
            if (!ret) {
                LBSLOGE(GEO_CONVERT, "SendGeocodeRequest failed errcode");
                return;
            }
            break;
        }
        default:
            break;
    }
}
} // namespace Location
} // namespace OHOS
#endif
