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
int GnssAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t lastCallingPid = IPCSkeleton::GetCallingPid();
    pid_t lastCallinguid = IPCSkeleton::GetCallingUid();
    LBSLOGI(GNSS, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), lastCallingPid, lastCallinguid);

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GNSS, "invalid token.");
        return REPLY_CODE_EXCEPTION;
    }

    int ret = REPLY_CODE_NO_EXCEPTION;
    switch (code) {
        case SEND_LOCATION_REQUEST: {
            int64_t interval = data.ReadInt64();
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            SendLocationRequest((uint64_t)interval, *workrecord);
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
            std::unique_ptr<CachedGnssLocationsRequest> requestConfig = std::make_unique<CachedGnssLocationsRequest>();
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
            ret = FlushCachedGnssLocations();
            break;
        }
        case SEND_COMMANDS: {
            std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
            locationCommand->scenario =  data.ReadInt32();
            locationCommand->command = data.ReadBool();
            SendCommand(locationCommand);
            break;
        }
        case ENABLE_LOCATION_MOCK: {
            std::unique_ptr<LocationMockConfig> mockConfig = LocationMockConfig::Unmarshalling(data);
            LocationMockConfig config;
            config.Set(*mockConfig);
            bool result = EnableMock(config);
            reply.WriteBool(result);
            break;
        }
        case DISABLE_LOCATION_MOCK: {
            std::unique_ptr<LocationMockConfig> mockConfig = LocationMockConfig::Unmarshalling(data);
            LocationMockConfig config;
            config.Set(*mockConfig);
            bool result = DisableMock(config);
            reply.WriteBool(result);
            break;
        }
        case SET_MOCKED_LOCATIONS: {
            std::unique_ptr<LocationMockConfig> mockConfig = LocationMockConfig::Unmarshalling(data);
            LocationMockConfig config;
            config.Set(*mockConfig);
            int locationSize = data.ReadInt32();
            std::vector<std::shared_ptr<Location>> vcLoc;
            for (int i = 0; i < locationSize; i++) {
                vcLoc.push_back(Location::UnmarshallingShared(data));
            }
            bool result = SetMocked(config, vcLoc);
            reply.WriteBool(result);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}
} // namespace Location
} // namespace OHOS