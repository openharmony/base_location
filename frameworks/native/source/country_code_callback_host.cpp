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

#include "country_code_callback_host.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"
#include "napi/native_api.h"
#include "country_code.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
CountryCodeCallbackHost::CountryCodeCallbackHost()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
}

CountryCodeCallbackHost::~CountryCodeCallbackHost()
{
}

int CountryCodeCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(COUNTRY_CODE_CALLBACK, "CountryCodeCallbackHost::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case COUNTRY_CODE_CHANGE_EVENT: {
            auto countryCodePtr = CountryCode::Unmarshalling(data);
            OnCountryCodeChange(countryCodePtr);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool CountryCodeCallbackHost::Send(const std::shared_ptr<CountryCode>& country)
{
    std::shared_lock<std::shared_mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    if (env_ == nullptr) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "env_ == nullptr.");
        return false;
    }
    if (country == nullptr) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "country == nullptr.");
        return false;
    }
    NAPI_CALL_BASE(env_, napi_get_uv_event_loop(env_, &loop), false);
    if (loop == nullptr) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "work == nullptr.");
        return false;
    }
    auto context = new (std::nothrow) CountryCodeContext(env_);
    if (context == nullptr) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "context == nullptr.");
        return false;
    }
    context->env = env_;
    context->callback[SUCCESS_CALLBACK] = handlerCb_;
    context->country = country;
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}

void CountryCodeCallbackHost::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            CountryCodeContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "work is nullptr!");
                return;
            }
            context = static_cast<CountryCodeContext *>(work->data);
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "context is nullptr!");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(COUNTRY_CODE_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            napi_value jsEvent;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_object(context->env, &jsEvent),
                scope, context, work);
            if (context->country) {
                CountryCodeToJs(context->env, context->country, jsEvent);
            } else {
                LBSLOGE(LOCATOR_STANDARD, "country is nullptr!");
            }
            if (context->callback[0] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_get_undefined(context->env, &undefine), scope, context, work);
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_get_reference_value(context->env, context->callback[SUCCESS_CALLBACK], &handler),
                    scope, context, work);
                if (napi_call_function(context->env, nullptr, handler, 1, &jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(COUNTRY_CODE_CALLBACK, "Report event failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

void CountryCodeCallbackHost::OnCountryCodeChange(const std::shared_ptr<CountryCode>& country)
{
    LBSLOGD(COUNTRY_CODE_CALLBACK, "CountryCodeCallbackHost::OnCountryCodeChange");
    Send(country);
}

void CountryCodeCallbackHost::SetEnv(napi_env env)
{
    std::shared_lock<std::shared_mutex> guard(mutex_);
    env_ = env;
}

void CountryCodeCallbackHost::SetCallback(napi_ref cb)
{
    std::shared_lock<std::shared_mutex> guard(mutex_);
    handlerCb_ = cb;
}

void CountryCodeCallbackHost::DeleteHandler()
{
    std::shared_lock<std::shared_mutex> guard(mutex_);
    if (handlerCb_ == nullptr || env_ == nullptr) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "handler or env is nullptr.");
        return;
    }
    auto context = new (std::nothrow) AsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(COUNTRY_CODE_CALLBACK, "context == nullptr.");
        return;
    }
    context->env = env_;
    context->callback[SUCCESS_CALLBACK] = handlerCb_;
    DeleteQueueWork(context);
    handlerCb_ = nullptr;
}
}  // namespace Location
}  // namespace OHOS
