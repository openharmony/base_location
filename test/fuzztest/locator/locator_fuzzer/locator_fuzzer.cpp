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
#include "locator.h"
#include "location_switch_callback_host.h"
#include "locator.h"
#include "locator_ability.h"
#include "locator_callback_host.h"
#include "location_log.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_host.h"
#endif
#include "request_config.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "satellite_status.h"
#endif

#include "network_ability.h"
#include "network_callback_host.h"
#include "passive_ability.h"
#include "geo_convert_service.h"
#include "gnss_ability.h"


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
    const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;

    bool TestStartLocating(const uint8_t* data, size_t size)
    {
        /* init locator and LocatorCallbackHost */
        auto locator = Locator::GetInstance();
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
        g_locatorImpl->IsLocationEnabled();
        g_locatorImpl->ShowNotification();
        g_locatorImpl->RequestPermission();
        g_locatorImpl->RequestEnableLocation();
        g_locatorImpl->EnableAbility(false);
        g_locatorImpl->EnableAbility(true);
        g_locatorImpl->GetCachedLocation();
#ifdef FEATURE_GEOCODE_SUPPORT
        g_locatorImpl->IsGeoServiceAvailable();
        MessageParcel parcel;
        std::list<std::shared_ptr<GeoAddress>> geoAddressList;
        g_locatorImpl->GetAddressByCoordinate(parcel, geoAddressList);
        g_locatorImpl->GetAddressByLocationName(parcel, geoAddressList);
#endif

        g_locatorImpl->IsLocationPrivacyConfirmed(data[index++]);
        g_locatorImpl->SetLocationPrivacyConfirmStatus(data[index++], true);
        g_locatorImpl->SetLocationPrivacyConfirmStatus(data[index++], false);
#ifdef FEATURE_GNSS_SUPPORT
        g_locatorImpl->GetCachedGnssLocationsSize();
        g_locatorImpl->FlushCachedGnssLocations();
        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        g_locatorImpl->SendCommand(command);
        std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
        g_locatorImpl->AddFence(fence);
        g_locatorImpl->RemoveFence(fence);
#endif
        g_locatorImpl->GetIsoCountryCode();
        g_locatorImpl->ProxyUidForFreeze(data[index++], false);
        return true;
    }

    bool LocatorImplV9FuzzerTest(const uint8_t* data, size_t size)
    {
        int index = 0;
        bool isEnabled = false;
        g_locatorImpl->IsLocationEnabledV9(isEnabled);
        g_locatorImpl->EnableAbilityV9(false);
        g_locatorImpl->EnableAbilityV9(true);
        std::unique_ptr<OHOS::Location::Location> loc =
            std::make_unique<OHOS::Location::Location>();
        g_locatorImpl->GetCachedLocationV9(loc);
#ifdef FEATURE_GEOCODE_SUPPORT
        bool isAvailable = false;
        g_locatorImpl->IsGeoServiceAvailableV9(isAvailable);
        MessageParcel parcel;
        std::list<std::shared_ptr<GeoAddress>> geoAddressList;
        g_locatorImpl->GetAddressByCoordinateV9(parcel, geoAddressList);
        g_locatorImpl->GetAddressByLocationNameV9(parcel, geoAddressList);
#endif
        bool isConfirmed = false;
        g_locatorImpl->IsLocationPrivacyConfirmedV9(data[index++], isConfirmed);
        g_locatorImpl->SetLocationPrivacyConfirmStatusV9(data[index++], true);
        g_locatorImpl->SetLocationPrivacyConfirmStatusV9(data[index++], false);
#ifdef FEATURE_GNSS_SUPPORT
        int locSize = -1;
        g_locatorImpl->GetCachedGnssLocationsSizeV9(locSize);
        g_locatorImpl->FlushCachedGnssLocationsV9();
        std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
        g_locatorImpl->SendCommandV9(command);
        std::unique_ptr<GeofenceRequest> fence = std::make_unique<GeofenceRequest>();
        g_locatorImpl->AddFenceV9(fence);
        g_locatorImpl->RemoveFenceV9(fence);
#endif
        std::shared_ptr<CountryCode> countryCode = std::make_shared<CountryCode>();
        g_locatorImpl->GetIsoCountryCodeV9(countryCode);
        g_locatorImpl->ProxyUidForFreezeV9(data[index++], false);
        return true;
    }

    bool TestCallbackRegister(const uint8_t* data, size_t size)
    {
        int index = 0;
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        g_locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), data[index++]);
        g_locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject());
#ifdef FEATURE_GNSS_SUPPORT
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        g_locatorImpl->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), data[index++]);
        g_locatorImpl->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject());
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        g_locatorImpl->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), data[index++]);
        g_locatorImpl->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject());
#endif
        auto countryCodeCallbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
        g_locatorImpl->RegisterCountryCodeCallback(countryCodeCallbackHost->AsObject(),
            data[index++]);
        g_locatorImpl->UnregisterCountryCodeCallback(countryCodeCallbackHost->AsObject());
