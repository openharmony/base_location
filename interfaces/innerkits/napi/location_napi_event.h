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

#ifndef LOCATION_NAPI_EVENT_H
#define LOCATION_NAPI_EVENT_H

#include <string>
#include "cached_locations_callback_host.h"
#include "geofence_state.h"
#include "gnss_status_callback_host.h"
#include "location_switch_callback_host.h"
#include "locator_callback_host.h"
#include "napi/native_api.h"
#include "nmea_message_callback_host.h"
#include "request_config.h"
#include "want_agent_helper.h"

namespace OHOS {
namespace Location {
class EventManager {
public:
    EventManager(napi_env env, napi_value thisVar);
    virtual ~EventManager();
    void SubscribeLocationServiceState(const std::string& name, napi_value& handler);
    bool IsCallbackEquals(napi_value& handler, napi_ref& savedCallback);
    void UnSubscribeLocationServiceState();
    void SubscribeLocationChange(const std::string& name, const napi_value& object, napi_value& handler, int fixNumber);
    void GetCallbackType(napi_env env, const size_t argc, const napi_value* argv, bool& isCallbackType,
        size_t& nonCallbackArgNum);
    void GenRequestConfig(napi_env env, const napi_value* argv,
        size_t& nonCallbackArgNum, std::unique_ptr<RequestConfig>& requestConfig);
    void GetTimeoutParam(napi_env env, const napi_value* argv,
        size_t& nonCallbackArgNum, int& timeout);
    napi_value RequestLocationOnce(napi_env env, const size_t argc, const napi_value* argv);
    void UnSubscribeLocationChange(sptr<ILocatorCallback>& callback);
    void DeleteHanderRef(napi_value& handler);

    void SubscribeGnssStatus(const std::string& name, napi_value& handler);
    void SubscribeNmeaMessage(const std::string& name, napi_value& handler);
    void UnSubscribeGnssStatus();
    void UnSubscribeNmeaMessage();

    void SubscribeCacheLocationChange(const std::string& name, const napi_value& object, napi_value& handler);
    void UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback);

    void SubscribeFenceStatusChange(const std::string& name, const napi_value& object, napi_value& handler);
    void UnSubscribeFenceStatusChange(const std::string& name, const napi_value& object, napi_value& handler);

    napi_env m_env;
    napi_ref m_thisVarRef;
    sptr<LocatorCallbackHost> m_locatorCallbackHost;
    sptr<LocatorCallbackHost> m_singleLocatorCallbackHost;
    sptr<LocationSwitchCallbackHost> m_switchCallbackHost;
    sptr<ILocatorCallback> m_locatorCallback;
    sptr<ILocatorCallback> m_singleLocatorCallback;
    sptr<ISwitchCallback> m_switchCallback;
    sptr<GnssStatusCallbackHost> m_gnssStatusCallbackHost;
    sptr<NmeaMessageCallbackHost> m_nmeaMessageCallbackHost;
    sptr<CachedLocationsCallbackHost> m_cachedCallbackHost;
    sptr<ICachedLocationsCallback> m_cachedCallback;

    std::vector<GeoFenceState*> mFences;
};

napi_value On(napi_env env, napi_callback_info cbinfo);
napi_value Off(napi_env env, napi_callback_info cbinfo);
napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo);
napi_value LocationEventListenerConstructor(napi_env env, napi_callback_info cbinfo);
}  // namespace Location
}  // namespace OHOS

#endif
