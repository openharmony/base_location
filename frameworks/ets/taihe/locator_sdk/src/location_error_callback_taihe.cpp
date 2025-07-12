/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "location_error_callback_taihe.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
LocationErrorCallbackTaihe::LocationErrorCallbackTaihe()
{
}

LocationErrorCallbackTaihe::~LocationErrorCallbackTaihe()
{
}

int LocationErrorCallbackTaihe::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGI(LOCATION_ERR_CALLBACK, "LocatorCallbackHost::OnRemoteRequest! code = %{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATION_ERR_CALLBACK, "invalid token.");
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

void LocationErrorCallbackTaihe::OnLocationReport(const std::unique_ptr<Location>& location)
{
}

void LocationErrorCallbackTaihe::OnLocatingStatusChange(const int status)
{
}

void LocationErrorCallbackTaihe::OnErrorReport(const int errorCode)
{
    LBSLOGI(LOCATION_ERR_CALLBACK, "OnErrorReport code = %{public}d", errorCode);
    if (callback_) {
        (*callback_)(static_cast<::ohos::geoLocationManager::LocationError::key_t>(errorCode));
    }
}
}  // namespace Location
}  // namespace OHOS
