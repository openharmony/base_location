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
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SendLocationRequestInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SetEnableInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REFRESH_REQUESTS)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return RefreshRequirementsInner(data, reply, identity);
    };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_GNSS_STATUS)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return RegisterGnssStatusCallbackInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_GNSS_STATUS)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return UnregisterGnssStatusCallbackInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_NMEA)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return RegisterNmeaMessageCallbackInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_NMEA)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return UnregisterNmeaMessageCallbackInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REG_CACHED)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return RegisterCachedCallbackInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::UNREG_CACHED)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return UnregisterCachedCallbackInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::GET_CACHED_SIZE)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return GetCachedGnssLocationsSizeInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::FLUSH_CACHED)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return FlushCachedGnssLocationsInner(data, reply, identity);
        };
}

void GnssAbilityStub::InitGnssEnhanceMsgHandleMap()
{
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SendCommandInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SetMockLocationsInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::ENABLE_LOCATION_MOCK)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return EnableMockInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::DISABLE_LOCATION_MOCK)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return DisableMockInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::ADD_FENCE_INFO)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return AddFenceInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FENCE_INFO)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return RemoveFenceInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::ADD_GNSS_GEOFENCE)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return AddGnssGeofenceInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::REMOVE_GNSS_GEOFENCE)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return RemoveGnssGeofenceInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::GET_GEOFENCE_SUPPORT_COORDINATE_SYSTEM_TYPE)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return QuerySupportCoordinateSystemTypeInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_NETWORK_LOCATION)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return SendNetworkLocationInner(data, reply, identity);
        };
    GnssMsgHandleMap_[static_cast<uint32_t>(GnssInterfaceCode::GET_ACTIVE_FENCES)] =
        [this](MessageParcel &data, MessageParcel &reply, AppIdentity &identity) {
        return GetActiveGeoFencesInner(data, reply, identity);
        };
}

GnssAbilityStub::GnssAbilityStub()
{
    InitGnssMsgHandleMap();
    InitGnssEnhanceMsgHandleMap();
}

int GnssAbilityStub::SendLocationRequestInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::SetMockLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::SetEnableInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RefreshRequirementsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(RefrashRequirements());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RegisterGnssStatusCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    AppIdentity appIdentity;
    appIdentity.ReadFromParcel(data);
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(RegisterGnssStatusCallback(client, appIdentity));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::UnregisterGnssStatusCallbackInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(UnregisterGnssStatusCallback(client));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RegisterNmeaMessageCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    AppIdentity appIdentity;
    appIdentity.ReadFromParcel(data);
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(RegisterNmeaMessageCallback(client, appIdentity));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::UnregisterNmeaMessageCallbackInner(MessageParcel &data, MessageParcel &reply,
    AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(UnregisterNmeaMessageCallback(client));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RegisterCachedCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
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
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
    reply.WriteInt32(UnregisterCachedCallback(callback));
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::GetCachedGnssLocationsSizeInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    int size = -1;
    reply.WriteInt32(GetCachedGnssLocationsSize(size));
    reply.WriteInt32(size);
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::FlushCachedGnssLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(FlushCachedGnssLocations());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::SendCommandInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::EnableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(EnableMock());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::DisableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    reply.WriteInt32(DisableMock());
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::AddFenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssAbilityInterfaceCode::ADD_FENCE), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RemoveFenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssAbilityInterfaceCode::REMOVE_FENCE), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::AddGnssGeofenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssAbilityInterfaceCode::ADD_GEOFENCE), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::RemoveGnssGeofenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssAbilityInterfaceCode::REMOVE_GEOFENCE), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::QuerySupportCoordinateSystemTypeInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    auto errCode = QuerySupportCoordinateSystemType(coordinateSystemTypes);
    reply.WriteInt32(errCode);
    if (errCode != ERRCODE_SUCCESS) {
        return errCode;
    }
    int size = static_cast<int>(coordinateSystemTypes.size()) > COORDINATE_SYSTEM_TYPE_SIZE ?
        COORDINATE_SYSTEM_TYPE_SIZE : static_cast<int>(coordinateSystemTypes.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        reply.WriteInt32(static_cast<int>(coordinateSystemTypes[i]));
    }
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::SendNetworkLocationInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    LBSLOGI(GNSS, "SendNetworkLocationInner");
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    SendMessage(static_cast<uint32_t>(GnssInterfaceCode::SEND_NETWORK_LOCATION), data, reply);
    isMessageRequest_ = true;
    return ERRCODE_SUCCESS;
}

int GnssAbilityStub::GetActiveGeoFencesInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return LOCATION_ERRCODE_PERMISSION_DENIED;
    }
    std::map<int, std::shared_ptr<Geofence>> fenceMap;
    std::string bundleName = data.ReadString();
    auto errCode = GetActiveGeoFences(bundleName, fenceMap);
    reply.WriteInt32(errCode);
    if (errCode != ERRCODE_SUCCESS) {
        return errCode;
    }
    reply.WriteInt32(fenceMap.size());
    for (const auto& pair : fenceMap) {
        reply.WriteInt32(pair.first);
        auto fence = pair.second;
        fence->Marshalling(reply);
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
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    CancelIdleState();
    int ret = ERRCODE_SUCCESS;
    isMessageRequest_ = false;
    auto handleFunc = GnssMsgHandleMap_.find(code);
    if (handleFunc != GnssMsgHandleMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        ret = memberFunc(data, reply, identity);
    } else {
        LBSLOGE(GNSS, "OnReceived cmd = %{public}u, unsupport service.", code);
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
    auto gnssAbility = GnssAbility::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->UnregisterGnssStatusCallback(remote.promote());
        gnssAbility->UnloadGnssSystemAbility();
        LBSLOGD(LOCATOR, "gnss status callback OnRemoteDied");
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
    auto gnssAbility = GnssAbility::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->UnregisterNmeaMessageCallback(remote.promote());
        gnssAbility->UnloadGnssSystemAbility();
        LBSLOGD(LOCATOR, "nmea callback OnRemoteDied");
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
    auto gnssAbility = GnssAbility::GetInstance();
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
    auto gnssAbility = GnssAbility::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->RemoveGnssGeofenceRequestByCallback(remote.promote());
        gnssAbility->UnloadGnssSystemAbility();
        LBSLOGI(LOCATOR, "gnss geofence location callback OnRemoteDied");
    }
}
} // namespace Location
} // namespace OHOS
#endif
