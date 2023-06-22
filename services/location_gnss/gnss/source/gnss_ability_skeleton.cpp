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

namespace OHOS {
namespace Location {
int GnssAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    LBSLOGI(GNSS, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GNSS, "invalid token.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }

    int ret = ERRCODE_SUCCESS;
    bool isMessageRequest = false;
    switch (code) {
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST): // fall through
        case static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            SendMessage(code, data, reply);
            isMessageRequest = true;
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            reply.WriteInt32(SetEnable(data.ReadBool()));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::REFRESH_REQUESTS): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            reply.WriteInt32(RefrashRequirements());
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::REG_GNSS_STATUS): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(RegisterGnssStatusCallback(client, callingUid));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::UNREG_GNSS_STATUS): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(UnregisterGnssStatusCallback(client));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::REG_NMEA): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(RegisterNmeaMessageCallback(client, callingUid));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::UNREG_NMEA): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(UnregisterNmeaMessageCallback(client));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::REG_CACHED): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
            requestConfig->reportingPeriodSec = data.ReadInt32();
            requestConfig->wakeUpCacheQueueFull = data.ReadBool();
            sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(RegisterCachedCallback(requestConfig, callback));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::UNREG_CACHED): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(UnregisterCachedCallback(callback));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::GET_CACHED_SIZE): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            int size = -1;
            reply.WriteInt32(GetCachedGnssLocationsSize(size));
            reply.WriteInt32(size);
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::FLUSH_CACHED): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            reply.WriteInt32(FlushCachedGnssLocations());
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
            locationCommand->scenario =  data.ReadInt32();
            locationCommand->command = data.ReadBool();
            reply.WriteInt32(SendCommand(locationCommand));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::ENABLE_LOCATION_MOCK): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            reply.WriteInt32(EnableMock());
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::DISABLE_LOCATION_MOCK): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            reply.WriteInt32(DisableMock());
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::ADD_FENCE_INFO): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
            request->scenario = data.ReadInt32();
            request->geofence.latitude = data.ReadDouble();
            request->geofence.longitude = data.ReadDouble();
            request->geofence.radius = data.ReadDouble();
            request->geofence.expiration = data.ReadDouble();
            reply.WriteInt32(AddFence(request));
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FENCE_INFO): {
            if (!CommonUtils::CheckCallingPermission(callingUid, callingPid, reply)) {
                return ERRCODE_PERMISSION_DENIED;
            }
            std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
            request->scenario = data.ReadInt32();
            request->geofence.latitude = data.ReadDouble();
            request->geofence.longitude = data.ReadDouble();
            request->geofence.radius = data.ReadDouble();
            request->geofence.expiration = data.ReadDouble();
            reply.WriteInt32(RemoveFence(request));
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    if (!isMessageRequest) {
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
} // namespace Location
} // namespace OHOS
#endif
