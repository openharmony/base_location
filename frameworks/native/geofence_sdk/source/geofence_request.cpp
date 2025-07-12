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

namespace OHOS {
namespace Location {
GeofenceRequest::GeofenceRequest()
{
    callback_ = nullptr;
    scenario_ = -1;
    fenceId_ = -1;
    uid_ = 0;
    appAliveStatus_ = true;
}

GeofenceRequest::GeofenceRequest(GeofenceRequest& geofenceRequest)
{
    this->SetGeofence(geofenceRequest.GetGeofence());
    this->SetScenario(geofenceRequest.GetScenario());
    Parcel agentParcelData;
    geofenceRequest.GetWantAgentParcelData(agentParcelData);
    this->SetWantAgentParcelData(agentParcelData);
    this->SetGeofenceTransitionEventList(geofenceRequest.GetGeofenceTransitionEventList());
#ifdef NOTIFICATION_ENABLE
    this->SetNotificationRequestList(geofenceRequest.GetNotificationRequestList());
#endif
    this->SetGeofenceTransitionCallback(geofenceRequest.GetGeofenceTransitionCallback());
    this->SetFenceId(geofenceRequest.GetFenceId());
    this->SetBundleName(geofenceRequest.GetBundleName());
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

void GeofenceRequest::SetWantAgentParcelData(const Parcel& data)
{
    std::vector<char>().swap(wantAgentBuffer_);
    char *first = reinterpret_cast<char*>(data.GetData());
    char *last = first + data.GetDataSize();
    wantAgentBuffer_.assign(first, last);
}

bool GeofenceRequest::GetWantAgentParcelData(Parcel& data)
{
    if (wantAgentBuffer_.empty()) {
        return false;
    }
    void* tempBuffer = malloc(wantAgentBuffer_.size());
    if (tempBuffer == NULL) {
        return false;
    }
    errno_t ret = memcpy_s(tempBuffer, wantAgentBuffer_.size(), wantAgentBuffer_.data(), wantAgentBuffer_.size());
    if (ret != EOK) {
        LBSLOGE(LOCATOR, "memcpy_s failed, error code:%{public}d", ret);
        return false;
    }
    return data.ParseFrom(reinterpret_cast<uintptr_t>(tempBuffer), wantAgentBuffer_.size());
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

int64_t GeofenceRequest::GetRequestExpirationTime()
{
    return requestExpirationTime_;
}

void GeofenceRequest::SetRequestExpirationTime(int64_t requestExpirationTime)
{
    requestExpirationTime_ = requestExpirationTime;
}

void GeofenceRequest::SetAppAliveStatus(bool appAliveStatus)
{
    appAliveStatus_ = appAliveStatus;
}

void GeofenceRequest::ReadFromParcel(Parcel& data)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    scenario_ = data.ReadInt32();
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
    uint32_t size = data.ReadUint32();
    const uint8_t *bufferPtr = data.ReadBuffer(size);
    std::vector<char>().swap(wantAgentBuffer_);
    if (bufferPtr != nullptr) {
        wantAgentBuffer_.assign(bufferPtr, bufferPtr + size);
        bufferPtr = nullptr;
    }
}

bool GeofenceRequest::Marshalling(Parcel& parcel) const
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    parcel.WriteInt32(scenario_);
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
        notificationRequestList_[i]->Marshalling(parcel);
    }
#endif
    parcel.WriteRemoteObject(callback_);
    parcel.WriteString(bundleName_);
    parcel.WriteInt32(uid_);
    parcel.WriteUint32(wantAgentBuffer_.size());
    parcel.WriteBuffer(wantAgentBuffer_.data(), wantAgentBuffer_.size());
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
} // namespace Location
} // namespace OHOS