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

#ifdef FEATURE_PASSIVE_SUPPORT
#include "passive_ability_proxy.h"

#include "message_parcel.h"
#include "message_option.h"

#include "location_log.h"
#include "subability_common.h"
#include "locationhub_ipc_interface_code.h"

namespace OHOS {
namespace Location {
PassiveAbilityProxy::PassiveAbilityProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IPassiveAbility>(impl)
{
}

LocationErrCode PassiveAbilityProxy::SendLocationRequest(WorkRecord &workrecord)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    workrecord.Marshalling(data);
    int error = Remote()->SendRequest(static_cast<uint32_t>(PassiveInterfaceCode::SEND_LOCATION_REQUEST),
                                      data,
                                      reply,
                                      option);
    LBSLOGD(PASSIVE, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode PassiveAbilityProxy::SetEnable(bool state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteBool(state);

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(static_cast<uint32_t>(PassiveInterfaceCode::SET_ENABLE), data, reply, option);
    LBSLOGD(PASSIVE, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode PassiveAbilityProxy::EnableMock()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(PASSIVE, "EnableLocationMock remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error =
        remote->SendRequest(static_cast<uint32_t>(PassiveInterfaceCode::ENABLE_LOCATION_MOCK), data, reply, option);
    LBSLOGD(PASSIVE, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode PassiveAbilityProxy::DisableMock()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(PASSIVE, "DisableLocationMock remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error =
        remote->SendRequest(static_cast<uint32_t>(PassiveInterfaceCode::DISABLE_LOCATION_MOCK), data, reply, option);
    LBSLOGD(PASSIVE, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode PassiveAbilityProxy::SetMocked(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(PASSIVE, "SetMockedLocations remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(PASSIVE, "write interfaceToken fail!");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(timeInterval);
    int locationSize = static_cast<int>(location.size());
    data.WriteInt32(locationSize);
    for (int i = 0; i < locationSize; i++) {
        location.at(i)->Marshalling(data);
    }
    int error =
        remote->SendRequest(static_cast<uint32_t>(PassiveInterfaceCode::SET_MOCKED_LOCATIONS), data, reply, option);
    LBSLOGD(PASSIVE, "%{public}s Transact Error = %{public}d", __func__, error);
    return LocationErrCode(reply.ReadInt32());
}
} // namespace Location
} // namespace OHOS
#endif
