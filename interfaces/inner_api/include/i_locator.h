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

#ifndef I_LOCATOR_H
#define I_LOCATOR_H

#include "iremote_broker.h"
#include "ipc_types.h"

#include "constant_definition.h"
#include "i_cached_locations_callback.h"
#include "i_locator_callback.h"
#include "location.h"
#include "request_config.h"
#include "satellite_status.h"
#include "location_mock_config.h"

namespace OHOS {
namespace Location {
class ILocator : public IRemoteBroker {
public:
    enum {
        GET_SWITCH_STATE = 1,
        REG_SWITCH_CALLBACK = 2,
        START_LOCATING = 3,
        STOP_LOCATING = 4,
        GET_CACHE_LOCATION = 5,
        ENABLE_ABILITY = 9,
        UPDATE_SA_ABILITY = 10,
        GEO_IS_AVAILABLE = 11,
        GET_FROM_COORDINATE = 12,
        GET_FROM_LOCATION_NAME = 13,
        UNREG_SWITCH_CALLBACK = 15,
        REG_GNSS_STATUS_CALLBACK = 16,
        UNREG_GNSS_STATUS_CALLBACK = 17,
        REG_NMEA_CALLBACK = 18,
        UNREG_NMEA_CALLBACK = 19,
        IS_PRIVACY_COMFIRMED = 20,
        SET_PRIVACY_COMFIRM_STATUS = 21,
        REG_CACHED_CALLBACK = 22,
        UNREG_CACHED_CALLBACK = 23,
        GET_CACHED_LOCATION_SIZE = 24,
        FLUSH_CACHED_LOCATIONS = 25,
        SEND_COMMAND = 26,
        ADD_FENCE = 27,
        REMOVE_FENCE = 28,
        GET_ISO_COUNTRY_CODE = 29,
        ENABLE_LOCATION_MOCK = 30,
        DISABLE_LOCATION_MOCK = 31,
        SET_MOCKED_LOCATIONS = 32,
        REG_COUNTRY_CODE_CALLBACK = 33,
        UNREG_COUNTRY_CODE_CALLBACK = 34,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"location.ILocator");
    virtual void UpdateSaAbility() = 0;
    virtual int GetSwitchState() = 0;
    virtual void EnableAbility(bool isEnabled) = 0;
    virtual void RegisterSwitchCallback(const sptr<IRemoteObject> &callback, pid_t uid) = 0;
    virtual void UnregisterSwitchCallback(const sptr<IRemoteObject> &callback) = 0;
    virtual void RegisterGnssStatusCallback(const sptr<IRemoteObject> &callback, pid_t uid) = 0;
    virtual void UnregisterGnssStatusCallback(const sptr<IRemoteObject> &callback) = 0;
    virtual void RegisterNmeaMessageCallback(const sptr<IRemoteObject> &callback, pid_t uid) = 0;
    virtual void UnregisterNmeaMessageCallback(const sptr<IRemoteObject> &callback) = 0;
    virtual void RegisterCountryCodeCallback(const sptr<IRemoteObject> &callback) = 0;
    virtual void UnregisterCountryCodeCallback(const sptr<IRemoteObject> &callback) = 0;
    virtual int StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid) = 0;
    virtual int StopLocating(sptr<ILocatorCallback>& callback) = 0;
    virtual int GetCacheLocation(MessageParcel &data, MessageParcel &replay) = 0;
    virtual int IsGeoConvertAvailable(MessageParcel &data, MessageParcel &replay) = 0;
    virtual int GetAddressByCoordinate(MessageParcel &data, MessageParcel &replay) = 0;
    virtual int GetAddressByLocationName(MessageParcel &data, MessageParcel &replay) = 0;
    virtual bool IsLocationPrivacyConfirmed(const int type) = 0;
    virtual void SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) = 0;

    virtual int RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName) = 0;
    virtual int UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) = 0;

    virtual int GetCachedGnssLocationsSize() = 0;
    virtual int FlushCachedGnssLocations() = 0;
    virtual void SendCommand(std::unique_ptr<LocationCommand>& commands) = 0;
    virtual void AddFence(std::unique_ptr<GeofenceRequest>& request) = 0;
    virtual void RemoveFence(std::unique_ptr<GeofenceRequest>& request) = 0;
    virtual std::shared_ptr<CountryCode> GetIsoCountryCode() = 0;
    virtual bool EnableLocationMock(const LocationMockConfig& config) = 0;
    virtual bool DisableLocationMock(const LocationMockConfig& config) = 0;
    virtual bool SetMockedLocations(
        const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // I_LOCATOR_H
