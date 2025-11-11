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
#include "country_code_manager.h"
#include "geo_address.h"
#include "geocoding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "locator_proxy.h"
#include "i_locating_required_data_callback.h"
#include "locating_required_data_config.h"
#include "location_data_manager.h"
#include "request_config.h"
#include "system_ability_status_change_stub.h"
#include "locationhub_ipc_interface_code.h"
#include "ibluetooth_scan_result_callback.h"
#include "ilocator_service.h"
#include "beacon_fence_request.h"
#include "beacon_fence.h"

namespace OHOS {
namespace Location {
class ICallbackResumeManager {
public:
    virtual ~ICallbackResumeManager() = default;
    virtual void ResumeCallback() = 0;
};

class LocatorSystemAbilityListener : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
private:
    bool needResume_ = false;
    std::mutex mutex_;
};

class LocatorImpl {
public:
    static std::shared_ptr<LocatorImpl> GetInstance();
    explicit LocatorImpl();
    ~LocatorImpl();

    /**
     * @brief Obtain current location switch status.
     *
     * @return Returns true if the location switch on, returns false otherwise.
     */
    bool IsLocationEnabled();
    void ShowNotification();
    void RequestPermission();
    void RequestEnableLocation();

    /**
     * @brief Enable location switch.
     *
     * @param enable Status of the location switch to be set.
     */
    void EnableAbility(bool enable);

    /**
     * @brief Subscribe location changed.
     *
     * @param requestConfig Indicates the location request parameters.
     * @param callback Indicates the callback for reporting the location result.
     */
    void StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback);

    /**
     * @brief Unsubscribe location changed.
     *
     * @param callback Indicates the callback for reporting the location result.
     */
    void StopLocating(sptr<ILocatorCallback>& callback);

    /**
     * @brief Obtain last known location.
     *
     * @return The last known location information.
     */
    std::unique_ptr<Location> GetCachedLocation();

    /**
     * @brief Subscribe location switch changed.
     *
     * @param callback Indicates the callback for reporting the location switch status.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    bool RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid);

    /**
     * @brief Unsubscribe location switch changed.
     *
     * @param callback Indicates the callback for reporting the location switch status.
     * @return Return whether the deregistration is successful.
     */
    bool UnregisterSwitchCallback(const sptr<IRemoteObject>& callback);

