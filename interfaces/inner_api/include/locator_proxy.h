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

#ifndef LOCATOR_PROXY_H
#define LOCATOR_PROXY_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "constant_definition.h"
#include "i_cached_locations_callback.h"
#include "i_locator.h"
#include "i_locator_callback.h"
#include "location.h"
#include "request_config.h"
#include "satellite_status.h"
#include "country_code.h"

namespace OHOS {
namespace Location {
class LocatorProxy : public IRemoteProxy<ILocator> {
public:
    explicit LocatorProxy(const sptr<IRemoteObject> &impl);
    ~LocatorProxy() = default;
    void UpdateSaAbility() override;
    int GetSwitchState() override;
    void EnableAbility(bool isEnabled) override;
    void RegisterSwitchCallback(const sptr<IRemoteObject> &callback, pid_t uid) override;
    void UnregisterSwitchCallback(const sptr<IRemoteObject> &callback) override;
    void RegisterGnssStatusCallback(const sptr<IRemoteObject> &callback, pid_t uid) override;
    void UnregisterGnssStatusCallback(const sptr<IRemoteObject> &callback) override;
    void RegisterNmeaMessageCallback(const sptr<IRemoteObject> &callback, pid_t uid) override;
    void UnregisterNmeaMessageCallback(const sptr<IRemoteObject> &callback) override;
    void RegisterCountryCodeCallback(const sptr<IRemoteObject> &callback, pid_t uid) override;
    void UnregisterCountryCodeCallback(const sptr<IRemoteObject> &callback) override;
    int StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid) override;
    int StopLocating(sptr<ILocatorCallback>& callback) override;
    int GetCacheLocation(MessageParcel &data, MessageParcel &replay) override;
    int IsGeoConvertAvailable(MessageParcel &data, MessageParcel &replay) override;
    int GetAddressByCoordinate(MessageParcel &data, MessageParcel &replay) override;
    int GetAddressByLocationName(MessageParcel &data, MessageParcel &replay) override;
    bool IsLocationPrivacyConfirmed(const int type) override;
    void SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) override;

    int RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName) override;
    int UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) override;

    int GetCachedGnssLocationsSize() override;
    int FlushCachedGnssLocations() override;
    void SendCommand(std::unique_ptr<LocationCommand>& commands) override;

    void AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    std::shared_ptr<CountryCode> GetIsoCountryCode() override;
    bool EnableLocationMock(const LocationMockConfig& config) override;
    bool DisableLocationMock(const LocationMockConfig& config) override;
    bool SetMockedLocations(
        const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location) override;

private:
    static inline BrokerDelegator<LocatorProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_PROXY_H
