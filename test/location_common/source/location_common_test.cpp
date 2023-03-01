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
#include "want.h"
#include "want_agent.h"

#include "app_identity.h"
#include "common_hisysevent.h"
#include "constant_definition.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "geofence_state.h"
#endif
#include "location.h"
#include "location_log.h"
#include "request_config.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "satellite_status.h"
#endif
#include "permission_status_change_cb.h"
#include "common_utils.h"
#include "locator_event_subscriber.h"
#include "country_code_manager.h"

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
const double VERIFY_LOCATION_FLOOR_ACC = 1000.0;
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
    parcel.WriteString16(u"localeLanguage");
    parcel.WriteString16(u"localeCountry");
    parcel.WriteInt32(1); // size
    parcel.WriteInt32(0); // line
    parcel.WriteString16(u"line");
    parcel.WriteString16(u"placeName");
    parcel.WriteString16(u"administrativeArea");
    parcel.WriteString16(u"subAdministrativeArea");
    parcel.WriteString16(u"locality");
    parcel.WriteString16(u"subLocality");
    parcel.WriteString16(u"roadName");
    parcel.WriteString16(u"subRoadName");
    parcel.WriteString16(u"premises");
    parcel.WriteString16(u"postalCode");
    parcel.WriteString16(u"countryCode");
    parcel.WriteString16(u"countryName");
    parcel.WriteInt32(1); // hasLatitude
    parcel.WriteDouble(MOCK_LATITUDE); // latitude
    parcel.WriteInt32(1); // hasLongitude
    parcel.WriteDouble(MOCK_LONGITUDE); // longitude
    parcel.WriteString16(u"phoneNumber");
    parcel.WriteString16(u"addressUrl");
    parcel.WriteBool(true);
    geoAddress->ReadFromParcel(parcel);
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
void LocationCommonTest::VerifyGeoAddressReadFromParcel(std::unique_ptr<GeoAddress>& geoAddress)
{
    EXPECT_EQ("localeLanguage", geoAddress->m_localeLanguage);
    EXPECT_EQ("localeCountry", geoAddress->m_localeCountry);
    EXPECT_EQ(1, geoAddress->m_descriptions.size());
    auto iter = geoAddress->m_descriptions.find(0);
    EXPECT_EQ(true, iter != geoAddress->m_descriptions.end());
    EXPECT_EQ(0, iter->first);
    EXPECT_EQ("line", iter->second);
    EXPECT_EQ("placeName", geoAddress->m_placeName);
    EXPECT_EQ("administrativeArea", geoAddress->m_administrativeArea);
    EXPECT_EQ("subAdministrativeArea", geoAddress->m_subAdministrativeArea);
    EXPECT_EQ("locality", geoAddress->m_locality);
    EXPECT_EQ("subLocality", geoAddress->m_subLocality);
    EXPECT_EQ("roadName", geoAddress->m_roadName);
    EXPECT_EQ("subRoadName", geoAddress->m_subRoadName);
    EXPECT_EQ("premises", geoAddress->m_premises);
    EXPECT_EQ("postalCode", geoAddress->m_postalCode);
    EXPECT_EQ("countryCode", geoAddress->m_countryCode);
    EXPECT_EQ("countryName", geoAddress->m_countryName);
    EXPECT_EQ(1, geoAddress->m_hasLatitude);
    EXPECT_EQ(MOCK_LATITUDE, geoAddress->m_latitude);
    EXPECT_EQ(1, geoAddress->m_hasLongitude);
    EXPECT_EQ(MOCK_LONGITUDE, geoAddress->m_longitude);
    EXPECT_EQ("phoneNumber", geoAddress->m_phoneNumber);
    EXPECT_EQ("addressUrl", geoAddress->m_addressUrl);
    EXPECT_EQ(true, geoAddress->m_isFromMock);
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
void LocationCommonTest::VerifyGeoAddressMarshalling(MessageParcel& newParcel)
{
    EXPECT_EQ("localeLanguage", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("localeCountry", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(0, newParcel.ReadInt32());
    EXPECT_EQ("line", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("placeName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("administrativeArea", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("subAdministrativeArea", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("locality", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("subLocality", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("roadName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("subRoadName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("premises", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("postalCode", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("countryCode", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("countryName", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(MOCK_LATITUDE, newParcel.ReadDouble());
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(MOCK_LONGITUDE, newParcel.ReadDouble());
    EXPECT_EQ("phoneNumber", Str16ToStr8(newParcel.ReadString16()));
    EXPECT_EQ("addressUrl", Str16ToStr8(newParcel.ReadString16()));
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
    EXPECT_EQ("additions", Str16ToStr8(newParcel.ReadString16())); // additions
    EXPECT_EQ(1, newParcel.ReadInt64()); // additionSize
    EXPECT_EQ(true, newParcel.ReadBool()); // isFromMock
    EXPECT_EQ(1, newParcel.ReadInt32()); // sourceType
    EXPECT_EQ(0, newParcel.ReadInt32()); // floorNo
    EXPECT_EQ(VERIFY_LOCATION_FLOOR_ACC, newParcel.ReadDouble()); // floorAccuracy
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
    geoAddress->m_hasLatitude = true;
    geoAddress->m_latitude = 1.0;
    EXPECT_EQ(1.0, geoAddress->GetLatitude());

    geoAddress->m_hasLatitude = false;
    EXPECT_EQ(0.0, geoAddress->GetLatitude());

    geoAddress->m_hasLongitude = true;
    geoAddress->m_longitude = 1.0;
    EXPECT_EQ(1.0, geoAddress->GetLongitude());

    geoAddress->m_hasLongitude = false;
    EXPECT_EQ(0.0, geoAddress->GetLongitude());
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
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(VERIFY_LOCATION_FLOOR_ACC); // floor acc
    location->ReadFromParcel(parcel);
    EXPECT_EQ(VERIFY_LOCATION_LATITUDE, location->GetLatitude());
    EXPECT_EQ(VERIFY_LOCATION_LONGITUDE, location->GetLongitude());
    EXPECT_EQ(VERIFY_LOCATION_ALTITUDE, location->GetAltitude());
    EXPECT_EQ(VERIFY_LOCATION_ACCURACY, location->GetAccuracy());
    EXPECT_EQ(VERIFY_LOCATION_SPEED, location->GetSpeed());
    EXPECT_EQ(VERIFY_LOCATION_DIRECTION, location->GetDirection());
    EXPECT_EQ(VERIFY_LOCATION_TIME, location->GetTimeStamp());
    EXPECT_EQ(VERIFY_LOCATION_TIMESINCEBOOT, location->GetTimeSinceBoot());
    EXPECT_EQ("additions", location->GetAdditions());
    EXPECT_EQ(1, location->GetAdditionSize());
    EXPECT_EQ(true, location->GetIsFromMock());
    EXPECT_EQ(1, location->GetSourceType());
    EXPECT_EQ(0, location->GetFloorNo());
    EXPECT_EQ(VERIFY_LOCATION_FLOOR_ACC, location->GetFloorAccuracy());

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
    }
    status->ReadFromParcel(parcel);
    EXPECT_EQ(2, status->GetSatellitesNumber());
    for (int i = 0; i < sateNum; i++) {
        EXPECT_EQ(i, status->GetSatelliteIds()[i]);
        EXPECT_EQ(i + 1.0, status->GetCarrierToNoiseDensitys()[i]);
        EXPECT_EQ(i + 2.0, status->GetAltitudes()[i]);
        EXPECT_EQ(i + 3.0, status->GetAzimuths()[i]);
        EXPECT_EQ(i + 4.0, status->GetCarrierFrequencies()[i]);
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
    parcel.WriteInt32(10); // distanceInterval
    parcel.WriteFloat(1000.0); // maxAccuracy
    parcel.WriteInt32(1); // fixNumber
    requestConfig->ReadFromParcel(parcel);
    EXPECT_EQ(1, requestConfig->GetScenario());
    EXPECT_EQ(2, requestConfig->GetPriority());
    EXPECT_EQ(3, requestConfig->GetTimeInterval());
    EXPECT_EQ(10, requestConfig->GetDistanceInterval());
    EXPECT_EQ(1000.0, requestConfig->GetMaxAccuracy());
    EXPECT_EQ(1, requestConfig->GetFixNumber());

    MessageParcel newParcel;
    requestConfig->Marshalling(newParcel);
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(2, newParcel.ReadInt32());
    EXPECT_EQ(3, newParcel.ReadInt32());
    EXPECT_EQ(10, newParcel.ReadInt32());
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
    requestConfigForSet1.SetDistanceInterval(4);
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

HWTEST_F(LocationCommonTest, CommonHisyseventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, CommonHisyseventTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] CommonHisyseventTest001 begin");
    std::string state = "state";
#ifdef FEATURE_GNSS_SUPPORT
    pid_t pid = 1;
    pid_t uid = 2;
    WriteGnssStateEvent(state, pid, uid);
#endif
    WriteLocationSwitchStateEvent(state);
    LBSLOGI(LOCATOR, "[LocationCommonTest] CommonHisyseventTest001 end");
}

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocationCommonTest, GeoFenceStateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationCommonTest, GeoFenceStateTest001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoFenceStateTest001 begin");
    GeoFence fence;
    fence.latitude = 1.0;
    fence.longitude = 2.0;
    fence.radius = 3.0;
    fence.expiration = 4.0;
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    auto state = new (std::nothrow) GeoFenceState(fence, wantAgent);
    EXPECT_NE(nullptr, state);
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoFenceStateTest001 end");
}
#endif

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
    geoAddress->GetDescriptions(0);
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
    geoAddress->GetDescriptions(1);
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
    geoAddress->GetDescriptions(-1);
    LBSLOGI(LOCATOR, "[LocationCommonTest] GeoAddressDescriptionsTest003 end");
}
#endif
} // namespace Location
} // namespace OHOS