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
#include "gnss_ability_skeleton.h"

#include "ipc_skeleton.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "gnss_ability.h"
#include "locationhub_ipc_interface_code.h"
#include "permission_manager.h"
#include "want_agent_helper.h"

namespace OHOS {
namespace Location {
void GnssAbilityStub::InitGnssMsgHandleMap()
{
    if (GnssMsgHandleMap_.size() != 0) {
        return;
    }
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST)] =
        &GnssAbilityStub::SendLocationRequestInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS)] =
        &GnssAbilityStub::SetMockLocationsInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE)] =
        &GnssAbilityStub::SetEnableInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REFRESH_REQUESTS)] =
        &GnssAbilityStub::RefreshRequirementsInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_GNSS_STATUS)] =
        &GnssAbilityStub::RegisterGnssStatusCallbackInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_GNSS_STATUS)] =
        &GnssAbilityStub::UnregisterGnssStatusCallbackInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_NMEA)] =
        &GnssAbilityStub::RegisterNmeaMessageCallbackInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_NMEA)] =
        &GnssAbilityStub::UnregisterNmeaMessageCallbackInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_CACHED)] =
        &GnssAbilityStub::RegisterCachedCallbackInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_CACHED)] =
        &GnssAbilityStub::UnregisterCachedCallbackInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::GET_CACHED_SIZE)] =
        &GnssAbilityStub::GetCachedGnssLocationsSizeInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::FLUSH_CACHED)] =
        &GnssAbilityStub::FlushCachedGnssLocationsInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS)] =
        &GnssAbilityStub::SendCommandInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::ENABLE_LOCATION_MOCK)] =
        &GnssAbilityStub::EnableMockInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::DISABLE_LOCATION_MOCK)] =
        &GnssAbilityStub::DisableMockInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::ADD_FENCE_INFO)] =
        &GnssAbilityStub::AddFenceInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FENCE_INFO)] =
        &GnssAbilityStub::RemoveFenceInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::ADD_GNSS_GEOFENCE)] =
        &GnssAbilityStub::AddGnssGeofenceInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REMOVE_GNSS_GEOFENCE)] =
        &GnssAbilityStub::RemoveGnssGeofenceInner;
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::QUERY_SUPPORT_COORDINATE_SYSTEM_TYPE)] =
        &GnssAbilityStub::QuerySupportCoordinateSystemTypeInner;
}

GnssAbilityStub::GnssAbilityStub()
{
    InitGnssMsgHandleMap();
}

