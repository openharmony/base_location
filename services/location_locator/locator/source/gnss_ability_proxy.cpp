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
#include "ipc_skeleton.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
GnssAbilityProxy::GnssAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGnssAbility>(impl)
{
}

void GnssAbilityProxy::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteInt64(interval);
    workrecord.Marshalling(data);
    int error = Remote()->SendRequest(ISubAbility::SEND_LOCATION_REQUEST, data, reply, option);
    LBSLOGD(GNSS, "RemoteRequest Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::SET_ENABLE, data, reply, option);
    LBSLOGD(GNSS, "Enable Transact ErrCode = %{public}d", error);
}

void GnssAbilityProxy::RemoteRequest(bool state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return;
    }
    data.WriteBool(state);

    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(ISubAbility::HANDLE_REMOTE_REQUEST, data, reply, option);
    LBSLOGD(GNSS, "RemoteRequest Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::REFRESH_REQUESTS, data, reply, option);
    LBSLOGD(GNSS, "RefrashRequirements Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::REG_GNSS_STATUS, data, reply, option);
    LBSLOGD(GNSS, "RegisterGnssStatusCallback Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::UNREG_GNSS_STATUS, data, reply, option);
    LBSLOGD(GNSS, "UnregisterGnssStatusCallback Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::REG_NMEA, data, reply, option);
    LBSLOGD(GNSS, "RegisterNmeaMessageCallback Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::UNREG_NMEA, data, reply, option);
    LBSLOGD(GNSS, "UnregisterNmeaMessageCallback Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::REG_CACHED, data, reply, option);
    LBSLOGD(GNSS, "RegisterCachedCallback Transact ErrCode = %{public}d", error);
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
    int error = Remote()->SendRequest(ISubAbility::UNREG_CACHED, data, reply, option);
    LBSLOGD(GNSS, "UnregisterCachedCallback Transact ErrCode = %{public}d", error);
}

int GnssAbilityProxy::GetCachedGnssLocationsSize()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "GetCachedGnssLocationsSize remote is null");
        return REPLY_CODE_EXCEPTION;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return REPLY_CODE_EXCEPTION;
    }

    int error = remote->SendRequest(ISubAbility::GET_CACHED_SIZE, data, reply, option);
    LBSLOGD(GNSS, "Proxy::GetCachedGnssLocationsSize Transact ErrCode = %{public}d", error);
    int size = 0;
    if (error == NO_ERROR) {
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
        return REPLY_CODE_EXCEPTION;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return REPLY_CODE_EXCEPTION;
    }
    int error = remote->SendRequest(ISubAbility::FLUSH_CACHED, data, reply, option);
    LBSLOGD(GNSS, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", error);
    return error;
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
    int error = remote->SendRequest(ISubAbility::SEND_COMMANDS, data, reply, option);
    LBSLOGD(GNSS, "Proxy::SendCommand Transact ErrCodes = %{public}d", error);
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
    data.WriteInt32(request->priority);
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
    int error = remote->SendRequest(ISubAbility::ADD_FENCE_INFO, data, reply, option);
    LBSLOGD(GNSS, "Proxy::AddFence Transact ErrCodes = %{public}d", error);
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
    data.WriteInt32(request->priority);
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
    int error = remote->SendRequest(ISubAbility::REMOVE_FENCE_INFO, data, reply, option);
    LBSLOGD(GNSS, "Proxy::RemoveFence Transact ErrCodes = %{public}d", error);
}
} // namespace Location
} // namespace OHOS