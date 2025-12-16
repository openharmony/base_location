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

#include "location_switch_callback_taihe.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
int LocationSwitchCallbackTaihe::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(SWITCH_CALLBACK, "LocationSwitchCallbackTaihe::OnRemoteRequest! code %{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(SWITCH_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_SWITCH_STATE_EVENT: {
            OnSwitchChange(data.ReadInt32());
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocationSwitchCallbackTaihe::OnSwitchChange(int switchState)
{
    if (callback_) {
        (*callback_)(switchState == 0? false: true);
    }
}
}  // namespace Location
}  // namespace OHOS
