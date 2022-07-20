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

bool NetworkAbilityProxy::EnableMock(const LocationMockConfig& config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "EnableLocationMock remote is null");
        return false;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return false;
    }
    config.Marshalling(data);
    int error = remote->SendRequest(ISubAbility::ENABLE_LOCATION_MOCK, data, reply, option);
    LBSLOGD(GNSS, "Proxy::EnableLocationMock Transact ErrCode = %{public}d", error);
    bool result = false;
    if (error == NO_ERROR) {
        result = reply.ReadBool();
    }
    return result;
}

bool NetworkAbilityProxy::DisableMock(const LocationMockConfig& config)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "DisableLocationMock remote is null");
        return false;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return false;
    }
    config.Marshalling(data);
    int error = remote->SendRequest(ISubAbility::DISABLE_LOCATION_MOCK, data, reply, option);
    LBSLOGD(GNSS, "Proxy::DisableLocationMock Transact ErrCode = %{public}d", error);
    bool result = false;
    if (error == NO_ERROR) {
        result = reply.ReadBool();
    }
    return result;
}

bool NetworkAbilityProxy::SetMocked(
    const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GNSS, "SetMockedLocations remote is null");
        return false;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GNSS, "write interfaceToken fail!");
        return false;
    }
    config.Marshalling(data);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    int error = remote->SendRequest(ISubAbility::SET_MOCKED_LOCATIONS, data, reply, option);
    LBSLOGD(GNSS, "Proxy::SetMockedLocations Transact ErrCode = %{public}d", error);
    bool result = false;
    if (error == NO_ERROR) {
        result = reply.ReadBool();
    }
    return result;
}
} // namespace Location
} // namespace OHOS