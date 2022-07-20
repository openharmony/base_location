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

#include "locator_proxy.h"

#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "country_code.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
LocatorProxy::LocatorProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocator>(impl)
{
}

int LocatorProxy::GetSwitchState()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetSwitchState remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(GET_SWITCH_STATE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetSwitchState Transact ErrCode = %{public}d", error);
    int state = 0;
    if (error == NO_ERROR) {
        state = reply.ReadInt32();
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetSwitchState return  %{public}d", state);
    return state;
}

void LocatorProxy::EnableAbility(bool isEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteBool(isEnabled);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "EnableAbility remote is null");
        return;
    }
    int error = remote->SendRequest(ENABLE_ABILITY, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableAbility Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UpdateSaAbility()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "UpdateSaAbility remote is null");
        return;
    }
    int error = remote->SendRequest(UPDATE_SA_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        LBSLOGE(LOCATOR_STANDARD, "Proxy::UpdateSaAbility update sa fail, ErrCodes=%{public}d", error);
    }
}

void LocatorProxy::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "uid is: %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "RegisterSwitchCallback remote is null");
        return;
    }
    int error = remote->SendRequest(REG_SWITCH_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterSwitchCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "UnregisterSwitchCallback remote is null");
        return;
    }
    int error = remote->SendRequest(UNREG_SWITCH_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterSwitchCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "uid is: %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "RegisterGnssStatusCallback remote is null");
        return;
    }
    int error = remote->SendRequest(REG_GNSS_STATUS_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterGnssStatusCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "UnregisterGnssStatusCallback remote is null");
        return;
    }
    int error = remote->SendRequest(UNREG_GNSS_STATUS_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterGnssStatusCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "uid is: %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "RegisterNmeaMessageCallback remote is null");
        return;
    }
    int error = remote->SendRequest(REG_NMEA_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterNmeaMessageCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "UnregisterNmeaMessageCallback remote is null");
        return;
    }
    int error = remote->SendRequest(UNREG_NMEA_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterNmeaMessageCallback Transact ErrCodes = %{public}d", error);
}


void LocatorProxy::RegisterCountryCodeCallback(const sptr<IRemoteObject> &callback, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "uid is: %{public}d", uid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "RegisterCountryCodeCallback remote is null");
        return;
    }
    int error = remote->SendRequest(REG_COUNTRY_CODE_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCountryCodeCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterCountryCodeCallback(const sptr<IRemoteObject> &callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteObject<IRemoteObject>(callback);

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "UnregisterCountryCodeCallback remote is null");
        return;
    }
    int error = remote->SendRequest(UNREG_COUNTRY_CODE_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCountryCodeCallback Transact ErrCodes = %{public}d", error);
}

int LocatorProxy::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "uid is: %{public}d, pid is: %{public}d", uid, pid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    if (requestConfig != nullptr) {
        requestConfig->Marshalling(data);
    }
    if (callback != nullptr) {
        data.WriteObject<IRemoteObject>(callback->AsObject());
    }
    data.WriteString16(Str8ToStr16(bundleName));

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StartLocating remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(START_LOCATING, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StartLocating Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::StopLocating(sptr<ILocatorCallback>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    if (callback != nullptr) {
        data.WriteObject<IRemoteObject>(callback->AsObject());
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StopLocating remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(STOP_LOCATING, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StopLocating Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::GetCacheLocation(MessageParcel &data, MessageParcel &reply)
{
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetCacheLocation remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(GET_CACHE_LOCATION, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCacheLocation Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::IsGeoConvertAvailable(MessageParcel &data, MessageParcel &reply)
{
    int error;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "IsGeoConvertAvailable remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    error = remote->SendRequest(GEO_IS_AVAILABLE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsGeoConvertAvailable result from server");
    return error;
}

int LocatorProxy::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    int error;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetAddressByCoordinate remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    error = remote->SendRequest(GET_FROM_COORDINATE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetAddressByCoordinate result from server.");
    return error;
}

int LocatorProxy::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    int error;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetAddressByLocationName remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    error = remote->SendRequest(GET_FROM_LOCATION_NAME, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetAddressByLocationName result from server.");
    return error;
}

bool LocatorProxy::IsLocationPrivacyConfirmed(const int type)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "IsLocationPrivacyConfirmed remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    data.WriteInt32(type);
    int error = remote->SendRequest(IS_PRIVACY_COMFIRMED, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed Transact ErrCode = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed return  %{public}d", state ? 1 : 0);
    return state;
}

void LocatorProxy::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(type);
    data.WriteBool(isConfirmed);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SetLocationPrivacyConfirmStatus remote is null");
        return;
    }
    int error = remote->SendRequest(SET_PRIVACY_COMFIRM_STATUS, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetLocationPrivacyConfirmStatus Transact ErrCodes = %{public}d", error);
}

int LocatorProxy::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback, std::string bundleName)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    if (request != nullptr) {
        data.WriteInt32(request->reportingPeriodSec);
        data.WriteBool(request->wakeUpCacheQueueFull);
    }
    if (callback != nullptr) {
        data.WriteRemoteObject(callback->AsObject());
    }
    data.WriteString16(Str8ToStr16(bundleName));

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "RegisterCachedLocationCallback remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(REG_CACHED_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCachedLocationCallback Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    if (callback != nullptr) {
        data.WriteRemoteObject(callback->AsObject());
    }

    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "UnregisterCachedLocationCallback remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(UNREG_CACHED_CALLBACK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCachedLocationCallback Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::GetCachedGnssLocationsSize()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetCachedGnssLocationsSize remote is null");
        return REPLY_CODE_EXCEPTION;
    }

    int error = remote->SendRequest(GET_CACHED_LOCATION_SIZE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize Transact ErrCode = %{public}d", error);
    int size = 0;
    if (error == NO_ERROR) {
        size = reply.ReadInt32();
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize return  %{public}d", size);
    return size;
}

int LocatorProxy::FlushCachedGnssLocations()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "FlushCachedGnssLocations remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(FLUSH_CACHED_LOCATIONS, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", error);
    return error;
}

