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
#include "iremote_object.h"
#include "constant_definition.h"
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "i_locator_callback.h"
#include "location.h"
#include "request_config.h"
#include "napi/native_api.h"
#include "location_mock_config.h"
#include "country_code.h"

namespace OHOS {
namespace Location {
class Locator {
public:
    static std::unique_ptr<Locator> GetInstance();

    virtual ~Locator();

    virtual bool IsLocationEnabled() = 0;

    virtual void RequestEnableLocation() = 0;

    virtual void EnableAbility(bool enable) = 0;

    virtual void StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback) = 0;

    virtual void StopLocating(sptr<ILocatorCallback>& callback) = 0;

    virtual std::unique_ptr<Location> GetCachedLocation() = 0;

    virtual bool RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual bool UnregisterSwitchCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual void ShowNotification() = 0;

    virtual void RequestPermission() = 0;

    virtual bool IsGeoServiceAvailable() = 0;

    virtual void GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList) = 0;

    virtual void GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList) = 0;

    virtual bool IsLocationPrivacyConfirmed(const int type) = 0;

    virtual void SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) = 0;

    virtual bool RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual bool UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual bool RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual bool UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual bool RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;

    virtual bool UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback) = 0;

    virtual void RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) = 0;

    virtual void UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) = 0;

    virtual int GetCachedGnssLocationsSize() = 0;

    virtual int FlushCachedGnssLocations() = 0;

    virtual bool SendCommand(std::unique_ptr<LocationCommand>& commands) = 0;

    virtual bool AddFence(std::unique_ptr<GeofenceRequest>& request) = 0;

    virtual bool RemoveFence(std::unique_ptr<GeofenceRequest>& request) = 0;

    virtual bool EnableReverseGeocodingMock() = 0;

    virtual bool DisableReverseGeocodingMock() = 0;

    virtual bool SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo) = 0;

    virtual std::shared_ptr<CountryCode> GetIsoCountryCode() = 0;

    virtual bool EnableLocationMock(const LocationMockConfig& config) = 0;

    virtual bool DisableLocationMock(const LocationMockConfig& config) = 0;

    virtual bool SetMockedLocations(
        const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location) = 0;

    virtual bool ProxyUidForFreeze(int32_t uid, bool isProxy) = 0;
    virtual bool ResetAllProxy() = 0;
};
} // namespace Location
} // namespace OHOS

#endif // LOCATOR_H
