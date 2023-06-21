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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability_proxy.h"

#include "string_ex.h"

#include "common_utils.h"
#include "location_log.h"
#include "locationhub_ipc_interface_code.h"

namespace OHOS {
namespace Location {
GnssAbilityProxy::GnssAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGnssAbility>(impl)
{
}

LocationErrCode GnssAbilityProxy::SendLocationRequest(WorkRecord &workrecord)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    workrecord.Marshalling(data);
    int error = Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST), 
                                      data, 
                                      reply, 
                                      option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::SetEnable(bool state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteBool(state);

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::RefrashRequirements()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = 
        Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::REFRESH_REQUESTS), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteRemoteObject(callback);
    int error = 
        Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_GNSS_STATUS), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteRemoteObject(callback);
    int error = 
        Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_GNSS_STATUS), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteRemoteObject(callback);
    int error = Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_NMEA), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteRemoteObject(callback);
    int error = Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_NMEA), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
    const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(request->reportingPeriodSec);
    data.WriteBool(request->wakeUpCacheQueueFull);
    data.WriteRemoteObject(callback);
    int error = Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_CACHED), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::UnregisterCachedCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteRemoteObject(callback);
    int error = 
        Remote()->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_CACHED), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::GetCachedGnssLocationsSize(int &size)
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
        return ERRCODE_SERVICE_UNAVAILABLE;
    }

    int error = 
        remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::GET_CACHED_SIZE), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    LocationErrCode errorCode = LocationErrCode(reply.ReadInt32());
    if (errorCode == ERRCODE_SUCCESS) {
        size = reply.ReadInt32();
    }
    LBSLOGD(GNSS, "Proxy::%{public}s return size = %{public}d", __func__, size);
    return errorCode;
}

LocationErrCode GnssAbilityProxy::FlushCachedGnssLocations()
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
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::FLUSH_CACHED), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(commands->scenario);
    data.WriteString16(Str8ToStr16(commands->command));
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "SendCommand remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "AddFence remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = 
        remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::ADD_FENCE_INFO), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "RemoveFence remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = 
        remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FENCE_INFO), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::EnableMock()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "EnableLocationMock remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = 
        remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::ENABLE_LOCATION_MOCK), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::DisableMock()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "DisableLocationMock remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = 
        remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::DISABLE_LOCATION_MOCK), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode GnssAbilityProxy::SetMocked(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "SetMockedLocations remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(timeInterval);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    int error = 
        remote->SendRequest(static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS), data, reply, option);
    LBSLOGD(GNSS, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}
} // namespace Location
} // namespace OHOS
#endif
