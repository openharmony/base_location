/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "gnss_fuzzer.h"

#include <vector>

#include "ipc_skeleton.h"
#include "iremote_object.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location.h"
#include "location_mock_config.h"
#include "subability_common.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int32_t MAX_CODE_LEN  = 512;
    const int32_t MAX_CODE_NUM = 27;

    int GnssAbilityTestFuzzer::OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        int ret = REPLY_CODE_NO_EXCEPTION;
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        pid_t callingUid = IPCSkeleton::GetCallingUid();
        LBSLOGI(GNSS, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
            code, option.GetFlags(), callingPid, callingUid);
        switch (code) {
            case SEND_LOCATION_REQUEST: {
                SendMessage(code, data);
                break;
            }
            case SET_ENABLE: {
                SetEnable(data.ReadBool());
                break;
            }
            case REFRESH_REQUESTS: {
                RefrashRequirements();
                break;
            }
            case REG_GNSS_STATUS: {
                sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
                RegisterGnssStatusCallback(client, callingUid);
                break;
            }
            case UNREG_GNSS_STATUS: {
                sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
                UnregisterGnssStatusCallback(client);
                break;
            }
            case REG_NMEA: {
                sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
                RegisterNmeaMessageCallback(client, callingUid);
                break;
            }
            case UNREG_NMEA: {
                sptr<IRemoteObject> client = data.ReadObject<IRemoteObject>();
                UnregisterNmeaMessageCallback(client);
                break;
            }
            case REG_CACHED: {
                std::unique_ptr<CachedGnssLocationsRequest> requestConfig =
                    std::make_unique<CachedGnssLocationsRequest>();
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
                locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
                    locationSize;
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

    bool GnssAbilityFuzzTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size <= 0)) {
            return false;
        }
        if (size > MAX_CODE_LEN) {
            return false;
        }
        MessageParcel parcel;
        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        code %= MAX_CODE_NUM;
        size -= sizeof(uint32_t);
        parcel.WriteBuffer(data + sizeof(uint32_t), size);
        parcel.RewindRead(0);
        sptr<GnssAbilityTestFuzzer> gnssAbilityFuzzer = new GnssAbilityTestFuzzer();
        MessageParcel reply;
        MessageOption option;
        int32_t ret = gnssAbilityFuzzer->OnRemoteRequest(code, parcel, reply, option);
        return ret;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GnssAbilityFuzzTest(data, size);
    return 0;
}