void LocatorProxy::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(commands->scenario);
    data.WriteString16(Str8ToStr16(commands->command));
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendCommand remote is null");
        return;
    }
    int error = remote->SendRequest(SEND_COMMAND, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendCommand Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(request->priority);
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "AddFence remote is null");
        return;
    }
    int error = remote->SendRequest(ADD_FENCE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::AddFence Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(request->priority);
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "RemoveFence remote is null");
        return;
    }
    int error = remote->SendRequest(REMOVE_FENCE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RemoveFence Transact ErrCodes = %{public}d", error);
}

std::shared_ptr<CountryCode> LocatorProxy::GetIsoCountryCode()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return nullptr;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "GetIsoCountryCode remote is null");
        return nullptr;
    }
    int error = remote->SendRequest(GET_ISO_COUNTRY_CODE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetIsoCountryCode Transact ErrCodes = %{public}d", error);
    std::string country = "";
    int countryType = 0;
    int result = 0;
    if (error == NO_ERROR) {
        country = reply.ReadString();
        countryType = reply.ReadInt32();
        result = reply.ReadInt32();
        auto countryCode = std::make_shared<CountryCode>();
        countryCode->SetCountryCodeStr(country);
        countryCode->SetCountryCodeType(countryType);
        return countryCode;
    } else {
        return nullptr;
    }
}

bool LocatorProxy::EnableLocationMock(const LocationMockConfig& config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "EnableLocationMock remote is null");
        return false;
    }
    config.Marshalling(data);
    int error = remote->SendRequest(ENABLE_LOCATION_MOCK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableLocationMock Transact ErrCodes = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::DisableLocationMock(const LocationMockConfig& config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "DisableLocationMock remote is null");
        return false;
    }
    config.Marshalling(data);
    int error = remote->SendRequest(DISABLE_LOCATION_MOCK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableLocationMock Transact ErrCodes = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::SetMockedLocations(
    const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SetMockedLocations remote is null");
        return false;
    }
    config.Marshalling(data);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    int error = remote->SendRequest(SET_MOCKED_LOCATIONS, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetMockedLocations Transact ErrCodes = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::EnableReverseGeocodingMock()
{
    bool state = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "EnableReverseGeocodingMock remote is null");
        return false;
    }
    int error = remote->SendRequest(ENABLE_REVERSE_GEOCODE_MOCK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableReverseGeocodingMock Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::DisableReverseGeocodingMock()
{
    bool state = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "DisableReverseGeocodingMock remote is null");
        return false;
    }
    int error = remote->SendRequest(DISABLE_REVERSE_GEOCODE_MOCK, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableReverseGeocodingMock Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    bool state = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    data.WriteInt32(mockInfo.size());
    for (size_t i = 0; i < mockInfo.size(); i++) {
        mockInfo[i]->Marshalling(data);
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SetReverseGeocodingMockInfo remote is null");
        return false;
    }
    int error = remote->SendRequest(SET_REVERSE_GEOCODE_MOCKINFO, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetReverseGeocodingMockInfo Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}
} // namespace Location
} // namespace OHOS
