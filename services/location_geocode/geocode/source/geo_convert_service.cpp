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
#include "common_utils.h"
#include "location_config_manager.h"
#include "location_dumper.h"
#include "location_sa_load_manager.h"
#include "system_ability_definition.h"
#ifdef LOCATION_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    GeoConvertService::GetInstance());
const uint32_t EVENT_SEND_GEOREQUEST = 0x0100;
const char* UNLOAD_GEOCONVERT_TASK = "geoconvert_sa_unload";
const int GEOCONVERT_CONNECT_TIME_OUT = 5;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
const int UNLOAD_GEOCONVERT_DELAY_TIME = 10 * EVENT_INTERVAL_UNITE;
const int TIMEOUT_WATCHDOG = 60; // s
const int MAX_CACHED_VALID_DISTANCE = 100; // m
const int MAX_CACHED_NUM = 10;
static const int MAX_RESULT = 10;

GeoConvertService* GeoConvertService::GetInstance()
{
    static GeoConvertService data;
    return &data;
}

GeoConvertService::GeoConvertService() : SystemAbility(LOCATION_GEO_CONVERT_SA_ID, true)
{
#ifndef TDD_CASES_ENABLED
    geoConvertHandler_ =
        std::make_shared<GeoConvertHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
#endif
    LBSLOGI(GEO_CONVERT, "GeoConvertService constructed.");
}

GeoConvertService::~GeoConvertService()
{
#ifndef TDD_CASES_ENABLED
    if (geoConvertHandler_ != nullptr) {
        geoConvertHandler_->RemoveTask(UNLOAD_GEOCONVERT_TASK);
    }
#endif
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

bool GeoConvertService::CheckGeoConvertAvailable()
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
        }
        std::string abilityName;
        bool res = LocationConfigManager::GetInstance()->GetGeocodeAbilityName(abilityName);
        if (!res || abilityName.empty()) {
            LBSLOGE(GEO_CONVERT, "get ability name failed!");
            return false;
        }
    }
    return true;
}

