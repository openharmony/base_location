/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "network_ability_skeleton.h"

#include "ipc_skeleton.h"

#include "common_utils.h"

namespace OHOS {
namespace Location {
NetworkAbilityProxy::NetworkAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetworkAbility>(impl)
{
    SetProxy(NETWORK_ABILITY, AsObject());
}

void NetworkAbilityProxy::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    SendRequest(interval, workrecord);
}

std::unique_ptr<Location> NetworkAbilityProxy::GetCachedLocation()
{
    return GetCache();
}

void NetworkAbilityProxy::SetEnable(bool state)
{
    Enable(state);
}

void NetworkAbilityProxy::SelfRequest(bool state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteBool(state);
    int error = Remote()->SendRequest(SELF_REQUEST, data, reply, option);
    LBSLOGD(NETWORK, "Proxy::SelfRequest Transact ErrCodes = %{public}d", error);
}

int NetworkAbilityStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t lastCallingPid = IPCSkeleton::GetCallingPid();
    pid_t lastCallinguid = IPCSkeleton::GetCallingUid();
    LBSLOGI(NETWORK, "OnRemoteRequest cmd = %{public}d, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), lastCallingPid, lastCallinguid);
    if (lastCallinguid > SYSTEM_UID) {
        LBSLOGE(NETWORK, "this remote request is not allowed");
        return EXCEPTION;
    }

    int ret = 0;
    switch (code) {
        case SEND_LOCATION_REQUEST: {
            int64_t interval = data.ReadInt64();
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            SendLocationRequest((uint64_t)interval, *workrecord);
            break;
        }
        case GET_CACHED_LOCATION: {
            std::unique_ptr<Location> location = GetCachedLocation();
            if (location != nullptr) {
                location->Marshalling(reply);
                LBSLOGD(NETWORK, "result: %{private}s", location->ToString().c_str());
            }
            break;
        }
        case SET_ENABLE: {
            SetEnable(data.ReadBool());
            break;
        }
        case SELF_REQUEST: {
            SelfRequest(data.ReadBool());
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}
} // namespace Location
} // namespace OHOS