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

#include "gnss_status_callback_host.h"

#include "location_log.h"

namespace OHOS {
namespace Location {
int GnssStatusCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(GNSS_STATUS_CALLBACK, "GnssStatusCallbackHost::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_STATUS_INFO_EVENT: {
            std::unique_ptr<SatelliteStatus> statusInfo = SatelliteStatus::Unmarshalling(data);
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

void GnssStatusCallbackHost::OnStatusChange(const std::unique_ptr<SatelliteStatus>& statusInfo)
{
}
}  // namespace Location
}  // namespace OHOS
