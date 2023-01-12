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
#include "location_switch_callback_host.h"
#include "nmea_message_callback_host.h"
#include "subability_common.h"
#include "work_record.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int32_t MIN_DATA_LEN = 4;

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
        int locSize;
        proxy->GetCachedGnssLocationsSize(locSize);
        proxy->FlushCachedGnssLocations();
        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        proxy->SendCommand(command);
        std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
        proxy->AddFence(fence);
        proxy->RemoveFence(fence);
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        proxy->EnableMock();
        proxy->DisableMock();
        proxy->SetMocked(data[index++], locations);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GnssProxyFuzzTest(data, size);
    return 0;
}

