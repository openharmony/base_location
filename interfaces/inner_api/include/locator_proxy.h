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
#include <list>

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
    LocationErrCode UpdateSaAbility();
    LocationErrCode GetSwitchState(int &state);
    LocationErrCode EnableAbility(bool isEnabled);
    LocationErrCode RegisterSwitchCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    LocationErrCode UnregisterSwitchCallback(const sptr<IRemoteObject> &callback);
    LocationErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    LocationErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject> &callback);
    LocationErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    LocationErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject> &callback);
    LocationErrCode RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject> &callback);
    LocationErrCode UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject> &callback);
    LocationErrCode RegisterCountryCodeCallback(const sptr<IRemoteObject> &callback, pid_t uid);
    LocationErrCode UnregisterCountryCodeCallback(const sptr<IRemoteObject> &callback);
    LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid);
    LocationErrCode StopLocating(sptr<ILocatorCallback>& callback);
    LocationErrCode GetCacheLocation(std::unique_ptr<Location> &loc);
    LocationErrCode IsGeoConvertAvailable(bool &isAvailable);
    LocationErrCode GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList);
    LocationErrCode GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList);
    LocationErrCode IsLocationPrivacyConfirmed(const int type, bool &isConfirmed);
    LocationErrCode SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed);
    LocationErrCode RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName);
    LocationErrCode UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback);
    LocationErrCode GetCachedGnssLocationsSize(int &size);
    LocationErrCode FlushCachedGnssLocations();
    LocationErrCode SendCommand(std::unique_ptr<LocationCommand>& commands);
    LocationErrCode AddFence(std::unique_ptr<GeofenceRequest>& request);
    LocationErrCode RemoveFence(std::unique_ptr<GeofenceRequest>& request);
    LocationErrCode GetIsoCountryCode(std::shared_ptr<CountryCode>& countryCode);
    LocationErrCode EnableLocationMock();
    LocationErrCode DisableLocationMock();
    LocationErrCode SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);
    LocationErrCode EnableReverseGeocodingMock();
    LocationErrCode DisableReverseGeocodingMock();
    LocationErrCode SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo);
    LocationErrCode SendMsgWithDataReply(const int msgId, MessageParcel& data, MessageParcel& reply);
    LocationErrCode SendMsgWithReply(const int msgId, MessageParcel& reply);
    LocationErrCode SendSimpleMsg(const int msgId);
    LocationErrCode SendRegisterMsgToRemote(const int msgId, const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode ProxyUidForFreeze(int32_t uid, bool isProxy);
    LocationErrCode ResetAllProxy();

private:
    static inline BrokerDelegator<LocatorProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_PROXY_H
