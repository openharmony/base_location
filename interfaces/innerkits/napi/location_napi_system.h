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

#ifndef LOCATION_NAPI_SYSTEM_H
#define LOCATION_NAPI_SYSTEM_H

#include "napi/native_api.h"

namespace OHOS {
namespace Location {
napi_value GetLocation(napi_env env, napi_callback_info cbinfo);
napi_value GetLocationType(napi_env env, napi_callback_info cbinfo);
napi_value Subscribe(napi_env env, napi_callback_info cbinfo);
napi_value Unsubscribe(napi_env env, napi_callback_info cbinfo);
napi_value GetSupportedCoordTypes(napi_env env, napi_callback_info cbinfo);
}  // namespace Location
}  // namespace OHOS

#endif // LOCATION_NAPI_SYSTEM_H
