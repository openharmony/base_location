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

#ifndef GNSS_STATUS_CALLBACK_H
#define GNSS_STATUS_CALLBACK_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "gnss_status_callback_napi.h"
#include "geolocationmanager_utils.h"

namespace OHOS {
namespace GeoLocationManager {
class GnssStatusCallback : public IRemoteStub<Location::IGnssStatusCallback> {
public:
    GnssStatusCallback();
    GnssStatusCallback(int64_t callbackId);
    virtual ~GnssStatusCallback();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool IsRemoteDied();
    bool Send(std::unique_ptr<Location::SatelliteStatus>& statusInfo);
    void OnStatusChange(const std::unique_ptr<Location::SatelliteStatus>& statusInfo) override;
    void DeleteHandler();
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);

    inline bool GetRemoteDied() const
    {
        return remoteDied_;
    }

    inline void SetRemoteDied(const bool remoteDied)
    {
        remoteDied_ = remoteDied;
    }

    int64_t GetCallBackId()
    {
        return callbackId_;
    };

private:
    int64_t callbackId_ = -1;
    std::function<void(const std::unique_ptr<Location::SatelliteStatus>& statusInfo)> callback_ = nullptr;
    std::mutex mutex_;
    bool remoteDied_;
};
}
}

#endif // GNSS_STATUS_CALLBACK_H