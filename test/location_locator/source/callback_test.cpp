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

#include "cached_locations_callback_host.h"
#include "country_code.h"
#include "country_code_callback_host.h"
#include "gnss_status_callback_host.h"
#include "location.h"
#include "location_switch_callback_host.h"
#include "locator_callback_host.h"
#include "mock_locator_callback_host.h"
#include "nmea_message_callback_host.h"
#include "satellite_status.h"

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

HWTEST_F(CallbackTest, OnCacheLocationsReport001, TestSize.Level1)
{
    auto cachedLocationsCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
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
    parcel.WriteFloat(4.0); // accuracy
    parcel.WriteFloat(5.0); // speed
    parcel.WriteDouble(6.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock
    EXPECT_NE(nullptr, location);
    location->ReadFromParcel(parcel);
    locations.emplace_back(std::move(location));
    cachedLocationsCallbackProxy->OnCacheLocationsReport(locations);
}

HWTEST_F(CallbackTest, OnSwitchChange001, TestSize.Level1)
{
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(nullptr, switchCallbackHost);
    auto switchCallbackProxy =
        new (std::nothrow) SwitchCallbackProxy(switchCallbackHost);
    EXPECT_NE(nullptr, switchCallbackProxy);
    int state = 1;
    switchCallbackProxy->OnSwitchChange(state);
}

HWTEST_F(CallbackTest, OnMessageChange001, TestSize.Level1)
{
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_NE(nullptr, nmeaCallbackHost);
    auto nmeaCallbackProxy =
        new (std::nothrow) NmeaMessageCallbackProxy(nmeaCallbackHost);
    EXPECT_NE(nullptr, nmeaCallbackProxy);
    std::string msg = "msg";
    nmeaCallbackProxy->OnMessageChange(msg);
}

HWTEST_F(CallbackTest, LocationCallbackProxy001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, LocationCallbackProxy001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy001 begin");
    auto locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
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
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
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
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    EXPECT_NE(nullptr, locatorCallbackHost);
    auto locatorCallbackProxy =
            new (std::nothrow) LocatorCallbackProxy(locatorCallbackHost);
    EXPECT_NE(nullptr, locatorCallbackProxy);
    int errorCode = 0;
    locatorCallbackProxy->OnErrorReport(errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy003 end");
}

HWTEST_F(CallbackTest, LocationCallbackProxy004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, LocationCallbackProxy004, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy004 begin");
    auto mLocatorCallbackHost =
        sptr<MockLocatorCallbackHost>(new (std::nothrow) MockLocatorCallbackHost());
    EXPECT_CALL(*mLocatorCallbackHost, SendRequest(_, _, _, _)).WillOnce(DoAll(Return(NO_ERROR)));
    auto locatorCallbackProxy =
        new (std::nothrow) LocatorCallbackProxy(mLocatorCallbackHost);
    EXPECT_NE(nullptr, locatorCallbackProxy);
    auto location = std::make_unique<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(1.0); // latitude
    parcel.WriteDouble(2.0); // longitude
    parcel.WriteDouble(3.0); // altitude
    parcel.WriteFloat(4.0); // accuracy
    parcel.WriteFloat(5.0); // speed
    parcel.WriteDouble(6.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock
    EXPECT_NE(nullptr, location);
    location->ReadFromParcel(parcel);
    locatorCallbackProxy->OnLocationReport(location);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] LocationCallbackProxy004 end");
}

HWTEST_F(CallbackTest, GnssStatusCallbackProxy001, TestSize.Level1)
{
    auto gnssStatusCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
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
    }
    statusInfo->ReadFromParcel(parcel);
    EXPECT_NE(nullptr, statusInfo);
    gnssStatusCallbackProxy->OnStatusChange(statusInfo);
}

HWTEST_F(CallbackTest, GnssStatusCallbackProxy002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, GnssStatusCallbackProxy002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackProxy002 begin");
    auto gnssStatusCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_NE(nullptr, gnssStatusCallbackHost);
    auto gnssStatusCallbackProxy =
        new (std::nothrow) GnssStatusCallbackProxy(gnssStatusCallbackHost);
    EXPECT_NE(nullptr, gnssStatusCallbackProxy);
    gnssStatusCallbackProxy->OnStatusChange(nullptr);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] GnssStatusCallbackProxy002 end");
}

HWTEST_F(CallbackTest, CountryCodeCallbackProxy001, TestSize.Level1)
{
    auto countryCodeCallbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
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
}

HWTEST_F(CallbackTest, CachedLocationsCallbackHost001, TestSize.Level1)
{
    auto cachedCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    EXPECT_NE(nullptr, cachedCallbackHost);
    EXPECT_NE(true, cachedCallbackHost->IsRemoteDied());

    std::vector<std::unique_ptr<Location>> locationsForReport;
    cachedCallbackHost->OnCacheLocationsReport(locationsForReport);
    EXPECT_EQ(0, locationsForReport.size());
    cachedCallbackHost->DeleteHandler();
}

HWTEST_F(CallbackTest, CountryCodeCallbackHost001, TestSize.Level1)
{
    auto callbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    EXPECT_NE(nullptr, callbackHost);
    callbackHost->SetEnv(nullptr);
    callbackHost->SetCallback(nullptr);
    callbackHost->DeleteHandler();
}

HWTEST_F(CallbackTest, GnssStatusCallbackHost001, TestSize.Level1)
{
    auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    EXPECT_NE(true, gnssCallbackHost->IsRemoteDied());
    gnssCallbackHost->OnStatusChange(nullptr);
    gnssCallbackHost->DeleteHandler();
}

HWTEST_F(CallbackTest, LocationSwitchCallbackHost001, TestSize.Level1)
{
    auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    EXPECT_NE(true, switchCallbackHost->IsRemoteDied());
    switchCallbackHost->PackResult(true);
    switchCallbackHost->DeleteHandler();
}

HWTEST_F(CallbackTest, LocationCallbackHost001, TestSize.Level1)
{
    auto callbackHost =
            sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    
    std::unique_ptr<Location> location =
        std::make_unique<Location>();
    EXPECT_NE(true, callbackHost->IsSystemGeoLocationApi());
    EXPECT_NE(true, callbackHost->IsSingleLocationRequest());
    callbackHost->CountDown();
    callbackHost->Wait(1);
    callbackHost->SetCount(1);
    EXPECT_EQ(0, callbackHost->GetCount());
    callbackHost->DeleteAllCallbacks();
}

HWTEST_F(CallbackTest, NmeaMessageCallbackHost001, TestSize.Level1)
{
    auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    EXPECT_NE(true, nmeaCallbackHost->IsRemoteDied());
    nmeaCallbackHost->PackResult("msg");
    nmeaCallbackHost->DeleteHandler();
}
}  // namespace Location
}  // namespace OHOS