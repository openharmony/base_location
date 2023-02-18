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
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(GNSS, "uid pid not match locationhub process.");
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }

    int ret = ERRCODE_SUCCESS;
    switch (code) {
        case SEND_LOCATION_REQUEST: // fall through
        case SET_MOCKED_LOCATIONS: {
            SendMessage(code, data, reply);
            break;
        }
        case SET_ENABLE: {
            reply.WriteInt32(SetEnable(data.ReadBool()));
            break;
        }
        case REFRESH_REQUESTS: {
            reply.WriteInt32(RefrashRequirements());
            break;
        }
        case REG_GNSS_STATUS: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(RegisterGnssStatusCallback(client, callingUid));
            break;
        }
        case UNREG_GNSS_STATUS: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(UnregisterGnssStatusCallback(client));
            break;
        }
        case REG_NMEA: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(RegisterNmeaMessageCallback(client, callingUid));
            break;
        }
        case UNREG_NMEA: {
            sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(UnregisterNmeaMessageCallback(client));
            break;
        }
        case REG_CACHED: {
            std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
            requestConfig->reportingPeriodSec = data.ReadInt32();
            requestConfig->wakeUpCacheQueueFull = data.ReadBool();
            sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(RegisterCachedCallback(requestConfig, callback));
            break;
        }
        case UNREG_CACHED: {
            sptr<IRemoteObject> callback = data.ReadObject<IRemoteObject>();
            reply.WriteInt32(UnregisterCachedCallback(callback));
            break;
        }
        case GET_CACHED_SIZE: {
            int size = -1;
            reply.WriteInt32(GetCachedGnssLocationsSize(size));
            reply.WriteInt32(size);
            break;
        }
        case FLUSH_CACHED: {
            reply.WriteInt32(FlushCachedGnssLocations());
            break;
        }
        case SEND_COMMANDS: {
            std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
            locationCommand->scenario =  data.ReadInt32();
            locationCommand->command = data.ReadBool();
            reply.WriteInt32(SendCommand(locationCommand));
            break;
        }
        case ENABLE_LOCATION_MOCK: {
            reply.WriteInt32(EnableMock());
            break;
        }
        case DISABLE_LOCATION_MOCK: {
            reply.WriteInt32(DisableMock());
            break;
        }
        case ADD_FENCE_INFO: {
            std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
            request->scenario = data.ReadInt32();
            request->geofence.latitude = data.ReadDouble();
            request->geofence.longitude = data.ReadDouble();
            request->geofence.radius = data.ReadDouble();
            request->geofence.expiration = data.ReadDouble();
            reply.WriteInt32(AddFence(request));
            break;
        }
        case REMOVE_FENCE_INFO: {
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
    return ret;
}
} // namespace Location
} // namespace OHOS
#endif
