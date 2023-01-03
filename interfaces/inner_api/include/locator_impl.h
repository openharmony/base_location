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
#include "locator.h"
#include "locator_proxy.h"

namespace OHOS {
namespace Location {
class LocatorImpl : public Locator {
public:
    explicit LocatorImpl();
    ~LocatorImpl() override;
    bool Init();
    LocationErrCode IsLocationEnabled(int &state) override;
    LocationErrCode ShowNotification() override;
    LocationErrCode RequestPermission() override;
    LocationErrCode RequestEnableLocation() override;
    LocationErrCode EnableAbility(bool enable) override;
    LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback) override;
    LocationErrCode StopLocating(sptr<ILocatorCallback>& callback) override;
    LocationErrCode GetCachedLocation(std::unique_ptr<Location> &loc) override;
    LocationErrCode RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterSwitchCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) override;
    LocationErrCode UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) override;
    LocationErrCode IsGeoServiceAvailable(bool &isAvailable) override;
    LocationErrCode GetAddressByCoordinate(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    LocationErrCode GetAddressByLocationName(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList) override;
    LocationErrCode IsLocationPrivacyConfirmed(const int type, bool &isConfirmed) override;
    LocationErrCode SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) override;
    LocationErrCode GetCachedGnssLocationsSize(int &size) override;
    LocationErrCode FlushCachedGnssLocations() override;
    LocationErrCode SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    LocationErrCode AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    LocationErrCode RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    LocationErrCode GetIsoCountryCode(std::shared_ptr<CountryCode>& countryCode) override;
    LocationErrCode EnableLocationMock() override;
    LocationErrCode DisableLocationMock() override;
    LocationErrCode SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;

    LocationErrCode EnableReverseGeocodingMock() override;

    LocationErrCode DisableReverseGeocodingMock() override;

    LocationErrCode SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo) override;

    LocationErrCode ProxyUidForFreeze(int32_t uid, bool isProxy) override;
    LocationErrCode ResetAllProxy() override;

private:
    std::unique_ptr<LocatorProxy> client_;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATOR_IMPL_H
