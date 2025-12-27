/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "geofence_request.h"
#include <parcel.h>
#include "common_utils.h"
#ifdef NOTIFICATION_ENABLE
#include "notification_request.h"
#endif
#include "want_agent.h"
#include "want_agent_helper.h"

namespace OHOS {
namespace Location {
GeofenceRequest::GeofenceRequest()
{
    callback_ = nullptr;
    scenario_ = -1;
    fenceId_ = -1;
    uid_ = 0;
    appAliveStatus_ = true;
    loiterTimeMs_ = 0;
    wantAgent_ = nullptr;
}

GeofenceRequest::GeofenceRequest(GeofenceRequest& geofenceRequest)
{
    this->SetGeofence(geofenceRequest.GetGeofence());
    this->SetScenario(geofenceRequest.GetScenario());
    this->SetWantAgent(geofenceRequest.GetWantAgent());
    this->SetGeofenceTransitionEventList(geofenceRequest.GetGeofenceTransitionEventList());
#ifdef NOTIFICATION_ENABLE
    this->SetNotificationRequestList(geofenceRequest.GetNotificationRequestList());
#endif
    this->SetGeofenceTransitionCallback(geofenceRequest.GetGeofenceTransitionCallback());
    this->SetFenceId(geofenceRequest.GetFenceId());
    this->SetBundleName(geofenceRequest.GetBundleName());
    this->SetLoiterTimeMs(geofenceRequest.GetLoiterTimeMs());
}

GeofenceRequest::~GeofenceRequest() {}

GeoFence GeofenceRequest::GetGeofence()
{
    return geofence_;
}

void GeofenceRequest::SetGeofence(GeoFence geofence)
{
    geofence_ = geofence;
}

int GeofenceRequest::GetScenario()
{
    return scenario_;
}

void GeofenceRequest::SetScenario(int scenario)
{
    scenario_ = scenario;
}

int GeofenceRequest::GetLoiterTimeMs()
{
    return loiterTimeMs_;
}

void GeofenceRequest::SetLoiterTimeMs(int loiterTimeMs)
{
    loiterTimeMs_ = loiterTimeMs;
}

void GeofenceRequest::SetWantAgent(const std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    wantAgent_ = wantAgent;
}

std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> GeofenceRequest::GetWantAgent()
{
    return wantAgent_;
}

std::vector<GeofenceTransitionEvent> GeofenceRequest::GetGeofenceTransitionEventList()
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    return transitionStatusList_;
}

void GeofenceRequest::SetGeofenceTransitionEvent(GeofenceTransitionEvent status)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    transitionStatusList_.push_back(status);
}

void GeofenceRequest::SetGeofenceTransitionEventList(std::vector<GeofenceTransitionEvent> statusList)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    for (auto it = statusList.begin(); it != statusList.end(); ++it) {
        transitionStatusList_.push_back(*it);
    }
}

#ifdef NOTIFICATION_ENABLE
std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> GeofenceRequest::GetNotificationRequestList()
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    return notificationRequestList_;
}

void GeofenceRequest::SetNotificationRequest(std::shared_ptr<OHOS::Notification::NotificationRequest> request)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    notificationRequestList_.push_back(request);
}

void GeofenceRequest::SetNotificationRequestList(
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> requestList)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    for (auto it = requestList.begin(); it != requestList.end(); ++it) {
        notificationRequestList_.push_back(*it);
    }
}
#endif

void GeofenceRequest::SetGeofenceTransitionCallback(const sptr<IRemoteObject>& callback)
{
    callback_ = callback;
}

sptr<IRemoteObject> GeofenceRequest::GetGeofenceTransitionCallback()
{
    return callback_;
}

int GeofenceRequest::GetFenceId()
{
    return fenceId_;
}

void GeofenceRequest::SetFenceId(int fenceId)
{
    fenceId_ = fenceId;
}

const std::string& GeofenceRequest::GetBundleName()
{
    return bundleName_;
}

void GeofenceRequest::SetBundleName(const std::string& bundleName)
{
    bundleName_ = bundleName;
}

int32_t GeofenceRequest::GetUid()
{
    return uid_;
}

void GeofenceRequest::SetUid(int32_t uid)
{
    uid_ = uid;
}

bool GeofenceRequest::GetAppAliveStatus()
{
    return appAliveStatus_;
}

