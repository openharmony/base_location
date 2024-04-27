/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "native_location_callback_host.h"

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"

#include "location.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
int NativeLocationCallbackHost::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
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
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void NativeLocationCallbackHost::OnLocationReport(const std::unique_ptr<Location>& location)
{
    LBSLOGD(LOCATOR_CALLBACK, "NativeLocationCallbackHost::OnLocationReport");
    LocationCallbackIfaces locationCallback = GetCallback();
    if (locationCallback.locationUpdate != nullptr) {
        locationCallback.locationUpdate(location);
    }
}

void NativeLocationCallbackHost::OnLocatingStatusChange(const int status)
{
}

void NativeLocationCallbackHost::OnErrorReport(const int errorCode)
{
}

void NativeLocationCallbackHost::OnNetWorkErrorReport(const int errorCode)
{
}
} // namespace Location
} // namespace OHOS
