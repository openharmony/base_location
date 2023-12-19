/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "locator_impl_test.h"

#include <singleton.h>
#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "iremote_object.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_host.h"
#endif
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "country_code_callback_host.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_host.h"
#include "i_cached_locations_callback.h"
#endif
#include "location.h"
#include "location_sa_load_manager.h"
#include "location_switch_callback_host.h"
#include "locator.h"
#include "locator_callback_proxy.h"
#include "locator_proxy.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_host.h"
#endif
#include "request_config.h"
#include "locating_required_data_callback_host.h"
#include "locator_agent.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const int INVALID_PRIVACY_TYPE = -1;
#ifdef FEATURE_GNSS_SUPPORT
const int INVALID_CACHED_SIZE = 0;
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
const double MOCK_LATITUDE = 99.0;
const double MOCK_LONGITUDE = 100.0;
#endif
void LocatorImplTest::SetUp()
{
    MockNativePermission();
    LoadSystemAbility();
    locatorImpl_ = Locator::GetInstance();
    ASSERT_TRUE(locatorImpl_ != nullptr);
    callbackStub_ = new (std::nothrow) LocatorCallbackStub();
    ASSERT_TRUE(callbackStub_ != nullptr);
}

void LocatorImplTest::TearDown()
{
}

void LocatorImplTest::LoadSystemAbility()
{
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_LOCATOR_SA_ID);
#ifdef FEATURE_GNSS_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_GNSS_SA_ID);
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
    DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_GEO_CONVERT_SA_ID);
#endif
}

void LocatorImplTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorImplTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

#ifdef FEATURE_GEOCODE_SUPPORT
std::vector<std::shared_ptr<GeocodingMockInfo>> LocatorImplTest::SetGeocodingMockInfo()
{
    std::vector<std::shared_ptr<GeocodingMockInfo>> geoMockInfos;
    std::shared_ptr<GeocodingMockInfo> geocodingMockInfo =
        std::make_shared<GeocodingMockInfo>();
    MessageParcel parcel;
    parcel.WriteString16(Str8ToStr16("locale"));
    parcel.WriteDouble(MOCK_LATITUDE); // latitude
    parcel.WriteDouble(MOCK_LONGITUDE); // longitude
    parcel.WriteInt32(1);
    parcel.WriteString("localeLanguage");
    parcel.WriteString("localeCountry");
    parcel.WriteInt32(1); // size
    parcel.WriteInt32(0); // line
    parcel.WriteString("line");
    parcel.WriteString("placeName");
    parcel.WriteString("administrativeArea");
    parcel.WriteString("subAdministrativeArea");
    parcel.WriteString("locality");
    parcel.WriteString("subLocality");
    parcel.WriteString("roadName");
    parcel.WriteString("subRoadName");
    parcel.WriteString("premises");
    parcel.WriteString("postalCode");
    parcel.WriteString("countryCode");
    parcel.WriteString("countryName");
    parcel.WriteInt32(1); // hasLatitude
    parcel.WriteDouble(MOCK_LATITUDE); // latitude
    parcel.WriteInt32(1); // hasLongitude
    parcel.WriteDouble(MOCK_LONGITUDE); // longitude
    parcel.WriteString("phoneNumber");
    parcel.WriteString("addressUrl");
    parcel.WriteBool(true);
    geocodingMockInfo->ReadFromParcel(parcel);
    geoMockInfos.emplace_back(std::move(geocodingMockInfo));
    return geoMockInfos;
}
#endif

HWTEST_F(LocatorImplTest, locatorImplEnableAbilityV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplEnableAbilityV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbilityV9001 begin");
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterSwitchCallbackV9(switchCallbackHost->AsObject()));
    sleep(1);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableAbilityV9(false));
    bool isEnabled = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationEnabledV9(isEnabled));
    EXPECT_EQ(false, isEnabled);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterSwitchCallbackV9(switchCallbackHost->AsObject()));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbilityV9001 end");
}

HWTEST_F(LocatorImplTest, locatorImplEnableAbilityV9002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplEnableAbilityV9002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbilityV9002 begin");
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterSwitchCallbackV9(switchCallbackHost->AsObject()));
    sleep(1);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableAbilityV9(true));
    bool isEnabled = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationEnabledV9(isEnabled));
    EXPECT_EQ(true, isEnabled);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterSwitchCallbackV9(switchCallbackHost->AsObject()));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbilityV9002 end");
}

