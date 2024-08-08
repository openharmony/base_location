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
#include "geo_coding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "i_locator.h"
#include "i_locator_callback.h"
#include "location.h"
#include "location_log.h"
#include "locationhub_ipc_interface_code.h"
#include "request_config.h"
#ifdef NOTIFICATION_ENABLE
#include "notification_request.h"
#endif

namespace OHOS {
namespace Location {
LocatorProxy::LocatorProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocator>(impl)
{
}

int LocatorProxy::GetSwitchState()
{
    MessageParcel reply;
    int error = SendMsgWithReply(static_cast<int>(LocatorInterfaceCode::GET_SWITCH_STATE), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetSwitchState Transact ErrCode = %{public}d", error);
    int state = 0;
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
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
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::ENABLE_ABILITY), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableAbility Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UpdateSaAbility()
{
    int state = SendSimpleMsg(static_cast<int>(LocatorInterfaceCode::UPDATE_SA_ABILITY));
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UpdateSaAbility return  %{public}d", state);
}

int LocatorProxy::SendMsgWithDataReply(const int msgId, MessageParcel& data, MessageParcel& reply)
{
    int error;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendMsgWithDataReply remote is null");
        reply.WriteInt32(REPLY_CODE_EXCEPTION);
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
        reply.WriteInt32(REPLY_CODE_EXCEPTION);
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
    int error =
        SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::REG_SWITCH_CALLBACK), callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterSwitchCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    int error =
        SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::UNREG_SWITCH_CALLBACK), callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterSwitchCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    int error =
        SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::REG_GNSS_STATUS_CALLBACK), callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterGnssStatusCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    int error =
        SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::UNREG_GNSS_STATUS_CALLBACK), callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterGnssStatusCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    int error = SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::REG_NMEA_CALLBACK), callback, uid);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterNmeaMessageCallback Transact ErrCodes = %{public}d", error);
}

void LocatorProxy::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    int error = SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::UNREG_NMEA_CALLBACK), callback, 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterNmeaMessageCallback Transact ErrCodes = %{public}d", error);
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
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::START_LOCATING), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StartLocating Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::StopLocating(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StopLocating callback is nullptr");
        return REPLY_CODE_EXCEPTION;
    }
    int error =
        SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::STOP_LOCATING), callback->AsObject(), 0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StopLocating Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::GetCacheLocation(MessageParcel &reply)
{
    int error = SendMsgWithReply(static_cast<int>(LocatorInterfaceCode::GET_CACHE_LOCATION), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCacheLocation Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::IsGeoConvertAvailable(MessageParcel &reply)
{
    return SendMsgWithReply(static_cast<int>(LocatorInterfaceCode::GEO_IS_AVAILABLE), reply);
}

int LocatorProxy::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    return SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::GET_FROM_COORDINATE), data, reply);
}

int LocatorProxy::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    return SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::GET_FROM_LOCATION_NAME), data, reply);
}

bool LocatorProxy::IsLocationPrivacyConfirmed(const int type)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    data.WriteInt32(type);
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::IS_PRIVACY_COMFIRMED), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed Transact ErrCode = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        state = reply.ReadBool();
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed return  %{public}d", state ? 1 : 0);
    return state;
}

int LocatorProxy::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SetLocationPrivacyConfirmStatus, WriteInterfaceToken failed.");
        return REPLY_CODE_EXCEPTION;
    }
    data.WriteInt32(type);
    data.WriteBool(isConfirmed);
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::SET_PRIVACY_COMFIRM_STATUS), data, reply);
    if (error != NO_ERROR) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: SendMsgWithDataReply failed, ErrCodes = %{public}d", __func__, error);
    }
    error = reply.ReadInt32();
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetLocationPrivacyConfirmStatus Transact ErrCodes = %{public}d", error);
    return error;
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
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::REG_CACHED_CALLBACK), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCachedLocationCallback Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    int error = SendRegisterMsgToRemote(static_cast<int>(LocatorInterfaceCode::UNREG_CACHED_CALLBACK),
                                        callback->AsObject(),
                                        0);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCachedLocationCallback Transact ErrCodes = %{public}d", error);
    return error;
}

int LocatorProxy::GetCachedGnssLocationsSize()
{
    MessageParcel reply;
    int error = SendMsgWithReply(static_cast<int>(LocatorInterfaceCode::GET_CACHED_LOCATION_SIZE), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize Transact ErrCode = %{public}d", error);
    int size = 0;
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        size = reply.ReadInt32();
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize return  %{public}d", size);
    return size;
}

int LocatorProxy::FlushCachedGnssLocations()
{
    MessageParcel reply;
    int error = SendMsgWithReply(static_cast<int>(LocatorInterfaceCode::FLUSH_CACHED_LOCATIONS), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR) {
        return reply.ReadInt32();
    }
    return REPLY_CODE_EXCEPTION;
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
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::SEND_COMMAND), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendCommand Transact ErrCodes = %{public}d", error);
}

bool LocatorProxy::EnableLocationMock()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableLocationMock Transact ErrCodes = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        state = true;
    }
    return state;
}

