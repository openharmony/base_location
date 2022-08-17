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

#include "ipc_types.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "string_ex.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "geo_coding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "i_locator.h"
#include "i_locator_callback.h"
#include "location.h"
#include "location_log.h"
#include "location_mock_config.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
LocatorProxy::LocatorProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocator>(impl)
{
}

int LocatorProxy::GetSwitchState()
{
    MessageParcel reply;
    int error = SendMsgWithReply(GET_SWITCH_STATE, reply);
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
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteBool(isEnabled);
    int error = SendMsgWithDataReply(ENABLE_ABILITY, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableAbility Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UpdateSaAbility()
{
    int state = SendSimpleMsg(UPDATE_SA_ABILITY);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UpdateSaAbility return  %{public}d", state);
}

int LocatorProxy::SendMsgWithDataReply(const int msgId, MessageParcel& data, MessageParcel& reply)
{
    int error;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendMsgWithDataReply remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    error = remote->SendRequest(msgId, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendMsgWithDataReply result from server.");
    return error;
}

int LocatorProxy::SendMsgWithReply(const int msgId, MessageParcel& reply)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SendMsgWithReply WriteInterfaceToken failed");
        return REPLY_CODE_EXCEPTION;
    }
    return SendMsgWithDataReply(msgId, data, reply);
}

int LocatorProxy::SendSimpleMsg(const int msgId)
{
    MessageParcel reply;
    return SendMsgWithReply(msgId, reply);
}

int LocatorProxy::SendRegisterMsgToRemote(const int msgId, const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote WriteInterfaceToken failed");
        return REPLY_CODE_EXCEPTION;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote callback is nullptr");
        return REPLY_CODE_EXCEPTION;
    }
    data.WriteObject<IRemoteObject>(callback);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    return remote->SendRequest(msgId, data, reply, option);
}

void LocatorProxy::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    int error = SendRegisterMsgToRemote(REG_SWITCH_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterSwitchCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    int error = SendRegisterMsgToRemote(UNREG_SWITCH_CALLBACK, callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterSwitchCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    int error = SendRegisterMsgToRemote(REG_GNSS_STATUS_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterGnssStatusCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    int error = SendRegisterMsgToRemote(UNREG_GNSS_STATUS_CALLBACK, callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterGnssStatusCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    int error = SendRegisterMsgToRemote(REG_NMEA_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterNmeaMessageCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    int error = SendRegisterMsgToRemote(UNREG_NMEA_CALLBACK, callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterNmeaMessageCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RegisterCountryCodeCallback(const sptr<IRemoteObject> &callback, pid_t uid)
{
    int error = SendRegisterMsgToRemote(REG_COUNTRY_CODE_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCountryCodeCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterCountryCodeCallback(const sptr<IRemoteObject> &callback)
{
    int error = SendRegisterMsgToRemote(UNREG_COUNTRY_CODE_CALLBACK, callback, 0);
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
    int error = SendMsgWithDataReply(START_LOCATING, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StartLocating Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::StopLocating(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StopLocating callback is nullptr");
        return REPLY_CODE_EXCEPTION;
    }
    int error = SendRegisterMsgToRemote(STOP_LOCATING, callback->AsObject(), 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCountryCodeCallback Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::GetCacheLocation(MessageParcel &reply)
{
    int error = SendMsgWithReply(GET_CACHE_LOCATION, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCacheLocation Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::IsGeoConvertAvailable(MessageParcel &reply)
{
    return SendMsgWithReply(GEO_IS_AVAILABLE, reply);
}

int LocatorProxy::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    return SendMsgWithDataReply(GET_FROM_COORDINATE, data, reply);
}

int LocatorProxy::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    return SendMsgWithDataReply(GET_FROM_LOCATION_NAME, data, reply);
}

bool LocatorProxy::IsLocationPrivacyConfirmed(const int type)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    data.WriteInt32(type);
    int error = SendMsgWithDataReply(IS_PRIVACY_COMFIRMED, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed Transact ErrCode = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed return  %{public}d", state ? 1 : 0);
    return state;
}

bool LocatorProxy::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SetLocationPrivacyConfirmStatus, WriteInterfaceToken failed.");
        return false;
    }
    data.WriteInt32(type);
    data.WriteBool(isConfirmed);
    SendMsgWithDataReply(SET_PRIVACY_COMFIRM_STATUS, data, reply);
    int error = reply.ReadInt32();
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetLocationPrivacyConfirmStatus Transact ErrCodes = %{public}d", error);
    return error == REPLY_CODE_NO_EXCEPTION ? true : false;
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
    int error = SendMsgWithDataReply(REG_CACHED_CALLBACK, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCachedLocationCallback Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    int error = SendRegisterMsgToRemote(UNREG_CACHED_CALLBACK, callback->AsObject(), 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCachedLocationCallback Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::GetCachedGnssLocationsSize()
{
    MessageParcel reply;
    int error = SendMsgWithReply(GET_CACHED_LOCATION_SIZE, reply);
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
    int error = SendSimpleMsg(FLUSH_CACHED_LOCATIONS);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", error);
    return error;
}

void LocatorProxy::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(commands->scenario);
    data.WriteString16(Str8ToStr16(commands->command));
    int error = SendMsgWithDataReply(SEND_COMMAND, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendCommand Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(request->priority);
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    int error = SendMsgWithDataReply(ADD_FENCE, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::AddFence Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    data.WriteInt32(request->priority);
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    int error = SendMsgWithDataReply(REMOVE_FENCE, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RemoveFence Transact ErrCodes = %{public}d", error);
}

std::shared_ptr<CountryCode> LocatorProxy::GetIsoCountryCode()
{
    MessageParcel reply;
    int error = SendMsgWithReply(GET_ISO_COUNTRY_CODE, reply);
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
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    config.Marshalling(data);
    int error = SendMsgWithDataReply(ENABLE_LOCATION_MOCK, data, reply);
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
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    config.Marshalling(data);
    int error = SendMsgWithDataReply(DISABLE_LOCATION_MOCK, data, reply);
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
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    config.Marshalling(data);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    int error = SendMsgWithDataReply(SET_MOCKED_LOCATIONS, data, reply);
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
    MessageParcel reply;
    int error = SendMsgWithReply(ENABLE_REVERSE_GEOCODE_MOCK, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableReverseGeocodingMock Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::DisableReverseGeocodingMock()
{
    bool state = false;
    MessageParcel reply;
    int error = SendMsgWithReply(DISABLE_REVERSE_GEOCODE_MOCK, reply);
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
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    data.WriteInt32(mockInfo.size());
    for (size_t i = 0; i < mockInfo.size(); i++) {
        mockInfo[i]->Marshalling(data);
    }
    int error = SendMsgWithDataReply(SET_REVERSE_GEOCODE_MOCKINFO, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetReverseGeocodingMockInfo Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::ProxyUidForFreeze(int32_t uid, bool isProxy)
{
    bool state = false;
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }

    if (!data.WriteInt32(uid) || !data.WriteBool(isProxy)) {
        LBSLOGE(LOCATOR_STANDARD, "[ProxyUid] fail: write data failed");
        return false;
    }
    int error = SendMsgWithDataReply(PROXY_UID_FOR_FREEZE, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::ProxyUid Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}

bool LocatorProxy::ResetAllProxy()
{
    bool state = false;
    MessageParcel reply;
    int error = SendMsgWithReply(RESET_ALL_PROXY, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::ResetAllProxy Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        state = reply.ReadBool();
    }
    return state;
}
} // namespace Location
} // namespace OHOS
