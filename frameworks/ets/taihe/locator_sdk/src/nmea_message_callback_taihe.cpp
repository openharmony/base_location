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
#include "nmea_message_callback_taihe.h"

#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
NmeaMessageCallbackTaihe::NmeaMessageCallbackTaihe()
{
}

NmeaMessageCallbackTaihe::~NmeaMessageCallbackTaihe()
{
}

int NmeaMessageCallbackTaihe::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(NMEA_MESSAGE_CALLBACK, "NmeaMessageCallbackTaihe::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(NMEA_MESSAGE_CALLBACK, "invalid token.");
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

void NmeaMessageCallbackTaihe::OnMessageChange(int64_t timestamp, const std::string msg)
{
    LBSLOGD(NMEA_MESSAGE_CALLBACK, "NmeaMessageCallbackTaihe::OnMessageChange");
    if (callback_) {
        (*callback_)(msg);
    }
}
}  // namespace Location
}  // namespace OHOS
