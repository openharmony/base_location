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

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "iremote_object.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_napi.h"
#endif
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "country_code_callback_napi.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_napi.h"
#include "i_cached_locations_callback.h"
#endif
#include "location.h"
#include "location_sa_load_manager.h"
#include "location_switch_callback_napi.h"
#include "locator.h"
#include "locator_callback_proxy.h"
#include "locator_proxy.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_napi.h"
#endif
#include "request_config.h"
#include "locating_required_data_callback_napi.h"
#include "locator_agent.h"
#include "permission_manager.h"
#include "geofence_request.h"
#include "geofence_sdk.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 6;
const int INVALID_PRIVACY_TYPE = -1;
const int32_t DEFAULT_USER = 100;
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
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_LOCATOR_SA_ID);
#ifdef FEATURE_GNSS_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_GNSS_SA_ID);
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_GEO_CONVERT_SA_ID);
#endif
}

void LocatorImplTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        ACCESS_CONTROL_LOCATION_SWITCH.c_str(), ACCESS_LOCATION_SWITCH_IGNORED.c_str(),
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
        sptr<LocationSwitchCallbackNapi>(new (std::nothrow) LocationSwitchCallbackNapi());
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
        sptr<LocationSwitchCallbackNapi>(new (std::nothrow) LocationSwitchCallbackNapi());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterSwitchCallbackV9(switchCallbackHost->AsObject()));
    sleep(1);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableAbilityV9(true));
    bool isEnabled = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationEnabledV9(isEnabled));

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterSwitchCallbackV9(switchCallbackHost->AsObject()));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbilityV9002 end");
}

HWTEST_F(LocatorImplTest, locatorImplGetCachedLocationV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetCachedLocationV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedLocationV9 begin");

    locatorImpl_->EnableLocationMockV9(); // mock switch on

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
    parcel.WriteInt32(0); // isFromMock is false
    locations.push_back(Location::UnmarshallingShared(parcel));
    locatorImpl_->SetMockedLocationsV9(timeInterval, locations); // set fake locations
    sleep(1);

    std::unique_ptr<Location> loc = std::make_unique<Location>();
    locatorImpl_->GetCachedLocationV9(loc); // get last location
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocatingV9(callbackStub_));

    locatorImpl_->DisableLocationMockV9();
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedLocationV9 end");
}

HWTEST_F(LocatorImplTest, locatorImplDisableLocationMockV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplDisableLocationMockV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplDisableLocationMockV9 begin");
    Locator::GetInstance()->DisableLocationMockV9();
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplDisableLocationMockV9 end");
}

HWTEST_F(LocatorImplTest, locatorImplPrivacyStateV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplPrivacyStateV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplPrivacyStateV9001 begin");
    bool isConfirmed = false;
    locatorImpl_->SetLocationPrivacyConfirmStatusV9(INVALID_PRIVACY_TYPE, true);
    locatorImpl_->IsLocationPrivacyConfirmedV9(INVALID_PRIVACY_TYPE, isConfirmed);
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


HWTEST_F(LocatorImplTest, locatorImplGetIsoCountryCodeV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetIsoCountryCodeV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetIsoCountryCodeV9 begin");

    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
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

HWTEST_F(LocatorImplTest, locatorImplProxyForFreeze, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplProxyForFreeze, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplProxyForFreeze begin");
    std::set<int> pidList;
    pidList.insert(SYSTEM_UID);
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl_->ProxyForFreeze(pidList, false));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplProxyForFreeze end");
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
    locatorImpl_->EnableReverseGeocodingMockV9();

    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    locatorImpl_->SetReverseGeocodingMockInfoV9(mockInfos);
    request001.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request001.WriteDouble(MOCK_LATITUDE); // latitude
    request001.WriteDouble(MOCK_LONGITUDE); // longitude
    request001.WriteInt32(3); // maxItems
    request001.WriteInt32(1); // locale object size = 1
    request001.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request001.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request001.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request001.WriteString16(Str8ToStr16("")); // ""
    locatorImpl_->GetAddressByCoordinateV9(request001, geoAddressList001);
    EXPECT_EQ(true, geoAddressList001.empty());

    locatorImpl_->DisableReverseGeocodingMockV9();
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
    locatorImpl_->DisableReverseGeocodingMockV9();

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
        sptr<CachedLocationsCallbackNapi>(new (std::nothrow) CachedLocationsCallbackNapi());
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
        sptr<GnssStatusCallbackNapi>(new (std::nothrow) GnssStatusCallbackNapi());
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
        sptr<NmeaMessageCallbackNapi>(new (std::nothrow) NmeaMessageCallbackNapi());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocatingV9(callbackStub_)); // after reg, stop locating
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallbackV9001 end");
}
#endif

