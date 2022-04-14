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

#include "locator_callback_host.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "i_locator_callback.h"
#include "lbs_log.h"
#include "location_napi_adapter.h"
#include "location_util.h"
#include "locator.h"

namespace OHOS {
namespace Location {
LocatorCallbackHost::LocatorCallbackHost()
{
    m_env = nullptr;
    m_handlerCb = nullptr;
    m_successHandlerCb = nullptr;
    m_failHandlerCb = nullptr;
    m_completeHandlerCb = nullptr;
    m_deferred = nullptr;
    m_lastCallingUid = 0;
    m_lastCallingPid = 0;
    m_fixNumber = 0;
}

LocatorCallbackHost::~LocatorCallbackHost()
{
}

int LocatorCallbackHost::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }
    int uid = IPCSkeleton::GetCallingUid();
    if (uid > SYSTEM_UID) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid uid!");
        return -1;
    }
    switch (code) {
        case RECEIVE_LOCATION_INFO_EVENT: {
            std::unique_ptr<Location> location = Location::Unmarshalling(data);
            LBSLOGI(LOCATOR_STANDARD, "CallbackSutb receive LOCATION_EVENT.");
            Send(location);
            break;
        }
        case RECEIVE_LOCATION_STATUS_EVENT: {
            int status = data.ReadInt32();
            LBSLOGI(LOCATOR_STANDARD, "CallbackSutb receive STATUS_EVENT. status:%{public}d", status);
            OnLocatingStatusChange(status);
            break;
        }
        case RECEIVE_ERROR_INFO_EVENT: {
            int errorCode = data.ReadInt32();
            LBSLOGI(LOCATOR_STANDARD, "CallbackSutb receive ERROR_EVENT. errorCode:%{public}d", errorCode);
            OnErrorReport(errorCode);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocatorCallbackHost::DoSendWork(uv_loop_s *&loop, uv_work_t *&work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            LocationAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "work is nullptr!");
                return;
            }
            context = static_cast<LocationAsyncContext *>(work->data);
            if (context == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "context is nullptr!");
                delete work;
                work = nullptr;
                return;
            }
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "scope is nullptr");
                // close handle scope, release napi_value
                napi_close_handle_scope(context->env, scope);
                delete context;
                context = nullptr;
                delete work;
                work = nullptr;
                return;
            }
            napi_value jsOhosEvent = nullptr;
            napi_value jsSystemEvent = nullptr;
            if (context->loc != nullptr) {
                napi_create_object(context->env, &jsOhosEvent);
                LocationToJs(context->env, context->loc, jsOhosEvent);
                napi_create_object(context->env, &jsSystemEvent);
                SystemLocationToJs(context->env, context->loc, jsSystemEvent);
            }
            if (context->ohosCallback[0] != nullptr || context->systemCallback[0] != nullptr) {
                if (context->ohosCallback[0] != nullptr) {
                    napi_value undefine;
                    napi_value handler = nullptr;
                    napi_get_undefined(context->env, &undefine);
                    napi_get_reference_value(context->env, context->ohosCallback[0], &handler);
                    if (napi_call_function(context->env, nullptr, handler, 1,
                        &jsOhosEvent, &undefine) != napi_ok) {
                        LBSLOGE(LOCATOR_CALLBACK, "Report osho event failed");
                    }
                }
                if (context->systemCallback[0] != nullptr) {
                    napi_value undefine;
                    napi_value handler = nullptr;
                    napi_get_undefined(context->env, &undefine);
                    napi_get_reference_value(context->env, context->systemCallback[0], &handler);
                    if (napi_call_function(context->env, nullptr, handler, 1,
                        &jsSystemEvent, &undefine) != napi_ok) {
                        LBSLOGE(LOCATOR_CALLBACK, "Report system event failed");
                    }
                }
            } else if (context->deferred != nullptr) {
                if (jsOhosEvent != nullptr) {
                    napi_resolve_deferred(context->env, context->deferred, jsOhosEvent);
                } else {
                    napi_reject_deferred(context->env, context->deferred, jsOhosEvent);
                }
                if (jsSystemEvent != nullptr) {
                    napi_resolve_deferred(context->env, context->deferred, jsSystemEvent);
                } else {
                    napi_reject_deferred(context->env, context->deferred, jsSystemEvent);
                }
            }
            napi_close_handle_scope(context->env, scope);
            delete context;
            context = nullptr;
            delete work;
            work = nullptr;
    });
}

bool LocatorCallbackHost::Send(std::unique_ptr<Location>& location)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "work == nullptr.");
        return false;
    }
    LocatorAsyncContext *context = new (std::nothrow) LocatorAsyncContext(m_env);
    if (context == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "context == nullptr.");
        return false;
    }
    context->env = m_env;
    context->ohosCallback[0] = m_handlerCb;
    context->systemCallback[0] = m_successHandlerCb;
    context->deferred = m_deferred;
    context->loc = std::move(location);
    work->data = context;
    DoSendWork(loop, work);
    return true;
}

void LocatorCallbackHost::DoSendErrorCode(uv_loop_s *&loop, uv_work_t *&work)
{
    uv_queue_work(
        loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            JsContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            context = static_cast<JsContext *>(work->data);
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "scope is nullptr");
                napi_close_handle_scope(context->env, scope);
                delete context;
                context = nullptr;
                delete work;
                work = nullptr;
                return;
            }
            if (context->systemCallback[1] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                napi_get_undefined(context->env, &undefine);
                napi_get_reference_value(context->env, context->systemCallback[0], &handler);
                if (napi_call_function(context->env, nullptr, handler, 1,
                    &context->m_jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(LOCATOR_CALLBACK, "Report system error failed");
                }
            }
            napi_close_handle_scope(context->env, scope);
            delete context;
            context = nullptr;
            delete work;
            work = nullptr;
    });
}

bool LocatorCallbackHost::SendErrorCode(const int& errorCode)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "work == nullptr.");
        return false;
    }
    JsContext *context = new (std::nothrow) JsContext(m_env);
    if (context == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "context == nullptr.");
        return false;
    }
    napi_value nVerrorCode;
    napi_create_int32(m_env, errorCode, &nVerrorCode);
    context->env = m_env;
    context->systemCallback[1] = m_failHandlerCb;
    context->m_jsEvent = nVerrorCode;
    work->data = context;
    DoSendErrorCode(loop, work);
    return true;
}

void LocatorCallbackHost::OnLocationReport(const std::unique_ptr<Location>& location)
{
}

void LocatorCallbackHost::OnLocatingStatusChange(const int status)
{
}

void LocatorCallbackHost::OnErrorReport(const int errorCode)
{
    SendErrorCode(errorCode);
}

void LocatorCallbackHost::DeleteHandler()
{
    LBSLOGD(LOCATOR_CALLBACK, "before DeleteHandler");
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_handlerCb);
    m_handlerCb = nullptr;
}

void LocatorCallbackHost::DeleteSuccessHandler()
{
    LBSLOGD(LOCATOR_CALLBACK, "before DeleteSuccessHandler");
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_successHandlerCb);
    m_successHandlerCb = nullptr;
}

void LocatorCallbackHost::DeleteFailHandler()
{
    LBSLOGD(LOCATOR_CALLBACK, "before DeleteFailHandler");
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_failHandlerCb);
    m_failHandlerCb = nullptr;
}

void LocatorCallbackHost::DeleteCompleteHandler()
{
    LBSLOGD(LOCATOR_CALLBACK, "before DeleteCompleteHandler");
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_completeHandlerCb);
    m_completeHandlerCb = nullptr;
}
} // namespace Location
} // namespace OHOS

