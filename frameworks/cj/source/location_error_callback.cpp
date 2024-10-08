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
#include "location_error_callback.h"

namespace OHOS {
namespace GeoLocationManager {
LocationErrorCallback::LocationErrorCallback()
{
}

LocationErrorCallback::LocationErrorCallback(int64_t callbackId)
{
    this->callbackId_ = callbackId;
    auto cFunc = reinterpret_cast<void(*)(int32_t errorCode)>(callbackId);
    callback_ = [ lambda = CJLambda::Create(cFunc)](const int errorCode) ->
        void { lambda(errorCode); };
}

LocationErrorCallback::~LocationErrorCallback()
{
}

int LocationErrorCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGI(Location::LOCATION_ERR_CALLBACK, "LocatorCallbackHost::OnRemoteRequest! code = %{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(Location::LOCATION_ERR_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_ERROR_INFO_EVENT: {
            OnErrorReport(data.ReadInt32());
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocationErrorCallback::OnLocationReport(const std::unique_ptr<Location::Location>& location)
{
}

void LocationErrorCallback::OnLocatingStatusChange(const int status)
{
}

void LocationErrorCallback::OnErrorReport(const int errorCode)
{
    LBSLOGI(Location::LOCATION_ERR_CALLBACK, "LocatorCallback::OnRemoteRequest! errorCode = %{public}d", errorCode);
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_ != nullptr) {
        callback_(errorCode);
    }
}
}
}
