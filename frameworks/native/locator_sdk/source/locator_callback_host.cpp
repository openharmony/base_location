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

#include "locator_callback_host.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
int LocatorCallbackHost::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(LOCATOR_CALLBACK, "LocatorCallbackHost::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_LOCATION_INFO_EVENT: {
            std::unique_ptr<Location> location = Location::Unmarshalling(data);
            OnLocationReport(location);
            break;
        }
        case RECEIVE_LOCATION_STATUS_EVENT: {
            int status = data.ReadInt32();
            OnLocatingStatusChange(status);
            break;
        }
        case RECEIVE_ERROR_INFO_EVENT: {
            int errorCode = data.ReadInt32();
            LBSLOGI(LOCATOR_STANDARD, "CallbackSutb receive ERROR_EVENT. errorCode:%{public}d", errorCode);
            OnErrorReport(errorCode);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}
void LocatorCallbackHost::OnLocationReport(const std::unique_ptr<Location>& location)
{
}

void LocatorCallbackHost::OnLocatingStatusChange(const int status)
{
}

void LocatorCallbackHost::OnErrorReport(const int errorCode)
{
}
} // namespace Location
} // namespace OHOS