int64_t GeofenceRequest::GetRequestExpirationTimeStamp()
{
    return requestExpirationTimeStamp_;
}

void GeofenceRequest::SetRequestExpirationTimeStamp(int64_t requestExpirationTimeStamp)
{
    requestExpirationTimeStamp_ = requestExpirationTimeStamp;
}

void GeofenceRequest::SetAppAliveStatus(bool appAliveStatus)
{
    appAliveStatus_ = appAliveStatus;
}

void GeofenceRequest::SetTransitionCallbackDeathRecipient(const sptr<IRemoteObject::DeathRecipient>& deathRecipient)
{
    transitionCallbackDeathRecipient_ = deathRecipient;
}

sptr<IRemoteObject::DeathRecipient> GeofenceRequest::GetTransitionCallbackRecipient()
{
    return transitionCallbackDeathRecipient_;
}

void GeofenceRequest::ReadFromParcel(Parcel& data)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    fenceId_ = data.ReadInt32();
    scenario_ = data.ReadInt32();
    loiterTimeMs_ = data.ReadInt32();
    geofence_.latitude = data.ReadDouble();
    geofence_.longitude = data.ReadDouble();
    geofence_.radius = data.ReadDouble();
    geofence_.expiration = data.ReadDouble();
    geofence_.coordinateSystemType = static_cast<CoordinateSystemType>(data.ReadInt32());
    int monitorGeofenceTransitionSize = data.ReadInt32();
    if (monitorGeofenceTransitionSize > MAX_TRANSITION_SIZE) {
        LBSLOGE(LOCATOR, "fence transition list size should not be greater than 3");
        return;
    }
    for (int i = 0; i < monitorGeofenceTransitionSize; i++) {
        transitionStatusList_.push_back(static_cast<GeofenceTransitionEvent>(data.ReadInt32()));
    }
#ifdef NOTIFICATION_ENABLE
    int requestSize = data.ReadInt32();
    if (requestSize > MAX_NOTIFICATION_REQUEST_LIST_SIZE) {
        LBSLOGE(LOCATOR, "request size should not be greater than 3");
        return;
    }
    for (int i = 0; i < requestSize; i++) {
        std::shared_ptr<OHOS::Notification::NotificationRequest> request(
            OHOS::Notification::NotificationRequest::Unmarshalling(data));
        if (request != nullptr) {
            notificationRequestList_.push_back(request);
        }
    }
#endif
    callback_ = data.ReadObject<IRemoteObject>();
    data.ReadString(bundleName_);
    uid_ = data.ReadInt32();
    AbilityRuntime::WantAgent::WantAgent* wantAgentData =
        AbilityRuntime::WantAgent::WantAgent::Unmarshalling(data);
    if (wantAgentData == nullptr) {
        return;
    }
    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgent =
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(*wantAgentData);
    if (wantAgent != nullptr) {
        wantAgent_ = wantAgent;
    }
    delete wantAgentData;
    wantAgentData = nullptr;
}

bool GeofenceRequest::Marshalling(Parcel& parcel) const
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    parcel.WriteInt32(fenceId_);
    parcel.WriteInt32(scenario_);
    parcel.WriteInt32(loiterTimeMs_);
    parcel.WriteDouble(geofence_.latitude);
    parcel.WriteDouble(geofence_.longitude);
    parcel.WriteDouble(geofence_.radius);
    parcel.WriteDouble(geofence_.expiration);
    parcel.WriteInt32(static_cast<int>(geofence_.coordinateSystemType));
    if (transitionStatusList_.size() > MAX_TRANSITION_SIZE) {
        LBSLOGE(LOCATOR, "fence transition list size should not be greater than 3");
        return false;
    }
    parcel.WriteInt32(transitionStatusList_.size());
    for (size_t i = 0; i < transitionStatusList_.size(); i++) {
        parcel.WriteInt32(static_cast<int>(transitionStatusList_[i]));
    }
#ifdef NOTIFICATION_ENABLE
    parcel.WriteInt32(notificationRequestList_.size());
    if (notificationRequestList_.size() > MAX_NOTIFICATION_REQUEST_LIST_SIZE) {
        LBSLOGE(LOCATOR, "request size should not be greater than 3");
        return false;
    }
    for (size_t i = 0; i < notificationRequestList_.size(); i++) {
        if (notificationRequestList_[i] != nullptr) {
            notificationRequestList_[i]->Marshalling(parcel);
        }
    }
