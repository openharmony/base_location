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

#include "location_gnss_geofence_callback_proxy.h"
#include "message_option.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
LocationGnssGeofenceCallbackProxy::LocationGnssGeofenceCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGnssGeofenceCallback>(impl)
{
}

void LocationGnssGeofenceCallbackProxy::OnTransitionStatusChange(GeofenceTransition transition)
{
    MessageParcel dataParcel;
    MessageParcel reply;
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataParcel.WriteInt32(transition.fenceId);
    dataParcel.WriteInt32(static_cast<int>(transition.event));
    if (transition.beaconFence != nullptr) {
        BeaconFence beacon = *transition.beaconFence;
        dataParcel.WriteParcelable(&beacon);
    }
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(RECEIVE_TRANSITION_STATUS_EVENT, dataParcel, reply, option);
    LBSLOGD(LOCATION_GNSS_GEOFENCE_CALLBACK, "OnTransitionStatusChange Transact ErrCode = %{public}d", error);
}

void LocationGnssGeofenceCallbackProxy::OnReportOperationResult(int fenceId, int type, int result)
{
    MessageParcel dataParcel;
    MessageParcel reply;
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        return;
    }
    dataParcel.WriteInt32(fenceId);
    dataParcel.WriteInt32(type);
    dataParcel.WriteInt32(result);
    MessageOption option = { MessageOption::TF_ASYNC };
    int error = Remote()->SendRequest(REPORT_OPERATION_RESULT_EVENT, dataParcel, reply, option);
    LBSLOGD(LOCATION_GNSS_GEOFENCE_CALLBACK, "OnReportOperationResult Transact ErrCode = %{public}d", error);
}
} // namespace Location
} // namespace OHOS