HWTEST_F(LocatorImplTest, locatorImplGetDistanceBetweenLocations001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetDistanceBetweenLocations001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetDistanceBetweenLocations001 begin");
    ASSERT_TRUE(locatorImpl_ != nullptr);
    Location location1;
    location1.SetLatitude(31.123);
    location1.SetLongitude(121.123);
    Location location2;
    location2.SetLatitude(31.456);
    location2.SetLongitude(121.456);
    double distance;
    locatorImpl_->GetDistanceBetweenLocations(location1, location2, distance);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetDistanceBetweenLocations001 end");
}

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
        sptr<LocatingRequiredDataCallbackNapi>(new (std::nothrow) LocatingRequiredDataCallbackNapi());
    if (singleCallbackHost) {
        singleCallbackHost->SetFixNumber(1);
    }
    std::unique_ptr<LocatingRequiredDataConfig> requestConfig = std::make_unique<LocatingRequiredDataConfig>();
    auto callbackPtr = sptr<ILocatingRequiredDataCallback>(singleCallbackHost);
    locatorImpl_->RegisterLocatingRequiredDataCallback(requestConfig, callbackPtr);
    locatorImpl_->UnRegisterLocatingRequiredDataCallback(callbackPtr);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterLocatingRequiredDataCallback001 end");
}

HWTEST_F(LocatorImplTest, locatorImplUnRegisterLocatingRequiredDataCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplUnRegisterLocatingRequiredDataCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplUnRegisterLocatingRequiredDataCallback001 begin");
    auto singleCallbackHost =
        sptr<LocatingRequiredDataCallbackNapi>(new (std::nothrow) LocatingRequiredDataCallbackNapi());
    if (singleCallbackHost) {
        singleCallbackHost->SetFixNumber(1);
    }
    auto callbackPtr = sptr<ILocatingRequiredDataCallback>(singleCallbackHost);
    locatorImpl_->UnRegisterLocatingRequiredDataCallback(callbackPtr);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplUnRegisterLocatingRequiredDataCallback001 end");
}

HWTEST_F(LocatorImplTest, locatorImplSubscribeLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplSubscribeLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSubscribeLocationError001 begin");
    auto locatorCallbackHostForTest =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());

    if (locatorCallbackHostForTest) {
        locatorCallbackHostForTest->SetFixNumber(1);
    }
    sptr<ILocatorCallback> callbackPtr = sptr<ILocatorCallback>(locatorCallbackHostForTest);
    locatorImpl_->SubscribeLocationError(callbackPtr);
    locatorImpl_->UnSubscribeLocationError(callbackPtr);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSubscribeLocationError001 end");
}

HWTEST_F(LocatorImplTest, locatorImplUnSubscribeLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplUnSubscribeLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplUnSubscribeLocationError001 begin");
    auto locatorCallbackHostForTest =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());

    if (locatorCallbackHostForTest) {
        locatorCallbackHostForTest->SetFixNumber(1);
    }
    sptr<ILocatorCallback> callbackPtr = sptr<ILocatorCallback>(locatorCallbackHostForTest);
    locatorImpl_->UnSubscribeLocationError(callbackPtr);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplUnSubscribeLocationError001 end");
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
    LBSLOGI(LOCATOR, "[LocatorImplTest] SvStatusCallback begin");
    EXPECT_EQ(true, statusInfo != nullptr);
    LBSLOGI(LOCATOR, "[LocatorImplTest] SvStatusCallback end");
}

static void TestNmeaCallback(int64_t timestamp, const std::string msg)
{
    LBSLOGI(LOCATOR, "[LocatorImplTest] NmeaCallback begin");
    EXPECT_EQ(true, msg != "");
    LBSLOGI(LOCATOR, "[LocatorImplTest] NmeaCallback end");
}

HWTEST_F(LocatorImplTest, locatorAgentTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorAgentTest1, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorAgentTest1 begin");
    auto locatorAgent =
        LocatorAgentManager::GetInstance();
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

HWTEST_F(LocatorImplTest, CallbackResumeManager001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, CallbackResumeManager001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] CallbackResumeManager001 begin");
    std::shared_ptr<CallbackResumeManager> callbackResumer = std::make_shared<CallbackResumeManager>();
    callbackResumer->ResumeCallback();
    LBSLOGI(LOCATOR, "[LocatorImplTest] CallbackResumeManager001 end");
}

HWTEST_F(LocatorImplTest, OnAddSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, OnAddSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] OnAddSystemAbility001 begin");
    auto saStatusListener = sptr<LocatorSystemAbilityListener>(new LocatorSystemAbilityListener());
    saStatusListener->OnAddSystemAbility(0, "deviceId");
    LBSLOGI(LOCATOR, "[LocatorImplTest] OnAddSystemAbility001 end");
}