    /**
     * @brief Subscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    bool RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid);

    /**
     * @brief Unsubscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @return Return whether the deregistration is successful.
     */
    bool UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback);

    /**
     * @brief Subscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    bool RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid);

    /**
     * @brief Unsubscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @return Return whether the deregistration is successful.
     */
    bool UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback);

    /**
     * @brief Registering the callback function for listening to country code changes.
     *
     * @param callback Indicates the callback for reporting country code changes.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    bool RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid);

    /**
     * @brief Unregistering the callback function for listening to country code changes.
     *
     * @param callback Indicates the callback for reporting country code changes.
     * @return Return whether the deregistration is successful.
     */
    bool UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback);

    /**
     * @brief Subscribe to cache GNSS locations update messages.
     *
     * @param request Indicates the cached GNSS locations request parameters.
     * @param callback Indicates the callback for reporting the cached GNSS locations.
     * @return Return whether the registration is successful.
     */
    void RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Unsubscribe to cache GNSS locations update messages.
     *
     * @param callback Indicates the callback for reporting the cached gnss locations.
     * @return Return whether the deregistration is successful.
     */
    void UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Obtain geocoding service status.
     *
     * @return Returns true if geocoding service is available, returns false otherwise.
     */
    bool IsGeoServiceAvailable();

    /**
     * @brief Obtain address info from location.
     *
     * @param data Indicates the reverse geocode query parameters.
     * @param replyList Indicates the result of the address info.
     */
    void GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Obtain latitude and longitude info from location address.
     *
     * @param data Indicates the geocode query parameters.
     * @param replyList Indicates the result of the address info.
     */
    void GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Querying location privacy protocol confirmation status.
     *
     * @param type Indicates location privacy protocol type.
     * @return Returns true if the location privacy protocol has been confirmed, returns false otherwise.
     */
    bool IsLocationPrivacyConfirmed(const int type);

    /**
     * @brief Set location privacy protocol confirmation status.
     *
     * @param type Indicates location privacy protocol type.
     * @param isConfirmed Indicates whether the location privacy protocol should be confirmed.
     * @return Returns 1 if the location privacy protocol has been set, returns 0 otherwise.
     */
    int SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed);

    /**
     * @brief Obtain the number of cached GNSS locations.
     *
     * @return Returns the result of the cached GNSS locations size.
     */
    int GetCachedGnssLocationsSize();

    /**
     * @brief All prepared GNSS locations are returned to the application through the callback function,
     * and the bottom-layer buffer is cleared.
     *
     * @return Returns 1 if the cached gnss location has been flushed, returns 0 otherwise.
     */
    int FlushCachedGnssLocations();

    /**
     * @brief Send extended commands to location subsystem.
     *
     * @param commands Indicates the extended command message body.
     * @return Returns true if the command has been sent successfully, returns false otherwise.
     */
    bool SendCommand(std::unique_ptr<LocationCommand>& commands);

    /**
     * @brief Obtain the current country code.
     *
     * @return Returns the result of the country code.
     */
    std::shared_ptr<CountryCode> GetIsoCountryCode();

    /**
     * @brief Enable the geographical location simulation function.
     *
     * @return Returns true if the mock location function has been enabled successfully, returns false otherwise.
     */
    bool EnableLocationMock();

    /**
     * @brief Disable the geographical location simulation function.
     *
     * @return Returns true if the mock location function has been disabled successfully, returns false otherwise.
     */
    bool DisableLocationMock();

    /**
     * @brief Set the configuration parameters for location simulation.
     *
     * @param timeInterval Indicates how often the simulated location is reported.
     * @param location Indicates the simulated location to be reported.
     * @return Returns true if the mock location config has been set successfully, returns false otherwise.
     */
    bool SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);

    /**
     * @brief Enable the reverse geocoding simulation function.
     *
     * @return Returns true if the mock reverse geocoding function has been enabled successfully,
     * returns false otherwise.
     */
    bool EnableReverseGeocodingMock();

    /**
     * @brief Disable the reverse geocoding simulation function.
     *
     * @return Returns true if the mock reverse geocoding function has been disabled successfully,
     * returns false otherwise.
     */
    bool DisableReverseGeocodingMock();

    /**
     * @brief Set the configuration parameters for simulating reverse geocoding.
     *
     * @param mockInfo Indicates the set of locations and place names to be simulated.
     * @return Returns true if the mock reverse geocoding config has been set successfully, returns false otherwise.
     */
    bool SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);

    /**
     * @brief Obtain current location switch status.
     *
     * @param isEnabled Indicates if the location switch on.
     * @return Returns ERRCODE_SUCCESS if obtain current location switch status succeed.
     */
    LocationErrCode IsLocationEnabledV9(bool &isEnabled);

    /**
     * @brief Obtain current location switch status.
     *
     * @param isEnabled Indicates if the location switch on.
     * @return Returns ERRCODE_SUCCESS if obtain current location switch status succeed.
     */
    LocationErrCode IsLocationEnabledForUser(bool &isEnabled, int32_t userId);

    /**
     * @brief Enable location switch.
     *
     * @param enable Status of the location switch to be set.
     * @return Returns ERRCODE_SUCCESS if enable location switch succeed.
     */
    LocationErrCode EnableAbilityV9(bool enable);

    /**
     * @brief Enable location switch.
     *
     * @param enable Status of the location switch to be set.
     * @param userId userId of the user.
     * @return Returns ERRCODE_SUCCESS if enable location switch succeed.
     */
    LocationErrCode EnableAbilityForUser(bool enable, int32_t userId);

    /**
     * @brief Subscribe location changed.
     *
     * @param requestConfig Indicates the location request parameters.
     * @param callback Indicates the callback for reporting the location result.
     * @return Returns ERRCODE_SUCCESS if subscribe location changed succeed.
     */
    LocationErrCode StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback);

    /**
     * @brief Unsubscribe location changed.
     *
     * @param callback Indicates the callback for reporting the location result.
     * @return Returns ERRCODE_SUCCESS if Unsubscribe location changed succeed.
     */
    LocationErrCode StopLocatingV9(sptr<ILocatorCallback>& callback);

    /**
     * @brief Obtain last known location.
     *
     * @param loc Indicates the last known location information.
     * @return Returns ERRCODE_SUCCESS if obtain last known location succeed.
     */
    LocationErrCode GetCachedLocationV9(std::unique_ptr<Location> &loc);

    /**
     * @brief Subscribe location switch changed.
     *
     * @param callback Indicates the callback for reporting the location switch status.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Unsubscribe location switch changed.
     *
     * @param callback Indicates the callback for reporting the location switch status.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterSwitchCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Subscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Unsubscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Subscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Unsubscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Registering the callback function for listening to country code changes.
     *
     * @param callback Indicates the callback for reporting country code changes.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Unregistering the callback function for listening to country code changes.
     *
     * @param callback Indicates the callback for reporting country code changes.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback);

    /**
     * @brief Subscribe to cache GNSS locations update messages.
     *
     * @param request Indicates the cached GNSS locations request parameters.
     * @param callback Indicates the callback for reporting the cached GNSS locations.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Unsubscribe to cache GNSS locations update messages.
     *
     * @param callback Indicates the callback for reporting the cached gnss locations.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Obtain geocoding service status.
     *
     * @param isAvailable Indicates if geocoding service is available
     * @return Return ERRCODE_SUCCESS if obtain geocoding service status is successful.
     */
    LocationErrCode IsGeoServiceAvailableV9(bool &isAvailable);

    /**
     * @brief Obtain address info from location.
     *
     * @param data Indicates the reverse geocode query parameters.
     * @param replyList Indicates the result of the address info.
     * @return Return ERRCODE_SUCCESS if obtain address info from location is successful.
     */
    LocationErrCode GetAddressByCoordinateV9(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Obtain latitude and longitude info from location address.
     *
     * @param data Indicates the geocode query parameters.
     * @param replyList Indicates the result of the address info.
     * @return Return ERRCODE_SUCCESS if obtain latitude and longitude info from location address is successful.
     */
    LocationErrCode GetAddressByLocationNameV9(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Querying location privacy protocol confirmation status.
     *
     * @param type Indicates location privacy protocol type.
     * @param isConfirmed Indicates if the location privacy protocol has been confirmed
     * @return Return ERRCODE_SUCCESS if querying location privacy protocol confirmation status is successful.
     */
    LocationErrCode IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed);

    /**
     * @brief Set location privacy protocol confirmation status.
     *
     * @param type Indicates location privacy protocol type.
     * @param isConfirmed Indicates whether the location privacy protocol should be confirmed.
     * @return Return ERRCODE_SUCCESS if set location privacy protocol confirmation status is successful.
     */
    LocationErrCode SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed);

    /**
     * @brief Obtain the number of cached GNSS locations.
     *
     * @param size Indicates the cached GNSS locations size
     * @return Return ERRCODE_SUCCESS if obtain the number of cached GNSS locations is successful.
     */
    LocationErrCode GetCachedGnssLocationsSizeV9(int &size);

    /**
     * @brief All prepared GNSS locations are returned to the application through the callback function,
     * and the bottom-layer buffer is cleared.
     *
     * @return Return ERRCODE_SUCCESS if flush cached gnss locations is successful.
     */
    LocationErrCode FlushCachedGnssLocationsV9();

    /**
     * @brief Send extended commands to location subsystem.
     *
     * @param commands Indicates the extended command message body.
     * @return Returns ERRCODE_SUCCESS if the command has been sent successfully.
     */
    LocationErrCode SendCommandV9(std::unique_ptr<LocationCommand>& commands);

    /**
     * @brief Obtain the current country code.
     *
     * @param countryCode the result of the country code
     * @return Returns ERRCODE_SUCCESS if obtain the current country code successfully.
     */
    LocationErrCode GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode);

    /**
     * @brief Enable the geographical location simulation function.
     *
     * @return Returns ERRCODE_SUCCESS if the mock location function has been enabled successfully.
     */
    LocationErrCode EnableLocationMockV9();

    /**
     * @brief Disable the geographical location simulation function.
     *
     * @return Returns ERRCODE_SUCCESS if the mock location function has been disabled successfully.
     */
    LocationErrCode DisableLocationMockV9();

    /**
     * @brief Set the configuration parameters for location simulation.
     *
     * @param timeInterval Indicates how often the simulated location is reported.
     * @param location Indicates the simulated location to be reported.
     * @return Returns ERRCODE_SUCCESS if the mock location config has been set successfully.
     */
    LocationErrCode SetMockedLocationsV9(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);

    /**
     * @brief Enable the reverse geocoding simulation function.
     *
     * @return Returns ERRCODE_SUCCESS if the mock reverse geocoding function has been enabled successfully.
     */
    LocationErrCode EnableReverseGeocodingMockV9();

    /**
     * @brief Disable the reverse geocoding simulation function.
     *
     * @return Returns ERRCODE_SUCCESS if the mock reverse geocoding function has been disabled successfully.
     */
    LocationErrCode DisableReverseGeocodingMockV9();

    /**
     * @brief Set the configuration parameters for simulating reverse geocoding.
     *
     * @param mockInfo Indicates the set of locations and place names to be simulated.
     * @return Returns ERRCODE_SUCCESS if the mock reverse geocoding config has been set successfully.
     */
    LocationErrCode SetReverseGeocodingMockInfoV9(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);

    /**
     * @brief Used to freeze locating process with specified uid.
     *
     * @param pidList Indicates the calling pid.
     * @param isProxy Indicates if the locating process should be freezed.
     * @return Returns ERRCODE_SUCCESS if the process has been frozen successfully.
     */
    LocationErrCode ProxyForFreeze(std::set<int> pidList, bool isProxy);

    /**
     * @brief Used to reset the frozen status of all location processes.
     *
     * @return Returns ERRCODE_SUCCESS if the frozen status of process has been reset successfully.
     */
    LocationErrCode ResetAllProxy();

    /**
     * @brief Subscribe to changes in WiFi/BT scanning information.
     *
     * @param dataConfig Indicates the locating required data configuration parameters.
     * @param callback Indicates the callback for reporting WiFi/BT scan info.
     * @return Returns ERRCODE_SUCCESS if subscribe to changes in WiFi/BT scanning information successfully.
     */
    LocationErrCode RegisterLocatingRequiredDataCallback(std::unique_ptr<LocatingRequiredDataConfig>& dataConfig,
        sptr<ILocatingRequiredDataCallback>& callback);

    /**
     * @brief Unsubscribe to changes in WiFi/BT scanning information.
     *
     * @param callback Indicates the callback for reporting WiFi/BT scan info.
     * @return Returns ERRCODE_SUCCESS if Unsubscribe to changes in WiFi/BT scanning information successfully.
     */
    LocationErrCode UnRegisterLocatingRequiredDataCallback(sptr<ILocatingRequiredDataCallback>& callback);

    /**
     * @brief Subscribe location error changed.
     *
     * @param callback Indicates the callback for reporting the location error result.
     * @return Returns ERRCODE_SUCCESS if subscribe error changed succeed.
     */
    LocationErrCode SubscribeLocationError(sptr<ILocatorCallback>& callback);

    /**
     * @brief Subscribe bluetooth scan result change.
     *
     * @param callback Indicates the callback for reporting the location error result.
     * @return Returns ERRCODE_SUCCESS if subscribe error changed succeed.
     */
    LocationErrCode SubscribeBluetoothScanResultChange(sptr<IBluetoothScanResultCallback>& callback);

    /**
     * @brief Unsubscribe bluetooth scan result change.
     *
     * @param callback Indicates the callback for reporting the bluetooth scan result.
     * @return Returns ERRCODE_SUCCESS if subscribe error changed succeed.
     */
    LocationErrCode UnSubscribeBluetoothScanResultChange(sptr<IBluetoothScanResultCallback>& callback);

    /**
     * @brief Unsubscribe location errorcode changed.
     *
     * @param callback Indicates the callback for reporting the location error result.
     * @return Returns ERRCODE_SUCCESS if Unsubscribe error changed succeed.
     */
    LocationErrCode UnSubscribeLocationError(sptr<ILocatorCallback>& callback);

    /**
     * @brief Obtain last known location.
     *
     * @param loc Indicates the last known location information.
     * @return Returns ERRCODE_SUCCESS if obtain last known location succeed.
     */
    LocationErrCode GetCurrentWifiBssidForLocating(std::string& bssid);

    /**
     * @brief Gets the distance between two positions.
     *
     * @param loc location 1.
     * @param loc location 2.
     * @param loc distance.
     * @return Returns ERRCODE_SUCCESS if obtain last known location succeed.
     */
    LocationErrCode GetDistanceBetweenLocations(const Location& location1,
        const Location& location2, double& distance);
    
    /**
     * Obtaining the location switch status of a specified user.
     *
     * @param userId - Indicates the ID of a specified user.
     * @returns Returns {@code true} if the location switch on, returns {@code false} otherwise.
     */
    LocationErrCode SetLocationSwitchIgnored(bool enable);

    /**
     * Check whether the POI service is supported.
     *
     * @returns { boolean } Returns {@code true} if POI service is available, returns {@code false} otherwise.
     */
    bool IsPoiServiceSupported();

    /**
     * add beacon fence.
     *
     * @param beaconFenceRequest - beacon fence request parameters.
     * @return Returns ERRCODE_SUCCESS if add beacon fence succeed.
     */
    LocationErrCode AddBeaconFence(const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest);

    /**
     * remove beacon fence.
     *
     * @param beaconFence - beacon fence parameters.
     * @return Returns ERRCODE_SUCCESS if remove beacon fence succeed.
     */
    LocationErrCode RemoveBeaconFence(const std::shared_ptr<BeaconFence>& beaconFence);

    /**
     * Check whether the beacon fence is supported.
     *
     * @return { boolean } Returns {@code true} if beacon fence is supported, returns {@code false} otherwise.
     */
    bool IsBeaconFenceSupported();

    /**
     * Get apps perform locating.
     *
     * @return Returns ERRCODE_SUCCESS if Get apps perform locating.
     */
    LocationErrCode GetAppsPerformLocating(std::vector<AppIdentity>& appsPerformLocatingList);

    void ResetLocatorProxy(const wptr<IRemoteObject> &remote);
    sptr<ILocatorService> GetProxy();
    bool IsLocationCallbackRegistered(const sptr<ILocatorCallback>& callback);
    bool IsSatelliteStatusChangeCallbackRegistered(const sptr<IRemoteObject>& callback);
    bool IsNmeaCallbackRegistered(const sptr<IRemoteObject>& callback);
    bool HasGnssNetworkRequest();
    void AddLocationCallBack(std::unique_ptr<RequestConfig>& requestConfig, sptr<ILocatorCallback>& callback);
    void RemoveLocationCallBack(sptr<ILocatorCallback>& callback);
    void AddSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback);
    void RemoveSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback);
    void AddNmeaCallBack(const sptr<IRemoteObject>& callback);
    void RemoveNmeaCallBack(const sptr<IRemoteObject>& callback);
    void SetIsServerExist(bool isServerExist);

private:
    LocationErrCode CheckEdmPolicy(bool enable);

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
    bool IsCallbackResuming();
    void UpdateCallbackResumingState(bool state);
    void UnLoad();

    sptr<ILocatorService> client_ { nullptr };
    sptr<IRemoteObject::DeathRecipient> recipient_ { nullptr };
    LocationDataManager* locationDataManager_ { nullptr };
    bool isServerExist_ = false;
    bool isCallbackResuming_ = false;
    std::mutex mutex_;
    std::mutex resumeMutex_;
    static std::mutex locatorMutex_;
    static std::shared_ptr<LocatorImpl> instance_;
    sptr<ISystemAbilityStatusChange> saStatusListener_ =
        sptr<LocatorSystemAbilityListener>(new LocatorSystemAbilityListener());
};

class CallbackResumeManager : public ICallbackResumeManager {
public:
    CallbackResumeManager() = default;
    ~CallbackResumeManager() = default;
    void ResumeCallback() override;
private:
    void InitResumeCallbackFuncMap();
    void ResumeGnssStatusCallback();
    void ResumeNmeaMessageCallback();
    void ResumeLocating();
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATOR_IMPL_H
