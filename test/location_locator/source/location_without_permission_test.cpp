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

#include "location_without_permission_test.h"

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "cached_locations_callback_host.h"
#include "constant_definition.h"
#include "geo_address.h"
#include "gnss_status_callback_host.h"
#include "i_cached_locations_callback.h"
#include "i_locator_callback.h"
#include "location_switch_callback_host.h"
#include "locator.h"
#include "location_log.h"
#include "locator_callback_proxy.h"
#include "locator_impl.h"
#include "nmea_message_callback_host.h"
#include "request_config.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_WITHOUT_PERM = 1;
void LocationWithoutPermissionTest::SetUp()
{
    MockNativePermission();
}

void LocationWithoutPermissionTest::TearDown()
{
}

void LocationWithoutPermissionTest::MockNativePermission()
{
    const char *perms[] = {ACCESS_BACKGROUND_LOCATION.c_str()};
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_WITHOUT_PERM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorPermissionTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutSettingsPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutSettingsPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutSettingsPermission001 begin");
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->RegisterSwitchCallback(switchCallbackHost->AsObject(), 1000));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->UnregisterSwitchCallback(switchCallbackHost->AsObject()));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->EnableAbility(true));
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutSettingsPermission001 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermission001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission001 begin");
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_ACCURACY);
    sptr<ILocatorCallback> callbackStub = new (std::nothrow) LocatorCallbackStub();
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->StartLocating(requestConfig, callbackStub));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->StopLocating(callbackStub));

    std::unique_ptr loc = std::make_unique<Location>();
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->GetCachedLocation(loc));
    EXPECT_EQ(nullptr, loc);

    bool isAvailable = false;
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->IsGeoServiceAvailable(isAvailable));
    EXPECT_EQ(false, isAvailable);

    MessageParcel request001;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList001;
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->GetAddressByCoordinate(request001, geoAddressList001));
    EXPECT_EQ(true, geoAddressList001.empty());

    MessageParcel request002;
    std::list<std::shared_ptr<GeoAddress>> geoAddressList002;
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->GetAddressByLocationName(request002, geoAddressList002));
    EXPECT_EQ(true, geoAddressList002.empty());
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission001 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermission002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermission002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission002 begin");
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_NE(nullptr, gnssCallbackHost);
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->RegisterGnssStatusCallback(gnssCallbackHost->AsObject(), 1000));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->UnregisterGnssStatusCallback(gnssCallbackHost->AsObject()));

    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_NE(nullptr, nmeaCallbackHost);
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->RegisterNmeaMessageCallback(nmeaCallbackHost->AsObject(), 1000));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->UnregisterNmeaMessageCallback(nmeaCallbackHost->AsObject()));

    bool isConfirmed = false;
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->IsLocationPrivacyConfirmed(1, isConfirmed));
    EXPECT_EQ(false, isConfirmed);

    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->SetLocationPrivacyConfirmStatus(1, true));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->FlushCachedGnssLocations());

    std::unique_ptr<LocationCommand> command = std::make_unique<LocationCommand>();
    command->scenario = SCENE_NAVIGATION;
    command->command = "cmd";
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->SendCommand(command));

    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->ProxyUidForFreeze(1000, false));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->ResetAllProxy());
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission002 end");
}

HWTEST_F(LocationWithoutPermissionTest, LocatorWithoutLocationPermission003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationWithoutPermissionTest, LocatorWithoutLocationPermission003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission003 begin");
    std::unique_ptr<Locator> locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    std::unique_ptr<GeofenceRequest> fenceRequest = std::make_unique<GeofenceRequest>();
    fenceRequest->scenario = SCENE_NAVIGATION;
    GeoFence geofence;
    geofence.latitude = 1.0;
    geofence.longitude = 2.0;
    geofence.radius = 3.0;
    geofence.expiration = 4.0;
    fenceRequest->geofence = geofence;
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->AddFence(fenceRequest));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED,  locatorImpl->RemoveFence(fenceRequest));

    locatorImpl->GetCachedGnssLocationsSize();

    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    EXPECT_NE(nullptr, cachedLocationsCallbackHost);
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedLocationsCallbackHost);
    EXPECT_NE(nullptr, cachedCallback);
    auto request = std::make_unique<CachedGnssLocationsRequest>();
    EXPECT_NE(nullptr, request);
    request->reportingPeriodSec = 10;
    request->wakeUpCacheQueueFull = true;
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->RegisterCachedLocationCallback(request, cachedCallback));
    EXPECT_EQ(ERRCODE_PERMISSION_DENIED, locatorImpl->UnregisterCachedLocationCallback(cachedCallback));
    LBSLOGI(LOCATOR, "[LocationWithoutPermissionTest] LocatorWithoutLocationPermission003 end");
}
}  // namespace Location
}  // namespace OHOS