int GnssAbilityStub::SendLocationRequestInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::SetMockLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::SetEnableInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RefreshRequirementsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(RefrashRequirements());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RegisterGnssStatusCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(RegisterGnssStatusCallback(client, identity.GetUid()));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::UnregisterGnssStatusCallbackInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(UnregisterGnssStatusCallback(client));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RegisterNmeaMessageCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(RegisterNmeaMessageCallback(client, identity.GetUid()));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::UnregisterNmeaMessageCallbackInner(MessageParcel &data, MessageParcel &reply,
    AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(UnregisterNmeaMessageCallback(client));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RegisterCachedCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
    requestConfig->reportingPeriodSec = data.ReadInt32();
    requestConfig->wakeUpCacheQueueFull = data.ReadBool();
    sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(RegisterCachedCallback(requestConfig, callback));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::UnregisterCachedCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(UnregisterCachedCallback(callback));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::GetCachedGnssLocationsSizeInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    int size = -1;
    reply.WriteInt32(GetCachedGnssLocationsSize(size));
    reply.WriteInt32(size);
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::FlushCachedGnssLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(FlushCachedGnssLocations());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::SendCommandInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::EnableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(EnableMock());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::DisableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(DisableMock());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::AddFenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    std::shared_ptr<GeoFence> geofence = std::make_shared<GeoFence>();
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    request->SetScenario(data.ReadInt32());
    geofence->latitude = data.ReadDouble();
    geofence->longitude = data.ReadDouble();
    geofence->radius = data.ReadDouble();
    geofence->expiration = data.ReadDouble();
    geofence->coordinateSystemType = static_cast<CoordinateSystemType>(data.ReadInt32());
    request->SetGeofence(geofence);
    auto agent = data.ReadParcelable<AbilityRuntime::WantAgent::WantAgent>();
    request->SetWantAgent(*agent);
    reply.WriteInt32(AddFence(request));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RemoveFenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    std::shared_ptr<GeoFence> geofence = std::make_shared<GeoFence>();
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    request->SetScenario(data.ReadInt32());
    geofence->latitude = data.ReadDouble();
    geofence->longitude = data.ReadDouble();
    geofence->radius = data.ReadDouble();
    geofence->expiration = data.ReadDouble();
    geofence->coordinateSystemType = static_cast<CoordinateSystemType>(data.ReadInt32());
    request->SetGeofence(geofence);
    auto agent = data.ReadParcelable<AbilityRuntime::WantAgent::WantAgent>();
    request->SetWantAgent(*agent);
    reply.WriteInt32(RemoveFence(request));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::AddGnssGeofenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    auto request = GeofenceRequest::Unmarshalling(data);
    reply.WriteInt32(AddGnssGeofence(request));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RemoveGnssGeofenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
    request->SetFenceId(data.ReadInt32());
    request->SetBundleName(data.ReadString());
    reply.WriteInt32(RemoveGnssGeofence(request));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::QuerySupportCoordinateSystemTypeInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    auto errCode = QuerySupportCoordinateSystemType(coordinateSystemTypes);
    reply.WriteInt32(errCode);
    if (errCode != ERRCODE_SUCCESS) {
        return errCode;
    }
    int size = coordinateSystemTypes.size() > COORDINATE_SYSTEM_TYPE_SIZE ?
        COORDINATE_SYSTEM_TYPE_SIZE : coordinateSystemTypes.size();
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        reply.WriteInt32(static_cast<int>(coordinateSystemTypes[i]));
    }
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    AppIdentity identity;
    identity.SetPid(callingPid);
    identity.SetUid(callingUid);
    std::string currentTime = std::to_string(CommonUtils::GetCurrentTimeStamp());
    LBSLOGI(GNSS, "%{public}s cmd = %{public}u, flags= %{public}d, identity = %{public}s, timestamp = %{public}s",
        __func__, code, option.GetFlags(), identity.ToString().c_str(), currentTime.c_str());

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GNSS, "invalid token.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int ret = ERRCODE_SUCCESS;
    isMessageRequest_ = false;
    auto handleFunc = GnssMsgHandleMap_.find(code);
    if (handleFunc != GnssMsgHandleMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        ret = (this->*memberFunc)(data, reply, identity);
    } else {
        LBSLOGE(GNSS, "OnReceived cmd = %{public}u, unsupport service.", code);
        reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    if (!isMessageRequest_) {
        UnloadGnssSystemAbility();
    }
    return ret;
}

GnssStatusCallbackDeathRecipient::GnssStatusCallbackDeathRecipient()
{
}

GnssStatusCallbackDeathRecipient::~GnssStatusCallbackDeathRecipient()
{
}

void GnssStatusCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->UnregisterGnssStatusCallback(remote.promote());
        gnssAbility->UnloadGnssSystemAbility();
        LBSLOGI(LOCATOR, "gnss status callback OnRemoteDied");
    }
}

NmeaCallbackDeathRecipient::NmeaCallbackDeathRecipient()
{
}

NmeaCallbackDeathRecipient::~NmeaCallbackDeathRecipient()
{
}

void NmeaCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->UnregisterNmeaMessageCallback(remote.promote());
        gnssAbility->UnloadGnssSystemAbility();
        LBSLOGI(LOCATOR, "nmea callback OnRemoteDied");
    }
}

CachedLocationCallbackDeathRecipient::CachedLocationCallbackDeathRecipient()
{
}

CachedLocationCallbackDeathRecipient::~CachedLocationCallbackDeathRecipient()
{
}

void CachedLocationCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->UnregisterCachedCallback(remote.promote());
        gnssAbility->UnloadGnssSystemAbility();
        LBSLOGI(LOCATOR, "cached location callback OnRemoteDied");
    }
}

GnssGeofenceCallbackDeathRecipient::GnssGeofenceCallbackDeathRecipient()
{
}

GnssGeofenceCallbackDeathRecipient::~GnssGeofenceCallbackDeathRecipient()
{
}

void GnssGeofenceCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->RemoveGnssGeofenceRequestByCallback(remote.promote());
        gnssAbility->UnloadGnssSystemAbility();
        LBSLOGI(LOCATOR, "gnss geofence location callback OnRemoteDied");
    }
}
} // namespace Location
} // namespace OHOS
#endif
