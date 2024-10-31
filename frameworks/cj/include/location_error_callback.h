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

#ifndef LOCATION_ERROR_CALLBACK_H
#define LOCATION_ERROR_CALLBACK_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "location_error_callback_napi.h"
#include "geolocationmanager_utils.h"

namespace OHOS {
namespace GeoLocationManager {
class LocationErrorCallback : public IRemoteStub<Location::ILocatorCallback> {
public:
    LocationErrorCallback();
    LocationErrorCallback(int64_t callbackId);
    virtual ~LocationErrorCallback();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnLocationReport(const std::unique_ptr<Location::Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;

    int64_t GetCallBackId()
    {
        return callbackId_;
    };
private:
    int64_t callbackId_ = -1;
    std::function<void(const int errorCode)> callback_ = nullptr;
    std::mutex mutex_;
};
}
}

#endif // LOCATION_ERROR_CALLBACK_H