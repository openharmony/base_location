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

#include <vector>

#include "iremote_object.h"

#include "constant_definition.h"
#include "country_code.h"
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "i_locator.h"
#include "locator.h"
#include "locator_proxy.h"

namespace OHOS {
namespace Location {
class LocatorImpl : public Locator {
public:
    explicit LocatorImpl();
    ~LocatorImpl() override;
    bool Init();
    bool IsLocationEnabled() override;
    void ShowNotification() override;
    void RequestPermission() override;
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
    bool RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    bool UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) override;
    void UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) override;
    bool IsGeoServiceAvailable() override;
    void GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    void GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    bool IsLocationPrivacyConfirmed(const int type) override;
    int SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) override;
    int GetCachedGnssLocationsSize() override;
    int FlushCachedGnssLocations() override;
    bool SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    bool AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    bool RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    std::shared_ptr<CountryCode> GetIsoCountryCode() override;
    bool EnableLocationMock() override;
    bool DisableLocationMock() override;
    bool SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
    bool EnableReverseGeocodingMock() override;
    bool DisableReverseGeocodingMock() override;
    bool SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo) override;
    bool ProxyUidForFreeze(int32_t uid, bool isProxy) override;
    bool ResetAllProxy() override;

    LocationErrCode IsLocationEnabledV9(bool &isEnabled) override;
    LocationErrCode EnableAbilityV9(bool enable) override;
    LocationErrCode StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback) override;
    LocationErrCode StopLocatingV9(sptr<ILocatorCallback>& callback) override;
    LocationErrCode GetCachedLocationV9(std::unique_ptr<Location> &loc) override;
    LocationErrCode RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode UnregisterSwitchCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode UnregisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) override;
    LocationErrCode UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback) override;
    LocationErrCode IsGeoServiceAvailableV9(bool &isAvailable) override;
    LocationErrCode GetAddressByCoordinateV9(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    LocationErrCode GetAddressByLocationNameV9(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    LocationErrCode IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed) override;
    LocationErrCode SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed) override;
    LocationErrCode GetCachedGnssLocationsSizeV9(int &size) override;
    LocationErrCode FlushCachedGnssLocationsV9() override;
    LocationErrCode SendCommandV9(std::unique_ptr<LocationCommand>& commands) override;
    LocationErrCode AddFenceV9(std::unique_ptr<GeofenceRequest>& request) override;
    LocationErrCode RemoveFenceV9(std::unique_ptr<GeofenceRequest>& request) override;
    LocationErrCode GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode) override;
    LocationErrCode EnableLocationMockV9() override;
    LocationErrCode DisableLocationMockV9() override;
    LocationErrCode SetMockedLocationsV9(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
    LocationErrCode EnableReverseGeocodingMockV9() override;
    LocationErrCode DisableReverseGeocodingMockV9() override;
    LocationErrCode SetReverseGeocodingMockInfoV9(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo) override;
    LocationErrCode ProxyUidForFreezeV9(int32_t uid, bool isProxy) override;
    LocationErrCode ResetAllProxyV9() override;
    void ResetLocatorProxy(const wptr<IRemoteObject> &remote);

private:
    class LocatorDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit LocatorDeathRecipient(LocatorImpl &impl) : impl_(impl) {}
        ~LocatorDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            impl_.ResetLocatorProxy(remote);
        }
    private:
        LocatorImpl &impl_;
    };

private:
    sptr<LocatorProxy> GetProxy();

    sptr<LocatorProxy> client_ { nullptr };
    sptr<IRemoteObject::DeathRecipient> recipient_ { nullptr };
    bool state_ = false;
    std::mutex mutex_;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATOR_IMPL_H
