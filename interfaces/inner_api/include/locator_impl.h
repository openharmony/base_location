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
#include "geo_coding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "locator_proxy.h"
#include "location_data_manager.h"
namespace OHOS {
namespace Location {
class ICallbackResumeManager {
public:
    virtual ~ICallbackResumeManager() = default;
    virtual void ResumeCallback() = 0;
};

class LocatorImpl {
public:
    static std::shared_ptr<LocatorImpl> GetInstance();
    explicit LocatorImpl();
    ~LocatorImpl();
    bool Init();
    bool IsLocationEnabled();
    void ShowNotification();
    void RequestPermission();
    void RequestEnableLocation();
    void EnableAbility(bool enable);
    void StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback);
    void StopLocating(sptr<ILocatorCallback>& callback);
    std::unique_ptr<Location> GetCachedLocation();
    bool RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    bool UnregisterSwitchCallback(const sptr<IRemoteObject>& callback);
    bool RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    bool UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback);
    bool RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    bool UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback);
    bool RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    bool UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback);
    void RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback);
    void UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback);
    bool IsGeoServiceAvailable();
    void GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList);
    void GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList);
    bool IsLocationPrivacyConfirmed(const int type);
    int SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed);
    int GetCachedGnssLocationsSize();
    int FlushCachedGnssLocations();
    bool SendCommand(std::unique_ptr<LocationCommand>& commands);
    bool AddFence(std::unique_ptr<GeofenceRequest>& request);
    bool RemoveFence(std::unique_ptr<GeofenceRequest>& request);
    std::shared_ptr<CountryCode> GetIsoCountryCode();
    bool EnableLocationMock();
    bool DisableLocationMock();
    bool SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);
    bool EnableReverseGeocodingMock();
    bool DisableReverseGeocodingMock();
    bool SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);
    bool ProxyUidForFreeze(int32_t uid, bool isProxy);
    bool ResetAllProxy();

    LocationErrCode IsLocationEnabledV9(bool &isEnabled);
    LocationErrCode EnableAbilityV9(bool enable);
    LocationErrCode StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback);
    LocationErrCode StopLocatingV9(sptr<ILocatorCallback>& callback);
    LocationErrCode GetCachedLocationV9(std::unique_ptr<Location> &loc);
    LocationErrCode RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode UnregisterSwitchCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode UnregisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback);
    LocationErrCode UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback);
    LocationErrCode IsGeoServiceAvailableV9(bool &isAvailable);
    LocationErrCode GetAddressByCoordinateV9(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList);
    LocationErrCode GetAddressByLocationNameV9(MessageParcel &data,
        std::list<std::shared_ptr<GeoAddress>>& replyList);
    LocationErrCode IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed);
    LocationErrCode SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed);
    LocationErrCode GetCachedGnssLocationsSizeV9(int &size);
    LocationErrCode FlushCachedGnssLocationsV9();
    LocationErrCode SendCommandV9(std::unique_ptr<LocationCommand>& commands);
    LocationErrCode AddFenceV9(std::unique_ptr<GeofenceRequest>& request);
    LocationErrCode RemoveFenceV9(std::unique_ptr<GeofenceRequest>& request);
    LocationErrCode GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode);
    LocationErrCode EnableLocationMockV9();
    LocationErrCode DisableLocationMockV9();
    LocationErrCode SetMockedLocationsV9(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);
    LocationErrCode EnableReverseGeocodingMockV9();
    LocationErrCode DisableReverseGeocodingMockV9();
    LocationErrCode SetReverseGeocodingMockInfoV9(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);
    LocationErrCode ProxyUidForFreezeV9(int32_t uid, bool isProxy);
    LocationErrCode ResetAllProxyV9();
    void ResetLocatorProxy(const wptr<IRemoteObject> &remote);
    void SetResumer(std::shared_ptr<ICallbackResumeManager> resumer);

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
    bool IsCallbackResuming();
    void UpdateCallbackResumingState(bool state);

    sptr<LocatorProxy> client_ { nullptr };
    sptr<IRemoteObject::DeathRecipient> recipient_ { nullptr };
    std::shared_ptr<ICallbackResumeManager> resumer_ { nullptr };
    std::shared_ptr<LocationDataManager> locationDataManager_ { nullptr };
    bool isServerExist_ = false;
    bool isCallbackResuming_ = false;
    std::mutex mutex_;
    std::mutex resumeMutex_;
    static std::mutex locatorMutex_;
    static std::shared_ptr<LocatorImpl> instance_;
    std::shared_ptr<CountryCodeManager> countryCodeManager_ = nullptr;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATOR_IMPL_H
