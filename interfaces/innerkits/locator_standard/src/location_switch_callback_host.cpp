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

#include "location_switch_callback_host.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "i_switch_callback.h"
#include "lbs_log.h"

namespace OHOS {
namespace Location {
LocationSwitchCallbackHost::LocationSwitchCallbackHost()
{
    m_env = nullptr;
    m_handlerCb = nullptr;
    m_thisVarRef = nullptr;
    m_remoteDied = false;
    m_fixNumber = 0;
    m_lastCallingPid = 0;
    m_lastCallingUid = 0;
}

LocationSwitchCallbackHost::~LocationSwitchCallbackHost()
{
}

int LocationSwitchCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(SWITCH_CALLBACK, "LocatorCallbackHost::OnRemoteRequest!");
    if (m_remoteDied) {
        LBSLOGD(SWITCH_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }
    int uid = IPCSkeleton::GetCallingUid();
    if (uid > SYSTEM_UID) {
        LBSLOGE(SWITCH_CALLBACK, "invalid uid!");
        return false;
    }
    switch (code) {
        case RECEIVE_SWITCH_STATE_EVENT: {
            OnSwitchChange(data.ReadInt32());
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool LocationSwitchCallbackHost::IsRemoteDied()
{
    return m_remoteDied;
}

napi_value LocationSwitchCallbackHost::PackResult(int switchState)
{
    napi_value result;
    napi_get_boolean(m_env, (switchState == 1), &result);
    return result;
}

bool LocationSwitchCallbackHost::Send(int switchState)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);

    napi_value jsEvent = PackResult(switchState);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(SWITCH_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        LBSLOGE(SWITCH_CALLBACK, "work == nullptr.");
        return false;
    }

    JsContext *context = new (std::nothrow) JsContext(m_env);
    context->m_env = m_env;
    context->m_thisVarRef = m_thisVarRef;
    context->m_handlerCb = m_handlerCb;
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
                LBSLOGE(SWITCH_CALLBACK, "scope is nullptr");
                // close handle scope, release napi_value
                napi_close_handle_scope(context->m_env, scope);
                return;
            }
            if (context->m_handlerCb != nullptr) {
                napi_value thisVar = nullptr;
                napi_get_reference_value(context->m_env, context->m_thisVarRef, &thisVar);
                napi_value undefine;
                napi_value handler = nullptr;
                napi_get_undefined(context->m_env, &undefine);
                napi_get_reference_value(context->m_env, context->m_handlerCb, &handler);
                if (napi_call_function(context->m_env, thisVar, handler, 1,
                    &context->m_jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(SWITCH_CALLBACK, "Report event failed");
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

void LocationSwitchCallbackHost::OnSwitchChange(int switchState)
{
    LBSLOGD(SWITCH_CALLBACK, "LocatorCallbackHost::OnSwitchChange");
    Send(switchState);
}

void LocationSwitchCallbackHost::DeleteHandler()
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_handlerCb);
    m_handlerCb = nullptr;
}
}  // namespace Location
}  // namespace OHOS
