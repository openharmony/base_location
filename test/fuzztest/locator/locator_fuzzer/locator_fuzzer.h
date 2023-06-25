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
#ifndef LOCATOR_FUZZER_H
#define LOCATOR_FUZZER_H
#include <thread>

#include "securec.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_host.h"
#endif
#include "country_code_callback_host.h"
#include "locator_callback_proxy.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_host.h"
#endif
#include "i_locator.h"
#include "locator_proxy.h"
#include "location_switch_callback_host.h"
#include "locator_callback_host.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_host.h"
#endif


namespace OHOS {
namespace Location {
#define FUZZ_PROJECT_NAME "locator_fuzzer"
class LocatorProxyTestFuzzer : public LocatorProxy {
public:
    explicit LocatorProxyTestFuzzer(const sptr<IRemoteObject> &impl)
        : LocatorProxy(impl)
    {}
    sptr<IRemoteObject> GetRemote()
    {
        return Remote();
    }
};

void AddPermission();
bool LocatorProxySendRequestTest(const uint8_t* data, size_t size);
bool LocatorImplFuzzerTest(const uint8_t* data, size_t size);
bool TestMockFunc(const uint8_t* data, size_t size);
bool TestCallbackRegister(const uint8_t* data, size_t size);
#ifdef FEATURE_GNSS_SUPPORT
bool CachedLocationsCallbackHostFuzzerTest(const uint8_t* data, size_t size);
bool NmeaMessageCallbackHostFuzzerTest(const uint8_t* data, size_t size);
bool GnssStatusCallbackHostFuzzerTest(const uint8_t* data, size_t size);
bool CachedLocationsCallbackHostFuzzTest(const char* data, size_t size);
bool NmeaMessageCallbackHostFuzzTest(const char* data, size_t size);
bool GnssStatusCallbackHostFuzzTest(const char* data, size_t size);
bool GnssAbilityFuzzTest(const char* data, size_t size);
#endif
bool CountryCodeCallbackHostFuzzerTest(const uint8_t* data, size_t size);
bool LocationSwitchCallbackHostFuzzerTest(const uint8_t* data, size_t size);
bool LocationCallbackHostFuzzerTest(const uint8_t* data, size_t size);
bool LocatorAbility001FuzzerTest(const uint8_t* data, size_t size);
bool LocatorAbility002FuzzerTest(const uint8_t* data, size_t size);
bool LocatorAbility003FuzzerTest(const uint8_t* data, size_t size);
bool LocatorAbilityStubFuzzTest(const char* data, size_t size);
bool CountryCodeCallbackHostFuzzTest(const char* data, size_t size);
#ifdef FEATURE_NETWORK_SUPPORT
bool NetworkAbilityFuzzTest(const char* data, size_t size);
bool NetworkCallbackHostFuzzTest(const char* data, size_t size);
#endif
bool LocatorCallbackStubFuzzTest(const char* data, size_t size);
#ifdef FEATURE_PASSIVE_SUPPORT
bool PassiveAbilityFuzzTest(const char* data, size_t size);
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
bool GeoConvertServiceFuzzTest(const char* data, size_t size);
#endif
bool LocationSwitchCallbackHostFuzzTest(const char* data, size_t size);
bool LocatorCallbackHostFuzzTest(const char* data, size_t size);
bool LocatorAbility004FuzzerTest(const uint8_t* data, size_t size);
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_FUZZER_H