#ifdef FEATURE_GNSS_SUPPORT
        auto cachedLocationsCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
        auto request = std::make_unique<CachedGnssLocationsRequest>();
        g_locatorImpl->RegisterCachedLocationCallback(request, cachedCallback);
        g_locatorImpl->UnregisterCachedLocationCallback(cachedCallback);
#endif
        return true;
    }

    bool TestCallbackRegisterV9(const uint8_t* data, size_t size)
    {
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        g_locatorImpl->RegisterSwitchCallbackV9(switchCallbackHost->AsObject());
        g_locatorImpl->UnregisterSwitchCallbackV9(switchCallbackHost->AsObject());
#ifdef FEATURE_GNSS_SUPPORT
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        g_locatorImpl->RegisterGnssStatusCallbackV9(gnssCallbackHost->AsObject());
        g_locatorImpl->UnregisterGnssStatusCallbackV9(gnssCallbackHost->AsObject());
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        g_locatorImpl->RegisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject());
        g_locatorImpl->UnregisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject());
#endif
        auto countryCodeCallbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
        g_locatorImpl->RegisterCountryCodeCallbackV9(countryCodeCallbackHost->AsObject());
        g_locatorImpl->UnregisterCountryCodeCallbackV9(countryCodeCallbackHost->AsObject());
#ifdef FEATURE_GNSS_SUPPORT
        auto cachedLocationsCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
        auto request = std::make_unique<CachedGnssLocationsRequest>();
        g_locatorImpl->RegisterCachedLocationCallbackV9(request, cachedCallback);
        g_locatorImpl->UnregisterCachedLocationCallbackV9(cachedCallback);
#endif
        return true;
    }

    bool TestMockFunc(const uint8_t* data, size_t size)
    {
        int index = 0;
        g_locatorImpl->EnableLocationMock();
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        g_locatorImpl->SetMockedLocations(data[index++], locations);
        g_locatorImpl->DisableLocationMock();
#ifdef FEATURE_GEOCODE_SUPPORT
        g_locatorImpl->EnableReverseGeocodingMock();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfo;
        g_locatorImpl->SetReverseGeocodingMockInfo(geoMockInfo);
        g_locatorImpl->DisableReverseGeocodingMock();
#endif
        return true;
    }

    bool TestMockFuncV9(const uint8_t* data, size_t size)
    {
        int index = 0;
        g_locatorImpl->EnableLocationMockV9();
        std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
        g_locatorImpl->SetMockedLocationsV9(data[index++], locations);
        g_locatorImpl->DisableLocationMockV9();
#ifdef FEATURE_GEOCODE_SUPPORT
        g_locatorImpl->EnableReverseGeocodingMockV9();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfo;
        g_locatorImpl->SetReverseGeocodingMockInfoV9(geoMockInfo);
        g_locatorImpl->DisableReverseGeocodingMockV9();
#endif
        return true;
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

        std::vector<std::shared_ptr<OHOS::Location::Location>> locationsForSend;
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
        std::string msg((const char*) data, size);
        nmeaCallbackHost->PackResult(msg);
        nmeaCallbackHost->Send(msg);
        int64_t timestamp = 0;
        nmeaCallbackHost->OnMessageChange(timestamp, msg);
        nmeaCallbackHost->DeleteHandler();
        return true;
    }
#endif

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
#ifdef FEATURE_GNSS_SUPPORT
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        locatorAbility->RegisterGnssStatusCallback(gnssCallbackHost, data[index++]);
        locatorAbility->UnregisterGnssStatusCallback(gnssCallbackHost);
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        locatorAbility->RegisterNmeaMessageCallback(nmeaCallbackHost, data[index++]);
        locatorAbility->UnregisterNmeaMessageCallback(nmeaCallbackHost);
#endif
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
#ifdef FEATURE_GEOCODE_SUPPORT
        bool isAvailable = false;
        locatorAbility->IsGeoConvertAvailable(isAvailable);
        MessageParcel request;
        MessageParcel reply;
        locatorAbility->GetAddressByCoordinate(request, reply);
        locatorAbility->GetAddressByLocationName(request, reply);
#endif
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
#ifdef FEATURE_GNSS_SUPPORT
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
#endif
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
#ifdef FEATURE_GEOCODE_SUPPORT
        locatorAbility->EnableReverseGeocodingMock();
        locatorAbility->DisableReverseGeocodingMock();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfo;
        locatorAbility->SetReverseGeocodingMockInfo(geoMockInfo);
#endif
        auto location = std::make_unique<OHOS::Location::Location>();
        std::string abilityName((const char*) data, size);
        int timeInterval = 2;
        locatorAbility->ReportLocation(location, abilityName);
        locatorAbility->ProcessLocationMockMsg(timeInterval, locations, data[index++]);
#ifdef FEATURE_GNSS_SUPPORT
        locatorAbility->SendLocationMockMsgToGnssSa(nullptr, timeInterval, locations, data[index++]);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
        locatorAbility->SendLocationMockMsgToNetworkSa(nullptr, timeInterval, locations, data[index++]);
