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
#include "locator.h"
#include "locator_ability.h"
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
    const int32_t MAX_CODE_LEN  = 512;
    const int32_t MAX_CODE_NUM = 40;
    const int32_t MIN_SIZE_NUM = 10;
    const int32_t SWITCH_STATE_ON = 1;
    const int32_t WAIT_TIME_SEC = 1000;
    const int32_t COUNT = 10;

    bool TestStartLocating(const uint8_t* data, size_t size)
    {
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
        return true;
    }
    
    bool LocatorProxySendRequestTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size > MAX_CODE_LEN)) {
            LBSLOGE(LOCATOR, "param error");
            return false;
        }
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

    bool LocatorImplFuzzerTest(const uint8_t* data, size_t size)
    {
        int index = 0;
        int state = 1;
        g_locatorImpl->IsLocationEnabled(state);
        g_locatorImpl->ShowNotification();
        g_locatorImpl->RequestPermission();
        g_locatorImpl->RequestEnableLocation();

        g_locatorImpl->EnableAbility(false);
        g_locatorImpl->EnableAbility(true);
        std::unique_ptr<OHOS::Location::Location> loc =
            std::make_unique<OHOS::Location::Location>();
        g_locatorImpl->GetCachedLocation(loc);
        bool isAvailable = false;
        g_locatorImpl->IsGeoServiceAvailable(isAvailable);
        MessageParcel parcel;
        std::list<std::shared_ptr<GeoAddress>> geoAddressList;
        g_locatorImpl->GetAddressByCoordinate(parcel, geoAddressList);
        g_locatorImpl->GetAddressByLocationName(parcel, geoAddressList);

        bool isConfirmed = false;
        g_locatorImpl->IsLocationPrivacyConfirmed(data[index++], isConfirmed);
        g_locatorImpl->SetLocationPrivacyConfirmStatus(data[index++], true);
        g_locatorImpl->SetLocationPrivacyConfirmStatus(data[index++], false);

        int locSize = -1;
        g_locatorImpl->GetCachedGnssLocationsSize(locSize);
        g_locatorImpl->FlushCachedGnssLocations();

        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        g_locatorImpl->SendCommand(command);

        std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
        g_locatorImpl->AddFence(fence);
        g_locatorImpl->RemoveFence(fence);
        std::shared_ptr<CountryCode> countryCode = std::make_shared<CountryCode>();
        g_locatorImpl->GetIsoCountryCode(countryCode);
        g_locatorImpl->ProxyUidForFreeze(data[index++], true);
        g_locatorImpl->ProxyUidForFreeze(data[index++], false);
        g_locatorImpl->ResetAllProxy();
        return true;
    }

    bool TestCallbackRegister(const uint8_t* data, size_t size)
    {
        int index = 0;
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        g_locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), data[index++]);
        g_locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject());

        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        g_locatorImpl->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), data[index++]);
        g_locatorImpl->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject());

        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        g_locatorImpl->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), data[index++]);
        g_locatorImpl->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject());

        auto countryCodeCallbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
        g_locatorImpl->RegisterCountryCodeCallback(countryCodeCallbackHost->AsObject(),
            data[index++]);
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
        int index = 0;
        g_locatorImpl->EnableLocationMock();
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        g_locatorImpl->SetMockedLocations(data[index++], locations);
        g_locatorImpl->DisableLocationMock();

        g_locatorImpl->EnableReverseGeocodingMock();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfo;
        g_locatorImpl->SetReverseGeocodingMockInfo(geoMockInfo);
        g_locatorImpl->DisableReverseGeocodingMock();
        return true;
    }

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

        std::vector<std::shared_ptr<OHOS::Location::Location>> locationsForSend;
        cachedCallbackHost->Send(locationsForSend);
        std::vector<std::unique_ptr<OHOS::Location::Location>> locationsForReport;
        cachedCallbackHost->OnCacheLocationsReport(locationsForReport);
        cachedCallbackHost->DeleteHandler();
        return true;
    }

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
        std::string msg((const char*) data, size);
        nmeaCallbackHost->PackResult(msg);
        nmeaCallbackHost->Send(msg);

        nmeaCallbackHost->OnMessageChange(msg);
        nmeaCallbackHost->DeleteHandler();
        return true;
    }

    bool LocatorAbility001FuzzerTest(const uint8_t* data, size_t size)
    {
        int index = 0;
        auto locatorAbility =
            sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
        std::string deviceId((const char*) data, size);
        locatorAbility->OnAddSystemAbility(data[index++], deviceId);
        locatorAbility->OnRemoveSystemAbility(data[index++], deviceId);
        locatorAbility->QueryServiceState();
        locatorAbility->InitSaAbility();
        locatorAbility->InitRequestManagerMap();
        locatorAbility->UpdateSaAbility();
        int state = 0;
        locatorAbility->GetSwitchState(state);
        locatorAbility->EnableAbility(true);
        locatorAbility->EnableAbility(false);
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        locatorAbility->RegisterSwitchCallback(switchCallbackHost, data[index++]);
        locatorAbility->UnregisterSwitchCallback(switchCallbackHost);
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        locatorAbility->RegisterGnssStatusCallback(gnssCallbackHost, data[index++]);
        locatorAbility->UnregisterGnssStatusCallback(gnssCallbackHost);

        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        locatorAbility->RegisterNmeaMessageCallback(nmeaCallbackHost, data[index++]);
        locatorAbility->UnregisterNmeaMessageCallback(nmeaCallbackHost);

        auto countryCodeCallbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
        locatorAbility->RegisterCountryCodeCallback(countryCodeCallbackHost, data[index++]);
        locatorAbility->UnregisterCountryCodeCallback(countryCodeCallbackHost);
        sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(locatorCallbackHostForTest_);
        AppIdentity identity;
        locatorAbility->StopLocating(locatorCallback);
        std::unique_ptr<OHOS::Location::Location> loc =
            std::make_unique<OHOS::Location::Location>();
        locatorAbility->GetCacheLocation(loc, identity);
        bool isAvailable = false;
        locatorAbility->IsGeoConvertAvailable(isAvailable);

        MessageParcel request;
        MessageParcel reply;
        locatorAbility->GetAddressByCoordinate(request, reply);
        locatorAbility->GetAddressByLocationName(request, reply);
        return true;
    }

    bool LocatorAbility002FuzzerTest(const uint8_t* data, size_t size)
    {
        auto locatorAbility =
            sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
        int index = 0;
        bool isConfirmed = false;
        locatorAbility->IsLocationPrivacyConfirmed(data[index++], isConfirmed);
        locatorAbility->SetLocationPrivacyConfirmStatus(data[index++], true);
        locatorAbility->SetLocationPrivacyConfirmStatus(data[index++], false);
        auto cachedLocationsCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
        auto cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
        locatorAbility->RegisterCachedLocationCallback(cachedRequest, cachedCallback, "fuzz.test");
        locatorAbility->UnregisterCachedLocationCallback(cachedCallback);
        int locSize;
        locatorAbility->GetCachedGnssLocationsSize(locSize);
        locatorAbility->FlushCachedGnssLocations();

        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        locatorAbility->SendCommand(command);

        std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
        locatorAbility->AddFence(fence);
        locatorAbility->RemoveFence(fence);
        std::shared_ptr<CountryCode> country = std::make_shared<CountryCode>();
        locatorAbility->GetIsoCountryCode(country);
        return true;
    }

    bool LocatorAbility003FuzzerTest(const uint8_t* data, size_t size)
    {
        auto locatorAbility =
            sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
        int index = 0;
        locatorAbility->EnableLocationMock();
        locatorAbility->DisableLocationMock();
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        locatorAbility->SetMockedLocations(data[index++], locations);
        locatorAbility->EnableReverseGeocodingMock();
        locatorAbility->DisableReverseGeocodingMock();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfo;
        locatorAbility->SetReverseGeocodingMockInfo(geoMockInfo);

        auto location = std::make_unique<OHOS::Location::Location>();
        std::string abilityName((const char*) data, size);
        int timeInterval = 2;
        locatorAbility->ReportLocation(location, abilityName);
        locatorAbility->ProcessLocationMockMsg(timeInterval, locations, data[index++]);
        locatorAbility->SendLocationMockMsgToGnssSa(nullptr, timeInterval, locations, data[index++]);
        locatorAbility->SendLocationMockMsgToNetworkSa(nullptr, timeInterval, locations, data[index++]);
        locatorAbility->GetRequests();
        locatorAbility->GetReceivers();
        locatorAbility->GetProxyMap();
        locatorAbility->UpdateSaAbilityHandler();
        locatorAbility->RegisterAction();
        locatorAbility->ProxyUidForFreeze(data[index++], true);
        locatorAbility->ProxyUidForFreeze(data[index++], false);
        locatorAbility->ResetAllProxy();
        locatorAbility->IsProxyUid(data[index++]);
        locatorAbility->GetActiveRequestNum();
        std::vector<std::string> permissionNameList;
        locatorAbility->RegisterPermissionCallback(data[index++], permissionNameList);
        locatorAbility->UnregisterPermissionCallback(data[index++]);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_SIZE_NUM) {
        return 0;
    }
    /* Run your code on data */
    OHOS::AddPermission();
    OHOS::TestStartLocating(data, size);
    OHOS::LocatorProxySendRequestTest(data, size);
    OHOS::LocatorImplFuzzerTest(data, size);
    OHOS::TestMockFunc(data, size);
    OHOS::TestCallbackRegister(data, size);

    OHOS::LocatorAbility001FuzzerTest(data, size);
    OHOS::LocatorAbility002FuzzerTest(data, size);
    OHOS::LocatorAbility003FuzzerTest(data, size);

    OHOS::CachedLocationsCallbackHostFuzzerTest(data, size);
    OHOS::CountryCodeCallbackHostFuzzerTest(data, size);
    OHOS::GnssStatusCallbackHostFuzzerTest(data, size);
    OHOS::LocationSwitchCallbackHostFuzzerTest(data, size);
    OHOS::LocationCallbackHostFuzzerTest(data, size);
    OHOS::NmeaMessageCallbackHostFuzzerTest(data, size);
    return 0;
}

