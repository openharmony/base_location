/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "network_callback_host.h"
#include "location_log.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {

int NetworkCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(NETWORK, "invalid token.");
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

void NetworkCallbackHost::OnLocationReport(const std::unique_ptr<Location>& location)
{
    LBSLOGD(NETWORK, "NetworkCallbackHost::OnLocationReport");
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocation(location, NETWORK_ABILITY);
}

void NetworkCallbackHost::OnLocatingStatusChange(const int status)
{
    LBSLOGD(NETWORK, "NetworkCallbackHost::OnLocatingStatusChange!");
}

void NetworkCallbackHost::OnErrorReport(const int errorCode)
{
    LBSLOGD(NETWORK, "NetworkCallbackHost::OnErrorReport!");
}
} // namespace Location
} // namespace OHOS