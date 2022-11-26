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

#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "constant_definition.h"
#include "country_code.h"
#include "geo_coding_mock_info.h"
#include "i_cached_locations_callback.h"
#include "i_locator.h"
#include "i_locator_callback.h"
#include "location.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
class LocatorProxy : public IRemoteProxy<ILocator> {
public:
    explicit LocatorProxy(const sptr<IRemoteObject> &impl);
    ~LocatorProxy() = default;
    void UpdateSaAbility();
    int GetSwitchState();
    void EnableAbility(bool isEnabled);
    void RegisterSwitchCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    void UnregisterSwitchCallback(const sptr<IRemoteObject> &callback);
    void RegisterGnssStatusCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    void UnregisterGnssStatusCallback(const sptr<IRemoteObject> &callback);
    void RegisterNmeaMessageCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    void UnregisterNmeaMessageCallback(const sptr<IRemoteObject> &callback);
    int RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject> &callback);
    int UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject> &callback);
    void RegisterCountryCodeCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    void UnregisterCountryCodeCallback(const sptr<IRemoteObject> &callback);
    int StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid);
    int StopLocating(sptr<ILocatorCallback>& callback);
    int GetCacheLocation(MessageParcel &replay);
    int IsGeoConvertAvailable(MessageParcel &replay);
    int GetAddressByCoordinate(MessageParcel &data, MessageParcel &replay);
    int GetAddressByLocationName(MessageParcel &data, MessageParcel &replay);
    bool IsLocationPrivacyConfirmed(const int type);
    int SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed);

    int RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName);
    int UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback);

    int GetCachedGnssLocationsSize();
    int FlushCachedGnssLocations();
    void SendCommand(std::unique_ptr<LocationCommand>& commands);

    void AddFence(std::unique_ptr<GeofenceRequest>& request);
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request);
    std::shared_ptr<CountryCode> GetIsoCountryCode();
    bool EnableLocationMock();
    bool DisableLocationMock();
    bool SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);

    bool EnableReverseGeocodingMock();

    bool DisableReverseGeocodingMock();

    bool SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);

    int SendMsgWithDataReply(const int msgId, MessageParcel& data, MessageParcel& reply);

    int SendMsgWithReply(const int msgId, MessageParcel& reply);

    int SendSimpleMsg(const int msgId);

    int SendRegisterMsgToRemote(const int msgId, const sptr<IRemoteObject>& callback, pid_t uid);

    bool ProxyUidForFreeze(int32_t uid, bool isProxy);
    bool ResetAllProxy();

private:
    static inline BrokerDelegator<LocatorProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_PROXY_H
