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

#ifdef FEATURE_NETWORK_SUPPORT
#include "network_callback_host.h"

#include "common_utils.h"
#include "location_log.h"
#include "network_ability.h"

#include "location_log_event_ids.h"
#include "common_hisysevent.h"
#include <sys/time.h>

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
        case RECEIVE_LOCATION_INFO_EVENT_V9: {
            std::unique_ptr<Location> location = Location::UnmarshallingV9(data);
            OnLocationReport(location);
            break;
        }
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
    std::shared_ptr<Location> locationNew = std::make_shared<Location>(*location);
    DelayedSingleton<NetworkAbility>::GetInstance().get()->ReportLocationInfo(NETWORK_ABILITY, locationNew);
    DelayedSingleton<NetworkAbility>::GetInstance().get()->ReportLocationInfo(PASSIVE_ABILITY, locationNew);
    WriteLocationInnerEvent(NETWORK_CALLBACK_LOCATION, {"speed", std::to_string(location->GetSpeed()),
        "accuracy", std::to_string(location->GetAccuracy()),
        "locationTimestamp", std::to_string(location->GetTimeStamp() / MILLI_PER_SEC),
        "receiveTimestamp", std::to_string(CommonUtils::GetCurrentTimeStamp())});
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
#endif // FEATURE_NETWORK_SUPPORT