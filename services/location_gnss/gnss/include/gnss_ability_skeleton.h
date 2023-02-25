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

#ifndef GNSS_ABILITY_SKELETON_H
#define GNSS_ABILITY_SKELETON_H
#ifdef FEATURE_GNSS_SUPPORT

#include "message_option.h"
#include "message_parcel.h"
#include "iremote_object.h"
#include "iremote_stub.h"

#include "constant_definition.h"
#include "subability_common.h"

namespace OHOS {
namespace Location {
class IGnssAbility : public ISubAbility {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"location.IGnssAbility");
    virtual LocationErrCode RefrashRequirements() = 0;
    virtual LocationErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual LocationErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual LocationErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode UnregisterCachedCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual LocationErrCode GetCachedGnssLocationsSize(int &size) = 0;
    virtual LocationErrCode FlushCachedGnssLocations() = 0;
    virtual LocationErrCode SendCommand(std::unique_ptr<LocationCommand>& commands) = 0;
    virtual LocationErrCode AddFence(std::unique_ptr<GeofenceRequest>& request) = 0;
    virtual LocationErrCode RemoveFence(std::unique_ptr<GeofenceRequest>& request) = 0;
};

class GnssAbilityStub : public IRemoteStub<IGnssAbility> {
public:
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) = 0;
    virtual void UnloadGnssSystemAbility() = 0;
};

class GnssStatusCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    GnssStatusCallbackDeathRecipient();
    ~GnssStatusCallbackDeathRecipient() override;
};

class NmeaCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    NmeaCallbackDeathRecipient();
    ~NmeaCallbackDeathRecipient() override;
};

class CachedLocationCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    CachedLocationCallbackDeathRecipient();
    ~CachedLocationCallbackDeathRecipient() override;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
#endif // GNSS_ABILITY_SKELETON_H
