/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SEARCH_CALLBACK_NAPI_H
#define BLUETOOTH_SEARCH_CALLBACK_NAPI_H

#include "bluetooth_search_request_params.h"
#include "napi/native_api.h"

namespace OHOS {
namespace Location {
bool JsObjToBluetoothSearchRequest(const napi_env& env, const napi_value& object,
    BluetoothSearchRequestParams& params);
}  // namespace Location
}  // namespace OHOS
#endif  // BLUETOOTH_SEARCH_CALLBACK_NAPI_H