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
void InitOffFuncMap();
bool OffAllLocationServiceStateCallback(const napi_env& env);
bool OffAllLocationChangeCallback(const napi_env& env);
bool OffAllGnssStatusChangeCallback(const napi_env& env);
bool OffAllNmeaMessageChangeCallback(const napi_env& env);
bool OffAllCachedGnssLocationsReportingCallback(const napi_env& env);
bool OffAllCountryCodeChangeCallback(const napi_env& env);
bool OffLocationServiceStateCallback(const napi_env& env, const napi_value& handler);
bool OffLocationChangeCallback(const napi_env& env, const napi_value& handler);
bool OffGnssStatusChangeCallback(const napi_env& env, const napi_value& handler);
bool OffNmeaMessageChangeCallback(const napi_env& env, const napi_value& handler);
bool OffCachedGnssLocationsReportingCallback(const napi_env& env, const napi_value& handler);
bool OffCountryCodeChangeCallback(const napi_env& env, const napi_value& handler);
void SubscribeLocationServiceState(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackHost>& switchCallbackHost);
void SubscribeGnssStatus(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost);
void SubscribeNmeaMessage(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost);
void SubscribeLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackHost>& locatorCallbackHost);
void SubscribeCacheLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackHost>& cachedCallbackHost);
void SubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler);
void UnSubscribeLocationChange(sptr<ILocatorCallback>& callback);
void UnSubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler);
void UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback);
void UnSubscribeLocationServiceState(sptr<LocationSwitchCallbackHost>& switchCallbackHost);
void UnSubscribeGnssStatus(sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost);
void UnSubscribeNmeaMessage(sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost);
bool IsCallbackEquals(const napi_env& env, const napi_value& handler, const napi_ref& savedCallback);
void GenRequestConfig(const napi_env& env, const napi_value* argv,
    const size_t& objectArgsNum, std::unique_ptr<RequestConfig>& requestConfig);
napi_value RequestLocationOnce(const napi_env& env, const size_t argc, const napi_value* argv);
napi_value On(napi_env env, napi_callback_info cbinfo);
napi_value Off(napi_env env, napi_callback_info cbinfo);
napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo);
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_NAPI_EVENT_H