HWTEST_F(LocatorImplTest, locatorImplGetCachedLocationV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetCachedLocationV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedLocationV9 begin");

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableLocationMockV9()); // mock switch on

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StartLocatingV9(requestConfig, callbackStub_)); // startLocating first
    sleep(1);

    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> locations;
    Parcel parcel;
    parcel.WriteDouble(10.6); // latitude
    parcel.WriteDouble(10.5); // longitude
    parcel.WriteDouble(10.4); // altitude
    parcel.WriteDouble(1.0); // accuracy
    parcel.WriteDouble(5.0); // speed
    parcel.WriteDouble(10); // direction
    parcel.WriteInt64(1611000000); // timestamp
    parcel.WriteInt64(1611000000); // time since boot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock is false
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SetMockedLocationsV9(timeInterval, locations)); // set fake locations
    sleep(1);

    std::unique_ptr<Location> loc = std::make_unique<Location>();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetCachedLocationV9(loc)); // get last location
    ASSERT_TRUE(loc != nullptr);
    EXPECT_EQ(10.6, loc->GetLatitude());
    EXPECT_EQ(10.5, loc->GetLongitude());

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocatingV9(callbackStub_));

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->DisableLocationMockV9());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedLocationV9 end");
}

HWTEST_F(LocatorImplTest, locatorImplDisableLocationMockV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplDisableLocationMockV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplDisableLocationMockV9 begin");
    EXPECT_EQ(ERRCODE_SUCCESS, Locator::GetInstance()->DisableLocationMockV9());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplDisableLocationMockV9 end");
}

HWTEST_F(LocatorImplTest, locatorImplPrivacyStateV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplPrivacyStateV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplPrivacyStateV9001 begin");
    bool isConfirmed = false;
    EXPECT_EQ(ERRCODE_INVALID_PARAM, locatorImpl_->SetLocationPrivacyConfirmStatusV9(INVALID_PRIVACY_TYPE, true));
    EXPECT_EQ(ERRCODE_INVALID_PARAM, locatorImpl_->IsLocationPrivacyConfirmedV9(INVALID_PRIVACY_TYPE, isConfirmed));
    EXPECT_EQ(false, isConfirmed);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplPrivacyStateV9001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGetCachedGnssLocationsSizeV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetCachedGnssLocationsSizeV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedGnssLocationsSizeV9 begin");
    int size = INVALID_CACHED_SIZE;
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl_->GetCachedGnssLocationsSizeV9(size));
    EXPECT_EQ(INVALID_CACHED_SIZE, size);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedGnssLocationsSizeV9 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplFlushCachedGnssLocationsV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplFlushCachedGnssLocationsV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplFlushCachedGnssLocationsV9 begin");
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl_->FlushCachedGnssLocationsV9());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplFlushCachedGnssLocationsV9 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplSendCommandV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplSendCommandV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSendCommandV9 begin");
    std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
    command->scenario = SCENE_NAVIGATION;
    command->command = "cmd";
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SendCommandV9(command));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSendCommandV9 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplRequestFenceV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRequestFenceV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestFenceV9 begin");
    std::unique_ptr<GeofenceRequest> fenceRequest = std::make_unique<GeofenceRequest>();
    fenceRequest->scenario = SCENE_NAVIGATION;
    GeoFence geofence;
    geofence.latitude = 1.0;
    geofence.longitude = 2.0;
    geofence.radius = 3.0;
    geofence.expiration = 4.0;
    fenceRequest->geofence = geofence;
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl_->AddFenceV9(fenceRequest));
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl_->RemoveFenceV9(fenceRequest));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestFenceV9 end");
}
#endif

HWTEST_F(LocatorImplTest, locatorImplGetIsoCountryCodeV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetIsoCountryCodeV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetIsoCountryCodeV9 begin");

    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    EXPECT_NE(nullptr, countryCodeCallbackHost);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterCountryCodeCallbackV9(countryCodeCallbackHost->AsObject()));
    sleep(1);

    std::shared_ptr<CountryCode> countryCode = std::make_shared<CountryCode>();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetIsoCountryCodeV9(countryCode));
    ASSERT_TRUE(countryCode != nullptr);
    LBSLOGI(LOCATOR, "countrycode : %{public}s", countryCode->ToString().c_str());
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterCountryCodeCallbackV9(countryCodeCallbackHost->AsObject()));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetIsoCountryCodeV9 end");
}

