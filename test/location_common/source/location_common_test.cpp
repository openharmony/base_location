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

#include "location_common_test.h"

#include "string_ex.h"

#include "message_parcel.h"
#include "ipc_skeleton.h"
#include "common_event_subscriber.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "system_ability_definition.h"
#include "want.h"
#include "want_agent.h"

#include "app_identity.h"
#include "common_hisysevent.h"
#include "common_utils.h"
#include "country_code_manager.h"
#include "constant_definition.h"
#include "permission_status_change_cb.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#endif
#include "location.h"
#include "location_data_rdb_helper.h"
#include "location_data_rdb_manager.h"
#include "location_log.h"
#define private public
#include "location_data_rdb_observer.h"
#include "location_sa_load_manager.h"
#undef private
#include "locator_event_subscriber.h"
#include "request_config.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "satellite_status.h"
#endif
#include "hook_utils.h"
#include "hookmgr.h"
#include "work_record_statistic.h"
#include "permission_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
using Want = OHOS::AAFwk::Want;
#ifdef FEATURE_GEOCODE_SUPPORT
const double MOCK_LATITUDE = 99.0;
const double MOCK_LONGITUDE = 100.0;
#endif
const double VERIFY_LOCATION_LATITUDE = 12.0;
const double VERIFY_LOCATION_LONGITUDE = 13.0;
const double VERIFY_LOCATION_ALTITUDE = 14.0;
const double VERIFY_LOCATION_ACCURACY = 1000.0;
const double VERIFY_LOCATION_SPEED = 10.0;
const double VERIFY_LOCATION_DIRECTION = 90.0;
const double VERIFY_LOCATION_TIME = 1000000000;
const double VERIFY_LOCATION_TIMESINCEBOOT = 1000000000;
const int32_t UN_SAID = 999999;
const std::string UN_URI = "unknown_uri";
void LocationCommonTest::SetUp()
{
}

void LocationCommonTest::TearDown()
{
}

