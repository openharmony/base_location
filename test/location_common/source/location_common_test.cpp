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
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#include "geofence_state.h"
#include "location.h"
#include "location_log.h"
#include "request_config.h"
#include "satellite_status.h"
#include "permission_status_change_cb.h"
#include "common_utils.h"
#include "locator_event_subscriber.h"
#include "country_code_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
using Want = OHOS::AAFwk::Want;
const double MOCK_LATITUDE = 99.0;
const double MOCK_LONGITUDE = 100.0;
void LocationCommonTest::SetUp()
{
}

void LocationCommonTest::TearDown()
{
}

void LocationCommonTest::SetGeoAddress(std::unique_ptr<GeoAddress>& geoAddress)
{
    MessageParcel parcel;
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
    geoAddress->ReadFromParcel(parcel);
}

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

void LocationCommonTest::VerifyGeoAddressMarshalling(MessageParcel& newParcel)
{
    EXPECT_EQ("localeLanguage", newParcel.ReadString());
    EXPECT_EQ("localeCountry", newParcel.ReadString());
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(0, newParcel.ReadInt32());
    EXPECT_EQ("line", newParcel.ReadString());
    EXPECT_EQ("placeName", newParcel.ReadString());
    EXPECT_EQ("administrativeArea", newParcel.ReadString());
    EXPECT_EQ("subAdministrativeArea", newParcel.ReadString());
    EXPECT_EQ("locality", newParcel.ReadString());
    EXPECT_EQ("subLocality", newParcel.ReadString());
    EXPECT_EQ("roadName", newParcel.ReadString());
    EXPECT_EQ("subRoadName", newParcel.ReadString());
    EXPECT_EQ("premises", newParcel.ReadString());
    EXPECT_EQ("postalCode", newParcel.ReadString());
    EXPECT_EQ("countryCode", newParcel.ReadString());
    EXPECT_EQ("countryName", newParcel.ReadString());
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(MOCK_LATITUDE, newParcel.ReadDouble());
    EXPECT_EQ(1, newParcel.ReadInt32());
    EXPECT_EQ(MOCK_LONGITUDE, newParcel.ReadDouble());
    EXPECT_EQ("phoneNumber", newParcel.ReadString());
    EXPECT_EQ("addressUrl", newParcel.ReadString());
    EXPECT_EQ(true, newParcel.ReadBool());
}

