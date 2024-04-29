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

#ifndef GNSS_GEOFENCE_REQUEST_H
#define GNSS_GEOFENCE_REQUEST_H

#include <mutex>
#include <vector>
#include "geofence_definition.h"
#include "notification_request.h"
#include "want_agent_helper.h"
#include <parcel.h>

namespace OHOS {
namespace Location {
typedef struct {
    double latitude;
    double longitude;
    double radius;
    double expiration;
    CoordinateSystemType coordinateSystemType;
} GeoFence;

class GeofenceRequest : public Parcelable {
public:
    GeofenceRequest()
    {
        callback_ = nullptr;
        scenario_ = -1;
        fenceId_ = -1;
    }

    GeofenceRequest(GeofenceRequest& geofenceRequest)
    {
        this->SetGeofence(geofenceRequest.GetGeofence());
        this->SetScenario(geofenceRequest.GetScenario());
        this->SetWantAgent(geofenceRequest.GetWantAgent());
        this->SetGeofenceTransitionEventList(geofenceRequest.GetGeofenceTransitionEventList());
        this->SetNotificationRequestList(geofenceRequest.GetNotificationRequestList());
        this->SetGeofenceTransitionCallback(geofenceRequest.GetGeofenceTransitionCallback());
        this->SetFenceId(geofenceRequest.GetFenceId());
        this->SetBundleName(geofenceRequest.GetBundleName());
    }

    ~GeofenceRequest() {}

    inline GeoFence GetGeofence()
    {
        return geofence_;
    }

    inline void SetGeofence(GeoFence geofence)
    {
        geofence_ = geofence;
    }

    inline int GetScenario()
    {
        return scenario_;
    }

    inline void SetScenario(int scenario)
    {
        scenario_ = scenario;
    }

    inline void SetWantAgent(const AbilityRuntime::WantAgent::WantAgent wantAgent)
    {
        wantAgent_ = wantAgent;
    }

    inline AbilityRuntime::WantAgent::WantAgent GetWantAgent()
    {
        return wantAgent_;
    }

    inline std::vector<GeofenceTransitionEvent> GetGeofenceTransitionEventList()
    {
        std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
        return transitionStatusList_;
    }

    inline void SetGeofenceTransitionEvent(GeofenceTransitionEvent status)
    {
        std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
        transitionStatusList_.push_back(status);
    }

    inline void SetGeofenceTransitionEventList(std::vector<GeofenceTransitionEvent> statusList)
    {
        std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
        for (auto it = statusList.begin(); it != statusList.end(); ++it) {
            transitionStatusList_.push_back(*it);
        }
    }

    inline std::vector<Notification::NotificationRequest> GetNotificationRequestList()
    {
        std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
        return notificationRequestList_;
    }

    inline void SetNotificationRequest(Notification::NotificationRequest request)
    {
        std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
        notificationRequestList_.push_back(request);
    }

    inline void SetNotificationRequestList(std::vector<Notification::NotificationRequest> requestList)
    {
        std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
        for (auto it = requestList.begin(); it != requestList.end(); ++it) {
            notificationRequestList_.push_back(*it);
        }
    }

    inline void SetGeofenceTransitionCallback(const sptr<IRemoteObject>& callback)
    {
        callback_ = callback;
    }

    inline sptr<IRemoteObject> GetGeofenceTransitionCallback()
    {
        return callback_;
    }

    inline int GetFenceId()
    {
        return fenceId_;
    }

    inline void SetFenceId(int fenceId)
    {
        fenceId_ = fenceId;
    }

    inline std::string GetBundleName()
    {
        return bundleName_;
    }

    inline void SetBundleName(std::string bundleName)
    {
        bundleName_ = bundleName;
    }

    void ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static std::shared_ptr<GeofenceRequest> Unmarshalling(Parcel& parcel);
private:
    std::vector<GeofenceTransitionEvent> transitionStatusList_;
    std::vector<Notification::NotificationRequest> notificationRequestList_;
    sptr<IRemoteObject> callback_ = nullptr;
    GeoFence geofence_;
    int scenario_;
    int fenceId_;
    AbilityRuntime::WantAgent::WantAgent wantAgent_;
    std::string bundleName_;
    mutable std::mutex geofenceRequestMutex_;
};
} // namespace Location
} // namespace OHOS
#endif // GNSS_GEOFENCE_REQUEST_H
