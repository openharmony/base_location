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
#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "geo_address.h"
#include "common_utils.h"
#include "location_config_manager.h"
#include "location_dumper.h"
#include "location_sa_load_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<GeoConvertService>::GetInstance().get());
static constexpr int REQUEST_GEOCODE = 1;
static constexpr int REQUEST_REVERSE_GEOCODE = 2;
constexpr uint32_t WAIT_MS = 100;
const int MAX_RETRY_COUNT = 5;
GeoConvertService::GeoConvertService() : SystemAbility(LOCATION_GEO_CONVERT_SA_ID, true)
{
    LBSLOGI(GEO_CONVERT, "GeoConvertService constructed.");
}

GeoConvertService::~GeoConvertService()
{
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
        DelayedSingleton<GeoConvertService>::GetInstance().get()->NotifyConnected(remoteObject);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int) override
    {
        std::string uri = element.GetURI();
        LBSLOGD(GEO_CONVERT, "Disconnected uri is %{public}s.", uri.c_str());
        DelayedSingleton<GeoConvertService>::GetInstance().get()->NotifyDisConnected();
    }
};

bool GeoConvertService::ReConnectService()
{
    int retryCount = 0;
    if (IsConnect()) {
        LBSLOGI(GEO_CONVERT, "Connect success!");
        return true;
    }
    while (retryCount < MAX_RETRY_COUNT) {
        retryCount++;
        bool ret = ConnectService();
        if (ret) {
            LBSLOGI(GEO_CONVERT, "Connect success!");
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }
    return false;
}

bool GeoConvertService::ConnectService()
{
    LBSLOGD(GEO_CONVERT, "start ConnectService");
    if (!IsConnect()) {
        AAFwk::Want connectionWant;
        std::string serviceName;
        bool result = LocationConfigManager::GetInstance().GetGeocodeServiceName(serviceName);
        if (!result || serviceName.empty()) {
            LBSLOGE(GEO_CONVERT, "get service name failed!");
            return false;
        }
        std::string abilityName;
        bool res = LocationConfigManager::GetInstance().GetGeocodeAbilityName(abilityName);
        if (!res || abilityName.empty()) {
            LBSLOGE(GEO_CONVERT, "get service name failed!");
            return false;
        }
        connectionWant.SetElementName(serviceName, abilityName);
        sptr<AAFwk::IAbilityConnection> conn = new (std::nothrow) AbilityConnection();
        if (conn == nullptr) {
            LBSLOGE(GEO_CONVERT, "get connection failed!");
            return false;
        }
        int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(connectionWant, conn, -1);
        if (ret != ERR_OK) {
            LBSLOGE(GEO_CONVERT, "Connect cloud service failed!");
            return false;
        }
        std::unique_lock<std::mutex> uniqueLock(mutex_);
        auto waitStatus = connectCondition_.wait_for(
            uniqueLock, std::chrono::seconds(CONNECT_TIME_OUT), [this]() { return serviceProxy_ != nullptr; });
        if (!waitStatus) {
            LBSLOGE(GEO_CONVERT, "Connect cloudService timeout!");
            return false;
        }
    }
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
    bool result = LocationConfigManager::GetInstance().GetGeocodeServiceName(serviceName);
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

    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;

    if (!WriteInfoToParcel(data, dataParcel, true)) {
        reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
        return ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    sptr<GeoConvertCallbackHost> callback = new (std::nothrow) GeoConvertCallbackHost();
    if (callback == nullptr) {
        LBSLOGE(GEO_CONVERT, "can not get valid callback.");
        reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
        return ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());
    dataParcel.WriteString16(data.ReadString16()); // transId
    dataParcel.WriteString16(data.ReadString16()); // country
    bool ret = SendGeocodeRequest(REQUEST_REVERSE_GEOCODE, dataParcel, replyParcel, option);
    if (!ret) {
        reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
        return ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    std::list<std::shared_ptr<GeoAddress>> result = callback->GetResult();
    if (!WriteResultToParcel(result, reply, true)) {
        return ERRCODE_REVERSE_GEOCODING_FAIL;
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

    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;

    if (!WriteInfoToParcel(data, dataParcel, false)) {
        reply.WriteInt32(ERRCODE_GEOCODING_FAIL);
        return ERRCODE_GEOCODING_FAIL;
    }
    sptr<GeoConvertCallbackHost> callback = new (std::nothrow) GeoConvertCallbackHost();
    if (callback == nullptr) {
        LBSLOGE(GEO_CONVERT, "can not get valid callback.");
        reply.WriteInt32(ERRCODE_GEOCODING_FAIL);
        return ERRCODE_GEOCODING_FAIL;
    }
    dataParcel.WriteRemoteObject(callback->AsObject());
    dataParcel.WriteString16(data.ReadString16()); // transId
    dataParcel.WriteString16(data.ReadString16()); // country
    bool ret = SendGeocodeRequest(REQUEST_GEOCODE, dataParcel, replyParcel, option);
    if (!ret) {
        reply.WriteInt32(ERRCODE_GEOCODING_FAIL);
        return ERRCODE_GEOCODING_FAIL;
    }
    std::list<std::shared_ptr<GeoAddress>> result = callback->GetResult();
    if (!WriteResultToParcel(result, reply, false)) {
        return ERRCODE_GEOCODING_FAIL;
    }
    return ERRCODE_SUCCESS;
}

/*
 * get info from data and write to dataParcel.
 * flag: true for reverse geocoding, false for geocoding.
 */
bool GeoConvertService::WriteInfoToParcel(MessageParcel &data, MessageParcel &dataParcel, bool flag)
{
    if (flag) {
        dataParcel.WriteString16(data.ReadString16()); // locale
        dataParcel.WriteDouble(data.ReadDouble()); // latitude
        dataParcel.WriteDouble(data.ReadDouble()); // longitude
        dataParcel.WriteInt32(data.ReadInt32()); // maxItems
    } else {
        dataParcel.WriteString16(data.ReadString16()); // locale
        dataParcel.WriteString16(data.ReadString16()); // description
        dataParcel.WriteInt32(data.ReadInt32()); // maxItems
        dataParcel.WriteDouble(data.ReadDouble()); // minLatitude
        dataParcel.WriteDouble(data.ReadDouble()); // minLongitude
        dataParcel.WriteDouble(data.ReadDouble()); // maxLatitude
        dataParcel.WriteDouble(data.ReadDouble()); // maxLongitude
    }
    dataParcel.WriteString16(data.ReadString16()); // bundleName
    return true;
}

/*
 * write result info to reply.
 * flag: true for reverse geocoding, false for geocoding.
 */
bool GeoConvertService::WriteResultToParcel(const std::list<std::shared_ptr<GeoAddress>> result,
    MessageParcel &reply, bool flag)
{
    if (result.size() == 0) {
        LBSLOGE(GEO_CONVERT, "empty result!");
        reply.WriteInt32(flag ? ERRCODE_REVERSE_GEOCODING_FAIL : ERRCODE_GEOCODING_FAIL);
        return false;
    }
    reply.WriteInt32(ERRCODE_SUCCESS);
    reply.WriteInt32(result.size());
    for (auto iter = result.begin(); iter != result.end(); iter++) {
        std::shared_ptr<GeoAddress> address = *iter;
        if (address != nullptr) {
            address->Marshalling(reply);
        }
    }
    return true;
}

bool GeoConvertService::GetService()
{
    if (!IsConnect()) {
        std::string serviceName;
        bool result = LocationConfigManager::GetInstance().GetGeocodeServiceName(serviceName);
        if (!result || serviceName.empty()) {
            LBSLOGE(GEO_CONVERT, "get service name failed!");
            return false;
        }
        if (!CommonUtils::CheckAppInstalled(serviceName)) { // app is not installed
            LBSLOGE(GEO_CONVERT, "service is not available.");
            return false;
        } else if (!ReConnectService()) {
            return false;
        }
    }
    return true;
}

bool GeoConvertService::IsConnect()
{
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    return serviceProxy_ != nullptr;
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

void GeoConvertService::UnloadGeoConvertSystemAbility()
{
    auto locationSaLoadManager = DelayedSingleton<LocationSaLoadManager>::GetInstance();
    if (!CheckIfGeoConvertConnecting()) {
        LocationSaLoadManager::UnInitLocationSa(LOCATION_GEO_CONVERT_SA_ID);
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
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    if (serviceProxy_ == nullptr) {
        LBSLOGE(GEO_CONVERT, "%{public}s: geoServiceProxy_ is nullptr", __func__);
        return;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) GeoServiceDeathRecipient());
    if (death != nullptr) {
        serviceProxy_->AddDeathRecipient(death);
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

GeoServiceDeathRecipient::GeoServiceDeathRecipient()
{
}

GeoServiceDeathRecipient::~GeoServiceDeathRecipient()
{
}

void GeoServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto geoConvertService = DelayedSingleton<GeoConvertService>::GetInstance();
    if (geoConvertService != nullptr) {
        LBSLOGI(GEO_CONVERT, "geo OnRemoteDied");
        geoConvertService->ResetServiceProxy();
    }
}
} // namespace Location
} // namespace OHOS
#endif
