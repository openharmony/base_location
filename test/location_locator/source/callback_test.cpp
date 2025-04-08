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

#include "callback_test.h"

#include "message_parcel.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_napi.h"
#endif
#include "country_code.h"
#include "country_code_callback_napi.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_napi.h"
#endif
#include "location.h"
#include "location_switch_callback_napi.h"
#include "locator_callback_napi.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_napi.h"
#include "satellite_status.h"
#endif

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Location {
void CallbackTest::SetUp()
{
}

void CallbackTest::TearDown()
{
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(CallbackTest, OnCacheLocationsReport001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, OnCacheLocationsReport001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnCacheLocationsReport001 begin");
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackNapi>(new (std::nothrow) CachedLocationsCallbackNapi());
    EXPECT_NE(nullptr, cachedLocationsCallbackHost);
    auto cachedLocationsCallbackProxy =
        new (std::nothrow) CachedLocationsCallbackProxy(cachedLocationsCallbackHost);
    EXPECT_NE(nullptr, cachedLocationsCallbackProxy);
    std::vector<std::unique_ptr<Location>> locations;
    auto location =
        std::make_unique<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(1.0); // latitude
    parcel.WriteDouble(2.0); // longitude
    parcel.WriteDouble(3.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(6.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock
    EXPECT_NE(nullptr, location);
    location->ReadFromParcel(parcel);
    locations.emplace_back(std::move(location));
    cachedLocationsCallbackProxy->OnCacheLocationsReport(locations);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnCacheLocationsReport001 end");
}
#endif

HWTEST_F(CallbackTest, OnSwitchChange001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, OnSwitchChange001, TestSize.Level0";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnSwitchChange001 begin");
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackNapi>(new (std::nothrow) LocationSwitchCallbackNapi());
    EXPECT_NE(nullptr, switchCallbackHost);
    auto switchCallbackProxy =
        new (std::nothrow) SwitchCallbackProxy(switchCallbackHost);
    EXPECT_NE(nullptr, switchCallbackProxy);
    int state = 1;
    switchCallbackProxy->OnSwitchChange(state);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnSwitchChange001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(CallbackTest, OnMessageChange001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, OnMessageChange001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnMessageChange001 begin");
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackNapi>(new (std::nothrow) NmeaMessageCallbackNapi());
    EXPECT_NE(nullptr, nmeaCallbackHost);
    auto nmeaCallbackProxy =
        new (std::nothrow) NmeaMessageCallbackProxy(nmeaCallbackHost);
    EXPECT_NE(nullptr, nmeaCallbackProxy);
    std::string msg = "msg";
    int64_t timestamp = 0;
    nmeaCallbackProxy->OnMessageChange(timestamp, msg);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnMessageChange001 end");
}
#endif

HWTEST_F(CallbackTest, LocationCallbackProxy001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, LocationCallbackProxy001, TestSize.Level0";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy001 begin");
    auto locatorCallbackHost =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    EXPECT_NE(nullptr, locatorCallbackHost);
    auto locatorCallbackProxy =
            new (std::nothrow) LocatorCallbackProxy(locatorCallbackHost);
    EXPECT_NE(nullptr, locatorCallbackProxy);
    locatorCallbackProxy->OnLocationReport(nullptr); // nullptr error
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy001 end");
}

HWTEST_F(CallbackTest, LocationCallbackProxy002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, LocationCallbackProxy002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy002 begin");
    auto locatorCallbackHost =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    EXPECT_NE(nullptr, locatorCallbackHost);
    auto locatorCallbackProxy =
            new (std::nothrow) LocatorCallbackProxy(locatorCallbackHost);
    EXPECT_NE(nullptr, locatorCallbackProxy);
    int status = 1;
    locatorCallbackProxy->OnLocatingStatusChange(status);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy002 end");
}

HWTEST_F(CallbackTest, LocationCallbackProxy003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, LocationCallbackProxy003, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy003 begin");
    auto locatorCallbackHost =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    EXPECT_NE(nullptr, locatorCallbackHost);
    auto locatorCallbackProxy =
            new (std::nothrow) LocatorCallbackProxy(locatorCallbackHost);
    EXPECT_NE(nullptr, locatorCallbackProxy);
    int errorCode = 0;
    locatorCallbackProxy->OnErrorReport(errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy003 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(CallbackTest, GnssStatusCallbackProxy001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, GnssStatusCallbackProxy001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackProxy001 begin");
    auto gnssStatusCallbackHost =
        sptr<GnssStatusCallbackNapi>(new (std::nothrow) GnssStatusCallbackNapi());
    EXPECT_NE(nullptr, gnssStatusCallbackHost);
    auto gnssStatusCallbackProxy =
        new (std::nothrow) GnssStatusCallbackProxy(gnssStatusCallbackHost);
    EXPECT_NE(nullptr, gnssStatusCallbackProxy);
    auto statusInfo = std::make_unique<SatelliteStatus>();
    MessageParcel parcel;
    parcel.WriteInt64(2);
    for (int i = 0; i < 2; i++) {
        parcel.WriteInt64(i + 1);
        parcel.WriteDouble(i + 1.0);
        parcel.WriteDouble(i + 2.0);
        parcel.WriteDouble(i + 3.0);
        parcel.WriteDouble(i + 4.0);
        parcel.WriteInt64(i + 5.0);
    }
    statusInfo->ReadFromParcel(parcel);
    EXPECT_NE(nullptr, statusInfo);
    gnssStatusCallbackProxy->OnStatusChange(statusInfo);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackProxy001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(CallbackTest, GnssStatusCallbackProxy002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, GnssStatusCallbackProxy002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackProxy002 begin");
    auto gnssStatusCallbackHost =
        sptr<GnssStatusCallbackNapi>(new (std::nothrow) GnssStatusCallbackNapi());
    EXPECT_NE(nullptr, gnssStatusCallbackHost);
    auto gnssStatusCallbackProxy =
        new (std::nothrow) GnssStatusCallbackProxy(gnssStatusCallbackHost);
    EXPECT_NE(nullptr, gnssStatusCallbackProxy);
    gnssStatusCallbackProxy->OnStatusChange(nullptr);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackProxy002 end");
}
#endif

HWTEST_F(CallbackTest, CountryCodeCallbackProxy001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, CountryCodeCallbackProxy001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CountryCodeCallbackProxy001 begin");
    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    EXPECT_NE(nullptr, countryCodeCallbackHost);
    auto countryCodeCallbackProxy =
        new (std::nothrow) CountryCodeCallbackProxy(countryCodeCallbackHost);
    EXPECT_NE(nullptr, countryCodeCallbackProxy);
    auto country = std::make_shared<CountryCode>();
    MessageParcel parcel;
    parcel.WriteString("countryCodeStr");
    parcel.WriteInt64(1);
    country->ReadFromParcel(parcel);
    EXPECT_NE(nullptr, country);
    countryCodeCallbackProxy->OnCountryCodeChange(country);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CountryCodeCallbackProxy001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(CallbackTest, CachedLocationsCallbackHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, CachedLocationsCallbackHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CachedLocationsCallbackHost001 begin");
    auto cachedCallbackHost =
        sptr<CachedLocationsCallbackNapi>(new (std::nothrow) CachedLocationsCallbackNapi());
    EXPECT_NE(nullptr, cachedCallbackHost);
    EXPECT_NE(true, cachedCallbackHost->IsRemoteDied());

    std::vector<std::unique_ptr<Location>> locationsForReport;
    cachedCallbackHost->OnCacheLocationsReport(locationsForReport);
    EXPECT_EQ(0, locationsForReport.size());
    cachedCallbackHost->DeleteHandler();
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CachedLocationsCallbackHost001 end");
}
#endif

HWTEST_F(CallbackTest, CountryCodeCallbackHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, CountryCodeCallbackHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CountryCodeCallbackHost001 begin");
    auto callbackHost =
            sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    EXPECT_NE(nullptr, callbackHost);
    callbackHost->SetEnv(nullptr);
    callbackHost->SetCallback(nullptr);
    callbackHost->DeleteHandler();
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CountryCodeCallbackHost001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(CallbackTest, GnssStatusCallbackHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, GnssStatusCallbackHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackHost001 begin");
    auto gnssCallbackHost =
            sptr<GnssStatusCallbackNapi>(new (std::nothrow) GnssStatusCallbackNapi());
    EXPECT_NE(true, gnssCallbackHost->IsRemoteDied());
    gnssCallbackHost->OnStatusChange(nullptr);
    gnssCallbackHost->DeleteHandler();
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackHost001 end");
}
#endif

HWTEST_F(CallbackTest, LocationSwitchCallbackHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, LocationSwitchCallbackHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationSwitchCallbackHost001 begin");
    auto switchCallbackHost =
            sptr<LocationSwitchCallbackNapi>(new (std::nothrow) LocationSwitchCallbackNapi());
    EXPECT_NE(true, switchCallbackHost->IsRemoteDied());
    switchCallbackHost->PackResult(true);
    switchCallbackHost->DeleteHandler();
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationSwitchCallbackHost001 end");
}

HWTEST_F(CallbackTest, LocationCallbackHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, LocationCallbackHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackHost001 begin");
    auto callbackHost =
            sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());

    std::unique_ptr<Location> location = std::make_unique<Location>();
    EXPECT_NE(true, callbackHost->IsSystemGeoLocationApi());
    EXPECT_NE(true, callbackHost->IsSingleLocationRequest());
    callbackHost->CountDown();
    callbackHost->Wait(1);
    callbackHost->SetCount(1);
    EXPECT_EQ(0, callbackHost->GetCount());
    callbackHost->DeleteAllCallbacks();
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackHost001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(CallbackTest, NmeaMessageCallbackHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, NmeaMessageCallbackHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] NmeaMessageCallbackHost001 begin");
    auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackNapi>(new (std::nothrow) NmeaMessageCallbackNapi());
    EXPECT_NE(true, nmeaCallbackHost->IsRemoteDied());
    nmeaCallbackHost->PackResult("msg");
    nmeaCallbackHost->DeleteHandler();
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] NmeaMessageCallbackHost001 end");
}
#endif

HWTEST_F(CallbackTest, CountryCodeCallbackProxy002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, CountryCodeCallbackProxy002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CountryCodeCallbackProxy002 begin");
    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    EXPECT_NE(nullptr, countryCodeCallbackHost);
    auto countryCodeCallbackProxy =
        new (std::nothrow) CountryCodeCallbackProxy(countryCodeCallbackHost);
    EXPECT_NE(nullptr, countryCodeCallbackProxy);
    countryCodeCallbackProxy->OnCountryCodeChange(nullptr);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] CountryCodeCallbackProxy002 end");
}
}  // namespace Location
}  // namespace OHOS