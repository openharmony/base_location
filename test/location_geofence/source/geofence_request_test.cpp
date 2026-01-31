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
#include "geofence_request.cpp"
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

HWTEST_F(GeofenceSdkTest, SetGeofenceTransitionEventTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, SetGeofenceTransitionEventTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    gnssGeofenceRequest->SetGeofenceTransitionEvent(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventTest001 end");
}

HWTEST_F(GeofenceSdkTest, SetGeofenceTransitionEventListTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, SetGeofenceTransitionEventListTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventListTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    std::vector<GeofenceTransitionEvent> statusList;
    statusList.push_back(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER);
    gnssGeofenceRequest->SetGeofenceTransitionEventList(statusList);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetGeofenceTransitionEventListTest001 end");
}

HWTEST_F(GeofenceSdkTest, SetNotificationRequestTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, SetNotificationRequestTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetNotificationRequestTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    std::shared_ptr<OHOS::Notification::NotificationRequest> request;
    gnssGeofenceRequest->SetNotificationRequest(request);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetNotificationRequestTest001 end");
}

HWTEST_F(GeofenceSdkTest, GetUidTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, GetUidTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetUidTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    gnssGeofenceRequest->GetUid();
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetUidTest001 end");
}

HWTEST_F(GeofenceSdkTest, SetAppAliveStatusTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, SetAppAliveStatusTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetAppAliveStatusTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    gnssGeofenceRequest->SetAppAliveStatus(true);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] SetAppAliveStatusTest001 end");
}

HWTEST_F(GeofenceSdkTest, GetAppAliveStatusTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, GetAppAliveStatusTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetAppAliveStatusTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    gnssGeofenceRequest->GetAppAliveStatus();
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetAppAliveStatusTest001 end");
}

HWTEST_F(GeofenceSdkTest, GetRequestExpirationTimeStampTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, GetRequestExpirationTimeStampTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetRequestExpirationTimeStampTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    gnssGeofenceRequest->GetRequestExpirationTimeStamp();
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetRequestExpirationTimeStampTest001 end");
}

HWTEST_F(GeofenceSdkTest, GetTransitionCallbackRecipientTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, GetTransitionCallbackRecipientTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetTransitionCallbackRecipientTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    gnssGeofenceRequest->GetTransitionCallbackRecipient();
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] GetTransitionCallbackRecipientTest001 end");
}

HWTEST_F(GeofenceSdkTest, ReadFromParcelTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, ReadFromParcelTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ReadFromParcelTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    Parcel data;
    gnssGeofenceRequest->ReadFromParcel(data);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ReadFromParcelTest001 end");
}

HWTEST_F(GeofenceSdkTest, UnmarshallingSharedTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, UnmarshallingSharedTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] UnmarshallingSharedTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    Parcel data;
    gnssGeofenceRequest->UnmarshallingShared(data);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] UnmarshallingSharedTest001 end");
}

HWTEST_F(GeofenceSdkTest, UnmarshallingTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, UnmarshallingTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] UnmarshallingTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    Parcel data;
    gnssGeofenceRequest->Unmarshalling(data);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] UnmarshallingTest001 end");
}

HWTEST_F(GeofenceSdkTest, ToJsonTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, UnmarshallingTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] UnmarshallingTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    nlohmann::json jsonObject;
    gnssGeofenceRequest->ToJson(jsonObject);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] UnmarshallingTest001 end");
}

HWTEST_F(GeofenceSdkTest, ConvertGeoFenceInfoTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, ConvertGeoFenceInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertGeoFenceInfoTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["latitude "] = 30;
    jsonObject["longitude "] = 120;
    jsonObject["radius "] = 1000;
    jsonObject["expiration "] = 1000;
    jsonObject["coordinateSystemType "] = 1;
    gnssGeofenceRequest->ConvertGeoFenceInfo(jsonObject);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertGeoFenceInfoTest001 end");
}

HWTEST_F(GeofenceSdkTest, ConvertTransitionEventInfoTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, ConvertTransitionEventInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertTransitionEventInfoTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequestParam = std::make_shared<GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["transitionStatusList"] = {1, 2, 4};
    gnssGeofenceRequest->ConvertTransitionEventInfo(gnssGeofenceRequestParam, jsonObject);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertTransitionEventInfoTest001 end");
}

HWTEST_F(GeofenceSdkTest, ConvertNotificationInfoTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, ConvertNotificationInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertNotificationInfoTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequestParam = std::make_shared<GeofenceRequest>();
    nlohmann::json jsonObject;
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> notificationRequestList;
    std::shared_ptr<OHOS::Notification::NotificationRequest> notificationRequest =
        std::make_shared<OHOS::Notification::NotificationRequest>();
    nlohmann::json notificationArr = nlohmann::json::array(); 
    if (notificationRequest != nullptr) {
        nlohmann::json jsonObj;
        notificationRequest->ToJson(jsonObj);
        notificationArr.emplace_back(jsonObj);
    }
    jsonObject["notificationRequestList"] = notificationArr;
    gnssGeofenceRequest->ConvertNotificationInfo(gnssGeofenceRequestParam, jsonObject);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertNotificationInfoTest001 end");
}

HWTEST_F(GeofenceSdkTest, ConvertWantAgentInfoTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, ConvertWantAgentInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertWantAgentInfoTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequestParam = std::make_shared<GeofenceRequest>();
    nlohmann::json jsonObject;
    jsonObject["wantAgent"] = "wantAgent";
    gnssGeofenceRequest->ConvertWantAgentInfo(gnssGeofenceRequestParam, jsonObject);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertWantAgentInfoTest001 end");
}

HWTEST_F(GeofenceSdkTest, ConvertGeofenceRequestInfoTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, ConvertGeofenceRequestInfoTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertGeofenceRequestInfoTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequestParam = std::make_shared<GeofenceRequest>();
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
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] ConvertGeofenceRequestInfoTest001 end");
}

HWTEST_F(GeofenceSdkTest, FromJsonTest001, Test001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, FromJsonTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] FromJsonTest001 begin");
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
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
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceRequestTest] FromJsonTest001 end");
}
} // namespace Location
} // namespace OHOS