bool LocatorProxy::DisableLocationMock()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableLocationMock Transact ErrCodes = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        state = true;
    }
    return state;
}

bool LocatorProxy::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return false;
    }
    data.WriteInt32(timeInterval);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    int error = SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetMockedLocations Transact ErrCodes = %{public}d", error);
    bool state = false;
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        state = true;
    }
    return state;
}

bool LocatorProxy::EnableReverseGeocodingMock()
{
    bool state = false;
    MessageParcel reply;
    int error = SendMsgWithReply(static_cast<int>(LocatorInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableReverseGeocodingMock Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        state = true;
    }
    return state;
}

bool LocatorProxy::DisableReverseGeocodingMock()
{
    bool state = false;
    MessageParcel reply;
    int error = SendMsgWithReply(static_cast<int>(LocatorInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableReverseGeocodingMock Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        state = true;
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
    int error =
        SendMsgWithDataReply(static_cast<int>(LocatorInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetReverseGeocodingMockInfo Transact ErrCodes = %{public}d", error);
    if (error == NO_ERROR && reply.ReadInt32() == ERRCODE_SUCCESS) {
        state = true;
    }
    return state;
}

LocationErrCode LocatorProxy::GetSwitchStateV9(bool &isEnabled)
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithReplyV9(static_cast<int>(LocatorInterfaceCode::GET_SWITCH_STATE), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetSwitchState Transact ErrCode = %{public}d", errorCode);
    int state = DISABLED;
    if (errorCode == ERRCODE_SUCCESS) {
        state = reply.ReadInt32();
    }
    isEnabled = (state == ENABLED);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetSwitchState return %{public}d", isEnabled);
    return errorCode;
}

LocationErrCode LocatorProxy::EnableAbilityV9(bool isEnabled)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteBool(isEnabled);
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::ENABLE_ABILITY), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableAbility Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UpdateSaAbilityV9()
{
    LocationErrCode errorCode = SendSimpleMsgV9(static_cast<int>(LocatorInterfaceCode::UPDATE_SA_ABILITY));
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UpdateSaAbility Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SendMsgWithDataReplyV9(const int msgId, MessageParcel& data, MessageParcel& reply)
{
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendMsgWithDataReply remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = remote->SendRequest(msgId, data, reply, option);
    if (error != NO_ERROR) {
        LBSLOGE(LOCATOR_STANDARD, "msgid = %{public}d, send request error: %{public}d", msgId, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendMsgWithDataReply result from server.");
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode LocatorProxy::SendMsgWithReplyV9(const int msgId, MessageParcel& reply)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SendMsgWithReply WriteInterfaceToken failed");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return SendMsgWithDataReplyV9(msgId, data, reply);
}

LocationErrCode LocatorProxy::SendSimpleMsgV9(const int msgId)
{
    MessageParcel reply;
    return SendMsgWithReplyV9(msgId, reply);
}

LocationErrCode LocatorProxy::SendRegisterMsgToRemoteV9(const int msgId, const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote WriteInterfaceToken failed");
        return ERRCODE_SERVICE_UNAVAILABLE;
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
    if (error != NO_ERROR) {
        LBSLOGE(LOCATOR_STANDARD, "msgid = %{public}d, send request error: %{public}d", msgId, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode LocatorProxy::RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback)
{
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::REG_SWITCH_CALLBACK), callback);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterSwitchCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterSwitchCallbackV9(const sptr<IRemoteObject>& callback)
{
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::UNREG_SWITCH_CALLBACK), callback);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterSwitchCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback)
{
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::REG_GNSS_STATUS_CALLBACK), callback);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterGnssStatusCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback)
{
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::UNREG_GNSS_STATUS_CALLBACK), callback);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterGnssStatusCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "WriteInterfaceToken failed");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    data.WriteObject<IRemoteObject>(callback);
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::REG_NMEA_CALLBACK_V9), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterNmeaMessageCallbackV9 Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "WriteInterfaceToken failed");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    data.WriteObject<IRemoteObject>(callback);
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::UNREG_NMEA_CALLBACK_V9), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnRegisterNmeaMessageCallbackV9 Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (requestConfig != nullptr) {
        requestConfig->Marshalling(data);
    }
    if (callback != nullptr) {
        data.WriteObject<IRemoteObject>(callback->AsObject());
    }
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::START_LOCATING), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StartLocating Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::StopLocatingV9(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StopLocating callback is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::STOP_LOCATING), callback->AsObject());
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StopLocatingV9 Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetCacheLocationV9(std::unique_ptr<Location> &loc)
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithReplyV9(static_cast<int>(LocatorInterfaceCode::GET_CACHE_LOCATION), reply);
    if (errorCode == ERRCODE_SUCCESS) {
        loc = Location::Unmarshalling(reply);
    } else {
        loc = nullptr;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCacheLocation Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::IsGeoConvertAvailableV9(bool &isAvailable)
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithReplyV9(static_cast<int>(LocatorInterfaceCode::GEO_IS_AVAILABLE), reply);
    if (errorCode == ERRCODE_SUCCESS) {
        isAvailable = reply.ReadBool();
    } else {
        isAvailable = false;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsGeoConvertAvailable Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetAddressByCoordinateV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::GET_FROM_COORDINATE), data, reply);
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