/*
 * @tc.name: GeoAddressTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LocationCommonTest, GeoAddressTest001, TestSize.Level1)
{
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    VerifyGeoAddressReadFromParcel(geoAddress);
    
    MessageParcel newParcel;
    geoAddress->Marshalling(newParcel);
    VerifyGeoAddressMarshalling(newParcel);
}

/*
 * @tc.name: GeoAddressTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
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

/*
 * @tc.name: LocationTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LocationCommonTest, LocationTest001, TestSize.Level1)
{
    auto location = std::make_shared<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteFloat(1000.0); // accuracy
    parcel.WriteFloat(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000000); // timeSinceBoot
    parcel.WriteString("additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteBool(true); // isFromMock
    location->ReadFromParcel(parcel);
    EXPECT_EQ(12.0, location->GetLatitude());
    EXPECT_EQ(13.0, location->GetLongitude());
    EXPECT_EQ(14.0, location->GetAltitude());
    EXPECT_EQ(1000.0, location->GetAccuracy());
    EXPECT_EQ(10.0, location->GetSpeed());
    EXPECT_EQ(90.0, location->GetDirection());
    EXPECT_EQ(1000000000, location->GetTimeStamp());
    EXPECT_EQ(1000000000, location->GetTimeSinceBoot());
    EXPECT_EQ("additions", location->GetAdditions());
    EXPECT_EQ(1, location->GetAdditionSize());
    EXPECT_EQ(true, location->GetIsFromMock());

    MessageParcel newParcel;
    location->Marshalling(newParcel);
    EXPECT_EQ(12.0, newParcel.ReadDouble());
    EXPECT_EQ(13.0, newParcel.ReadDouble());
    EXPECT_EQ(14.0, newParcel.ReadDouble());
    EXPECT_EQ(1000.0, newParcel.ReadFloat());
    EXPECT_EQ(10.0, newParcel.ReadFloat());
    EXPECT_EQ(90.0, newParcel.ReadDouble());
    EXPECT_EQ(1000000000, newParcel.ReadInt64());
    EXPECT_EQ(1000000000, newParcel.ReadInt64());
    EXPECT_EQ("additions", newParcel.ReadString());
    EXPECT_EQ(1, newParcel.ReadInt64());
    EXPECT_EQ(true, newParcel.ReadBool());
}

/*
 * @tc.name: SateLLiteStatusTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LocationCommonTest, SateLLiteStatusTest001, TestSize.Level1)
{
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
}

/*
 * @tc.name: RequestConfigTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LocationCommonTest, RequestConfigTest001, TestSize.Level1)
{
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
}

HWTEST_F(LocationCommonTest, RequestConfigTest002, TestSize.Level1)
{
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
}

/*
 * @tc.name: GeocodingMockInfoTest001
 * @tc.desc: read from parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LocationCommonTest, GeocodingMockInfoTest001, TestSize.Level1)
{
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
}

HWTEST_F(LocationCommonTest, AppIdentityTest001, TestSize.Level1)
{
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
}

HWTEST_F(LocationCommonTest, CommonHisyseventTest001, TestSize.Level1)
{
    std::string state = "state";
    pid_t pid = 1;
    pid_t uid = 2;
    WriteGnssStateEvent(state, pid, uid);
    WriteLocationSwitchStateEvent(state);
}

HWTEST_F(LocationCommonTest, GeoFenceStateTest001, TestSize.Level1)
{
    GeoFence fence;
    fence.latitude = 1.0;
    fence.longitude = 2.0;
    fence.radius = 3.0;
    fence.expiration = 4.0;
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    auto state = new (std::nothrow) GeoFenceState(fence, wantAgent);
    EXPECT_NE(nullptr, state);
}

HWTEST_F(LocationCommonTest, GeocodingMockInfoTest002, TestSize.Level1)
{
    std::unique_ptr<GeocodingMockInfo> mockInfo = std::make_unique<GeocodingMockInfo>();
    
    auto reverseGeocodeRequest = std::make_shared<ReverseGeocodeRequest>();
    mockInfo->SetLocation(reverseGeocodeRequest);
    EXPECT_NE(nullptr, mockInfo->GetLocation());

    std::shared_ptr<GeoAddress> geoAddress = std::make_shared<GeoAddress>();
    mockInfo->SetGeoAddressInfo(geoAddress);
    EXPECT_NE(nullptr, mockInfo->GetGeoAddressInfo());
}

HWTEST_F(LocationCommonTest, PermStateChangeCallbackTest001, TestSize.Level1)
{
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    PermStateChangeScope scopeInfo;
    scopeInfo.permList = {"ohos.permission.LOCATION"};
    scopeInfo.tokenIDs = {callingTokenId};
    auto callbackPtr = std::make_shared<PermissionStatusChangeCb>(scopeInfo);
    struct PermStateChangeInfo result{0, callingTokenId, ACCESS_LOCATION};
    callbackPtr->PermStateChangeCallback(result);
}

HWTEST_F(LocationCommonTest, LocatorEventSubscriberTest001, TestSize.Level1)
{
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto locatorEventSubscriber = std::make_shared<LocatorEventSubscriber>(subscriberInfo);
    Want want;
    want.SetAction("usual.event.location.MODE_STATE_CHANGED");
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    locatorEventSubscriber->OnReceiveEvent(data);
}

HWTEST_F(LocationCommonTest, LocatorEventSubscriberTest002, TestSize.Level1)
{
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto locatorEventSubscriber = std::make_shared<LocatorEventSubscriber>(subscriberInfo);
    Want want;
    want.SetAction("Invalid action");
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    locatorEventSubscriber->OnReceiveEvent(data);
}

HWTEST_F(LocationCommonTest, GeoAddressDescriptionsTest001, TestSize.Level1)
{
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    geoAddress->GetDescriptions(0);
}

HWTEST_F(LocationCommonTest, GeoAddressDescriptionsTest002, TestSize.Level1)
{
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    geoAddress->GetDescriptions(1);
}

HWTEST_F(LocationCommonTest, GeoAddressDescriptionsTest003, TestSize.Level1)
{
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    SetGeoAddress(geoAddress);
    geoAddress->GetDescriptions(-1);
}
} // namespace Location
} // namespace OHOS