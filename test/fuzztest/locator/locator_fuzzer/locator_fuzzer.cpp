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

#include "cached_locations_callback_host.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code_callback_host.h"
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#include "gnss_status_callback_host.h"
#include "i_locator_callback.h"
#include "location.h"
#include "locator.h"
#include "location_switch_callback_host.h"
#include "locator_callback_host.h"
#include "location_log.h"
#include "nmea_message_callback_host.h"
#include "request_config.h"
#include "satellite_status.h"

namespace OHOS {
    using namespace OHOS::Location;
    auto locatorCallbackHostForTest_ =
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    auto g_locatorImpl = Locator::GetInstance();
    bool g_isGrant = false;
    const int FUZZ_DATA_LEN = 8;
    const int32_t MAX_CODE_LEN  = 512;
    const int32_t MAX_CODE_NUM = 40;
    const int32_t MIN_SIZE_NUM = 4;
    const int32_t SWITCH_STATE_ON = 1;
    const int32_t WAIT_TIME_SEC = 1000;
    const int32_t COUNT = 10;

    bool TestStartLocating(const uint8_t* data, size_t size)
    {
        bool result = false;
        if (size < FUZZ_DATA_LEN) {
            return false;
        }
        /* init locator and LocatorCallbackHost */
        std::unique_ptr<Locator> locator = Locator::GetInstance();
        int index = 0;
        locatorCallbackHostForTest_->SetFixNumber(data[index++]);
        /* init requestConfig */
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetScenario(data[index++]);
        requestConfig->SetPriority(data[index++]);
        requestConfig->SetTimeInterval(data[index++]);
        requestConfig->SetDistanceInterval(data[index++]);
        requestConfig->SetMaxAccuracy(data[index++]);
        requestConfig->SetFixNumber(data[index++]);
        requestConfig->SetTimeOut(data[index++]);
        /* test StartLocating */
        sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(locatorCallbackHostForTest_);
        locator->StartLocating(requestConfig, locatorCallback);
        /* test StopLocating */
        locator->StopLocating(locatorCallback);
        /* test GetCurrentLocation */
        requestConfig->SetFixNumber(1);
        locator->StartLocating(requestConfig, locatorCallback);
        locator->StopLocating(locatorCallback);
        return result;
    }
    
