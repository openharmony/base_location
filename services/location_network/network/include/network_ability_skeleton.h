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

#ifndef NETWORK_ABILITY_SKELETON_H
#define NETWORK_ABILITY_SKELETON_H
#ifdef FEATURE_NETWORK_SUPPORT
#include <map>
#include "message_parcel.h"
#include "message_option.h"
#include "iremote_stub.h"

#include "subability_common.h"
#include "app_identity.h"

namespace OHOS {
namespace Location {
class INetworkAbility : public ISubAbility {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"location.INetworkAbility");
    virtual LocationErrCode SelfRequest(bool state) = 0;
};

class NetworkAbilityStub : public IRemoteStub<INetworkAbility> {
public:
    using NetworkMsgHandle = std::function<int(MessageParcel &, MessageParcel &, AppIdentity &)>;
    using NetworkMsgHandleMap = std::map<int, NetworkMsgHandle>;
    NetworkAbilityStub();
    virtual ~NetworkAbilityStub() = default;
    void InitNetworkMsgHandleMap();
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) = 0;
    virtual bool CancelIdleState() = 0;
    virtual void UnloadNetworkSystemAbility() = 0;
private:
    int SendLocationRequestInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int SetMockLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int SelfRequestInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int SetEnableInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int EnableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int DisableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    bool CheckLocationSwitchState(MessageParcel &reply);
private:
    bool isMessageRequest_ = false;
    NetworkMsgHandleMap NetworkMsgHandleMap_;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_NETWORK_SUPPORT
#endif // NETWORK_ABILITY_SKELETON_H