int GeoConvertService::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    if (mockEnabled_) {
        ReportAddressMock(data, reply);
        return ERRCODE_SUCCESS;
    }
    if (!CheckGeoConvertAvailable()) {
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
    request.locale = Str16ToStr8(data.ReadString16());
    request.latitude = data.ReadDouble();
    request.longitude = data.ReadDouble();
    request.maxItems = data.ReadInt32();
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
    if (!CheckGeoConvertAvailable()) {
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
    SystemAbilityState state = GetAbilityState();
    if (state != SystemAbilityState::IDLE) {
        return true;
    }
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
    auto task = [this]() {
        if (CheckIfGeoConvertConnecting()) {
            return;
        }
        SaLoadWithStatistic::UnInitLocationSa(LOCATION_GEO_CONVERT_SA_ID);
        GeoConvertService::GetInstance()->DisconnectAbilityConnect();
    };
    geoConvertHandler_->PostTask(task, UNLOAD_GEOCONVERT_TASK, UNLOAD_GEOCONVERT_DELAY_TIME);
}

void GeoConvertService::DisconnectAbilityConnect()
{
    if (conn_ != nullptr) {
        UnRegisterGeoServiceDeathRecipient();
        AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn_);
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
    if (!GetService()) {
        LBSLOGE(GEO_CONVERT, "GetService error!");
        return false;
    }
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    if (serviceProxy_ == nullptr) {
        LBSLOGE(GEO_CONVERT, "serviceProxy is nullptr!");
        return false;
    }
    MessageParcel data;
    data.WriteInterfaceToken(serviceProxy_->GetInterfaceDescriptor());
    data.Append(dataParcel);
    int error = serviceProxy_->SendRequest(code, data, replyParcel, option);
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

void GeoConvertService::AddCahedGeoAddress(GeoConvertRequest geoConvertRequest, MessageParcel& dataParcel)
{
    auto geoAddressList = GetCahedGeoAddress(std::make_unique<GeoConvertRequest>(geoConvertRequest));
    if (geoAddressList.size() > 0) {
        return;
    }
    int errCode = dataParcel.ReadInt32();
    if (errCode != 0) {
        LBSLOGE(GEO_CONVERT, "something wrong, errCode = %{public}d", errCode);
        return;
    }
    int cnt = dataParcel.ReadInt32();
    if (cnt > MAX_RESULT) {
        cnt = MAX_RESULT;
    }
    std::list<std::shared_ptr<GeoAddress>> result;
    for (int i = 0; i < cnt; i++) {
        auto geoAddress = GeoAddress::Unmarshalling(dataParcel);
        if (geoAddress->placeName_.empty()) {
            return;
        }
        result.push_back(std::make_shared<GeoAddress>(*geoAddress));
    }
    std::unique_lock<std::mutex> uniqueLock(cachedGeoAddressMapListMutex_);
    if (static_cast<int32_t>(cachedGeoAddressMapList_.size()) >= MAX_CACHED_NUM) {
        DeleteAgedGeoAddress();
    }
    geoConvertRequest.SetTimeStamp(CommonUtils::GetCurrentTimeStamp());
    cachedGeoAddressMapList_[std::make_shared<GeoConvertRequest>(geoConvertRequest)] = result;
}

std::list<std::shared_ptr<GeoAddress>> GeoConvertService::GetCahedGeoAddress(
    std::unique_ptr<GeoConvertRequest> geoConvertRequest)
{
    std::unique_lock<std::mutex> uniqueLock(cachedGeoAddressMapListMutex_);
    std::list<std::shared_ptr<GeoAddress>> result;
    for (auto iter = cachedGeoAddressMapList_.begin(); iter != cachedGeoAddressMapList_.end(); ++iter) {
        auto request = iter->first;
        if (request->GetLocale() != geoConvertRequest->GetLocale()) {
            continue;
        }
        if (Location::GetDistanceBetweenLocations(request->GetLatitude(), request->GetLongitude(),
            geoConvertRequest->GetLatitude(), geoConvertRequest->GetLongitude()) > MAX_CACHED_VALID_DISTANCE) {
            continue;
        }
        result = iter->second;
        request->SetTimeStamp(CommonUtils::GetCurrentTimeStamp());
        request->SetPriority(request->GetPriority() + 1);
    }
    return result;
}

void GeoConvertService::DeleteAgedGeoAddress()
{
    int minPriority = 0;
    int minTimeStamp = 0;
    auto iterDelete = cachedGeoAddressMapList_.begin();
    for (auto iter = cachedGeoAddressMapList_.begin(); iter != cachedGeoAddressMapList_.end(); ++iter) {
        auto geoConvertRequest = iter->first;
        if (geoConvertRequest->GetPriority() < minPriority ||
            (geoConvertRequest->GetPriority() == minPriority && geoConvertRequest->GetTimeStamp() < minTimeStamp)) {
            minPriority = geoConvertRequest->GetPriority();
            minTimeStamp = geoConvertRequest->GetTimeStamp();
            iterDelete = iter;
        }
    }
    cachedGeoAddressMapList_.erase(iterDelete);
}

void GeoConvertService::SendCacheAddressToRequest(
    std::unique_ptr<GeoConvertRequest> geoConvertRequest, std::list<std::shared_ptr<GeoAddress>> result)
{
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    dataParcel.WriteInterfaceToken(geoConvertRequest->GetCallback()->GetInterfaceDescriptor());
    WriteResultToParcel(result, dataParcel);
    int32_t errCode = geoConvertRequest->GetCallback()->SendRequest(
        GeoCodeCallback::RECEIVE_GEOCODE_INFO_EVENT, dataParcel, reply, option);
    LBSLOGD(GEO_CONVERT, "SendRequest RECEIVE_GEOCODE_INFO_EVENT, errCode=%{public}d", errCode);
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

GeoConvertHandler::GeoConvertHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitGeoConvertHandlerEventMap();
}

GeoConvertHandler::~GeoConvertHandler() {}

void GeoConvertHandler::InitGeoConvertHandlerEventMap()
{
    if (geoConvertHandlerEventMap_.size() != 0) {
        return;
    }
    geoConvertHandlerEventMap_[EVENT_SEND_GEOREQUEST] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { SendGeocodeRequest(event); };
}

void GeoConvertHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(GEO_CONVERT, "ProcessEvent event:%{public}d", eventId);
    auto handleFunc = geoConvertHandlerEventMap_.find(eventId);
    if (handleFunc != geoConvertHandlerEventMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
#ifdef LOCATION_HICOLLIE_ENABLE
        int tid = gettid();
        std::string moduleName = "GeoConvertHandler";
        XCollieCallback callbackFunc = [moduleName, eventId, tid](void *) {
            LBSLOGE(GEO_CONVERT,
                "TimeoutCallback tid:%{public}d moduleName:%{public}s excute eventId:%{public}u timeout.",
                tid, moduleName.c_str(), eventId);
        };
        std::string dfxInfo = moduleName + "_" + std::to_string(eventId) + "_" + std::to_string(tid);
        int timerId = HiviewDFX::XCollie::GetInstance().SetTimer(dfxInfo, TIMEOUT_WATCHDOG, callbackFunc, nullptr,
            HiviewDFX::XCOLLIE_FLAG_LOG|HiviewDFX::XCOLLIE_FLAG_RECOVERY);
        memberFunc(event);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
#else
        memberFunc(event);
#endif
    } else {
        LBSLOGE(GEO_CONVERT, "ProcessEvent event:%{public}d, unsupport service.", eventId);
    }
}

void GeoConvertHandler::SendGeocodeRequest(const AppExecFwk::InnerEvent::Pointer& event)
{
    std::unique_ptr<GeoConvertRequest> geoConvertRequest = event->GetUniqueObject<GeoConvertRequest>();
    if (geoConvertRequest == nullptr) {
        return;
    }
    auto geoConvertService = GeoConvertService::GetInstance();
    if (geoConvertRequest->GetRequestType() == GeoCodeType::REQUEST_REVERSE_GEOCODE) {
        auto result = geoConvertService->GetCahedGeoAddress(
            std::make_unique<GeoConvertRequest>(*geoConvertRequest));
        if (result.size() > 0) {
            geoConvertService->SendCacheAddressToRequest(
                std::make_unique<GeoConvertRequest>(*geoConvertRequest), result);
            return;
        }
    }
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    sptr<GeoCodeCallback> callback = new GeoCodeCallback();
    callback->request_ = *geoConvertRequest;
    geoConvertRequest->SetCallback(callback);
    geoConvertRequest->Marshalling(dataParcel);
    bool ret = geoConvertService->SendGeocodeRequest(static_cast<int>(geoConvertRequest->GetRequestType()),
        dataParcel, replyParcel, option);
    if (!ret) {
        LBSLOGE(GEO_CONVERT, "SendGeocodeRequest failed errcode");
    }
}

bool GeoConvertService::WriteResultToParcel(const std::list<std::shared_ptr<GeoAddress>> result,
    MessageParcel& data)
{
    data.WriteInt32(ERRCODE_SUCCESS);
    data.WriteInt32(result.size());
    for (auto iter = result.begin(); iter != result.end(); iter++) {
        std::shared_ptr<GeoAddress> address = *iter;
        if (address != nullptr) {
            address->Marshalling(data);
        }
    }
    return true;
}


int GeoCodeCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(GEO_CONVERT, "GeoCodeCallback::OnRemoteRequest, code=%{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GEO_CONVERT, "invalid token.");
        return -1;
    }
    switch (code) {
        case RECEIVE_GEOCODE_INFO_EVENT: {
            int32_t errCode =
                request_.GetCallback()->SendRequest(RECEIVE_GEOCODE_INFO_EVENT, data, reply, option);
            LBSLOGD(GEO_CONVERT, "SendRequest RECEIVE_GEOCODE_INFO_EVENT, errCode=%{public}d", errCode);
            if (request_.GetRequestType() == GeoCodeType::REQUEST_REVERSE_GEOCODE) {
                GeoConvertService::GetInstance()->AddCahedGeoAddress(request_, data);
            }
            break;
        }
        case ERROR_INFO_EVENT: {
            int32_t errCode = request_.GetCallback()->SendRequest(ERROR_INFO_EVENT, data, reply, option);
            LBSLOGD(GEO_CONVERT, "SendRequest ERROR_INFO_EVENT, errCode=%{public}d", errCode);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void GeoCodeCallback::OnResults(std::list<std::shared_ptr<GeoAddress>> &results)
{
}

void GeoCodeCallback::OnErrorReport(const int errorCode)
{
}
} // namespace Location
} // namespace OHOS
#endif