    bool LocatorProxySendRequestTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size > MAX_CODE_LEN) || (size < MIN_SIZE_NUM)) {
            LBSLOGE(LOCATOR, "param error");
            return false;
        }
        uint32_t cmdCode = *(reinterpret_cast<const uint32_t*>(data));
        cmdCode %= MAX_CODE_NUM;
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
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
        size -= sizeof(uint32_t);
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        int32_t result = remote->SendRequest(cmdCode, request, reply, option);
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

    bool LocatorImplFuzzerTest(const uint8_t* data, size_t size)
    {
        g_locatorImpl->IsLocationEnabled();
        g_locatorImpl->ShowNotification();
        g_locatorImpl->RequestPermission();
        g_locatorImpl->RequestEnableLocation();

        g_locatorImpl->EnableAbility(false);
        g_locatorImpl->EnableAbility(true);

        g_locatorImpl->GetCachedLocation();
        g_locatorImpl->IsGeoServiceAvailable();
        MessageParcel parcel;
        std::list<std::shared_ptr<GeoAddress>> geoAddressList;
        g_locatorImpl->GetAddressByCoordinate(parcel, geoAddressList);
        g_locatorImpl->GetAddressByLocationName(parcel, geoAddressList);

        int type = *(reinterpret_cast<const int*>(data));
        g_locatorImpl->IsLocationPrivacyConfirmed(type);
        g_locatorImpl->SetLocationPrivacyConfirmStatus(type, true);
        g_locatorImpl->SetLocationPrivacyConfirmStatus(type, false);

        g_locatorImpl->GetCachedGnssLocationsSize();
        g_locatorImpl->FlushCachedGnssLocations();

        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        g_locatorImpl->SendCommand(command);

        std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
        g_locatorImpl->AddFence(fence);
        g_locatorImpl->RemoveFence(fence);
        g_locatorImpl->GetIsoCountryCode();
        g_locatorImpl->ProxyUidForFreeze(*(reinterpret_cast<const int32_t*>(data)), true);
        g_locatorImpl->ProxyUidForFreeze(*(reinterpret_cast<const int32_t*>(data)), false);
        g_locatorImpl->ResetAllProxy();
        return true;
    }

    bool TestCallbackRegister(const uint8_t* data, size_t size)
    {
        pid_t uid = *reinterpret_cast<const pid_t*>(data);
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        g_locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), uid);
        g_locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject());

        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        g_locatorImpl->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), uid);
        g_locatorImpl->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject());

        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        g_locatorImpl->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), uid);
        g_locatorImpl->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject());

        auto countryCodeCallbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
        g_locatorImpl->RegisterCountryCodeCallback(countryCodeCallbackHost->AsObject(), uid);
        g_locatorImpl->UnregisterCountryCodeCallback(countryCodeCallbackHost->AsObject());

        auto cachedLocationsCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
        auto request = std::make_unique<CachedGnssLocationsRequest>();
        g_locatorImpl->RegisterCachedLocationCallback(request, cachedCallback);
        g_locatorImpl->UnregisterCachedLocationCallback(cachedCallback);
        return true;
    }

    bool TestMockFunc(const uint8_t* data, size_t size)
    {
        LocationMockConfig mockInfo;
        mockInfo.SetScenario(*(reinterpret_cast<const int32_t*>(data)));
        mockInfo.SetTimeInterval(*(reinterpret_cast<const int32_t*>(data)));
        g_locatorImpl->EnableLocationMock(mockInfo);
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        g_locatorImpl->SetMockedLocations(mockInfo, locations);
        g_locatorImpl->DisableLocationMock(mockInfo);

        g_locatorImpl->EnableReverseGeocodingMock();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfo;
        g_locatorImpl->SetReverseGeocodingMockInfo(geoMockInfo);
        g_locatorImpl->DisableReverseGeocodingMock();
        return true;
    }

    bool CachedLocationsCallbackHostFuzzerTest(const uint8_t* data, size_t size)
    {
        auto cachedCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        MessageParcel request;
        MessageParcel reply;
        MessageOption option;
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        cachedCallbackHost->OnRemoteRequest(code, request, reply, option);
        cachedCallbackHost->IsRemoteDied();

        std::vector<std::shared_ptr<OHOS::Location::Location>> locationsForSend;
        cachedCallbackHost->Send(locationsForSend);
        std::vector<std::unique_ptr<OHOS::Location::Location>> locationsForReport;
        cachedCallbackHost->OnCacheLocationsReport(locationsForReport);
        cachedCallbackHost->DeleteHandler();
        return true;
    }

    bool CountryCodeCallbackHostFuzzerTest(const uint8_t* data, size_t size)
    {
        auto callbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        MessageParcel request;
        MessageParcel reply;
        MessageOption option;
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        callbackHost->OnRemoteRequest(code, request, reply, option);

        auto countryCodePtr = CountryCode::Unmarshalling(request);
        callbackHost->Send(countryCodePtr);
        callbackHost->SetEnv(nullptr);
        callbackHost->SetCallback(nullptr);
        callbackHost->OnCountryCodeChange(countryCodePtr);
        callbackHost->DeleteHandler();
        return true;
    }

    bool GnssStatusCallbackHostFuzzerTest(const uint8_t* data, size_t size)
    {
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        MessageParcel request;
        MessageParcel reply;
        MessageOption option;
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        gnssCallbackHost->OnRemoteRequest(code, request, reply, option);
        gnssCallbackHost->IsRemoteDied();
        std::unique_ptr<SatelliteStatus> statusInfo = nullptr;
        gnssCallbackHost->Send(statusInfo);

        gnssCallbackHost->OnStatusChange(statusInfo);
        gnssCallbackHost->DeleteHandler();
        return true;
    }

    bool LocationSwitchCallbackHostFuzzerTest(const uint8_t* data, size_t size)
    {
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        MessageParcel request;
        MessageParcel reply;
        MessageOption option;
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        switchCallbackHost->OnRemoteRequest(code, request, reply, option);
        switchCallbackHost->IsRemoteDied();
        switchCallbackHost->PackResult(true);
        switchCallbackHost->Send(SWITCH_STATE_ON);

        switchCallbackHost->OnSwitchChange(SWITCH_STATE_ON);
        switchCallbackHost->DeleteHandler();
        return true;
    }

    bool LocationCallbackHostFuzzerTest(const uint8_t* data, size_t size)
    {
        auto callbackHost =
            sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        MessageParcel request;
        MessageParcel reply;
        MessageOption option;
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        callbackHost->OnRemoteRequest(code, request, reply, option);
        callbackHost->OnErrorReport(SUCCESS);
        std::unique_ptr<OHOS::Location::Location> location =
            std::make_unique<OHOS::Location::Location>();
        callbackHost->OnLocationReport(location);
        callbackHost->OnLocatingStatusChange(SWITCH_STATE_ON);

        callbackHost->DeleteAllCallbacks();
        callbackHost->IsSystemGeoLocationApi();
        callbackHost->IsSingleLocationRequest();
        callbackHost->CountDown();
        int ms = *(reinterpret_cast<const int*>(data));
        callbackHost->Wait(ms % WAIT_TIME_SEC);
        int count = *(reinterpret_cast<const int*>(data));
        callbackHost->SetCount(count % COUNT);
        callbackHost->GetCount();
        return true;
    }

    bool NmeaMessageCallbackHostFuzzerTest(const uint8_t* data, size_t size)
    {
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
        MessageParcel request;
        MessageParcel reply;
        MessageOption option;
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        nmeaCallbackHost->OnRemoteRequest(code, request, reply, option);
        nmeaCallbackHost->IsRemoteDied();
        std::string msg((const char*) data, size);
        nmeaCallbackHost->PackResult(msg);
        nmeaCallbackHost->Send(msg);

        nmeaCallbackHost->OnMessageChange(msg);
        nmeaCallbackHost->DeleteHandler();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::AddPermission();
    OHOS::TestStartLocating(data, size);
    OHOS::LocatorProxySendRequestTest(data, size);
    OHOS::LocatorImplFuzzerTest(data, size);
    OHOS::TestMockFunc(data, size);
    OHOS::TestCallbackRegister(data, size);

    OHOS::CachedLocationsCallbackHostFuzzerTest(data, size);
    OHOS::CountryCodeCallbackHostFuzzerTest(data, size);
    OHOS::GnssStatusCallbackHostFuzzerTest(data, size);
    OHOS::LocationSwitchCallbackHostFuzzerTest(data, size);
    OHOS::LocationCallbackHostFuzzerTest(data, size);
    OHOS::NmeaMessageCallbackHostFuzzerTest(data, size);
    return 0;
}

