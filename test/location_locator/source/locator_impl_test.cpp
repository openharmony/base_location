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
#include "token_setproc.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "country_code_callback_host.h"
#include "geo_address.h"
#include "gnss_status_callback_host.h"
#include "i_cached_locations_callback.h"
#include "location.h"
#include "location_switch_callback_host.h"
#include "locator_callback_proxy.h"
#include "nmea_message_callback_host.h"
#include "request_config.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
const int INVALID_PRIVACY_TYPE = -1;
const int INVALID_CACHED_SIZE = -1;
void LocatorImplTest::SetUp()
{
    MockNativePermission();
    locatorImpl_ = Locator::GetInstance();
    ASSERT_TRUE(locatorImpl_ != nullptr);
    callbackStub_ = new (std::nothrow) LocatorCallbackStub();
    ASSERT_TRUE(callbackStub_ != nullptr);
}

void LocatorImplTest::TearDown()
{
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

HWTEST_F(LocatorImplTest, locatorImplShowNotification, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplShowNotification, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplShowNotification begin");
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->ShowNotification());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplShowNotification end");
}

HWTEST_F(LocatorImplTest, locatorImplRequestPermission, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRequestPermission, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestPermission begin");
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RequestPermission());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestPermission end");
}

HWTEST_F(LocatorImplTest, locatorImplRequestEnableLocation, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRequestEnableLocation, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestEnableLocation begin");
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RequestEnableLocation());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestEnableLocation end");
}

HWTEST_F(LocatorImplTest, locatorImplEnableAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplEnableAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbility001 begin");
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterSwitchCallback(switchCallbackHost->AsObject(), 1000));
    sleep(1);
    
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableAbility(false));
    int state = STATE_CLOSE;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationEnabled(state));
    EXPECT_EQ(STATE_CLOSE, state);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterSwitchCallback(switchCallbackHost->AsObject()));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbility001 end");
}

HWTEST_F(LocatorImplTest, locatorImplEnableAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplEnableAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbility002 begin");
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterSwitchCallback(switchCallbackHost->AsObject(), 1000));
    sleep(1);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableAbility(true));
    int state = STATE_CLOSE;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationEnabled(state));
    EXPECT_EQ(STATE_OPEN, state);

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterSwitchCallback(switchCallbackHost->AsObject()));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplEnableAbility002 end");
}

HWTEST_F(LocatorImplTest, locatorImplGetCachedLocation, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetCachedLocation, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedLocation begin");
    
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableLocationMock()); // mock switch on

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StartLocating(requestConfig, callbackStub_)); // startLocating first
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
    parcel.WriteBool(false); // isFromMock is false
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc
    locations.push_back(Location::UnmarshallingShared(parcel));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SetMockedLocations(timeInterval, locations)); // set fake locations
    sleep(1);

    std::unique_ptr<Location> loc = std::make_unique<Location>();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetCachedLocation(loc)); // get last location
    EXPECT_NE(nullptr, loc);
    EXPECT_EQ(10.6, loc->GetLatitude());
    EXPECT_EQ(10.5, loc->GetLongitude());

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocating(callbackStub_));

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->DisableLocationMock());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedLocation end");
}

HWTEST_F(LocatorImplTest, locatorImplPrivacyState001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplPrivacyState001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplPrivacyState001 begin");
    bool isConfirmed = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_STARTUP, false));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_STARTUP, isConfirmed));
    EXPECT_EQ(false, isConfirmed);
    
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SetLocationPrivacyConfirmStatus(PRIVACY_TYPE_STARTUP, true));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsLocationPrivacyConfirmed(PRIVACY_TYPE_STARTUP, isConfirmed));
    EXPECT_EQ(true, isConfirmed);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplPrivacyState001 end");
}