HWTEST_F(LocatorImplTest, OnRemoveSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, OnRemoveSystemAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] OnRemoveSystemAbility001 begin");
    auto saStatusListener = sptr<LocatorSystemAbilityListener>(new LocatorSystemAbilityListener());
    saStatusListener->OnRemoveSystemAbility(0, "deviceId");
    LBSLOGI(LOCATOR, "[LocatorImplTest] OnRemoveSystemAbility001 end");
}

HWTEST_F(LocatorImplTest, HasGnssNetworkRequest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, HasGnssNetworkRequest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] HasGnssNetworkRequest001 begin");
    locatorImpl_->HasGnssNetworkRequest();
    LBSLOGI(LOCATOR, "[LocatorImplTest] HasGnssNetworkRequest001 end");
}

HWTEST_F(LocatorImplTest, locatorImplEnableAbilityForUser, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplEnableAbilityForUser, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbilityForUser begin");
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableAbilityForUser(false, DEFAULT_USER));
    bool isEnabled = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationEnabledForUser(isEnabled, DEFAULT_USER));
    EXPECT_EQ(false, isEnabled);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableAbilityForUser(true, DEFAULT_USER));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbilityForUser end");
}

HWTEST_F(LocatorImplTest, GetCurrentWifiBssidForLocating, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, GetCurrentWifiBssidForLocating, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] GetCurrentWifiBssidForLocating begin");
    std::string bssid;
    auto errCode = locatorImpl_->GetCurrentWifiBssidForLocating(bssid);
    if (errCode == ERRCODE_SUCCESS) {
        EXPECT_NE(0, bssid.size());
    } else {
        EXPECT_EQ(0, bssid.size());
    }
    LBSLOGI(LOCATOR, "[LocatorImplTest] GetCurrentWifiBssidForLocating end");
}

HWTEST_F(LocatorImplTest, IsPoiServiceSupport, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, IsPoiServiceSupport, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] IsPoiServiceSupport begin");
    bool poiServiceSupportState = false;
    poiServiceSupportState = locatorImpl_->IsPoiServiceSupport();
    LBSLOGI(LOCATOR, "[LocatorImplTest] IsPoiServiceSupport end");
}

HWTEST_F(LocatorImplTest, SetLocationSwitchIgnored001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, SetLocationSwitchIgnored001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] SetLocationSwitchIgnored001 begin");
    std::string bssid;
    auto errCode = locatorImpl_->SetLocationSwitchIgnored(true);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR, "[LocatorImplTest] SetLocationSwitchIgnored001 end");
}

HWTEST_F(LocatorImplTest, SetLocationSwitchIgnored002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, SetLocationSwitchIgnored002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] SetLocationSwitchIgnored002 begin");
    std::string bssid;
    auto errCode = locatorImpl_->SetLocationSwitchIgnored(false);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR, "[LocatorImplTest] SetLocationSwitchIgnored002 end");
}

/*
 * @tc.name: CheckLocatingForScenario001
 * @tc.desc: Check start locating based on scenario expect success
 * @tc.type: FUNC
 */
HWTEST_F(LocatorImplTest, CheckLocatingForScenario001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call system ability and start locating for SCENE_VEHICLE_NAVIGATION.
     * @tc.expected: step1. get reply state is successful.
     */
    GTEST_LOG_(INFO)
        << "LocatorImplTest, CheckLocatingForScenario001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] CheckLocatingForScenario001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetScenario(SCENE_NAVIGATION);
    auto locatorImpl = Locator::GetInstance();
    locatorImpl->StartLocating(requestConfig, callbackStub_);
    locatorImpl->StopLocating(callbackStub_);
    LBSLOGI(LOCATOR, "[LocatorImplTest] CheckLocatingForScenario001 end");
}

/*
 * @tc.name: CheckStopLocating001
 * @tc.desc: Check stop locating with illegal param and expect fail
 * @tc.type: FUNC
 */
HWTEST_F(LocatorImplTest, CheckStopLocating001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. Call system ability and stop locating whit illegal param.
     * @tc.expected: step1. get reply state is false.
     */
    GTEST_LOG_(INFO)
        << "LocatorImplTest, CheckStopLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] CheckStopLocating001 begin");
    auto locatorImpl = Locator::GetInstance();
    ASSERT_TRUE(locatorImpl != nullptr);
    locatorImpl->StopLocating(callbackStub_);
    LBSLOGI(LOCATOR, "[LocatorImplTest] CheckStopLocating001 end");
}