LocationErrCode LocatorProxy::GetAddressByLocationNameV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::GET_FROM_LOCATION_NAME), data, reply);
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

LocationErrCode LocatorProxy::IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(type);
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::IS_PRIVACY_COMFIRMED), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed Transact ErrCodes = %{public}d", errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        isConfirmed = reply.ReadBool();
    } else {
        isConfirmed = false;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::IsLocationPrivacyConfirmed return  %{public}d", isConfirmed);
    return errorCode;
}

LocationErrCode LocatorProxy::SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(LOCATOR_STANDARD, "SetLocationPrivacyConfirmStatus, WriteInterfaceToken failed.");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(type);
    data.WriteBool(isConfirmed);
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::SET_PRIVACY_COMFIRM_STATUS), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetLocationPrivacyConfirmStatus Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback, std::string bundleName)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
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
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::REG_CACHED_CALLBACK), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RegisterCachedLocationCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback)
{
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::UNREG_CACHED_CALLBACK), callback->AsObject());
    LBSLOGD(LOCATOR_STANDARD, "Proxy::UnregisterCachedLocationCallback Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::GetCachedGnssLocationsSizeV9(int &size)
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithReplyV9(static_cast<int>(LocatorInterfaceCode::GET_CACHED_LOCATION_SIZE), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize Transact ErrCode = %{public}d", errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        size = reply.ReadInt32();
    } else {
        size = 0;
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize return  %{public}d", size);
    return errorCode;
}

LocationErrCode LocatorProxy::FlushCachedGnssLocationsV9()
{
    LocationErrCode errorCode = SendSimpleMsgV9(static_cast<int>(LocatorInterfaceCode::FLUSH_CACHED_LOCATIONS));
    LBSLOGD(LOCATOR_STANDARD, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SendCommandV9(std::unique_ptr<LocationCommand>& commands)
{
    if (commands == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(commands->scenario);
    data.WriteString16(Str8ToStr16(commands->command));
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::SEND_COMMAND), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendCommand Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::EnableLocationMockV9()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableLocationMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::DisableLocationMockV9()
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableLocationMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SetMockedLocationsV9(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(timeInterval);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetMockedLocations Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::EnableReverseGeocodingMockV9()
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithReplyV9(static_cast<int>(LocatorInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::EnableReverseGeocodingMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::DisableReverseGeocodingMockV9()
{
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithReplyV9(static_cast<int>(LocatorInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::DisableReverseGeocodingMock Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SetReverseGeocodingMockInfoV9(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(mockInfo.size());
    for (size_t i = 0; i < mockInfo.size(); i++) {
        mockInfo[i]->Marshalling(data);
    }
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SetReverseGeocodingMockInfo Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::ProxyForFreeze(std::set<int> pidList, bool isProxy)
{
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }

    data.WriteInt32(pidList.size());
    for (auto it = pidList.begin(); it != pidList.end(); it++) {
        data.WriteInt32(*it);
    }
    data.WriteBool(isProxy);
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::PROXY_PID_FOR_FREEZE), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::ProxyForFreeze Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::ResetAllProxy()
{
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithReplyV9(static_cast<int>(LocatorInterfaceCode::RESET_ALL_PROXY), reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::ResetAllProxy Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::RegisterLocatingRequiredDataCallback(
    std::unique_ptr<LocatingRequiredDataConfig>& dataConfig, sptr<ILocatingRequiredDataCallback>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (dataConfig != nullptr) {
        dataConfig->Marshalling(data);
    }
    if (callback != nullptr) {
        data.WriteObject<IRemoteObject>(callback->AsObject());
    }
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::REG_LOCATING_REQUIRED_DATA_CALLBACK),
        data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::%{public}s Transact ErrCodes = %{public}d", __func__, errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnRegisterLocatingRequiredDataCallback(sptr<ILocatingRequiredDataCallback>& callback)
{
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::UNREG_LOCATING_REQUIRED_DATA_CALLBACK),
            callback->AsObject());
    LBSLOGD(LOCATOR_STANDARD, "Proxy::%{public}s Transact ErrCodes = %{public}d", __func__, errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::SubscribeLocationError(sptr<ILocatorCallback>& callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback != nullptr) {
        data.WriteObject<IRemoteObject>(callback->AsObject());
    }
    MessageParcel reply;
    LocationErrCode errorCode =
        SendMsgWithDataReplyV9(static_cast<int>(LocatorInterfaceCode::REG_LOCATION_ERROR), data, reply);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StartLocating Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode LocatorProxy::UnSubscribeLocationError(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StopLocating callback is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode =
        SendRegisterMsgToRemoteV9(static_cast<int>(LocatorInterfaceCode::UNREG_LOCATION_ERROR), callback->AsObject());
    LBSLOGD(LOCATOR_STANDARD, "Proxy::StopLocatingV9 Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}
} // namespace Location
} // namespace OHOS
