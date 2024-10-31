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

#include "location_log.h"
#include "cj_lambda.h"
#include "gnss_status_callback.h"

namespace OHOS {
namespace GeoLocationManager {
GnssStatusCallback::GnssStatusCallback()
{
    remoteDied_ = false;
}

GnssStatusCallback::GnssStatusCallback(int64_t callbackId)
{
    remoteDied_ = false;
    this->callbackId_ = callbackId;
    auto cFunc = reinterpret_cast<void(*)(CJSatelliteStatusInfo info)>(callbackId);
    callback_ = [ lambda = CJLambda::Create(cFunc)](const std::unique_ptr<Location::SatelliteStatus>& statusInfo) ->
        void { lambda(SatelliteStatusInfoToCJSatelliteStatus(statusInfo)); };
}

GnssStatusCallback::~GnssStatusCallback()
{
}

int GnssStatusCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(Location::GNSS_STATUS_CALLBACK, "GnssStatusCallback::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(Location::GNSS_STATUS_CALLBACK, "invalid token.");
        return -1;
    }
    if (remoteDied_) {
        LBSLOGD(Location::GNSS_STATUS_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }

    switch (code) {
        case Location::IGnssStatusCallback::RECEIVE_STATUS_INFO_EVENT: {
            std::unique_ptr<Location::SatelliteStatus> statusInfo = Location::SatelliteStatus::Unmarshalling(data);
            OnStatusChange(statusInfo);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool GnssStatusCallback::IsRemoteDied()
{
    return remoteDied_;
}

void GnssStatusCallback::OnStatusChange(const std::unique_ptr<Location::SatelliteStatus>& statusInfo)
{
    LBSLOGD(Location::GNSS_STATUS_CALLBACK, "GnssStatusCallback::OnStatusChange");
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_ != nullptr) {
        callback_(statusInfo);
    }
}
}
}