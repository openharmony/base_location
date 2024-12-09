/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef FENCEEXTENSION_JS_FENCE_EXTENSION_CONTEXT_H
#define FENCEEXTENSION_JS_FENCE_EXTENSION_CONTEXT_H
#include "fence_extension_context.h"

#include "napi/native_api.h"
#include "native_engine/native_engine.h"
#include "js_extension_context.h"
#include "js_runtime_utils.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "napi/native_api.h"
#include "napi_common_want.h"
#include "want.h"
#include "ability_business_error.h"

namespace OHOS {
namespace Location {
using namespace OHOS::AbilityRuntime;
class JsFenceExtensionContext {
public:
    explicit JsFenceExtensionContext(const std::shared_ptr<FenceExtensionContext> &context);

    ~JsFenceExtensionContext();

    static ::napi_value CreateJsFenceExtensionContext(
        const ::napi_env &env, std::shared_ptr<FenceExtensionContext> context);

    static ::napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsFenceExtensionContext, OnStartAbility);
    }

private:
    ::napi_value OnStartExtensionAbility(AAFwk::Want want, napi_env env, AbilityRuntime::NapiCallbackInfo &info);
    ::napi_value OnStartAbility(napi_env env, AbilityRuntime::NapiCallbackInfo &info);
    static void Finalizer(::napi_env env, void *data, void *hint);

    std::weak_ptr<FenceExtensionContext> context_;
};
}  // namespace Location
}  // namespace OHOS
#endif  // FENCEEXTENSION_JS_FENCE_EXTENSION_CONTEXT_H