#endif
    parcel.WriteRemoteObject(callback_);
    parcel.WriteString(bundleName_);
    parcel.WriteInt32(uid_);
    if (wantAgent_ != nullptr) {
        wantAgent_->Marshalling(parcel);
    }
    return true;
}

std::shared_ptr<GeofenceRequest> GeofenceRequest::UnmarshallingShared(Parcel& parcel)
{
    std::shared_ptr<GeofenceRequest> geofenceRequest = std::make_shared<GeofenceRequest>();
    geofenceRequest->ReadFromParcel(parcel);
    return geofenceRequest;
}

GeofenceRequest* GeofenceRequest::Unmarshalling(Parcel& parcel)
{
    auto geofenceRequest = new GeofenceRequest();
    geofenceRequest->ReadFromParcel(parcel);
    return geofenceRequest;
}

bool GeofenceRequest::ToJson(nlohmann::json &jsonObject)
{
    jsonObject["transitionStatusList"] = transitionStatusList_;
#ifdef NOTIFICATION_ENABLE
    nlohmann::json notificationArr = nlohmann::json::array();
    for (auto notificationRequest : notificationRequestList_) {
        if (notificationRequest != nullptr) {
            nlohmann::json jsonObj;
            notificationRequest->ToJson(jsonObj);
            notificationArr.emplace_back(jsonObj);
        }
    }
    jsonObject["notificationRequestList"] = notificationArr;
#endif

    nlohmann::json jsonGeofenceObj;
    jsonGeofenceObj["latitude"] = geofence_.latitude;
    jsonGeofenceObj["longitude"] = geofence_.longitude;
    jsonGeofenceObj["radius"] = geofence_.radius;
    jsonGeofenceObj["expiration"] = geofence_.expiration;
    jsonGeofenceObj["coordinateSystemType"] = geofence_.coordinateSystemType;
    jsonObject["geofence"] = jsonGeofenceObj;
    jsonObject["scenario"] = scenario_;
    jsonObject["fenceId"] = fenceId_;
    jsonObject["uid"] = uid_;
    jsonObject["wantAgent"] = wantAgent_ ? AbilityRuntime::WantAgent::WantAgentHelper::ToString(wantAgent_) : "";
    jsonObject["bundleName"] = bundleName_;
    jsonObject["appAliveStatus"] = appAliveStatus_;
    jsonObject["requestExpirationTimeStamp"] = requestExpirationTimeStamp_;
    return true;
}

void GeofenceRequest::ConvertGeoFenceInfo(const nlohmann::json &geofenceObj, GeoFence& geofence)
{
    if (geofenceObj.find("latitude") != geofenceObj.cend() && geofenceObj.at("latitude").is_number()) {
        geofence.latitude = geofenceObj.at("latitude").get<double>();
    }
    if (geofenceObj.find("longitude") != geofenceObj.cend() && geofenceObj.at("longitude").is_number()) {
        geofence.longitude = geofenceObj.at("longitude").get<double>();
    }
    if (geofenceObj.find("radius") != geofenceObj.cend() && geofenceObj.at("radius").is_number()) {
        geofence.radius = geofenceObj.at("radius").get<double>();
    }
    if (geofenceObj.find("expiration") != geofenceObj.cend() && geofenceObj.at("expiration").is_number()) {
        geofence.expiration = geofenceObj.at("expiration").get<double>();
    }
    if (geofenceObj.find("coordinateSystemType") != geofenceObj.cend() &&
        geofenceObj.at("coordinateSystemType").is_number_integer()) {
        geofence.coordinateSystemType = geofenceObj.at("coordinateSystemType");
    }
}

void GeofenceRequest::ConvertTransitionEventInfo(std::shared_ptr<GeofenceRequest>& request,
    const nlohmann::json &jsonObject)
{
    if (jsonObject.find("transitionStatusList") != jsonObject.cend() &&
        jsonObject.at("transitionStatusList").is_array()) {
        for (const auto& elem : jsonObject.at("transitionStatusList")) {
            if (elem.is_number_integer()) {
                request->transitionStatusList_.emplace_back(elem.get<GeofenceTransitionEvent>());
            }
        }
    }
}

