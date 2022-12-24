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

#ifndef GNSS_ABILITY_PROXY_H
#define GNSS_ABILITY_PROXY_H

#include <vector>

#include "iremote_object.h"
#include "iremote_proxy.h"

#include "gnss_ability_skeleton.h"

namespace OHOS {
namespace Location {
class GnssAbilityProxy : public IRemoteProxy<IGnssAbility> {
public:
    explicit GnssAbilityProxy(const sptr<IRemoteObject> &impl);
    ~GnssAbilityProxy() = default;
    void SendLocationRequest(WorkRecord &workrecord) override;
    void SetEnable(bool state) override;
    void RefrashRequirements() override;
    void RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback) override;
    void UnregisterCachedCallback(const sptr<IRemoteObject>& callback) override;
    int GetCachedGnssLocationsSize() override;
    int FlushCachedGnssLocations() override;
    void SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    void AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    bool EnableMock() override;
    bool DisableMock() override;
    bool SetMocked(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
private:
    static inline BrokerDelegator<GnssAbilityProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // GNSS_ABILITY_PROXY_H
