/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "geofence_request_test.h"
#include "location_log.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void GeofenceRequestTest::SetUp()
{
}

void GeofenceRequestTest::TearDown()
{
}

HWTEST_F(GeofenceRequestTest, SetGeofenceTransitionEventTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, SetGeofenceTransitionEventTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    gnssGeofenceRequest->SetGeofenceTransitionEvent(Location::GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventTest001 end");
}

HWTEST_F(GeofenceRequestTest, SetGeofenceTransitionEventListTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, SetGeofenceTransitionEventListTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventListTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    std::vector<Location::GeofenceTransitionEvent> statusList;
    statusList.push_back(Location::GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER);
    gnssGeofenceRequest->SetGeofenceTransitionEventList(statusList);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventListTest001 end");
}

HWTEST_F(GeofenceRequestTest, GetUidTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, GetUidTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetUidTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    gnssGeofenceRequest->GetUid();
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetUidTest001 end");
}

HWTEST_F(GeofenceRequestTest, SetAppAliveStatusTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, SetAppAliveStatusTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] SetAppAliveStatusTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    gnssGeofenceRequest->SetAppAliveStatus(true);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] SetAppAliveStatusTest001 end");
}

HWTEST_F(GeofenceRequestTest, GetAppAliveStatusTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, GetAppAliveStatusTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetAppAliveStatusTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    gnssGeofenceRequest->GetAppAliveStatus();
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetAppAliveStatusTest001 end");
}

HWTEST_F(GeofenceRequestTest, GetRequestExpirationTimeStampTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, GetRequestExpirationTimeStampTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetRequestExpirationTimeStampTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    gnssGeofenceRequest->GetRequestExpirationTimeStamp();
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetRequestExpirationTimeStampTest001 end");
}

HWTEST_F(GeofenceRequestTest, GetTransitionCallbackRecipientTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, GetTransitionCallbackRecipientTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetTransitionCallbackRecipientTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    gnssGeofenceRequest->GetTransitionCallbackRecipient();
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] GetTransitionCallbackRecipientTest001 end");
}

HWTEST_F(GeofenceRequestTest, ReadFromParcelTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, ReadFromParcelTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ReadFromParcelTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    Parcel data;
    gnssGeofenceRequest->ReadFromParcel(data);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ReadFromParcelTest001 end");
}

HWTEST_F(GeofenceRequestTest, UnmarshallingSharedTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, UnmarshallingSharedTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] UnmarshallingSharedTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    Parcel data;
    gnssGeofenceRequest->UnmarshallingShared(data);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] UnmarshallingSharedTest001 end");
}

HWTEST_F(GeofenceRequestTest, UnmarshallingTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, UnmarshallingTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] UnmarshallingTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    Parcel data;
    gnssGeofenceRequest->Unmarshalling(data);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] UnmarshallingTest001 end");
}

HWTEST_F(GeofenceRequestTest, ToJsonTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, UnmarshallingTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] UnmarshallingTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    nlohmann::json jsonObject;
    gnssGeofenceRequest->ToJson(jsonObject);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] UnmarshallingTest001 end");
}

HWTEST_F(GeofenceRequestTest, ConvertGeoFenceInfoTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, ConvertGeoFenceInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertGeoFenceInfoTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["latitude "] = 30;
    jsonObject["longitude "] = 120;
    jsonObject["radius "] = 1000;
    jsonObject["expiration "] = 1000;
    jsonObject["coordinateSystemType "] = 1;
    gnssGeofenceRequest->ConvertGeoFenceInfo(jsonObject);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertGeoFenceInfoTest001 end");
}

HWTEST_F(GeofenceRequestTest, ConvertTransitionEventInfoTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, ConvertTransitionEventInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertTransitionEventInfoTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequestParam = std::make_shared<Location::GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["transitionStatusList"] = {1, 2, 4};
    gnssGeofenceRequest->ConvertTransitionEventInfo(gnssGeofenceRequestParam, jsonObject);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertTransitionEventInfoTest001 end");
}

HWTEST_F(GeofenceRequestTest, ConvertWantAgentInfoTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, ConvertWantAgentInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertWantAgentInfoTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequestParam = std::make_shared<Location::GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["wantAgent"] = "wantAgent";
    gnssGeofenceRequest->ConvertWantAgentInfo(gnssGeofenceRequestParam, jsonObject);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertWantAgentInfoTest001 end");
}

HWTEST_F(GeofenceRequestTest, ConvertGeofenceRequestInfoTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, ConvertGeofenceRequestInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertGeofenceRequestInfoTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequestParam = std::make_shared<Location::GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["scenario"] = 1;
    jsonObject["loiterTimeMs"] = 1000;
    jsonObject["fenceId"] = 1;
    jsonObject["uid"] = 1001;
    jsonObject["appAliveStatus"] = true;
    jsonObject["fenceExtensionAbilityName"] = "fenceExtensionAbilityName";
    jsonObject["requestExpirationTimeStamp"] = 1000000;
    jsonObject["bundleName"] = "bundleName";
    gnssGeofenceRequest->ConvertGeofenceRequestInfo(gnssGeofenceRequestParam, jsonObject);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] ConvertGeofenceRequestInfoTest001 end");
}

HWTEST_F(GeofenceRequestTest, FromJsonTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceRequestTest, FromJsonTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] FromJsonTest001 begin");
    std::shared_ptr<Location::GeofenceRequest> gnssGeofenceRequest = std::make_shared<Location::GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["scenario"] = 1;
    jsonObject["loiterTimeMs"] = 1000;
    jsonObject["fenceId"] = 1;
    jsonObject["uid"] = 1001;
    jsonObject["appAliveStatus"] = true;
    jsonObject["fenceExtensionAbilityName"] = "fenceExtensionAbilityName";
    jsonObject["requestExpirationTimeStamp"] = 1000000;
    jsonObject["bundleName"] = "bundleName";
    gnssGeofenceRequest->FromJson(jsonObject);
    LBSLOGI(GEOFENCE_REQUEST_TEST, "[GeofenceRequestTest] FromJsonTest001 end");
}
} // namespace Location
} // namespace OHOS