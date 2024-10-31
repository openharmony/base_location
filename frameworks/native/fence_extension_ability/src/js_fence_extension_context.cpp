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

#include "js_fence_extension_context.h"

#include "location_log.h"

namespace OHOS {
namespace Location {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
JsFenceExtensionContext::JsFenceExtensionContext(const std::shared_ptr<FenceExtensionContext> &context)
    : context_(context)
{}

JsFenceExtensionContext::~JsFenceExtensionContext()
{}

::napi_value JsFenceExtensionContext::CreateJsFenceExtensionContext(
    const ::napi_env &env, std::shared_ptr<FenceExtensionContext> context)
{
    LBSLOGI(FENCE_EXTENSION, "Begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    } else {
        return nullptr;
    }
    ::napi_value objValue = AbilityRuntime::CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsFenceExtensionContext> jsContext = std::make_unique<JsFenceExtensionContext>(context);
    ::napi_status retStatus =
        ::napi_wrap(env, objValue, jsContext.release(), JsFenceExtensionContext::Finalizer, nullptr, nullptr);
    if (retStatus != ::napi_status::napi_ok) {
        LBSLOGI(FENCE_EXTENSION, "Bind native context to js context error");
    }
    const char *moduleName = "JsFenceExtensionContext";
    AbilityRuntime::BindNativeFunction(
        env, objValue, "startAbility", moduleName, JsFenceExtensionContext::StartAbility);
    LBSLOGI(FENCE_EXTENSION, "End");
    return objValue;
}

napi_value JsFenceExtensionContext::OnStartAbility(napi_env env, AbilityRuntime::NapiCallbackInfo &info)
{
    LBSLOGI(FENCE_EXTENSION, "OnStartAbility");
    if (info.argc < ARGC_ONE) {
        LBSLOGE(FENCE_EXTENSION, "invalid argc");
        AbilityRuntime::ThrowTooFewParametersError(env);
        return AbilityRuntime::CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[ARGC_ZERO], want)) {
        AbilityRuntime::ThrowInvalidParamError(env, "Parse param want failed, must be a Want.");
        return AbilityRuntime::CreateJsUndefined(env);
    }
    auto isStartService = want.GetBoolParam("startService", true);
    if (isStartService) {
        return JsFenceExtensionContext::OnStartExtensionAbility(want, env, info);
    }
    LBSLOGE(FENCE_EXTENSION, "not support start ui ability");
    AbilityRuntime::ThrowInvalidParamError(env, "Not support start ui ability.");
    return AbilityRuntime::CreateJsUndefined(env);
}

napi_value JsFenceExtensionContext::OnStartExtensionAbility(
    AAFwk::Want want, napi_env env, AbilityRuntime::NapiCallbackInfo &info)
{
    LBSLOGI(FENCE_EXTENSION, "OnStartExtensionAbility");

    AbilityRuntime::NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want](napi_env env, AbilityRuntime::NapiAsyncTask &task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                LBSLOGW(FENCE_EXTENSION, "context is released");
                task.Reject(env, CreateJsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = context->StartServiceExtensionAbility(want);
            if (innerErrorCode == 0) {
                task.Resolve(env, AbilityRuntime::CreateJsUndefined(env));
            } else {
                task.Reject(env, AbilityRuntime::CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[ARGC_ONE];
    napi_value result = nullptr;
    AbilityRuntime::NapiAsyncTask::ScheduleHighQos("JsFenceExtensionContext::OnStartExtensionAbility",
        env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

void JsFenceExtensionContext::Finalizer(::napi_env env, void *data, void *hint)
{
    std::unique_ptr<JsFenceExtensionContext>(static_cast<JsFenceExtensionContext *>(data));
}
}  // namespace Location
}  // namespace OHOS