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
#include "location_util.h"
#include "locator.h"

namespace OHOS {
namespace Location {
LocatorCallbackHost::LocatorCallbackHost()
{
    m_env = nullptr;
    m_handlerCb = nullptr;
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
            OnLocationReport(location);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool LocatorCallbackHost::Send(const std::unique_ptr<Location>& location)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);

    napi_value jsEvent = nullptr;
    if (location != nullptr) {
        napi_create_object(m_env, &jsEvent);
        LocationToJs(m_env, location, jsEvent);
    }

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "work == nullptr.");
        return false;
    }
    JsContext *context = new (std::nothrow) JsContext(m_env);
    if (context == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "context == nullptr.");
        return false;
    }
    context->m_env = m_env;
    context->m_handlerCb = m_handlerCb;
    context->m_deferred = m_deferred;
    context->m_jsEvent = jsEvent;
    work->data = context;

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            JsContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "work is nullptr!");
                return;
            }
            context = static_cast<JsContext *>(work->data);
            if (context == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "context is nullptr!");
                return;
            }
            napi_open_handle_scope(context->m_env, &scope);
            if (scope == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "scope is nullptr");
                // close handle scope, release napi_value
                napi_close_handle_scope(context->m_env, scope);
                return;
            }
            if (context->m_handlerCb != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                napi_get_undefined(context->m_env, &undefine);
                napi_get_reference_value(context->m_env, context->m_handlerCb, &handler);
                if (napi_call_function(context->m_env, nullptr, handler, 1,
                    &context->m_jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(LOCATOR_CALLBACK, "Report event failed");
                }
            } else if (context->m_deferred != nullptr) {
                if (context->m_jsEvent != nullptr) {
                    napi_resolve_deferred(context->m_env, context->m_deferred, context->m_jsEvent);
                } else {
                    napi_reject_deferred(context->m_env, context->m_deferred, context->m_jsEvent);
                }
            }
            napi_close_handle_scope(context->m_env, scope);
            delete context;
            context = nullptr;
            delete work;
            work = nullptr;
    });

    return true;
}

void LocatorCallbackHost::OnLocationReport(const std::unique_ptr<Location>& location)
{
    Send(location);
}

void LocatorCallbackHost::OnLocatingStatusChange(const int status)
{
}

void LocatorCallbackHost::OnErrorReport(const int errorCode)
{
    Send(nullptr);
}

void LocatorCallbackHost::DeleteHandler()
{
    LBSLOGD(LOCATOR_CALLBACK, "before DeleteHandler");
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_handlerCb);
    m_handlerCb = nullptr;
}
} // namespace Location
} // namespace OHOS

