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

#include "network_ability_proxy.h"
#include "ipc_skeleton.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
NetworkAbilityProxy::NetworkAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INetworkAbility>(impl)
{
}

void NetworkAbilityProxy::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(NETWORK, "write interfaceToken fail!");
        return;
    }
    data.WriteInt64(interval);
    workrecord.Marshalling(data);
    int error = Remote()->SendRequest(ISubAbility::SEND_LOCATION_REQUEST, data, reply, option);
    LBSLOGD(NETWORK, "RemoteRequest Transact ErrCode = %{public}d", error);
}

std::unique_ptr<Location> NetworkAbilityProxy::GetCachedLocation()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(NETWORK, "write interfaceToken fail!");
        return nullptr;
    }
    int error = Remote()->SendRequest(ISubAbility::GET_CACHED_LOCATION, data, reply, option);
    std::unique_ptr<Location> location = Location::Unmarshalling(reply);
    LBSLOGD(NETWORK, "GetCache Transact ErrCode = %{public}d", error);
    return location;
}

void NetworkAbilityProxy::SetEnable(bool state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(NETWORK, "write interfaceToken fail!");
        return;
    }
    data.WriteBool(state);

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(ISubAbility::SET_ENABLE, data, reply, option);
    LBSLOGD(NETWORK, "Enable Transact ErrCode = %{public}d", error);
}

void NetworkAbilityProxy::SelfRequest(bool state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(NETWORK, "write interfaceToken fail!");
        return;
    }
    data.WriteBool(state);
    int error = Remote()->SendRequest(SELF_REQUEST, data, reply, option);
    LBSLOGD(NETWORK, "Proxy::SelfRequest Transact ErrCodes = %{public}d", error);
}
} // namespace Location
} // namespace OHOS