HWTEST_F(LocatorImplTest, locatorImplPrivacyState002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplPrivacyState002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplPrivacyState002 begin");
    bool isConfirmed = false;
    EXPECT_EQ(ERRCODE_INVALID_PARAM, locatorImpl_->SetLocationPrivacyConfirmStatus(INVALID_PRIVACY_TYPE, true));
    EXPECT_EQ(ERRCODE_INVALID_PARAM, locatorImpl_->IsLocationPrivacyConfirmed(INVALID_PRIVACY_TYPE));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplPrivacyState002 end");
}

HWTEST_F(LocatorImplTest, locatorImplGetCachedGnssLocationsSize, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetCachedGnssLocationsSize, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedGnssLocationsSize begin");
    int size = INVALID_CACHED_SIZE;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetCachedGnssLocationsSize(size));
    EXPECT_EQ(INVALID_CACHED_SIZE, size);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetCachedGnssLocationsSize end");
}

HWTEST_F(LocatorImplTest, locatorImplFlushCachedGnssLocations, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplFlushCachedGnssLocations, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplFlushCachedGnssLocations begin");
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, locatorImpl_->FlushCachedGnssLocations());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplFlushCachedGnssLocations end");
}

HWTEST_F(LocatorImplTest, locatorImplSendCommand, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplSendCommand, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSendCommand begin");
    std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
    command->scenario = SCENE_NAVIGATION;
    command->command = "cmd";
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SendCommand(command));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplSendCommand end");
}

HWTEST_F(LocatorImplTest, locatorImplRequestFence, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRequestFence, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestFence begin");
    std::unique_ptr<GeofenceRequest> fenceRequest = std::make_unique<GeofenceRequest>();
    fenceRequest->scenario = SCENE_NAVIGATION;
    GeoFence geofence;
    geofence.latitude = 1.0;
    geofence.longitude = 2.0;
    geofence.radius = 3.0;
    geofence.expiration = 4.0;
    fenceRequest->geofence = geofence;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->AddFence(fenceRequest));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RemoveFence(fenceRequest));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRequestFence end");
}

HWTEST_F(LocatorImplTest, locatorImplGetIsoCountryCode, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetIsoCountryCode, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetIsoCountryCode begin");

    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    EXPECT_NE(nullptr, countryCodeCallbackHost);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterCountryCodeCallback(countryCodeCallbackHost->AsObject(), 1000));
    sleep(1);

    std::shared_ptr<CountryCode> countryCode = std::make_shared<CountryCode>();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetIsoCountryCode(countryCode));
    EXPECT_NE(nullptr, countryCode);
    if (countryCode != nullptr) {
        LBSLOGI(LOCATOR, "countrycode : %{public}s", countryCode->ToString().c_str());
    } else {
        LBSLOGI(LOCATOR, "countrycode : NULL");
    }
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterCountryCodeCallback(countryCodeCallbackHost->AsObject()));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetIsoCountryCode end");
}

HWTEST_F(LocatorImplTest, locatorImplProxyUidForFreeze, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplProxyUidForFreeze, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplProxyUidForFreeze begin");
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->ProxyUidForFreeze(SYSTEM_UID, true));

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->ProxyUidForFreeze(SYSTEM_UID, false));

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->ResetAllProxy());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplProxyUidForFreeze end");
}

HWTEST_F(LocatorImplTest, locatorImplIsGeoServiceAvailable001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplIsGeoServiceAvailable001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplIsGeoServiceAvailable001 begin");
    bool isAvailable = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableReverseGeocodingMock());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsGeoServiceAvailable(isAvailable));
    EXPECT_EQ(true, isAvailable);

    isAvailable = false;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->DisableReverseGeocodingMock());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->IsGeoServiceAvailable(isAvailable));
    EXPECT_EQ(false, isAvailable);
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplIsGeoServiceAvailable001 end");
}