HWTEST_F(LocatorImplTest, locatorImpl001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImpl001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImpl001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    locatorImpl->ShowNotification();
    locatorImpl->RequestPermission();
    locatorImpl->RequestEnableLocation();

    locatorImpl->EnableAbility(false);
    EXPECT_EQ(false, locatorImpl->IsLocationEnabled());
    locatorImpl->EnableAbility(true);

    locatorImpl->GetCachedLocation();

    locatorImpl->SetLocationPrivacyConfirmStatus(1, true);
    EXPECT_EQ(true, locatorImpl->IsLocationPrivacyConfirmed(1));
    locatorImpl->SetLocationPrivacyConfirmStatus(-1, true);
    locatorImpl->IsLocationPrivacyConfirmed(-1);
#ifdef FEATURE_GNSS_SUPPORT
    EXPECT_EQ(0, locatorImpl->GetCachedGnssLocationsSize());
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl->FlushCachedGnssLocations());
    std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
    command->scenario = SCENE_NAVIGATION;
    command->command = "cmd";
    EXPECT_EQ(true, locatorImpl->SendCommand(command));
#endif
    EXPECT_NE(nullptr, locatorImpl->GetIsoCountryCode());
    int timeInterval = 2;
    locatorImpl->EnableLocationMock();
    std::vector<std::shared_ptr<Location>> locations;
    locatorImpl->SetMockedLocations(timeInterval, locations);
    locatorImpl->DisableLocationMock();
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImpl001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGeocodingMock001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    locatorImpl->EnableReverseGeocodingMock();
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    locatorImpl->SetReverseGeocodingMockInfo(mockInfos);
    locatorImpl->DisableReverseGeocodingMock();
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGeocodingMock001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGetAddressByCoordinate001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByCoordinate001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    MessageParcel request001;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList001;
    locatorImpl->EnableReverseGeocodingMock();

    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    locatorImpl->SetReverseGeocodingMockInfo(mockInfos);
    request001.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request001.WriteDouble(MOCK_LATITUDE); // latitude
    request001.WriteDouble(MOCK_LONGITUDE); // longitude
    request001.WriteInt32(3); // maxItems
    request001.WriteInt32(1); // locale object size = 1
    request001.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request001.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request001.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request001.WriteString16(Str8ToStr16("")); // ""
    locatorImpl->GetAddressByCoordinate(request001, geoAddressList001);
    EXPECT_EQ(true, geoAddressList001.empty());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGetAddressByCoordinate002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByCoordinate002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate002 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    MessageParcel request002;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList002;
    locatorImpl->DisableReverseGeocodingMock();
    request002.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request002.WriteDouble(1.0); // latitude
    request002.WriteDouble(2.0); // longitude
    request002.WriteInt32(3); // maxItems
    request002.WriteInt32(1); // locale object size = 1
    request002.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request002.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request002.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request002.WriteString16(Str8ToStr16("")); // ""
    locatorImpl->GetAddressByCoordinate(request002, geoAddressList002);
    EXPECT_EQ(true, geoAddressList002.empty());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate002 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorImplTest, locatorImplGetAddressByLocationName001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByLocationName001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByLocationName001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
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
    locatorImpl->GetAddressByLocationName(request003, geoAddressList003);
    EXPECT_EQ(true, geoAddressList003.empty());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByLocationName001 end");
}
#endif

HWTEST_F(LocatorImplTest, locatorImplRegisterAndUnregisterCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRegisterAndUnregisterCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterAndUnregisterCallback001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackNapi>(new (std::nothrow) LocationSwitchCallbackNapi());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject()));
#ifdef FEATURE_GNSS_SUPPORT
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackNapi>(new (std::nothrow) GnssStatusCallbackNapi());
    EXPECT_NE(nullptr, gnssCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject()));
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackNapi>(new (std::nothrow) NmeaMessageCallbackNapi());
    EXPECT_NE(nullptr, nmeaCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject()));
#endif
    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    EXPECT_NE(nullptr, countryCodeCallbackHost);
    EXPECT_EQ(true, locatorImpl->RegisterCountryCodeCallback(countryCodeCallbackHost->AsObject(), 1000));
    EXPECT_EQ(true, locatorImpl->UnregisterCountryCodeCallback(countryCodeCallbackHost->AsObject()));
#ifdef FEATURE_GNSS_SUPPORT
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackNapi>(new (std::nothrow) CachedLocationsCallbackNapi());
    EXPECT_NE(nullptr, cachedLocationsCallbackHost);
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    EXPECT_NE(nullptr, cachedCallback);
    auto request = std::make_unique<CachedGnssLocationsRequest>();
    EXPECT_NE(nullptr, request);
    request->reportingPeriodSec = 10;
    request->wakeUpCacheQueueFull = true;
    locatorImpl->RegisterCachedLocationCallback(request, cachedCallback);
    locatorImpl->UnregisterCachedLocationCallback(cachedCallback);
#endif
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterAndUnregisterCallback001 end");
}
}  // namespace Location
}  // namespace OHOS
