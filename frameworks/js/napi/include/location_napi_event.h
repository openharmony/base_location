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

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_host.h"
#include "geofence_state.h"
#include "gnss_status_callback_host.h"
#include "nmea_message_callback_host.h"
#endif
#include "location_switch_callback_host.h"
#include "locator_callback_host.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
void InitOnFuncMap();
bool OnLocationServiceStateCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnLocationChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnCountryCodeChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
#ifdef FEATURE_GNSS_SUPPORT
bool OnCachedGnssLocationsReportingCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnGnssStatusChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnNmeaMessageChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnFenceStatusChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
#endif

void InitOffFuncMap();
bool OffAllLocationServiceStateCallback(const napi_env& env);
bool OffAllLocationChangeCallback(const napi_env& env);
bool OffAllCountryCodeChangeCallback(const napi_env& env);
#ifdef FEATURE_GNSS_SUPPORT
bool OffAllGnssStatusChangeCallback(const napi_env& env);
bool OffAllNmeaMessageChangeCallback(const napi_env& env);
bool OffAllCachedGnssLocationsReportingCallback(const napi_env& env);
#endif

bool OffLocationServiceStateCallback(const napi_env& env, const napi_value& handler);
bool OffLocationChangeCallback(const napi_env& env, const napi_value& handler);
bool OffCountryCodeChangeCallback(const napi_env& env, const napi_value& handler);
#ifdef FEATURE_GNSS_SUPPORT
bool OffGnssStatusChangeCallback(const napi_env& env, const napi_value& handler);
bool OffNmeaMessageChangeCallback(const napi_env& env, const napi_value& handler);
bool OffCachedGnssLocationsReportingCallback(const napi_env& env, const napi_value& handler);
#endif

void SubscribeLocationServiceState(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackHost>& switchCallbackHost);
void SubscribeLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackHost>& locatorCallbackHost);
#ifdef FEATURE_GNSS_SUPPORT
void SubscribeGnssStatus(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost);
void SubscribeNmeaMessage(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost);
void SubscribeCacheLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackHost>& cachedCallbackHost);
void SubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler);
#endif

void UnSubscribeLocationChange(sptr<ILocatorCallback>& callback);
void UnSubscribeLocationServiceState(sptr<LocationSwitchCallbackHost>& switchCallbackHost);
#ifdef FEATURE_GNSS_SUPPORT
void UnSubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler);
void UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback);
void UnSubscribeGnssStatus(sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost);
void UnSubscribeNmeaMessage(sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost);
#endif

bool IsCallbackEquals(const napi_env& env, const napi_value& handler, const napi_ref& savedCallback);
void GenRequestConfig(const napi_env& env, const napi_value* argv,
    const size_t& objectArgsNum, std::unique_ptr<RequestConfig>& requestConfig);
napi_value RequestLocationOnce(const napi_env& env, const size_t argc, const napi_value* argv);
napi_value On(napi_env env, napi_callback_info cbinfo);
napi_value Off(napi_env env, napi_callback_info cbinfo);
napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo);

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeLocationServiceStateV9(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackHost>& switchCallbackHost);
LocationErrCode SubscribeLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackHost>& locatorCallbackHost);
#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode SubscribeGnssStatusV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost);
LocationErrCode SubscribeNmeaMessageV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost);
LocationErrCode SubscribeCacheLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackHost>& cachedCallbackHost);
LocationErrCode SubscribeFenceStatusChangeV9(const napi_env& env, const napi_value& object, const napi_value& handler);
#endif

LocationErrCode UnSubscribeLocationChangeV9(sptr<ILocatorCallback>& callback);
LocationErrCode UnSubscribeLocationServiceStateV9(sptr<LocationSwitchCallbackHost>& switchCallbackHost);
#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode UnSubscribeFenceStatusChangeV9(const napi_env& env,
    const napi_value& object, const napi_value& handler);
LocationErrCode UnSubscribeCacheLocationChangeV9(sptr<ICachedLocationsCallback>& callback);
LocationErrCode UnSubscribeGnssStatusV9(sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost);
LocationErrCode UnSubscribeNmeaMessageV9(sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost);
#endif
napi_value RequestLocationOnceV9(const napi_env& env, const size_t argc, const napi_value* argv);
LocationErrCode CheckLocationSwitchEnable();
#endif
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_NAPI_EVENT_H
