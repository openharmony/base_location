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

#include "native_nmea_callback_host.h"

#include "ipc_skeleton.h"
#include "ipc_object_stub.h"

#include "location_log.h"

namespace OHOS {
namespace Location {
int NativeNmeaCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGI(NMEA_MESSAGE_CALLBACK, "invalid token");
        return -1;
    }

    switch (code) {
        case RECEIVE_NMEA_MESSAGE_EVENT: {
            int64_t timestamp = data.ReadInt64();
            std::string msg = Str16ToStr8(data.ReadString16());
            OnMessageChange(timestamp, msg);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void NativeNmeaCallbackHost::OnMessageChange(int64_t timestamp, const std::string msg)
{
    LBSLOGD(NMEA_MESSAGE_CALLBACK, "NativeNmeaCallbackHost::OnMessageChange");
    GnssNmeaCallbackIfaces nmeaCallback = GetCallback();
    nmeaCallback.nmeaUpdate(timestamp, msg);
}
}  // namespace Location
}  // namespace OHOS
