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

#ifndef PASSIVE_ABILITY_SKELETON_H
#define PASSIVE_ABILITY_SKELETON_H
#ifdef FEATURE_PASSIVE_SUPPORT

#include "iremote_stub.h"

#include "message_option.h"
#include "message_parcel.h"

#include "subability_common.h"

namespace OHOS {
namespace Location {
class IPassiveAbility : public ISubAbility {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"location.IPassiveAbility");
};

class PassiveAbilityStub : public IRemoteStub<IPassiveAbility> {
public:
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) = 0;
    virtual void UnloadPassiveSystemAbility() = 0;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_PASSIVE_SUPPORT
#endif // PASSIVE_ABILITY_SKELETON_H
