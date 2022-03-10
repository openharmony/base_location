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

#include "gnss_ability_skeleton.h"

#include "ipc_skeleton.h"

#include "common_utils.h"

namespace OHOS {
namespace Location {
GnssAbilityProxy::GnssAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGnssAbility>(impl)
{
    SetProxy(GNSS_ABILITY, AsObject());
}

void GnssAbilityProxy::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    SendRequest(interval, workrecord);
}

std::unique_ptr<Location> GnssAbilityProxy::GetCachedLocation()
{
    return GetCache();
}

void GnssAbilityProxy::SetEnable(bool state)
{
    Enable(state);
}

void GnssAbilityProxy::RemoteRequest(bool state)
{
    MessageParcel data;
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
    int error = Remote()->SendRequest(ISubAbility::REFRESH_REQUESTS, data, reply, option);
    LBSLOGD(GNSS, "RefrashRequirements Transact ErrCode = %{public}d", error);
}

void GnssAbilityProxy::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    data.WriteRemoteObject(callback);
    int error = Remote()->SendRequest(ISubAbility::REG_GNSS_STATUS, data, reply, option);
    LBSLOGD(GNSS, "RegisterGnssStatusCallback Transact ErrCode = %{public}d", error);
}

void GnssAbilityProxy::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    data.WriteRemoteObject(callback);
    int error = Remote()->SendRequest(ISubAbility::UNREG_GNSS_STATUS, data, reply, option);
    LBSLOGD(GNSS, "UnregisterGnssStatusCallback Transact ErrCode = %{public}d", error);
}

void GnssAbilityProxy::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    data.WriteRemoteObject(callback);
    int error = Remote()->SendRequest(ISubAbility::REG_NMEA, data, reply, option);
    LBSLOGD(GNSS, "RegisterNmeaMessageCallback Transact ErrCode = %{public}d", error);
}

void GnssAbilityProxy::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    data.WriteRemoteObject(callback);
    int error = Remote()->SendRequest(ISubAbility::UNREG_NMEA, data, reply, option);
    LBSLOGD(GNSS, "UnregisterNmeaMessageCallback Transact ErrCode = %{public}d", error);
}

void GnssAbilityProxy::RegisterCachedCallback(const std::unique_ptr<CachedGnssLoactionsRequest>& request,
    const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
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
        LBSLOGE(LOCATOR_STANDARD, "GetCachedGnssLocationsSize remote is null");
        return EXCEPTION;
    }

    int error = remote->SendRequest(ISubAbility::GET_CACHED_SIZE, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize Transact ErrCode = %{public}d", error);
    int size = 0;
    if (error == NO_ERROR) {
        size = reply.ReadInt32();
    }
    LBSLOGD(LOCATOR_STANDARD, "Proxy::GetCachedGnssLocationsSize return  %{public}d", size);
    return size;
}

void GnssAbilityProxy::FlushCachedGnssLocations()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "FlushCachedGnssLocations remote is null");
        return;
    }
    int error = remote->SendRequest(ISubAbility::FLUSH_CACHED, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::FlushCachedGnssLocations Transact ErrCodes = %{public}d", error);
}

void GnssAbilityProxy::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(commands->scenario);
    data.WriteString16(Str8ToStr16(commands->command));
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendCommand remote is null");
        return;
    }
    int error = remote->SendRequest(ISubAbility::SEND_COMMANDS, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::SendCommand Transact ErrCodes = %{public}d", error);
}

void GnssAbilityProxy::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
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
    int error = remote->SendRequest(ISubAbility::ADD_FENCE_INFO, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::AddFence Transact ErrCodes = %{public}d", error);
}

void GnssAbilityProxy::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
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
    int error = remote->SendRequest(ISubAbility::REMOVE_FENCE_INFO, data, reply, option);
    LBSLOGD(LOCATOR_STANDARD, "Proxy::RemoveFence Transact ErrCodes = %{public}d", error);
}

int GnssAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t lastCallingPid = IPCSkeleton::GetCallingPid();
    pid_t lastCallinguid = IPCSkeleton::GetCallingUid();
    LBSLOGI(GNSS, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), lastCallingPid, lastCallinguid);
    if (lastCallinguid > SYSTEM_UID) {
        LBSLOGE(GNSS, "this remote request is not allowed");
        return EXCEPTION;
    }

    int ret = REPLY_NO_EXCEPTION;
    switch (code) {
        case SEND_LOCATION_REQUEST: {
            int64_t interval = data.ReadInt64();
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            SendLocationRequest((uint64_t)interval, *workrecord);
            break;
        }
        case GET_CACHED_LOCATION: {
            std::unique_ptr<Location> location = GetCachedLocation();
            if (location != nullptr) {
                location->Marshalling(reply);
            }
            break;
        }
        case SET_ENABLE: {
            SetEnable(data.ReadBool());
            break;
        }
        case HANDLE_REMOTE_REQUEST: {
            bool state = data.ReadBool();
            RemoteRequest(state);
            break;
        }
        case REFRESH_REQUESTS: {
            RefrashRequirements();
            break;
        }
        case REG_GNSS_STATUS: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            RegisterGnssStatusCallback(client, lastCallinguid);
            break;
        }
        case UNREG_GNSS_STATUS: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            UnregisterGnssStatusCallback(client);
            break;
        }
        case REG_NMEA: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            RegisterNmeaMessageCallback(client, lastCallinguid);
            break;
        }
        case UNREG_NMEA: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            UnregisterNmeaMessageCallback(client);
            break;
        }
        case REG_CACHED: {
            std::unique_ptr<CachedGnssLoactionsRequest> requestConfig = std::make_unique<CachedGnssLoactionsRequest>();
            requestConfig->reportingPeriodSec = data.ReadInt32();
            requestConfig->wakeUpCacheQueueFull = data.ReadBool();
            sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
            RegisterCachedCallback(requestConfig, callback);
            break;
        }
        case UNREG_CACHED: {
            sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
            UnregisterCachedCallback(callback);
            break;
        }
        case GET_CACHED_SIZE: {
            reply.WriteInt32(GetCachedGnssLocationsSize());
            break;
        }
        case FLUSH_CACHED: {
            FlushCachedGnssLocations();
            break;
        }
        case SEND_COMMANDS: {
            std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
            locationCommand->scenario =  data.ReadInt32();
            locationCommand->command = data.ReadBool();
            SendCommand(locationCommand);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}
} // namespace Location
} // namespace OHOS