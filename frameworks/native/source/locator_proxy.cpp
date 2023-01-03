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
#include "request_config.h"

namespace OHOS {
namespace Location {
LocatorProxy::LocatorProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocator>(impl)
{
}

LocationErrCode LocatorProxy::GetSwitchState(int &state)
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(GET_SWITCH_STATE, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetSwitchState Transact ErrCode = %{public}d", errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        state = reply.ReadInt32();
    } else {
        state = 0;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetSwitchState return %{public}d", state);
    return errorCode;
}

LocationErrCode LocatorProxy::EnableAbility(bool isEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    data.WriteBool(isEnabled);
    LocationErrCode errorCode = SendMsgWithDataReply(ENABLE_ABILITY, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableAbility Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UpdateSaAbility()
{
    LocationErrCode errorCode = SendSimpleMsg(UPDATE_SA_ABILITY);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UpdateSaAbility Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SendMsgWithDataReply(const int msgId, MessageParcel& data, MessageParcel& reply)
{
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendMsgWithDataReply remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = remote->SendRequest(msgId, data, reply, option);
    if (error != ERR_OK) {
        LBSLOGE(LOCATOR_STANDARD, "msgid = %{public}d, send request error: %{public}d", msgId, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendMsgWithDataReply result from server.");
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode LocatorProxy::SendMsgWithReply(const int msgId, MessageParcel& reply)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SendMsgWithReply WriteInterfaceToken failed");
        return ERRCODE_INVALID_TOKEN;
    }
    return SendMsgWithDataReply(msgId, data, reply);
}

LocationErrCode LocatorProxy::SendSimpleMsg(const int msgId)
{
    MessageParcel reply;
    return SendMsgWithReply(msgId, reply);
}

LocationErrCode LocatorProxy::SendRegisterMsgToRemote(const int msgId, const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote WriteInterfaceToken failed");
        return ERRCODE_INVALID_TOKEN;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    data.WriteObject<IRemoteObject>(callback);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = remote->SendRequest(msgId, data, reply, option);
    if (error != ERR_OK) {
        LBSLOGE(LOCATOR_STANDARD, "msgid = %{public}d, send request error: %{public}d", msgId, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode LocatorProxy::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(REG_SWITCH_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterSwitchCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(UNREG_SWITCH_CALLBACK, callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterSwitchCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(REG_GNSS_STATUS_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterGnssStatusCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(UNREG_GNSS_STATUS_CALLBACK, callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterGnssStatusCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(REG_NMEA_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterNmeaMessageCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(UNREG_NMEA_CALLBACK, callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterNmeaMessageCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "WriteInterfaceToken failed");
        return ERRCODE_INVALID_TOKEN;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    data.WriteObject<IRemoteObject>(callback);
    LocationErrCode errorCode = SendMsgWithDataReply(REG_NMEA_CALLBACK_v9, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterNmeaMessageCallbackV9 Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "WriteInterfaceToken failed");
        return ERRCODE_INVALID_TOKEN;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    data.WriteObject<IRemoteObject>(callback);
    LocationErrCode errorCode = SendMsgWithDataReply(UNREG_NMEA_CALLBACK_v9, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterNmeaMessageCallbackV9 Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterCountryCodeCallback(const sptr<IRemoteObject> &callback, pid_t uid)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(REG_COUNTRY_CODE_CALLBACK, callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCountryCodeCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterCountryCodeCallback(const sptr<IRemoteObject> &callback)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(UNREG_COUNTRY_CODE_CALLBACK, callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCountryCodeCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid)
{
    LBSLOGD(LOCATOR_STANDARD, "uid is: %{public}d, pid is: %{public}d", uid, pid);
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    if (requestConfig != nullptr) {
        requestConfig->Marshalling(data);
    }
    if (callback != nullptr) {
        data.WriteObject<IRemoteObject>(callback->AsObject());
    }
    data.WriteString16(Str8ToStr16(bundleName));
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithDataReply(START_LOCATING, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StartLocating Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::StopLocating(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StopLocating callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    LocationErrCode errorCode = SendRegisterMsgToRemote(STOP_LOCATING, callback->AsObject(), 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCountryCodeCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetCacheLocation(std::unique_ptr<Location> &loc)
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(GET_CACHE_LOCATION, reply);
    if (errorCode == ERRCODE_SUCCESS) {
        loc = Location::Unmarshalling(reply);
    } else {
        loc = nullptr;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCacheLocation Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::IsGeoConvertAvailable(bool &isAvailable)
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(GEO_IS_AVAILABLE, reply);
    if (errorCode == ERRCODE_SUCCESS) {
        isAvailable = (reply.ReadInt32() == 1);
    } else {
        isAvailable = false;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsGeoConvertAvailable Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetAddressByCoordinate(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithDataReply(GET_FROM_COORDINATE, data, reply);
    if (errorCode == ERRCODE_SUCCESS) {
        int resultSize = reply.ReadInt32();
        if (resultSize > GeoAddress::MAX_RESULT) {
            resultSize = GeoAddress::MAX_RESULT;
        }
        for (int i = 0; i < resultSize; i++) {
            replyList.push_back(GeoAddress::Unmarshalling(reply));
        }
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetAddressByCoordinate Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetAddressByLocationName(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithDataReply(GET_FROM_LOCATION_NAME, data, reply);
    if (errorCode == ERRCODE_SUCCESS) {
        int resultSize = reply.ReadInt32();
        if (resultSize > GeoAddress::MAX_RESULT) {
            resultSize = GeoAddress::MAX_RESULT;
        }
        for (int i = 0; i < resultSize; i++) {
            replyList.push_back(GeoAddress::Unmarshalling(reply));
        }
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetAddressByLocationName Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::IsLocationPrivacyConfirmed(const int type, bool &isConfirmed)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_PARAM;
    }
    data.WriteInt32(type);
    LocationErrCode errorCode = SendMsgWithDataReply(IS_PRIVACY_COMFIRMED, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed Transact ErrCodes = %{public}d", errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        isConfirmed = reply.ReadBool();
    } else {
        isConfirmed = false;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed return  %{public}d", isConfirmed);
    return errorCode;
}

LocationErrCode LocatorProxy::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SetLocationPrivacyConfirmStatus, WriteInterfaceToken failed.");
        return ERRCODE_INVALID_TOKEN;
    }
    data.WriteInt32(type);
    data.WriteBool(isConfirmed);
    LocationErrCode errorCode = SendMsgWithDataReply(SET_PRIVACY_COMFIRM_STATUS, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetLocationPrivacyConfirmStatus Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback, std::string bundleName)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
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
    LocationErrCode errorCode = SendMsgWithDataReply(REG_CACHED_CALLBACK, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCachedLocationCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    LocationErrCode errorCode = SendRegisterMsgToRemote(UNREG_CACHED_CALLBACK, callback->AsObject(), 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCachedLocationCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetCachedGnssLocationsSize(int &size)
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(GET_CACHED_LOCATION_SIZE, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize Transact ErrCode = %{public}d", errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        size = reply.ReadInt32();
    } else {
        size = 0;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize return  %{public}d", size);
    return errorCode;
}

LocationErrCode LocatorProxy::FlushCachedGnssLocations()
{
    LocationErrCode errorCode = SendSimpleMsg(FLUSH_CACHED_LOCATIONS);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    data.WriteInt32(commands->scenario);
    data.WriteString16(Str8ToStr16(commands->command));
    LocationErrCode errorCode = SendMsgWithDataReply(SEND_COMMAND, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendCommand Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    LocationErrCode errorCode = SendMsgWithDataReply(ADD_FENCE, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::AddFence Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    LocationErrCode errorCode = SendMsgWithDataReply(REMOVE_FENCE, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RemoveFence Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetIsoCountryCode(std::shared_ptr<CountryCode>& countryCode)
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(GET_ISO_COUNTRY_CODE, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetIsoCountryCode Transact ErrCodes = %{public}d", errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        std::string country = reply.ReadString();
        int countryType = reply.ReadInt32();
        countryCode->SetCountryCodeStr(country);
        countryCode->SetCountryCodeType(countryType);
    } else {
        countryCode = nullptr;
    }
    return errorCode;
}

LocationErrCode LocatorProxy::EnableLocationMock()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    LocationErrCode errorCode = SendMsgWithDataReply(ENABLE_LOCATION_MOCK, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableLocationMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::DisableLocationMock()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    LocationErrCode errorCode = SendMsgWithDataReply(DISABLE_LOCATION_MOCK, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableLocationMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    data.WriteInt32(timeInterval);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    LocationErrCode errorCode = SendMsgWithDataReply(SET_MOCKED_LOCATIONS, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetMockedLocations Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::EnableReverseGeocodingMock()
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(ENABLE_REVERSE_GEOCODE_MOCK, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableReverseGeocodingMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::DisableReverseGeocodingMock()
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(DISABLE_REVERSE_GEOCODE_MOCK, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableReverseGeocodingMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    data.WriteInt32(mockInfo.size());
    for (size_t i = 0; i < mockInfo.size(); i++) {
        mockInfo[i]->Marshalling(data);
    }
    LocationErrCode errorCode = SendMsgWithDataReply(SET_REVERSE_GEOCODE_MOCKINFO, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetReverseGeocodingMockInfo Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::ProxyUidForFreeze(int32_t uid, bool isProxy)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }

    if (!data.WriteInt32(uid) || !data.WriteBool(isProxy)) {
        LBSLOGE(LOCATOR_STANDARD, "[ProxyUid] fail: write data failed");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode = SendMsgWithDataReply(PROXY_UID_FOR_FREEZE, data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::ProxyUid Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::ResetAllProxy()
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReply(RESET_ALL_PROXY, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::ResetAllProxy Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}
} // namespace Location
} // namespace OHOS
