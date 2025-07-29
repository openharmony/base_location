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

#ifndef BEACON_FENCE_NAPI_H
#define BEACON_FENCE_NAPI_H

#include <map>
#include <mutex>

#include "iremote_stub.h"
#include "napi/native_api.h"
#include "uv.h"

#include "constant_definition.h"
#include "location_log.h"
#include "beacon_fence_request.h"
#include "beacon_fence.h"

namespace OHOS {
namespace Location {
bool JsObjToBeaconFence(const napi_env& env, const napi_value& object,
    const std::shared_ptr<BeaconFence>& beaconFence);
bool GenBeaconManufactureDataRequest(const napi_env& env, const napi_value& object,
    std::shared_ptr<BeaconFence> beaconFence);
bool ParseBeaconFenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<BeaconFenceRequest>& request);
bool GenBeaconFenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<BeaconFenceRequest>& request);
bool JsObjToTransitionCallback(const napi_env& env, const napi_value& object);
void JsObjToBeaconFenceTransitionCallback(const napi_env& env, const napi_value& object,
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost);
bool ParseArrayBufferParams(napi_env env, napi_value object, const char *name, std::vector<uint8_t> &outParam);
bool ParseArrayBuffer(napi_env env, uint8_t** data, size_t &size, napi_value args);
} // namespace Location
} // namespace OHOS
#endif // BEACON_FENCE_NAPI_H
