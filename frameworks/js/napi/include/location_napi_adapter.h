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

#ifndef LOCATION_NAPI_ADAPTER_H
#define LOCATION_NAPI_ADAPTER_H

#include "common_utils.h"
#include "locator.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
napi_value GetLastLocation(napi_env env, napi_callback_info info);
napi_value IsLocationEnabled(napi_env env, napi_callback_info info);
napi_value EnableLocation(napi_env env, napi_callback_info info);
napi_value DisableLocation(napi_env env, napi_callback_info info);
napi_value RequestEnableLocation(napi_env env, napi_callback_info info);
napi_value IsGeoServiceAvailable(napi_env env, napi_callback_info info);
napi_value GetAddressesFromLocation(napi_env env, napi_callback_info info);
napi_value GetAddressesFromLocationName(napi_env env, napi_callback_info info);
napi_value GetCachedGnssLocationsSize(napi_env env, napi_callback_info info);
napi_value FlushCachedGnssLocations(napi_env env, napi_callback_info info);
napi_value SendCommand(napi_env env, napi_callback_info info);
#ifdef ENABLE_NAPI_MANAGER
napi_value IsLocationPrivacyConfirmed(napi_env env, napi_callback_info info);
napi_value SetLocationPrivacyConfirmStatus(napi_env env, napi_callback_info info);
napi_value GetIsoCountryCode(napi_env env, napi_callback_info info);
napi_value EnableLocationMock(napi_env env, napi_callback_info info);
napi_value DisableLocationMock(napi_env env, napi_callback_info info);
napi_value SetMockedLocations(napi_env env, napi_callback_info info);
napi_value EnableReverseGeocodingMock(napi_env env, napi_callback_info info);
napi_value DisableReverseGeocodingMock(napi_env env, napi_callback_info info);
napi_value SetReverseGeocodingMockInfo(napi_env env, napi_callback_info info);
napi_value HandleGetCachedLocation(napi_env env);
LocationErrCode CheckLocationSwitchState();
#endif
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_NAPI_ADAPTER_H
