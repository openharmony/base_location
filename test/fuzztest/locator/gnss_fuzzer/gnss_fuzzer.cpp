/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "gnss_fuzzer.h"

#include <vector>
#include <thread>

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
#include "location_switch_callback_host.h"
#include "nmea_message_callback_host.h"
#include "subability_common.h"
#include "work_record.h"
#include "geofence_request.h"
#include "work_record_statistic.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int32_t MIN_DATA_LEN = 4;
    const int32_t SLEEP_TIMES = 1000;
    bool GnssProxyFuzzTest001(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        int index = 0;
        sptr<OHOS::Location::GnssAbility> ability = new (std::nothrow) GnssAbility();
        sptr<OHOS::Location::GnssAbilityProxy> proxy =
            new (std::nothrow) GnssAbilityProxy(ability);
        proxy->SetEnable(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->SetEnable(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->RefrashRequirements();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        proxy->RegisterGnssStatusCallback(gnssCallbackHost, data[index++]);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->UnregisterGnssStatusCallback(gnssCallbackHost);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        proxy->RegisterNmeaMessageCallback(nmeaCallbackHost, data[index++]);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->UnregisterNmeaMessageCallback(nmeaCallbackHost);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        WorkRecordStatistic::DestroyInstance();
        return true;
    }

    bool GnssProxyFuzzTest002(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        int index = 0;
        sptr<OHOS::Location::GnssAbility> ability = new (std::nothrow) GnssAbility();
        sptr<OHOS::Location::GnssAbilityProxy> proxy =
            new (std::nothrow) GnssAbilityProxy(ability);
        auto cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
        auto cachedLocationsCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        proxy->RegisterCachedCallback(cachedRequest, cachedLocationsCallbackHost);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->UnregisterCachedCallback(cachedLocationsCallbackHost);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        int locSize;
        proxy->GetCachedGnssLocationsSize(locSize);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->FlushCachedGnssLocations();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        proxy->SendCommand(command);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        std::shared_ptr<GeofenceRequest> fence = std::make_shared<GeofenceRequest>();
        proxy->AddFence(fence);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->RemoveFence(fence);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        proxy->EnableMock();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->DisableMock();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->SetMocked(data[index++], locations);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        WorkRecordStatistic::DestroyInstance();
        return true;
    }

    bool GnssProxyFuzzTest003(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        sptr<OHOS::Location::GnssAbility> ability = new (std::nothrow) GnssAbility();
        sptr<OHOS::Location::GnssAbilityProxy> proxy =
            new (std::nothrow) GnssAbilityProxy(ability);
        std::shared_ptr<GeofenceRequest> fence = std::make_shared<GeofenceRequest>();
        proxy->AddGnssGeofence(fence);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->RemoveGnssGeofence(fence);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        std::vector<CoordinateSystemType> coordinateSystemTypes;
        proxy->QuerySupportCoordinateSystemType(coordinateSystemTypes);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        WorkRecordStatistic::DestroyInstance();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GnssProxyFuzzTest001(data, size);
    OHOS::GnssProxyFuzzTest002(data, size);
    OHOS::GnssProxyFuzzTest003(data, size);
    return 0;
}
#endif // FEATURE_GNSS_SUPPORT
