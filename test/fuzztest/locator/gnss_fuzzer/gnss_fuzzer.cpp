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

#include "cached_locations_callback_host.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code_callback_host.h"
#include "gnss_ability.h"
#include "gnss_ability_proxy.h"
#include "gnss_status_callback_host.h"
#include "location.h"
#include "location_mock_config.h"
#include "location_switch_callback_host.h"
#include "nmea_message_callback_host.h"
#include "subability_common.h"
#include "work_record.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int32_t MAX_CODE_LEN  = 512;
    const int32_t MAX_CODE_NUM = 27;
    const int32_t MIN_DATA_LEN = 4;
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
        if ((data == nullptr) || (size == 0)) {
            return false;
        }
        if (size > MAX_CODE_LEN) {
            return false;
        }
        MessageParcel parcel;
        MessageParcel reply;
        MessageOption option;
        sptr<GnssAbilityTestFuzzer> gnssAbilityFuzzer = new GnssAbilityTestFuzzer();
        int index = 0;
        int32_t ret = gnssAbilityFuzzer->OnRemoteRequest(data[index++] % MAX_CODE_NUM, parcel, reply, option);
        return ret;
    }

    bool GnssProxyFuzzTest(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        int index = 0;
        sptr<OHOS::Location::GnssAbility> ability = new (std::nothrow) GnssAbility();
        sptr<OHOS::Location::GnssAbilityProxy> proxy =
            new (std::nothrow) GnssAbilityProxy(ability);
        proxy->SetEnable(true);
        proxy->SetEnable(false);
        proxy->RefrashRequirements();
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        proxy->RegisterGnssStatusCallback(gnssCallbackHost, data[index++]);
        proxy->UnregisterGnssStatusCallback(gnssCallbackHost);
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        proxy->RegisterNmeaMessageCallback(nmeaCallbackHost, data[index++]);
        proxy->UnregisterNmeaMessageCallback(nmeaCallbackHost);
        auto cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
        auto cachedLocationsCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        proxy->RegisterCachedCallback(cachedRequest, cachedLocationsCallbackHost);
        proxy->UnregisterCachedCallback(cachedLocationsCallbackHost);

        proxy->GetCachedGnssLocationsSize();
        proxy->FlushCachedGnssLocations();
        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        proxy->SendCommand(command);
        std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
        proxy->AddFence(fence);
        proxy->RemoveFence(fence);
        LocationMockConfig mockInfo;
        mockInfo.SetScenario(data[index++]);
        mockInfo.SetTimeInterval(data[index++]);
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        proxy->EnableMock(mockInfo);
        proxy->DisableMock(mockInfo);
        proxy->SetMocked(mockInfo, locations);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GnssAbilityFuzzTest(data, size);
    OHOS::GnssProxyFuzzTest(data, size);
    return 0;
}

