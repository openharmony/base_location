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
#ifndef LOCATOR_H
#define LOCATOR_H

#include <list>
#include <vector>

#include "iremote_object.h"

#include "constant_definition.h"
#include "country_code.h"
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "i_locator_callback.h"
#include "location.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
class Locator {
public:
    static std::unique_ptr<Locator> GetInstance();

    virtual ~Locator();

    virtual LocationErrCode IsLocationEnabled(int &state) = 0;

    virtual LocationErrCode RequestEnableLocation() = 0;

    virtual LocationErrCode EnableAbility(bool enable) = 0;

    virtual LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback) = 0;

    virtual LocationErrCode StopLocating(sptr<ILocatorCallback>& callback) = 0;

    virtual LocationErrCode GetCachedLocation(std::unique_ptr<Location> &loc) = 0;

    virtual LocationErrCode RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual LocationErrCode UnregisterSwitchCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual LocationErrCode ShowNotification() = 0;

    virtual LocationErrCode RequestPermission() = 0;

    virtual LocationErrCode IsGeoServiceAvailable(bool &isAvailable) = 0;

    virtual LocationErrCode GetAddressByCoordinate(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList) = 0;

    virtual LocationErrCode GetAddressByLocationName(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList) = 0;

    virtual LocationErrCode IsLocationPrivacyConfirmed(const int type, bool &isConfirmed) = 0;

    virtual LocationErrCode SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) = 0;

    virtual LocationErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual LocationErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual LocationErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual LocationErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual LocationErrCode RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) = 0;

    virtual LocationErrCode UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) = 0;

    virtual LocationErrCode RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual LocationErrCode UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual LocationErrCode RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) = 0;

    virtual LocationErrCode UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) = 0;

    virtual LocationErrCode GetCachedGnssLocationsSize(int &size) = 0;

    virtual LocationErrCode FlushCachedGnssLocations() = 0;

    virtual LocationErrCode SendCommand(std::unique_ptr<LocationCommand>& commands) = 0;

    virtual LocationErrCode AddFence(std::unique_ptr<GeofenceRequest>& request) = 0;

    virtual LocationErrCode RemoveFence(std::unique_ptr<GeofenceRequest>& request) = 0;

    virtual LocationErrCode EnableReverseGeocodingMock() = 0;

    virtual LocationErrCode DisableReverseGeocodingMock() = 0;

    virtual LocationErrCode SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo) = 0;

    virtual LocationErrCode GetIsoCountryCode(std::shared_ptr<CountryCode>& countryCode) = 0;

    virtual LocationErrCode EnableLocationMock() = 0;

    virtual LocationErrCode DisableLocationMock() = 0;

    virtual LocationErrCode SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) = 0;

    virtual LocationErrCode ProxyUidForFreeze(int32_t uid, bool isProxy) = 0;
    virtual LocationErrCode ResetAllProxy() = 0;
};
} // namespace Location
} // namespace OHOS

#endif // LOCATOR_H