HWTEST_F(LocatorImplTest, locatorImplProxyUidForFreezeV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplProxyUidForFreezeV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplProxyUidForFreezeV9 begin");
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->ProxyUidForFreezeV9(SYSTEM_UID, false));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplProxyUidForFreezeV9 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplIsGeoServiceAvailableV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplIsGeoServiceAvailableV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplIsGeoServiceAvailableV9001 begin");
    bool isAvailable = true;
    locatorImpl_->DisableReverseGeocodingMockV9();
    locatorImpl_->IsGeoServiceAvailableV9(isAvailable);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplIsGeoServiceAvailableV9001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGetAddressByCoordinateV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByCoordinateV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinateV9001 begin");
    MessageParcel request001;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList001;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableReverseGeocodingMockV9());

    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SetReverseGeocodingMockInfoV9(mockInfos));
    request001.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request001.WriteDouble(MOCK_LATITUDE); // latitude
    request001.WriteDouble(MOCK_LONGITUDE); // longitude
    request001.WriteInt32(3); // maxItems
    request001.WriteInt32(1); // locale object size = 1
    request001.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request001.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request001.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request001.WriteString16(Str8ToStr16("")); // ""
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetAddressByCoordinateV9(request001, geoAddressList001));
    EXPECT_EQ(true, geoAddressList001.empty());

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->DisableReverseGeocodingMockV9());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinateV9001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGetAddressByCoordinateV9002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByCoordinateV9002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinateV9002 begin");
    MessageParcel request002;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList002;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->DisableReverseGeocodingMockV9());

    request002.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request002.WriteDouble(1.0); // latitude
    request002.WriteDouble(2.0); // longitude
    request002.WriteInt32(3); // maxItems
    request002.WriteInt32(1); // locale object size = 1
    request002.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request002.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request002.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request002.WriteString16(Str8ToStr16("")); // ""
    EXPECT_EQ(ERRCODE_REVERSE_GEOCODING_FAIL, locatorImpl_->GetAddressByCoordinateV9(request002, geoAddressList002));
    EXPECT_EQ(true, geoAddressList002.empty());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinateV9002 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGetAddressByLocationNameV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByLocationNameV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByLocationNameV9001 begin");
    MessageParcel request003;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList003;
    request003.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request003.WriteString16(Str8ToStr16("description")); // description
    request003.WriteDouble(1.0); // minLatitude
    request003.WriteDouble(2.0); // minLongitude
    request003.WriteDouble(3.0); // maxLatitude
    request003.WriteDouble(4.0); // maxLongitude
    request003.WriteInt32(3); // maxItems
    request003.WriteInt32(1); // locale object size = 1
    request003.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request003.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request003.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request003.WriteString16(Str8ToStr16("")); // ""
    EXPECT_EQ(ERRCODE_GEOCODING_FAIL, locatorImpl_->GetAddressByLocationNameV9(request003, geoAddressList003));
    EXPECT_EQ(true, geoAddressList003.empty());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByLocationNameV9001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplRegisterAndUnregisterCallbackV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRegisterAndUnregisterCallbackV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterAndUnregisterCallbackV9001 begin");
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    EXPECT_NE(nullptr, cachedLocationsCallbackHost);
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    EXPECT_NE(nullptr, cachedCallback);
    auto request = std::make_unique<CachedGnssLocationsRequest>();
    EXPECT_NE(nullptr, request);
    request->reportingPeriodSec = 10;
    request->wakeUpCacheQueueFull = true;
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl_->RegisterCachedLocationCallbackV9(request, cachedCallback));
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl_->UnregisterCachedLocationCallbackV9(cachedCallback));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterAndUnregisterCallbackV9001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGnssStatusCallbackV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGnssStatusCallbackV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGnssStatusCallbackV9 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StartLocatingV9(requestConfig, callbackStub_)); // startLocating first
    sleep(1);
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterGnssStatusCallbackV9(gnssCallbackHost->AsObject()));
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterGnssStatusCallbackV9(gnssCallbackHost->AsObject()));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocatingV9(callbackStub_)); // after reg, stop locating
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGnssStatusCallbackV9 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplNmeaMessageCallbackV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplNmeaMessageCallbackV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallbackV9001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StartLocatingV9(requestConfig, callbackStub_)); // startLocating first
    sleep(1);
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocatingV9(callbackStub_)); // after reg, stop locating
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallbackV9001 end");
}
#endif

HWTEST_F(LocatorImplTest, locatorImplResetLocatorProxy001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplResetLocatorProxy001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplResetLocatorProxy001 begin");
    ASSERT_TRUE(locatorImpl_ != nullptr);
    wptr<IRemoteObject> remote = nullptr;
    locatorImpl_->ResetLocatorProxy(remote); // remote is nullptr
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplResetLocatorProxy001 end");
}

HWTEST_F(LocatorImplTest, locatorImplSetResumer001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplSetResumer001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSetResumer001 begin");
    std::shared_ptr<MockCallbackResumeManager> callbackResumer = std::make_shared<MockCallbackResumeManager>();
    locatorImpl_->SetResumer(callbackResumer); //resumer isn't nullptr
    EXPECT_NE(nullptr, locatorImpl_->resumer_);

    callbackResumer = nullptr;
    locatorImpl_->SetResumer(callbackResumer); //resumer is nullptr
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSetResumer001 end");
}

