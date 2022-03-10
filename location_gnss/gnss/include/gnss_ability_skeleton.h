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

#ifndef OHOS_LOCATION_GNSS_ABILITY_SKELETON_H
#define OHOS_LOCATION_GNSS_ABILITY_SKELETON_H

#include "constant_definition.h"
#include "subability_common.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Location {
class IGnssAbility : public ISubAbility {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"location.IGnssAbility");
    virtual void RemoteRequest(bool state) = 0;
    virtual void RefrashRequirements() = 0;
    virtual void RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual void UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual void RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual void UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual void RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback) = 0;
    virtual void UnregisterCachedCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual int GetCachedGnssLocationsSize() = 0;
    virtual void FlushCachedGnssLocations() = 0;
    virtual void SendCommand(std::unique_ptr<LocationCommand>& commands) = 0;
    virtual void AddFence(std::unique_ptr<GeofenceRequest>& request) = 0;
    virtual void RemoveFence(std::unique_ptr<GeofenceRequest>& request) = 0;
};

class GnssAbilityProxy : public IRemoteProxy<IGnssAbility>, public SubAbilityProxy {
public:
    explicit GnssAbilityProxy(const sptr<IRemoteObject> &impl);
    ~GnssAbilityProxy() = default;
    void SendLocationRequest(uint64_t interval, WorkRecord &workrecord) override;
    std::unique_ptr<Location> GetCachedLocation() override;
    void SetEnable(bool state) override;
    void RemoteRequest(bool state) override;
    void RefrashRequirements() override;
    void RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback) override;
    void UnregisterCachedCallback(const sptr<IRemoteObject>& callback) override;

    int GetCachedGnssLocationsSize() override;
    void FlushCachedGnssLocations() override;
    void SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    void AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
};

class GnssAbilityStub : public IRemoteStub<IGnssAbility> {
public:
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    static inline BrokerDelegator<GnssAbilityProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_GNSS_ABILITY_SKELETON_H