HWTEST_F(LocatorImplTest, locatorImplGetAddressByCoordinate001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByCoordinate001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate001 begin");
    MessageParcel request001;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList001;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->EnableReverseGeocodingMock());

    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfos = SetGeocodingMockInfo();
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->SetReverseGeocodingMockInfo(mockInfos));
    request001.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request001.WriteDouble(MOCK_LATITUDE); // latitude
    request001.WriteDouble(MOCK_LONGITUDE); // longitude
    request001.WriteInt32(3); // maxItems
    request001.WriteInt32(1); // locale object size = 1
    request001.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request001.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request001.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request001.WriteString16(Str8ToStr16("")); // ""
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetAddressByCoordinate(request001, geoAddressList001));
    EXPECT_EQ(false, geoAddressList001.empty());

    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->DisableReverseGeocodingMock());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate001 end");
}

HWTEST_F(LocatorImplTest, locatorImplGetAddressByCoordinate002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByCoordinate002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate002 begin");
    MessageParcel request002;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList002;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->DisableReverseGeocodingMock());

    request002.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    request002.WriteDouble(1.0); // latitude
    request002.WriteDouble(2.0); // longitude
    request002.WriteInt32(3); // maxItems
    request002.WriteInt32(1); // locale object size = 1
    request002.WriteString16(Str8ToStr16("Language")); // locale.getLanguage()
    request002.WriteString16(Str8ToStr16("Country")); // locale.getCountry()
    request002.WriteString16(Str8ToStr16("Variant")); // locale.getVariant()
    request002.WriteString16(Str8ToStr16("")); // ""
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetAddressByCoordinate(request002, geoAddressList002));
    EXPECT_EQ(true, geoAddressList002.empty());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByCoordinate002 end");
}

HWTEST_F(LocatorImplTest, locatorImplGetAddressByLocationName001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplGetAddressByLocationName001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByLocationName001 begin");
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
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->GetAddressByLocationName(request003, geoAddressList003));
    EXPECT_EQ(true, geoAddressList003.empty());
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplGetAddressByLocationName001 end");
}

HWTEST_F(LocatorImplTest, locatorImplRegisterAndUnregisterCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplRegisterAndUnregisterCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterAndUnregisterCallback001 begin");
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    EXPECT_NE(nullptr, cachedLocationsCallbackHost);
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    EXPECT_NE(nullptr, cachedCallback);
    auto request = std::make_unique<CachedGnssLocationsRequest>();
    EXPECT_NE(nullptr, request);
    request->reportingPeriodSec = 10;
    request->wakeUpCacheQueueFull = true;
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterCachedLocationCallback(request, cachedCallback));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterCachedLocationCallback(cachedCallback));
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplRegisterAndUnregisterCallback001 end");
}

HWTEST_F(LocatorImplTest, locatorImplNmeaMessageCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplNmeaMessageCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallback begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StartLocating(requestConfig, callbackStub_)); // startLocating first
    sleep(1);
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), SYSTEM_UID));
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject()));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocating(callbackStub_)); // after reg, stop locating
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallback end");
}

HWTEST_F(LocatorImplTest, locatorImplNmeaMessageCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplNmeaMessageCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallback begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StartLocating(requestConfig, callbackStub_)); // startLocating first
    sleep(1);
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), SYSTEM_UID));
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject()));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocating(callbackStub_)); // after reg, stop locating
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallback end");
}

HWTEST_F(LocatorImplTest, locatorImplNmeaMessageCallbackV9001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorImplTest, locatorImplNmeaMessageCallbackV9001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallbackV9001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StartLocating(requestConfig, callbackStub_)); // startLocating first
    sleep(1);
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->RegisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    sleep(1);
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->UnregisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject()));
    EXPECT_EQ(ERRCODE_SUCCESS, locatorImpl_->StopLocating(callbackStub_)); // after reg, stop locating
    LBSLOGI(LOCATOR, "[LocatorImplTest] locatorImplNmeaMessageCallbackV9001 end");
}
}  // namespace Location
}  // namespace OHOS