HWTEST_F(LocatorImplTest, locatorImplOnRemoteDied001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplOnRemoteDied001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplOnRemoteDied001 begin");
    auto impl = Locator::GetInstance();
    ASSERT_TRUE(impl != nullptr);
    auto recipient =
        sptr<LocatorImpl::LocatorDeathRecipient>(new (std::nothrow) LocatorImpl::LocatorDeathRecipient(*impl));
    wptr<IRemoteObject> remote;
    recipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplOnRemoteDied001 end");
}

HWTEST_F(LocatorImplTest, locatorImplRegisterLocatingRequiredDataCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRegisterLocatingRequiredDataCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterLocatingRequiredDataCallback001 begin");
    auto singleCallbackHost =
        sptr<LocatingRequiredDataCallbackHost>(new (std::nothrow) LocatingRequiredDataCallbackHost());
    if (singleCallbackHost) {
        singleCallbackHost->SetFixNumber(1);
    }
    std::unique_ptr<LocatingRequiredDataConfig> requestConfig = std::make_unique<LocatingRequiredDataConfig>();
    auto callbackPtr = sptr<ILocatingRequiredDataCallback>(singleCallbackHost);
    locatorImpl_->RegisterLocatingRequiredDataCallback(requestConfig, callbackPtr);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterLocatingRequiredDataCallback001 end");
}

HWTEST_F(LocatorImplTest, locatorImplUnRegisterLocatingRequiredDataCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplUnRegisterLocatingRequiredDataCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplUnRegisterLocatingRequiredDataCallback001 begin");
    auto singleCallbackHost =
        sptr<LocatingRequiredDataCallbackHost>(new (std::nothrow) LocatingRequiredDataCallbackHost());
    if (singleCallbackHost) {
        singleCallbackHost->SetFixNumber(1);
    }
    auto callbackPtr = sptr<ILocatingRequiredDataCallback>(singleCallbackHost);
    locatorImpl_->UnRegisterLocatingRequiredDataCallback(callbackPtr);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplUnRegisterLocatingRequiredDataCallback001 end");
}

static void TestLocationUpdate(
    const std::unique_ptr<OHOS::Location::Location>& location)
{
    LBSLOGI(LOCATOR, "[LocatorImplTest] LocationUpdate enter");
    EXPECT_EQ(true, location != nullptr);
}

static void TestSvStatusCallback(
    const std::unique_ptr<OHOS::Location::SatelliteStatus>& statusInfo)
{
    LBSLOGI(LOCATOR, "[LocatorImplTest] SvStatusCallback enter");
    EXPECT_EQ(true, statusInfo != nullptr);
}

static void TestNmeaCallback(int64_t timestamp, const std::string msg)
{
    LBSLOGI(LOCATOR, "[LocatorImplTest] NmeaCallback enter");
    EXPECT_EQ(true, msg != "");
}

HWTEST_F(LocatorImplTest, locatorAgentTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorAgentTest1, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorAgentTest1 begin");
    auto locatorAgent =
        DelayedSingleton<LocatorAgentManager>::GetInstance();
    ASSERT_TRUE(locatorAgent != nullptr);
    static OHOS::Location::LocationCallbackIfaces locationCallback;
    locationCallback.locationUpdate = TestLocationUpdate;
    locatorAgent->StartGnssLocating(locationCallback);
    sleep(1);
    static OHOS::Location::SvStatusCallbackIfaces svCallback;
    svCallback.svStatusUpdate = TestSvStatusCallback;
    locatorAgent->RegisterGnssStatusCallback(svCallback);
    sleep(1);
    static OHOS::Location::GnssNmeaCallbackIfaces nmeaCallback;
    nmeaCallback.nmeaUpdate = TestNmeaCallback;
    locatorAgent->RegisterNmeaMessageCallback(nmeaCallback);
    sleep(1);

    locatorAgent->UnregisterNmeaMessageCallback();
    locatorAgent->UnregisterGnssStatusCallback();
    locatorAgent->StopGnssLocating();
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorAgentTest1 end");
}

HWTEST_F(LocatorImplTest, locatorImplCheckEdmPolicy001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplCheckEdmPolicy001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplCheckEdmPolicy001 begin");
    locatorImpl_->CheckEdmPolicy(true);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplCheckEdmPolicy001 end");
}

HWTEST_F(LocatorImplTest, locatorImplCheckEdmPolicy002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplCheckEdmPolicy002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplCheckEdmPolicy002 begin");
    locatorImpl_->CheckEdmPolicy(false);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplCheckEdmPolicy002 end");
}
}  // namespace Location
}  // namespace OHOS
