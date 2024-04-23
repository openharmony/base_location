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
#include <map>
#include "message_option.h"
#include "message_parcel.h"
#include "iremote_object.h"
#include "iremote_stub.h"

#include "constant_definition.h"
#include "subability_common.h"

#include "app_identity.h"
#include "geofence_request.h"

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
    virtual LocationErrCode AddFence(std::shared_ptr<GeofenceRequest>& request) = 0;
    virtual LocationErrCode RemoveFence(std::shared_ptr<GeofenceRequest>& request) = 0;
    virtual LocationErrCode AddGnssGeofence(
        std::shared_ptr<GeofenceRequest>& request, const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode RemoveGnssGeofence(
        std::shared_ptr<GeofenceRequest>& request) = 0;
};

class GnssAbilityStub : public IRemoteStub<IGnssAbility> {
public:
    using GnssMsgHandle = int (GnssAbilityStub::*)(
        MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    using GnssMsgHandleMap = std::map<int, GnssMsgHandle>;
    GnssAbilityStub();
    virtual ~GnssAbilityStub() = default;
    void InitGnssMsgHandleMap();
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) = 0;
    virtual void UnloadGnssSystemAbility() = 0;
private:
    int SendLocationRequestInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int SetMockLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int SetEnableInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int RefreshRequirementsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int RegisterGnssStatusCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int UnregisterGnssStatusCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int RegisterNmeaMessageCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int UnregisterNmeaMessageCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int RegisterCachedCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int UnregisterCachedCallbackInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int GetCachedGnssLocationsSizeInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int FlushCachedGnssLocationsInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int SendCommandInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int EnableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int DisableMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int AddFenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int RemoveFenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int AddGnssGeofenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int RemoveGnssGeofenceInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
private:
    bool isMessageRequest_ = false;
    GnssMsgHandleMap GnssMsgHandleMap_;
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
