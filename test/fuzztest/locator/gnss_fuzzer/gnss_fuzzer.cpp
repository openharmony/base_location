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

#include "gnss_fuzzer.h"

#include <vector>
#include <thread>

#include "ipc_skeleton.h"
#include "iremote_object.h"

#include "cached_locations_callback_napi.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code_callback_napi.h"
#include "gnss_ability.h"
#include "gnss_ability_proxy.h"
#include "gnss_status_callback_napi.h"
#include "location.h"
#include "location_switch_callback_napi.h"
#include "nmea_message_callback_napi.h"
#include "subability_common.h"
#include "work_record.h"
#include "geofence_request.h"
#include "work_record_statistic.h"
#include "if_system_ability_manager.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"

namespace OHOS {
    using namespace OHOS::Location;
#ifdef FEATURE_GNSS_SUPPORT
    const int32_t MIN_DATA_LEN = 4;
    const int32_t SLEEP_TIMES = 1000;
    bool GnssProxyFuzzTest001(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        int index = 0;
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
            return true;
        }
        sptr<IRemoteObject> obj = sam->CheckSystemAbility(LOCATION_GNSS_SA_ID);
        if (obj == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "%{public}s: get remote service failed.", __func__);
            return true;
        }
        auto proxy = sptr<GnssAbilityProxy>(new (std::nothrow) GnssAbilityProxy(obj));
        proxy->SetEnable(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->SetEnable(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->RefrashRequirements();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackNapi>(new (std::nothrow) GnssStatusCallbackNapi());
        AppIdentity identity;
        identity.SetPid(data[index++]);
        proxy->RegisterGnssStatusCallback(gnssCallbackHost, identity);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        proxy->UnregisterGnssStatusCallback(gnssCallbackHost);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackNapi>(new (std::nothrow) NmeaMessageCallbackNapi());
        proxy->RegisterNmeaMessageCallback(nmeaCallbackHost, identity);
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
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
            return true;
        }
        sptr<IRemoteObject> obj = sam->CheckSystemAbility(LOCATION_GNSS_SA_ID);
        if (obj == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "%{public}s: get remote service failed.", __func__);
            return true;
        }
        auto proxy = sptr<GnssAbilityProxy>(new (std::nothrow) GnssAbilityProxy(obj));
        auto cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
        auto cachedLocationsCallbackHost =
            sptr<CachedLocationsCallbackNapi>(new (std::nothrow) CachedLocationsCallbackNapi());
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
        sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
            return true;
        }
        sptr<IRemoteObject> obj = sam->CheckSystemAbility(LOCATION_GNSS_SA_ID);
        if (obj == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "%{public}s: get remote service failed.", __func__);
            return true;
        }
        auto proxy = sptr<GnssAbilityProxy>(new (std::nothrow) GnssAbilityProxy(obj));
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
#endif // FEATURE_GNSS_SUPPORT
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
#ifdef FEATURE_GNSS_SUPPORT
    /* Run your code on data */
    OHOS::GnssProxyFuzzTest001(data, size);
    OHOS::GnssProxyFuzzTest002(data, size);
    OHOS::GnssProxyFuzzTest003(data, size);
    return 0;
#endif // FEATURE_GNSS_SUPPORT
}
