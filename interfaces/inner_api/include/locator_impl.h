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
#ifndef LOCATOR_IMPL_H
#define LOCATOR_IMPL_H

#include "constant_definition.h"
#include "i_cached_locations_callback.h"
#include "geo_coding_mock_info.h"
#include "locator.h"
#include "locator_proxy.h"
#include "geo_address.h"

namespace OHOS {
namespace Location {
class LocatorImpl : public Locator {
public:
    explicit LocatorImpl();
    ~LocatorImpl();
    bool Init();
    bool IsLocationEnabled() override;
    void ShowNotification() override;
    void RequestPermission(napi_env env) override;
    void RequestEnableLocation() override;
    void EnableAbility(bool enable) override;
    void StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback) override;
    void StopLocating(sptr<ILocatorCallback>& callback) override;
    std::unique_ptr<Location> GetCachedLocation() override;
    bool RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    bool UnregisterSwitchCallback(const sptr<IRemoteObject>& callback) override;
    bool RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    bool UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    bool RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    bool UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) override;
    void UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) override;
    bool IsGeoServiceAvailable() override;
    void GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    void GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    bool IsLocationPrivacyConfirmed(const int type) override;
    void SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) override;
    int GetCachedGnssLocationsSize() override;
    int FlushCachedGnssLocations() override;
    bool SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    bool AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    bool RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    int GetIsoCountryCode(std::string& code) override;
    bool EnableLocationMock(const LocationMockConfig& config) override;
    bool DisableLocationMock(const LocationMockConfig& config) override;
    bool SetMockedLocations(
        const LocationMockConfig& config,      const std::vector<std::shared_ptr<Location>> &location) override;
    
    bool EnableReverseGeocodingMock() override;

    bool DisableReverseGeocodingMock() override;

    bool SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mokeInfo) override;
private:
    std::unique_ptr<LocatorProxy> client_;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATOR_IMPL_H
