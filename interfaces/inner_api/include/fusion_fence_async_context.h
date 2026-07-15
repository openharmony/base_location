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

#ifndef FUSION_FENCE_ASYNC_CONTEXT_H
#define FUSION_FENCE_ASYNC_CONTEXT_H

#include <string>
#include <memory>
#include "napi/native_api.h"
#include "async_context.h"
#include "fusion_fence_request.h"
#include "fusion_fence_callback_napi.h"
#include "location_gnss_geofence_callback_napi.h"

namespace OHOS {
namespace Location {
class FusionFenceAsyncContext : public AsyncContext {
public:
    std::string identifier_;
    std::shared_ptr<FusionFenceRequest> fusionRequest_;
    sptr<FusionFenceCallbackNapi> callbackHost_;
    int64_t beginTime;
    FusionFenceTransition transition_;
    std::string fenceId_;
    GnssGeofenceOperateType operateType_;
    LocationErrCode errorCode_;
 
    explicit FusionFenceAsyncContext(
        napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), beginTime(0),
        operateType_(GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD), errorCode_(ERRCODE_SUCCESS) {}
 
    FusionFenceAsyncContext() = delete;
    ~FusionFenceAsyncContext() override {}
};
} // namespace Location
} // namespace OHOS
#endif // FUSION_FENCE_ASYNC_CONTEXT_H