void GeofenceRequest::ConvertNotificationInfo(std::shared_ptr<GeofenceRequest>& request,
    const nlohmann::json &jsonObject)
{
#ifdef NOTIFICATION_ENABLE
    if (jsonObject.find("notificationRequestList") != jsonObject.cend()) {
        auto notificationArray = jsonObject.at("notificationRequestList");
        for (auto &notificationJson : notificationArray) {
            OHOS::Notification::NotificationRequest *notification =
                OHOS::Notification::NotificationRequest::FromJson(notificationJson);
            if (notification == nullptr) {
                continue;
            }
            auto notificationPtr = std::make_shared<OHOS::Notification::NotificationRequest>(*notification);
            if (notificationPtr != nullptr) {
                request->SetNotificationRequest(notificationPtr);
            }
            delete notification;
        }
    }
#endif
}

void GeofenceRequest::ConvertWantAgentInfo(std::shared_ptr<GeofenceRequest>& request,
    const nlohmann::json &jsonObject)
{
    if (jsonObject.find("wantAgent") != jsonObject.cend() && jsonObject.at("wantAgent").is_string()) {
        auto wantAgentValue  = jsonObject.at("wantAgent").get<std::string>();
        auto wantAgent = AbilityRuntime::WantAgent::WantAgentHelper::FromString(wantAgentValue);
        if (wantAgent != nullptr) {
            AbilityRuntime::WantAgent::WantAgentConstant::OperationType operationType =
                AbilityRuntime::WantAgent::WantAgentHelper::GetType(wantAgent);
            std::shared_ptr<AAFwk::Want> want = OHOS::AbilityRuntime::WantAgent::WantAgentHelper::GetWant(wantAgent);
            std::vector<std::shared_ptr<AAFwk::Want>> wants;
            wants.push_back(want);
            std::vector<OHOS::AbilityRuntime::WantAgent::WantAgentConstant::Flags> flags;
            flags.push_back(OHOS::AbilityRuntime::WantAgent::WantAgentConstant::Flags::UPDATE_PRESENT_FLAG);
            AbilityRuntime::WantAgent::WantAgentInfo wantAgentInfo(
                0, operationType, flags, wants, nullptr);
            request->wantAgent_ = AbilityRuntime::WantAgent::WantAgentHelper::GetWantAgent(wantAgentInfo);
        }
    }
}

std::shared_ptr<GeofenceRequest> GeofenceRequest::FromJson(const nlohmann::json &jsonObject)
{
    if (jsonObject.is_null() || !jsonObject.is_object()) {
        LBSLOGE(LOCATOR, "Invalid JSON object");
        return nullptr;
    }
    auto request = std::make_shared<GeofenceRequest>();
    if (request == nullptr) {
        LBSLOGE(LOCATOR, "Failed to create request instance");
        return nullptr;
    }
    ConvertTransitionEventInfo(request, jsonObject);
    ConvertNotificationInfo(request, jsonObject);
    ConvertWantAgentInfo(request, jsonObject);
    if (jsonObject.find("geofence") != jsonObject.cend()) {
        auto geofenceObj = jsonObject.at("geofence");
        if (!geofenceObj.is_null()) {
            ConvertGeoFenceInfo(geofenceObj, request->geofence_);
        }
    }
    if (jsonObject.find("scenario") != jsonObject.cend() && jsonObject.at("scenario").is_number()) {
        request->scenario_ = jsonObject.at("scenario").get<int>();
    }
    if (jsonObject.find("fenceId") != jsonObject.cend() && jsonObject.at("fenceId").is_number()) {
        request->fenceId_ = jsonObject.at("fenceId").get<int>();
    }
    if (jsonObject.find("uid") != jsonObject.cend() && jsonObject.at("uid").is_number()) {
        request->uid_ = jsonObject.at("uid").get<int>();
    }
    if (jsonObject.find("bundleName") != jsonObject.cend() && jsonObject.at("bundleName").is_string()) {
        request->bundleName_ = jsonObject.at("bundleName").get<std::string>();
    }
    if (jsonObject.find("appAliveStatus") != jsonObject.cend() && jsonObject.at("appAliveStatus").is_boolean()) {
        request->appAliveStatus_ = jsonObject.at("appAliveStatus").get<bool>();
    }
    if (jsonObject.find("requestExpirationTimeStamp") != jsonObject.cend() &&
        jsonObject.at("requestExpirationTimeStamp").is_string()) {
        request->requestExpirationTimeStamp_ = jsonObject.at("requestExpirationTimeStamp").get<int64_t>();
    }
    return request;
}
} // namespace Location
} // namespace OHOS