#endif
        locatorAbility->GetRequests();
        locatorAbility->GetReceivers();
        locatorAbility->GetProxyMap();
        locatorAbility->UpdateSaAbilityHandler();
        locatorAbility->RegisterAction();
        locatorAbility->ProxyUidForFreeze(data[index++], false);
        locatorAbility->IsProxyUid(data[index++]);
        locatorAbility->GetActiveRequestNum();
        std::vector<std::string> permissionNameList;
        locatorAbility->RegisterPermissionCallback(data[index++], permissionNameList);
        locatorAbility->UnregisterPermissionCallback(data[index++]);
        return true;
    }
    uint32_t GetU32Data(const char* ptr)
    {
        // 随机数字生成
        return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    }

    char* ParseData(const uint8_t* data, size_t size)
    {
        if (data == nullptr) {
            return nullptr;
        }

        if (size > MAX_MEM_SIZE) {
            return nullptr;
        }

        char* ch = (char *)malloc(size + 1);
        if (ch == nullptr) {
            return nullptr;
        }

        (void)memset_s(ch, size + 1, 0x00, size + 1);
        if (memcpy_s(ch, size, data, size) != EOK) {
            free(ch);
            ch = nullptr;
            return nullptr;
        }
        return ch;
    }

    // 函数名称需要改
    bool LocatorSkeletonFuzzTest(const char* data, size_t size)
    {
        // 1、生成随机的sa code
        uint32_t code = GetU32Data(data);
        // 2、构造request parcel
        MessageParcel requestParcel;
        // stub端需要对parcel做token校验（u"location.ILocator"需要改的）
        requestParcel.WriteInterfaceToken(u"location.ILocator");
        // 根据传入的data生成随机的数据填入request parcel中
        requestParcel.WriteBuffer(data, size);
        // 指定读指针回归初始状态
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        // LocatorAbilityStub需要改的
        DelayedSingleton<LocatorAbilityStub>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool CountryCodeCodeCallbackHostFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.ICountryCodeCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<CountryCodeCallbackHost>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }
	
	bool NetworkAbilitySkeletonFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.INetworkAbility");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<NetworkAbility>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }   
    
    bool LocatorCallbackProxyFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.ILocatorCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<LocatorCallbackStub>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool PassiveAbilitySkeletonFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.IPassiveAbility");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<PassiveAbility>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool GeoConvertServiceStubFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<GeoConvertService>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool GnssAbilitySkeletonFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<GnssAbility>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool NetworkCallbackHostFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.ILocatorCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<NetworkCallbackHost>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool CachedCallbackHostFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.ICachedLocationsCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<CachedLocationsCallbackHost>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool GnssStatusCallbackHostFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.IGnssStatusCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<GnssStatusCallbackHost>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool LocationSwitchCallbackHostFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.ISwitchCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<LocationSwitchCallbackHost>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool LocatorCallbackHostFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.ILocatorCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<LocatorCallbackHost>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
        return true;
    }

    bool NmeaMessageCallbackHostFuzzTest(const char* data, size_t size)
    {
        uint32_t code = GetU32Data(data);
        MessageParcel requestParcel;
        requestParcel.WriteInterfaceToken(u"location.INmeaMessageCallback");
        requestParcel.WriteBuffer(data, size);
        requestParcel.RewindRead(0);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<NmeaMessageCallbackHost>::GetInstance()->OnRemoteRequest(code, requestParcel, reply, option);
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

    OHOS::LocatorImplV9FuzzerTest(data, size);
    OHOS::TestMockFuncV9(data, size);
    OHOS::TestCallbackRegisterV9(data, size);

    OHOS::LocatorAbility001FuzzerTest(data, size);
    OHOS::LocatorAbility002FuzzerTest(data, size);
    OHOS::LocatorAbility003FuzzerTest(data, size);
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
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
        OHOS::LocatorSkeletonFuzzTest(ch, size);
        OHOS::CountryCodeCodeCallbackHostFuzzTest(ch, size);
        OHOS::LocatorCallbackProxyFuzzTest(ch, size);
        OHOS::NetworkAbilitySkeletonFuzzTest(ch, size);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        OHOS::PassiveAbilitySkeletonFuzzTest(ch, size);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        OHOS::GeoConvertServiceStubFuzzTest(ch, size);
        OHOS::GnssAbilitySkeletonFuzzTest(ch, size);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        OHOS::NetworkCallbackHostFuzzTest(ch, size);
        OHOS::CachedCallbackHostFuzzTest(ch, size);
        OHOS::GnssStatusCallbackHostFuzzTest(ch, size);
        OHOS::LocationSwitchCallbackHostFuzzTest(ch, size);
        OHOS::LocatorCallbackHostFuzzTest(ch, size);
        OHOS::NmeaMessageCallbackHostFuzzTest(ch, size);
        // 释放内存
        free(ch);
        ch = nullptr;
    }
    return 0;
}

