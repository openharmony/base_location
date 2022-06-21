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
#include "ipc_skeleton.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
PassiveAbilityProxy::PassiveAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IPassiveAbility>(impl)
{
}

void PassiveAbilityProxy::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return;
    }
    data.WriteInt64(interval);
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
} // namespace Location
} // namespace OHOS