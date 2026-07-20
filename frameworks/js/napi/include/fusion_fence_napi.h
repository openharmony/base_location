/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef FUSION_FENCE_NAPI_H
#define FUSION_FENCE_NAPI_H

#include <string>
#include <memory>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "fusion_fence_request.h"
#include "fusion_fence_callback_napi.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
bool ParseFusionFenceRequest(napi_env env, napi_value object, std::shared_ptr<FusionFenceRequest>& request);
bool ParseFusionFenceTransitionCallback(napi_env env, napi_value object,
    sptr<FusionFenceCallbackNapi>& callbackHost);
napi_value CreateFusionFenceTransitionJsObj(napi_env env, const FusionFenceTransition& transition);
bool GetStringFromValue(const napi_env& env, napi_value value, std::string& result);
}
}

#endif