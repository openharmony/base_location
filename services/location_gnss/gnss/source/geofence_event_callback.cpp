/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "geofence_event_callback.h"
#include "location_log.h"
#include "gnss_ability.h"
#include "reminder_helper.h"
#include "ipc_skeleton.h"
#include "notification_constant.h"
#include "notification_request.h"
#include "notification_normal_content.h"
#include "notification_content.h"
#include "notification_helper.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
int32_t GeofenceEventCallback::ReportGeofenceAvailability(bool isAvailable)
{
    return 0;
}

int32_t GeofenceEventCallback::ReportGeofenceEvent(
    int32_t fenceIndex, const LocationInfo& location, GeofenceEvent event, int64_t timestamp)
{
    LBSLOGD(GNSS, "ReportGeofenceEvent enter");
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "gnssAbility is nullptr");
        return -1;
    }
    auto request = gnssAbility->GetGeofenceRequestByFenceId(fenceIndex);
    if (request == nullptr) {
        LBSLOGE(GNSS, "request is nullptr");
        return -1;
    }
    auto callback = request->GetGeofenceTransitionCallback();
    if (callback == nullptr) {
        LBSLOGE(GNSS, "callback is nullptr");
        return -1;
    }
    sptr<IGnssGeofenceCallback> gnssGeofenceCallback = iface_cast<IGnssGeofenceCallback>(callback);
    auto transitionStatusList = request->GetGeofenceTransitionEventList();
    auto notificationRequestList = request->GetNotificationRequestList();
    if (transitionStatusList.size() != notificationRequestList.size()) {
        LBSLOGE(GNSS, "transitionStatusList size does not equals to notificationRequestList size");
        return -1;
    }
    for (int i = 0; i < transitionStatusList.size(); i++) {
        if (static_cast<int>(transitionStatusList[i]) != static_cast<int>(event)) {
            continue;
        }
        GeofenceTransition geofenceTransition;
        geofenceTransition.fenceId = fenceIndex;
        geofenceTransition.event = transitionStatusList[i];
        gnssGeofenceCallback->OnTransitionStatusChange(geofenceTransition);
        auto notificationRequest = notificationRequestList[i];
        if (notificationRequest != nullptr) {
            notificationRequest->SetCreatorUid(IPCSkeleton::GetCallingUid());
            Notification::NotificationHelper::PublishNotification(*notificationRequest);
        }
    }
    return 0;
}

int32_t GeofenceEventCallback::ReportGeofenceOperateResult(
    int32_t fenceIndex, GeofenceOperateType type, GeofenceOperateResult result)
{
    LBSLOGD(GNSS, "ReportGeofenceOperateResult enter");
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "gnssAbility is nullptr");
        return -1;
    }
    if (type == TYPE_ADD) {
        auto geofenceRequest = gnssAbility->GetGeofenceRequestByFenceId(fenceIndex);
        auto callback = gnssAbility->GetGnssGeofenceCallbackByFenceId(fenceIndex);
        if (callback == nullptr || geofenceRequest == nullptr) {
            LBSLOGE(GNSS, "callback is nullptr");
            return -1;
        }
        sptr<IGnssGeofenceCallback> gnssGeofenceCallback = iface_cast<IGnssGeofenceCallback>(callback);
        if (result == GEOFENCE_OPERATION_SUCCESS) {
            gnssGeofenceCallback->OnFenceIdChange(fenceIndex);
        } else {
            // 上报错误码
            LBSLOGE(GNSS, "fence id:%{public}d, result:%{public}d", fenceIndex, result);
        }
    }
    return 0;
}
}  // namespace Location
}  // namespace OHOS
