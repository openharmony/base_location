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

#ifndef OHOS_LOCATOR_PROXY_H
#define OHOS_LOCATOR_PROXY_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

#include "constant_definition.h"
#include "i_cached_locations_callback.h"
#include "i_locator_callback.h"
#include "location.h"
#include "request_config.h"

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
        REPORT_LOCATION = 6,
        REPORT_LOCATION_STATUS = 7,
        REPORT_ERROR_STATUS = 8,
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
        REMOVE_FENCE = 28
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
    virtual int StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid) = 0;
    virtual int StopLocating(sptr<ILocatorCallback>& callback) = 0;
    virtual int ReportLocation(const std::unique_ptr<Location>& location, std::string abilityName) = 0;
    virtual int ReportLocationStatus(sptr<ILocatorCallback>& callback, int result) = 0;
    virtual int ReportErrorStatus(sptr<ILocatorCallback>& callback, int result) = 0;
    virtual int GetCacheLocation(MessageParcel &data, MessageParcel &replay) = 0;
    virtual int IsGeoConvertAvailable(MessageParcel &data, MessageParcel &replay) = 0;
    virtual int GetAddressByCoordinate(MessageParcel &data, MessageParcel &replay) = 0;
    virtual int GetAddressByLocationName(MessageParcel &data, MessageParcel &replay) = 0;
    virtual bool IsLocationPrivacyConfirmed(const LocationPrivacyType type) = 0;
    virtual void SetLocationPrivacyConfirmStatus(const LocationPrivacyType type, bool isConfirmed) = 0;

    virtual int RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName) = 0;
    virtual int UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) = 0;

    virtual int GetCachedGnssLocationsSize() = 0;
    virtual void FlushCachedGnssLocations() = 0;
    virtual void SendCommand(std::unique_ptr<LocationCommand>& commands) = 0;
    virtual void AddFence(std::unique_ptr<GeofenceRequest>& request) = 0;
    virtual void RemoveFence(std::unique_ptr<GeofenceRequest>& request) = 0;
};

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
    int StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid) override;
    int StopLocating(sptr<ILocatorCallback>& callback) override;
    int ReportLocation(const std::unique_ptr<Location>& location, std::string abilityName) override;
    int ReportLocationStatus(sptr<ILocatorCallback>& callback, int result) override;
    int ReportErrorStatus(sptr<ILocatorCallback>& callback, int result) override;
    int GetCacheLocation(MessageParcel &data, MessageParcel &replay) override;
    int IsGeoConvertAvailable(MessageParcel &data, MessageParcel &replay) override;
    int GetAddressByCoordinate(MessageParcel &data, MessageParcel &replay) override;
    int GetAddressByLocationName(MessageParcel &data, MessageParcel &replay) override;
    bool IsLocationPrivacyConfirmed(const LocationPrivacyType type) override;
    void SetLocationPrivacyConfirmStatus(const LocationPrivacyType type, bool isConfirmed) override;

    int RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName) override;
    int UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) override;

    int GetCachedGnssLocationsSize() override;
    void FlushCachedGnssLocations() override;
    void SendCommand(std::unique_ptr<LocationCommand>& commands) override;

    void AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
private:
    static inline BrokerDelegator<LocatorProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATOR_PROXY_H