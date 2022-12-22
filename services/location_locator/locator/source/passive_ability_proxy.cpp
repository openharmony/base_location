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

#include "passive_ability_proxy.h"

#include "message_parcel.h"
#include "message_option.h"

#include "location_log.h"
#include "subability_common.h"

namespace OHOS {
namespace Location {
PassiveAbilityProxy::PassiveAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IPassiveAbility>(impl)
{
}

void PassiveAbilityProxy::SendLocationRequest(WorkRecord &workrecord)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return;
    }
    workrecord.Marshalling(data);
    int error = Remote()->SendRequest(ISubAbility::SEND_LOCATION_REQUEST, data, reply, option);
    LBSLOGD(PASSIVE, "RemoteRequest Transact ErrCode = %{public}d", error);
}

void PassiveAbilityProxy::SetEnable(bool state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return;
    }
    data.WriteBool(state);

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(ISubAbility::SET_ENABLE, data, reply, option);
    LBSLOGD(PASSIVE, "Enable Transact ErrCode = %{public}d", error);
}

bool PassiveAbilityProxy::EnableMock()
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
    int error = remote->SendRequest(ISubAbility::ENABLE_LOCATION_MOCK, data, reply, option);
    LBSLOGD(GNSS, "Proxy::EnableLocationMock Transact ErrCode = %{public}d", error);
    bool result = false;
    if (error == NO_ERROR) {
        result = reply.ReadBool();
    }
    return result;
}

bool PassiveAbilityProxy::DisableMock()
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
    int error = remote->SendRequest(ISubAbility::DISABLE_LOCATION_MOCK, data, reply, option);
    LBSLOGD(GNSS, "Proxy::DisableLocationMock Transact ErrCode = %{public}d", error);
    bool result = false;
    if (error == NO_ERROR) {
        result = reply.ReadBool();
    }
    return result;
}

bool PassiveAbilityProxy::SetMocked(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
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
    data.WriteInt32(timeInterval);
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