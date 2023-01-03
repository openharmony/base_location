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

#include "gnss_ability_proxy.h"

#include "string_ex.h"

#include "common_utils.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
GnssAbilityProxy::GnssAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGnssAbility>(impl)
{
}

void GnssAbilityProxy::SendLocationRequest(WorkRecord &workrecord)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    workrecord.Marshalling(data);
    Remote()->SendRequest(ISubAbility::SEND_LOCATION_REQUEST, data, reply, option);
    LBSLOGD(GNSS, "RemoteRequest Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::SetEnable(bool state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteBool(state);

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(ISubAbility::SET_ENABLE, data, reply, option);
    LBSLOGD(GNSS, "Enable Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::RefrashRequirements()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    Remote()->SendRequest(ISubAbility::REFRESH_REQUESTS, data, reply, option);
    LBSLOGD(GNSS, "RefrashRequirements Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteRemoteObject(callback);
    Remote()->SendRequest(ISubAbility::REG_GNSS_STATUS, data, reply, option);
    LBSLOGD(GNSS, "RegisterGnssStatusCallback Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteRemoteObject(callback);
    Remote()->SendRequest(ISubAbility::UNREG_GNSS_STATUS, data, reply, option);
    LBSLOGD(GNSS, "UnregisterGnssStatusCallback Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteRemoteObject(callback);
    Remote()->SendRequest(ISubAbility::REG_NMEA, data, reply, option);
    LBSLOGD(GNSS, "RegisterNmeaMessageCallback Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteRemoteObject(callback);
    Remote()->SendRequest(ISubAbility::UNREG_NMEA, data, reply, option);
    LBSLOGD(GNSS, "UnregisterNmeaMessageCallback Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
    const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteInt32(request->reportingPeriodSec);
    data.WriteBool(request->wakeUpCacheQueueFull);
    data.WriteRemoteObject(callback);
    Remote()->SendRequest(ISubAbility::REG_CACHED, data, reply, option);
    LBSLOGD(GNSS, "RegisterCachedCallback Transact ErrCode = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::UnregisterCachedCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteRemoteObject(callback);
    Remote()->SendRequest(ISubAbility::UNREG_CACHED, data, reply, option);
    LBSLOGD(GNSS, "UnregisterCachedCallback Transact ErrCode = %{public}d", reply.ReadInt32());
}

int GnssAbilityProxy::GetCachedGnssLocationsSize()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "GetCachedGnssLocationsSize remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_INVALID_TOKEN;
    }

    remote->SendRequest(ISubAbility::GET_CACHED_SIZE, data, reply, option);
    int errorCode = reply.ReadInt32();
    LBSLOGD(GNSS, "Proxy::GetCachedGnssLocationsSize Transact ErrCode = %{public}d", errorCode);
    int size = 0;
    if (errorCode == ERRCODE_SUCCESS) {
        size = reply.ReadInt32();
    }
    LBSLOGD(GNSS, "Proxy::GetCachedGnssLocationsSize return  %{public}d", size);
    return size;
}

int GnssAbilityProxy::FlushCachedGnssLocations()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "FlushCachedGnssLocations remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_INVALID_TOKEN;
    }
    remote->SendRequest(ISubAbility::FLUSH_CACHED, data, reply, option);
    int errorCode = reply.ReadInt32();
    LBSLOGD(GNSS, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

void GnssAbilityProxy::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteInt32(commands->scenario);
    data.WriteString16(Str8ToStr16(commands->command));
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "SendCommand remote is null");
        return;
    }
    remote->SendRequest(ISubAbility::SEND_COMMANDS, data, reply, option);
    LBSLOGD(GNSS, "Proxy::SendCommand Transact ErrCodes = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "AddFence remote is null");
        return;
    }
    remote->SendRequest(ISubAbility::ADD_FENCE_INFO, data, reply, option);
    LBSLOGD(GNSS, "Proxy::AddFence Transact ErrCodes = %{public}d", reply.ReadInt32());
}

void GnssAbilityProxy::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "RemoveFence remote is null");
        return;
    }
    remote->SendRequest(ISubAbility::REMOVE_FENCE_INFO, data, reply, option);
    LBSLOGD(GNSS, "Proxy::RemoveFence Transact ErrCodes = %{public}d", reply.ReadInt32());
}

bool GnssAbilityProxy::EnableMock()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "EnableLocationMock remote is null");
        return false;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return false;
    }
    remote->SendRequest(ISubAbility::ENABLE_LOCATION_MOCK, data, reply, option);
    int errorCode = reply.ReadInt32();
    LBSLOGD(GNSS, "Proxy::EnableLocationMock Transact ErrCode = %{public}d", errorCode);
    bool result = false;
    if (errorCode == ERRCODE_SUCCESS) {
        result = true;
    }
    return result;
}

bool GnssAbilityProxy::DisableMock()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "DisableLocationMock remote is null");
        return false;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return false;
    }
    remote->SendRequest(ISubAbility::DISABLE_LOCATION_MOCK, data, reply, option);
    int errorCode = reply.ReadInt32();
    LBSLOGD(GNSS, "Proxy::DisableLocationMock Transact ErrCode = %{public}d", errorCode);
    bool result = false;
    if (errorCode == ERRCODE_SUCCESS) {
        result = true;
    }
    return result;
}

bool GnssAbilityProxy::SetMocked(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "SetMockedLocations remote is null");
        return false;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return false;
    }
    data.WriteInt32(timeInterval);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    remote->SendRequest(ISubAbility::SET_MOCKED_LOCATIONS, data, reply, option);
    int errorCode = reply.ReadInt32();
    LBSLOGD(GNSS, "Proxy::SetMockedLocations Transact ErrCode = %{public}d", errorCode);
    bool result = false;
    if (errorCode == ERRCODE_SUCCESS) {
        result = true;
    }
    return result;
}
} // namespace Location
} // namespace OHOS