#ifdef FEATURE_GEOCODE_SUPPORT
void LocationCommonTest::SetGeoAddress(std::unique_ptr<GeoAddress>& geoAddress)
{
    MessageParcel parcel;
    parcel.WriteDouble(MOCK_LATITUDE); // latitude
    parcel.WriteDouble(MOCK_LONGITUDE); // longitude
    parcel.WriteString16(u"locale");
    parcel.WriteString16(u"placeName");
    parcel.WriteString16(u"countryCode");
    parcel.WriteString16(u"countryName");
    parcel.WriteString16(u"administrativeArea");
    parcel.WriteString16(u"subAdministrativeArea");
    parcel.WriteString16(u"locality");
    parcel.WriteString16(u"subLocality");
    parcel.WriteString16(u"roadName");
    parcel.WriteString16(u"subRoadName");
    parcel.WriteString16(u"premises");
    parcel.WriteString16(u"postalCode");
    parcel.WriteString16(u"phoneNumber");
    parcel.WriteString16(u"addressUrl");
    parcel.WriteInt32(1);
    parcel.WriteInt32(0);
    parcel.WriteString16(u"line");
    parcel.WriteBool(true);
    geoAddress->ReadFromParcel(parcel);
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
void LocationCommonTest::VerifyGeoAddressReadFromParcel(std::unique_ptr<GeoAddress>& geoAddress)
{
    EXPECT_EQ(MOCK_LATITUDE, geoAddress->latitude_);
    EXPECT_EQ(MOCK_LONGITUDE, geoAddress->longitude_);
    EXPECT_EQ("locale", geoAddress->locale_);
    EXPECT_EQ("placeName", geoAddress->placeName_);
    EXPECT_EQ("countryCode", geoAddress->countryCode_);
    EXPECT_EQ("countryName", geoAddress->countryName_);
    EXPECT_EQ("administrativeArea", geoAddress->administrativeArea_);
    EXPECT_EQ("subAdministrativeArea", geoAddress->subAdministrativeArea_);
    EXPECT_EQ("locality", geoAddress->locality_);
    EXPECT_EQ("subLocality", geoAddress->subLocality_);
    EXPECT_EQ("roadName", geoAddress->roadName_);
    EXPECT_EQ("subRoadName", geoAddress->subRoadName_);
    EXPECT_EQ("premises", geoAddress->premises_);
    EXPECT_EQ("postalCode", geoAddress->postalCode_);
    EXPECT_EQ("phoneNumber", geoAddress->phoneNumber_);
    EXPECT_EQ("addressUrl", geoAddress->addressUrl_);
    EXPECT_EQ(1, geoAddress->descriptions_.size());
    auto iter = geoAddress->descriptions_.find(0);
    EXPECT_EQ(true, iter != geoAddress->descriptions_.end());
    EXPECT_EQ(0, iter->first);
    EXPECT_EQ("line", iter->second);
    EXPECT_EQ(true, geoAddress->isFromMock_);
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
void LocationCommonTest::VerifyGeoAddressMarshalling(MessageParcel& newParcel)
{
    EXPECT_EQ(MOCK_LATITUDE, newParcel.ReadDouble());
    EXPECT_EQ(MOCK_LONGITUDE, newParcel.ReadDouble());
    EXPECT_EQ("locale", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("placeName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("countryCode", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("countryName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("administrativeArea", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("subAdministrativeArea", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("locality", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("subLocality", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("roadName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("subRoadName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("premises", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("postalCode", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("phoneNumber", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("addressUrl", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(0, newParcel.ReadInt32());
    EXPECT_EQ("line", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ(true, newParcel.ReadBool());
}
#endif

void LocationCommonTest::VerifyLocationMarshalling(MessageParcel& newParcel)
{
    EXPECT_EQ(VERIFY_LOCATION_LATITUDE, newParcel.ReadDouble()); // latitude
    EXPECT_EQ(VERIFY_LOCATION_LONGITUDE, newParcel.ReadDouble()); // longitude
    EXPECT_EQ(VERIFY_LOCATION_ALTITUDE, newParcel.ReadDouble()); // altitude
    EXPECT_EQ(VERIFY_LOCATION_ACCURACY, newParcel.ReadDouble()); // accuracy
    EXPECT_EQ(VERIFY_LOCATION_SPEED, newParcel.ReadDouble()); // speed
    EXPECT_EQ(VERIFY_LOCATION_DIRECTION, newParcel.ReadDouble()); // direction
    EXPECT_EQ(VERIFY_LOCATION_TIME, newParcel.ReadInt64()); // timeStamp
    EXPECT_EQ(VERIFY_LOCATION_TIMESINCEBOOT, newParcel.ReadInt64()); // timeSinceBoot
    EXPECT_EQ(1, newParcel.ReadInt64()); // additionSize
    std::vector<std::u16string> additions;
    newParcel.ReadString16Vector(&additions);
    EXPECT_EQ("additions", Str16ToStr8(additions[0])); // additions
    EXPECT_EQ(1, newParcel.ReadInt32()); // isFromMock
}

/*
 * @tc.name: GeoAddressTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationCommonTest, GeoAddressTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeoAddressTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressTest001 begin");
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    VerifyGeoAddressReadFromParcel(geoAddress);

    MessageParcel newParcel;
    geoAddress->Marshalling(newParcel);
    VerifyGeoAddressMarshalling(newParcel);
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressTest001 end");
}
#endif

/*
 * @tc.name: GeoAddressTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationCommonTest, GeoAddressTest002, TestSize.Level1)
{
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();

    geoAddress->latitude_ = 1.0;
    EXPECT_EQ(1.0, geoAddress->GetLatitude());

    geoAddress->longitude_ = 1.0;
    EXPECT_EQ(1.0, geoAddress->GetLongitude());
}
#endif

/*
 * @tc.name: LocationTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LocationCommonTest, LocationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocationTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationTest001 begin");
    auto location = std::make_shared<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(VERIFY_LOCATION_LATITUDE); // latitude
    parcel.WriteDouble(VERIFY_LOCATION_LONGITUDE); // longitude
    parcel.WriteDouble(VERIFY_LOCATION_ALTITUDE); // altitude
    parcel.WriteDouble(VERIFY_LOCATION_ACCURACY); // accuracy
    parcel.WriteDouble(VERIFY_LOCATION_SPEED); // speed
    parcel.WriteDouble(VERIFY_LOCATION_DIRECTION); // direction
    parcel.WriteInt64(VERIFY_LOCATION_TIME); // timeStamp
    parcel.WriteInt64(VERIFY_LOCATION_TIMESINCEBOOT); // timeSinceBoot
    parcel.WriteInt64(1); // additionSize
    std::vector<std::u16string> additions;
    additions.push_back(Str8ToStr16("additions"));
    parcel.WriteString16Vector(additions);
    parcel.WriteInt32(1); // isFromMock
    location->ReadFromParcel(parcel);
    EXPECT_EQ(VERIFY_LOCATION_LATITUDE, location->GetLatitude());
    EXPECT_EQ(VERIFY_LOCATION_LONGITUDE, location->GetLongitude());
    EXPECT_EQ(VERIFY_LOCATION_ALTITUDE, location->GetAltitude());
    EXPECT_EQ(VERIFY_LOCATION_ACCURACY, location->GetAccuracy());
    EXPECT_EQ(VERIFY_LOCATION_SPEED, location->GetSpeed());
    EXPECT_EQ(VERIFY_LOCATION_DIRECTION, location->GetDirection());
    EXPECT_EQ(VERIFY_LOCATION_TIME, location->GetTimeStamp());
    EXPECT_EQ(VERIFY_LOCATION_TIMESINCEBOOT, location->GetTimeSinceBoot());
    EXPECT_EQ(1, location->GetAdditionSize());
    if (location->GetAdditions().size() == 1) {
        EXPECT_EQ("additions", location->GetAdditions()[0]);
    }
    EXPECT_EQ(1, location->GetIsFromMock());

    MessageParcel newParcel;
    location->Marshalling(newParcel);
    VerifyLocationMarshalling(newParcel);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationTest001 end");
}

/*
 * @tc.name: SateLLiteStatusTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocationCommonTest, SateLLiteStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, SateLLiteStatusTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] SateLLiteStatusTest001 begin");
    SatelliteStatus oldStatus;
    std::unique_ptr<SatelliteStatus> status = std::make_unique<SatelliteStatus>(oldStatus);
    MessageParcel parcel;
    int sateNum = 2;
    parcel.WriteInt64(2); // satellitesNumber
    for (int i = 0; i < sateNum; i++) {
        parcel.WriteInt64(i); // satelliteId
        parcel.WriteDouble(i + 1.0); // carrierToNoiseDensity
        parcel.WriteDouble(i + 2.0); // altitude
        parcel.WriteDouble(i + 3.0); // azimuth
        parcel.WriteDouble(i + 4.0); // carrierFrequency
        parcel.WriteInt64(i + 5.0); // constellation type
        parcel.WriteInt64(i + 6.0); // additionalInfoList
    }
    status->ReadFromParcel(parcel);
    EXPECT_EQ(2, status->GetSatellitesNumber());
    for (int i = 0; i < sateNum; i++) {
        EXPECT_EQ(i, status->GetSatelliteIds()[i]);
        EXPECT_EQ(i + 1.0, status->GetCarrierToNoiseDensitys()[i]);
        EXPECT_EQ(i + 2.0, status->GetAltitudes()[i]);
        EXPECT_EQ(i + 3.0, status->GetAzimuths()[i]);
        EXPECT_EQ(i + 4.0, status->GetCarrierFrequencies()[i]);
        EXPECT_EQ(i + 5.0, status->GetConstellationTypes()[i]);
        EXPECT_EQ(i + 6.0, status->GetSatelliteAdditionalInfoList()[i]);
    }

    MessageParcel newParcel;
    status->Marshalling(newParcel);
    EXPECT_EQ(2, newParcel.ReadInt64());
    for (int i = 0; i < sateNum; i++) {
        EXPECT_EQ(i, newParcel.ReadInt64());
        EXPECT_EQ(i + 1.0, newParcel.ReadDouble());
        EXPECT_EQ(i + 2.0, newParcel.ReadDouble());
        EXPECT_EQ(i + 3.0, newParcel.ReadDouble());
        EXPECT_EQ(i + 4.0, newParcel.ReadDouble());
        EXPECT_EQ(i + 5.0, newParcel.ReadInt64());
        EXPECT_EQ(i + 6.0, newParcel.ReadInt64());
    }
    LBSLOGI(LOCATOR, "[LocationCommonTest] SateLLiteStatusTest001 end");
}
#endif

/*
 * @tc.name: RequestConfigTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LocationCommonTest, RequestConfigTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, RequestConfigTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] RequestConfigTest001 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    MessageParcel parcel;
    parcel.WriteInt32(1); // scenario
    parcel.WriteInt32(2); // priority
    parcel.WriteInt32(3); // timeInterval
    parcel.WriteDouble(10.0); // distanceInterval
    parcel.WriteFloat(1000.0); // maxAccuracy
    parcel.WriteInt32(1); // fixNumber
    requestConfig->ReadFromParcel(parcel);
    EXPECT_EQ(1, requestConfig->GetScenario());
    EXPECT_EQ(2, requestConfig->GetPriority());
    EXPECT_EQ(3, requestConfig->GetTimeInterval());
    EXPECT_EQ(10.0, requestConfig->GetDistanceInterval());
    EXPECT_EQ(1000.0, requestConfig->GetMaxAccuracy());
    EXPECT_EQ(1, requestConfig->GetFixNumber());

    MessageParcel newParcel;
    requestConfig->Marshalling(newParcel);
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(2, newParcel.ReadInt32());
    EXPECT_EQ(3, newParcel.ReadInt32());
    EXPECT_EQ(10.0, newParcel.ReadDouble()); // distanceInterval
    EXPECT_EQ(1000.0, newParcel.ReadFloat());
    EXPECT_EQ(1, newParcel.ReadInt32());
    LBSLOGI(LOCATOR, "[LocationCommonTest] RequestConfigTest001 end");
}

HWTEST_F(LocationCommonTest, RequestConfigTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, RequestConfigTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] RequestConfigTest002 begin");
    std::unique_ptr<RequestConfig> requestConfigForCompare =
        std::make_unique<RequestConfig>();

    RequestConfig requestConfigForSet1;
    requestConfigForSet1.SetScenario(1);
    requestConfigForSet1.SetPriority(2);
    requestConfigForSet1.SetTimeInterval(3);
    requestConfigForSet1.SetDistanceInterval(4.0);
    requestConfigForSet1.SetMaxAccuracy(1000.0); // accuracy
    requestConfigForSet1.SetFixNumber(1);
    requestConfigForCompare->Set(requestConfigForSet1);
    EXPECT_NE("", requestConfigForCompare->ToString());
    EXPECT_EQ(true, requestConfigForCompare->IsSame(requestConfigForSet1));

    RequestConfig requestConfigForSet2;
    requestConfigForSet2.SetScenario(2);
    EXPECT_NE("", requestConfigForCompare->ToString());
    EXPECT_EQ(false, requestConfigForCompare->IsSame(requestConfigForSet2));

    RequestConfig requestConfigForSet3;
    requestConfigForSet3.SetScenario(SCENE_UNSET);
    requestConfigForSet3.SetPriority(2);
    requestConfigForCompare->SetScenario(SCENE_UNSET);
    EXPECT_NE("", requestConfigForCompare->ToString());
    EXPECT_EQ(true, requestConfigForCompare->IsSame(requestConfigForSet3));

    RequestConfig requestConfigForSet4;
    requestConfigForSet4.SetScenario(SCENE_UNSET);
    requestConfigForSet4.SetPriority(1);
    requestConfigForCompare->SetScenario(SCENE_UNSET);
    EXPECT_NE("", requestConfigForCompare->ToString());
    EXPECT_EQ(false, requestConfigForCompare->IsSame(requestConfigForSet4));
    LBSLOGI(LOCATOR, "[LocationCommonTest] RequestConfigTest002 end");
}

/*
 * @tc.name: GeocodingMockInfoTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationCommonTest, GeocodingMockInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeocodingMockInfoTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeocodingMockInfoTest001 begin");
    MessageParcel parcel;
    parcel.WriteString16(Str8ToStr16("locale"));
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteInt32(1); // maxItems
    std::unique_ptr<GeocodingMockInfo> mockInfo = std::make_unique<GeocodingMockInfo>();
    mockInfo->ReadFromParcel(parcel);
    std::shared_ptr<ReverseGeocodeRequest> reverseGeocodeRequest = mockInfo->GetLocation();
    EXPECT_EQ(true, reverseGeocodeRequest != nullptr);
    EXPECT_EQ("locale", reverseGeocodeRequest->locale);
    EXPECT_EQ(12.0, reverseGeocodeRequest->latitude);
    EXPECT_EQ(13.0, reverseGeocodeRequest->longitude);
    EXPECT_EQ(1, reverseGeocodeRequest->maxItems);

    MessageParcel newParcel;
    mockInfo->Marshalling(newParcel);
    EXPECT_EQ("locale", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ(12.0, newParcel.ReadDouble());
    EXPECT_EQ(13.0, newParcel.ReadDouble());
    EXPECT_EQ(1, newParcel.ReadInt32());
    GeocodingMockInfo::Unmarshalling(newParcel);
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeocodingMockInfoTest001 end");
}
#endif

HWTEST_F(LocationCommonTest, AppIdentityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, AppIdentityTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] AppIdentityTest001 begin");
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    identity.SetTokenId(3);
    identity.SetFirstTokenId(4);
    identity.SetBundleName("bundleName");
    EXPECT_EQ(1, identity.GetPid());
    EXPECT_EQ(2, identity.GetUid());
    EXPECT_EQ(3, identity.GetTokenId());
    EXPECT_EQ(4, identity.GetFirstTokenId());
    EXPECT_EQ("bundleName", identity.GetBundleName());
    EXPECT_NE("", identity.ToString());
    LBSLOGI(LOCATOR, "[LocationCommonTest] AppIdentityTest001 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationCommonTest, GeocodingMockInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeocodingMockInfoTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeocodingMockInfoTest002 begin");
    std::unique_ptr<GeocodingMockInfo> mockInfo = std::make_unique<GeocodingMockInfo>();

    auto reverseGeocodeRequest = std::make_shared<ReverseGeocodeRequest>();
    mockInfo->SetLocation(reverseGeocodeRequest);
    EXPECT_NE(nullptr, mockInfo->GetLocation());

    std::shared_ptr<GeoAddress> geoAddress = std::make_shared<GeoAddress>();
    mockInfo->SetGeoAddressInfo(geoAddress);
    EXPECT_NE(nullptr, mockInfo->GetGeoAddressInfo());
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeocodingMockInfoTest002 end");
}
#endif

HWTEST_F(LocationCommonTest, PermStateChangeCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, PermStateChangeCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] PermStateChangeCallbackTest001 begin");
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    PermStateChangeScope scopeInfo;
    scopeInfo.permList = {"ohos.permission.LOCATION"};
    scopeInfo.tokenIDs = {callingTokenId};
    auto callbackPtr = std::make_shared<PermissionStatusChangeCb>(scopeInfo);
    struct PermStateChangeInfo result{0, callingTokenId, ACCESS_LOCATION};
    ASSERT_TRUE(callbackPtr != nullptr);
    callbackPtr->PermStateChangeCallback(result);
    LBSLOGI(LOCATOR, "[LocationCommonTest] PermStateChangeCallbackTest001 end");
}

HWTEST_F(LocationCommonTest, LocatorEventSubscriberTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocatorEventSubscriberTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocatorEventSubscriberTest001 begin");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto locatorEventSubscriber = std::make_shared<LocatorEventSubscriber>(subscriberInfo);
    Want want;
    want.SetAction("usual.event.location.MODE_STATE_CHANGED");
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    ASSERT_TRUE(locatorEventSubscriber != nullptr);
    locatorEventSubscriber->OnReceiveEvent(data);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocatorEventSubscriberTest001 end");
}

HWTEST_F(LocationCommonTest, LocatorEventSubscriberTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocatorEventSubscriberTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocatorEventSubscriberTest002 begin");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto locatorEventSubscriber = std::make_shared<LocatorEventSubscriber>(subscriberInfo);
    Want want;
    want.SetAction("Invalid action");
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    ASSERT_TRUE(locatorEventSubscriber != nullptr);
    locatorEventSubscriber->OnReceiveEvent(data);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocatorEventSubscriberTest002 end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationCommonTest, GeoAddressDescriptionsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeoAddressDescriptionsTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest001 begin");
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    EXPECT_EQ("line", geoAddress->GetDescriptions(0));
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest001 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationCommonTest, GeoAddressDescriptionsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeoAddressDescriptionsTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest002 begin");
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    EXPECT_EQ("", geoAddress->GetDescriptions(1));
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest002 end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocationCommonTest, GeoAddressDescriptionsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeoAddressDescriptionsTest003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest003 begin");
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    EXPECT_EQ("", geoAddress->GetDescriptions(-1));
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest003 end");
}
#endif

HWTEST_F(LocationCommonTest, LoadLocationSaTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LoadLocationSaTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LoadLocationSaTest001 begin");
    LocationErrCode err = LocationSaLoadManager::GetInstance()->LoadLocationSa(UN_SAID);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, err);

    err = LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_LOCATOR_SA_ID);
    EXPECT_EQ(ERRCODE_SUCCESS, err);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LoadLocationSaTest001 end");
}

HWTEST_F(LocationCommonTest, LoadLocationSaTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LoadLocationSaTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LoadLocationSaTest002 begin");
    LocationErrCode err = LocationSaLoadManager::GetInstance()->UnloadLocationSa(UN_SAID);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, err);

    // can not unload sa by another sa
    err = LocationSaLoadManager::GetInstance()->UnloadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, err);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LoadLocationSaTest002 end");
}

HWTEST_F(LocationCommonTest, LocationDataRdbHelperTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocationDataRdbHelperTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationDataRdbHelperTest001 begin");

    Uri unknownUri(UN_URI);
    auto dataRdbObserver =  sptr<LocationDataRdbObserver>(new (std::nothrow) LocationDataRdbObserver());
    LocationErrCode err =
        LocationDataRdbHelper::GetInstance()->RegisterDataObserver(unknownUri, dataRdbObserver);
    EXPECT_EQ(ERRCODE_SUCCESS, err);

    err = LocationDataRdbHelper::GetInstance()->UnregisterDataObserver(unknownUri, dataRdbObserver);
    EXPECT_EQ(ERRCODE_SUCCESS, err);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationDataRdbHelperTest001 end");
}

HWTEST_F(LocationCommonTest, LocationDataRdbHelperTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocationDataRdbHelperTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationDataRdbHelperTest002 begin");
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    int32_t state = DISABLED;
    EXPECT_EQ(ERRCODE_SUCCESS, LocationDataRdbHelper::GetInstance()->
        SetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state));

    EXPECT_EQ(ERRCODE_SUCCESS, LocationDataRdbHelper::GetInstance()->
        GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state));
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationDataRdbHelperTest002 end");
}

HWTEST_F(LocationCommonTest, LocationDataRdbHelperTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocationDataRdbHelperTest003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationDataRdbHelperTest003 begin");
    Uri unknownUri(UN_URI);
    int32_t state = DISABLED;
    EXPECT_EQ(ERRCODE_SUCCESS, LocationDataRdbHelper::GetInstance()->
        SetValue(unknownUri, LOCATION_DATA_COLUMN_ENABLE, state));

    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, LocationDataRdbHelper::GetInstance()->
        GetValue(unknownUri, LOCATION_DATA_COLUMN_ENABLE, state));
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationDataRdbHelperTest003 end");
}

HWTEST_F(LocationCommonTest, LocationSaLoadManager002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocationSaLoadManager002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationSaLoadManager002 begin");
    LocationSaLoadManager::GetInstance()->LoadSystemAbilityFail();

    auto locationSaLoadCallback = sptr<LocationSaLoadCallback>(new LocationSaLoadCallback());
    ASSERT_TRUE(locationSaLoadCallback != nullptr);
    locationSaLoadCallback->OnLoadSystemAbilityFail(0);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationSaLoadManager002 end");
}

HWTEST_F(LocationCommonTest, GeoAddressDescriptionsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeoAddressDescriptionsTest004, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest004 begin");
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    geoAddress->descriptionsSize_ = 1;
    EXPECT_EQ("", geoAddress->GetDescriptions(-1));
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest004 end");
}

HWTEST_F(LocationCommonTest, LocationTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LocationTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationTest002 begin");
    auto location = std::make_shared<Location>();
    location->SetLatitude(VERIFY_LOCATION_LATITUDE);
    location->SetLongitude(VERIFY_LOCATION_LONGITUDE);
    location->SetAltitude(VERIFY_LOCATION_ALTITUDE);
    location->SetAccuracy(VERIFY_LOCATION_ACCURACY);
    location->SetSpeed(VERIFY_LOCATION_SPEED);
    location->SetDirection(VERIFY_LOCATION_DIRECTION);
    location->SetTimeStamp(VERIFY_LOCATION_TIME);
    location->SetTimeSinceBoot(VERIFY_LOCATION_TIMESINCEBOOT);
    std::vector<std::string> additions;
    additions.push_back("additions");
    location->SetAdditions(additions, false);
    location->SetAdditionSize(1);
    location->SetIsFromMock(1);
    location->ToString();
    LBSLOGI(LOCATOR, "[LocationCommonTest] LocationTest002 end");
}

HWTEST_F(LocationCommonTest, AppIdentityTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, AppIdentityTest002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] AppIdentityTest002 begin");
    AppIdentity identity(1, 2, 3, 4, 5);
    LBSLOGI(LOCATOR, "[LocationCommonTest] AppIdentityTest002 end");
}

HWTEST_F(LocationCommonTest, RequestConfigTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, RequestConfigTest003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] RequestConfigTest003 begin");
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>(1);
    ASSERT_TRUE(requestConfig != nullptr);
    LBSLOGI(LOCATOR, "[LocationCommonTest] RequestConfigTest003 end");
}

#define LOCATION_LOADSA_TIMEOUT_MS_FOR_TEST = 0
#define LOCATION_LOADSA_TIMEOUT_MS LOCATION_LOADSA_TIMEOUT_MS_FOR_TEST
HWTEST_F(LocationCommonTest, LoadLocationSaTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, LoadLocationSaTest003, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] LoadLocationSaTest003 begin");
    LocationErrCode err =
        LocationSaLoadManager::GetInstance()->WaitLoadStateChange(LOCATION_LOCATOR_SA_ID);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, err);
    LBSLOGI(LOCATOR, "[LocationCommonTest] LoadLocationSaTest003 end");
}
#undef LOCATION_LOADSA_TIMEOUT_MS
#undef LOCATION_LOADSA_TIMEOUT_MS_FOR_TEST

static int OhosHookTest01(const HOOK_INFO *hookInfo, void *executionContext)
{
    LBSLOGI(LOCATOR, "[LocationCommonTest] %{public}s enter", __func__);
    EXPECT_NE(hookInfo, nullptr);
    EXPECT_EQ(hookInfo->prio, 0);
    EXPECT_EQ(executionContext, nullptr);
    return 0;
}

static int OhosHookTest02(const HOOK_INFO *hookInfo, void *executionContext)
{
    LBSLOGI(LOCATOR, "[LocationCommonTest] %{public}s enter", __func__);
    EXPECT_NE(hookInfo, nullptr);
    EXPECT_EQ(hookInfo->prio, 1);
    EXPECT_EQ(executionContext, nullptr);
    return 0;
}

typedef struct hookTestStruct {
    const char *hookTestName;
    LocationProcessStage state;
} HOOK_TEST;

static int OhosHookTest03(const HOOK_INFO *hookInfo, void *executionContext)
{
    LBSLOGI(LOCATOR, "[LocationCommonTest] %{public}s enter", __func__);
    EXPECT_NE(hookInfo, nullptr);
    EXPECT_EQ(hookInfo->prio, 0);

    EXPECT_NE(executionContext, nullptr);
    HOOK_TEST* hookTest = (HOOK_TEST*)executionContext;
    EXPECT_NE(hookTest, nullptr);
    LocationProcessStage state = hookTest->state;
    const char *hookTestName = hookTest->hookTestName;
    EXPECT_EQ(state, LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS);
    EXPECT_EQ(hookTestName, "HookUtils002");
    return 0;
}

static int OhosHookTest04(const HOOK_INFO *hookInfo, void *executionContext)
{
    LBSLOGI(LOCATOR, "[LocationCommonTest] %{public}s enter", __func__);
    EXPECT_NE(hookInfo, nullptr);
    EXPECT_EQ(hookInfo->prio, 1);

    EXPECT_NE(executionContext, nullptr);
    HOOK_TEST* hookTest = (HOOK_TEST*)executionContext;
    EXPECT_NE(hookTest, nullptr);
    LocationProcessStage state = hookTest->state;
    const char *hookTestName = hookTest->hookTestName;
    EXPECT_EQ(state, LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS);
    EXPECT_EQ(hookTestName, "HookUtils002");
    return 0;
}

HWTEST_F(LocationCommonTest, HookUtils001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, HookUtils001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] HookUtils001 begin");
    ASSERT_TRUE(HookUtils::GetLocationExtHookMgr() != nullptr);
    EXPECT_EQ(ERRCODE_SUCCESS,
        HookUtils::RegisterHook(LocationProcessStage::LOCATOR_SA_START_LOCATING, 0, OhosHookTest01));
    EXPECT_EQ(ERRCODE_SUCCESS,
        HookUtils::RegisterHook(LocationProcessStage::LOCATOR_SA_START_LOCATING, 1, OhosHookTest02));

    EXPECT_EQ(ERRCODE_SUCCESS,
        HookUtils::ExecuteHook(LocationProcessStage::LOCATOR_SA_START_LOCATING, nullptr, nullptr));
    
    HookUtils::UnregisterHook(LocationProcessStage::LOCATOR_SA_START_LOCATING, OhosHookTest01);
    HookUtils::UnregisterHook(LocationProcessStage::LOCATOR_SA_START_LOCATING, OhosHookTest02);
    LBSLOGI(LOCATOR, "[LocationCommonTest] HookUtils001 end");
}

HWTEST_F(LocationCommonTest, HookUtils002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, HookUtils002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] HookUtils002 begin");
    ASSERT_TRUE(HookUtils::GetLocationExtHookMgr() != nullptr);
    EXPECT_EQ(ERRCODE_SUCCESS,
        HookUtils::RegisterHook(LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS, 0, OhosHookTest03));
    EXPECT_EQ(ERRCODE_SUCCESS,
        HookUtils::RegisterHook(LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS, 1, OhosHookTest04));

    HOOK_TEST hookTestStruct;
    hookTestStruct.hookTestName = "HookUtils002";
    hookTestStruct.state = LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS;

    EXPECT_EQ(ERRCODE_SUCCESS,
        HookUtils::ExecuteHook(LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS, (void *)(&hookTestStruct), nullptr));
    HookUtils::UnregisterHook(LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS, OhosHookTest03);
    HookUtils::UnregisterHook(LocationProcessStage::LOCATOR_SA_REQUEST_PROCESS, OhosHookTest04);
    LBSLOGI(LOCATOR, "[LocationCommonTest] HookUtils002 end");
}

HWTEST_F(LocationCommonTest, WorkRecordStatistic001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, WorkRecordStatistic001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] WorkRecordStatistic001 begin");
    auto workRecordStatistic = WorkRecordStatistic::GetInstance();
    workRecordStatistic->Update("network", 1);
    LBSLOGI(LOCATOR, "[LocationCommonTest] WorkRecordStatistic001 end");
}

HWTEST_F(LocationCommonTest, WorkRecordStatistic002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, WorkRecordStatistic002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] WorkRecordStatistic002 begin");
    auto workRecordStatistic = WorkRecordStatistic::GetInstance();
    workRecordStatistic->Update("network", 0);
    LBSLOGI(LOCATOR, "[LocationCommonTest] WorkRecordStatistic002 end");
}
} // namespace Location
} // namespace OHOS
