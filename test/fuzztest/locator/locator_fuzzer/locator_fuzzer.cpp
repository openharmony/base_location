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

#include "locator_fuzzer.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_host.h"
#endif
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code_callback_host.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_host.h"
#endif
#include "i_locator_callback.h"
#include "location.h"
#include "location_switch_callback_host.h"
#include "locator_callback_host.h"
#include "location_log.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_host.h"
#endif
#include "request_config.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "satellite_status.h"
#endif
#include "permission_manager.h"

namespace OHOS {
using namespace OHOS::Location;
auto locatorCallbackHostForTest_ =
    sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
bool g_isGrant = false;
const int32_t MAX_CODE_LEN  = 512;
const int32_t MAX_CODE_NUM = 40;
const int32_t MIN_SIZE_NUM = 10;
const int32_t SWITCH_STATE_ON = 1;
const int32_t WAIT_TIME_SEC = 1000;
const int32_t COUNT = 10;

bool LocatorProxySendRequestTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size > MAX_CODE_LEN)) {
        LBSLOGE(LOCATOR, "param error");
        return false;
    }
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        LBSLOGE(LOCATOR, "systemAbilityManager is nullptr");
        return false;
    }
    sptr<IRemoteObject> object = systemAbilityManager->GetSystemAbility(LOCATION_LOCATOR_SA_ID);
    auto client = std::make_unique<LocatorProxyTestFuzzer>(object);
    if (client == nullptr) {
        LBSLOGE(LOCATOR, "client is nullptr");
        return false;
    }
    MessageParcel request;
    if (!request.WriteInterfaceToken(client->GetDescriptor())) {
        LBSLOGE(LOCATOR, "cannot write interface token");
        return false;
    }
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = client->GetRemote();
    if (remote == nullptr) {
        LBSLOGE(LOCATOR, "cannot get remote object");
        return false;
    }
    int index = 0;
    int32_t result = remote->SendRequest(data[index++] % MAX_CODE_NUM, request, reply, option);
    return result == SUCCESS;
}
    
void AddPermission()
{
    if (!g_isGrant) {
        const char *perms[] = {
            ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
            ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        };
        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 4,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "LocatorFuzzer",
            .aplStr = "system_basic",
        };
        uint64_t tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_isGrant = true;
    }
}

#ifdef FEATURE_GNSS_SUPPORT
bool CachedLocationsCallbackHostFuzzerTest(const uint8_t* data, size_t size)
{
    int index = 0;
    auto cachedCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    MessageParcel request;
    MessageParcel reply;
    MessageOption option;
    cachedCallbackHost->OnRemoteRequest(data[index++], request, reply, option);
    cachedCallbackHost->IsRemoteDied();

    std::vector<std::unique_ptr<OHOS::Location::Location>> locationsForSend;
    cachedCallbackHost->Send(locationsForSend);
    std::vector<std::unique_ptr<OHOS::Location::Location>> locationsForReport;
    cachedCallbackHost->OnCacheLocationsReport(locationsForReport);
    cachedCallbackHost->DeleteHandler();
    return true;
}
#endif

bool CountryCodeCallbackHostFuzzerTest(const uint8_t* data, size_t size)
{
    int index = 0;
    auto callbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    MessageParcel request;
    MessageParcel reply;
    MessageOption option;
    callbackHost->OnRemoteRequest(data[index++], request, reply, option);

    auto countryCodePtr = CountryCode::Unmarshalling(request);
    callbackHost->Send(countryCodePtr);
    callbackHost->SetEnv(nullptr);
    callbackHost->SetCallback(nullptr);
    callbackHost->OnCountryCodeChange(countryCodePtr);
    callbackHost->DeleteHandler();
    return true;
}

#ifdef FEATURE_GNSS_SUPPORT
bool GnssStatusCallbackHostFuzzerTest(const uint8_t* data, size_t size)
{
    int index = 0;
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    MessageParcel request;
    MessageParcel reply;
    MessageOption option;
    gnssCallbackHost->OnRemoteRequest(data[index++], request, reply, option);
    gnssCallbackHost->IsRemoteDied();
    std::unique_ptr<SatelliteStatus> statusInfo = nullptr;
    gnssCallbackHost->Send(statusInfo);

    gnssCallbackHost->OnStatusChange(statusInfo);
    gnssCallbackHost->DeleteHandler();
    return true;
}
#endif

bool LocationSwitchCallbackHostFuzzerTest(const uint8_t* data, size_t size)
{
    int index = 0;
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    MessageParcel request;
    MessageParcel reply;
    MessageOption option;
    switchCallbackHost->OnRemoteRequest(data[index++], request, reply, option);
    switchCallbackHost->IsRemoteDied();
    switchCallbackHost->PackResult(true);
    switchCallbackHost->Send(SWITCH_STATE_ON);

    switchCallbackHost->OnSwitchChange(SWITCH_STATE_ON);
    switchCallbackHost->DeleteHandler();
    return true;
}

bool LocationCallbackHostFuzzerTest(const uint8_t* data, size_t size)
{
    int index = 0;
    auto callbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    MessageParcel request;
    MessageParcel reply;
    MessageOption option;
    callbackHost->OnRemoteRequest(data[index++], request, reply, option);
    callbackHost->OnErrorReport(SUCCESS);
    std::unique_ptr<OHOS::Location::Location> location =
        std::make_unique<OHOS::Location::Location>();
    callbackHost->OnLocationReport(location);
    callbackHost->OnLocatingStatusChange(SWITCH_STATE_ON);

    callbackHost->DeleteAllCallbacks();
    callbackHost->IsSystemGeoLocationApi();
    callbackHost->IsSingleLocationRequest();
    callbackHost->CountDown();
    callbackHost->Wait(data[index++] % WAIT_TIME_SEC);
    callbackHost->SetCount(data[index++] % COUNT);
    callbackHost->GetCount();
    return true;
}

#ifdef FEATURE_GNSS_SUPPORT
bool NmeaMessageCallbackHostFuzzerTest(const uint8_t* data, size_t size)
{
    int index = 0;
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    MessageParcel request;
    MessageParcel reply;
    MessageOption option;
    nmeaCallbackHost->OnRemoteRequest(data[index++], request, reply, option);
    nmeaCallbackHost->IsRemoteDied();
    std::string msg(reinterpret_cast<const char*>(data), size);
    nmeaCallbackHost->PackResult(msg);
    nmeaCallbackHost->Send(msg);
    int64_t timestamp = 0;
    nmeaCallbackHost->OnMessageChange(timestamp, msg);
    nmeaCallbackHost->DeleteHandler();
    return true;
}
#endif
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_SIZE_NUM) {
        return 0;
    }
    /* Run your code on data */
    OHOS::AddPermission();
    OHOS::LocatorProxySendRequestTest(data, size);
#ifdef FEATURE_GNSS_SUPPORT
    OHOS::CachedLocationsCallbackHostFuzzerTest(data, size);
#endif
    OHOS::CountryCodeCallbackHostFuzzerTest(data, size);
#ifdef FEATURE_GNSS_SUPPORT
    OHOS::GnssStatusCallbackHostFuzzerTest(data, size);
#endif
    OHOS::LocationSwitchCallbackHostFuzzerTest(data, size);
    OHOS::LocationCallbackHostFuzzerTest(data, size);
#ifdef FEATURE_GNSS_SUPPORT
    OHOS::NmeaMessageCallbackHostFuzzerTest(data, size);
#endif
    return 0;
}

