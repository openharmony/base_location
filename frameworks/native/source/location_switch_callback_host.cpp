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
#include "location_log.h"
#include "location_napi_adapter.h"

namespace OHOS {
namespace Location {
LocationSwitchCallbackHost::LocationSwitchCallbackHost()
{
    m_env = nullptr;
    m_handlerCb = nullptr;
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
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(SWITCH_CALLBACK, "invalid token.");
        return -1;
    }
    if (m_remoteDied) {
        LBSLOGD(SWITCH_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
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

napi_value LocationSwitchCallbackHost::PackResult(bool switchState)
{
    napi_value result;
    napi_get_boolean(m_env, switchState, &result);
    return result;
}

bool LocationSwitchCallbackHost::Send(int switchState)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(SWITCH_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(SWITCH_CALLBACK, "work == nullptr.");
        return false;
    }
    SwitchAsyncContext *context = new (std::nothrow) SwitchAsyncContext(m_env);
    if (context == nullptr) {
        LBSLOGE(SWITCH_CALLBACK, "context == nullptr.");
        return false;
    }
    context->env = m_env;
    context->callback[SUCCESS_CALLBACK] = m_handlerCb;
    context->enable = (switchState == 1 ? true : false);
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}

void LocationSwitchCallbackHost::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            SwitchAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "work is nullptr!");
                return;
            }
            context = static_cast<SwitchAsyncContext *>(work->data);
            if (context == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "context is nullptr!");
                delete work;
                return;
            }
            napi_open_handle_scope(context->env, &scope);
            napi_value jsEvent;
            napi_get_boolean(context->env, context->enable, &jsEvent);
            if (scope == nullptr) {
                LBSLOGE(SWITCH_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            if (context->callback[0] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                napi_get_undefined(context->env, &undefine);
                napi_get_reference_value(context->env, context->callback[0], &handler);
                if (napi_call_function(context->env, nullptr, handler, 1,
                    &jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(SWITCH_CALLBACK, "Report event failed");
                }
            }
            napi_close_handle_scope(context->env, scope);
            delete context;
            delete work;
    });
}

void LocationSwitchCallbackHost::OnSwitchChange(int switchState)
{
    LBSLOGD(SWITCH_CALLBACK, "LocatorCallbackHost::OnSwitchChange");
    Send(switchState);
}

void LocationSwitchCallbackHost::DeleteHandler()
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    if (m_handlerCb) {
        napi_delete_reference(m_env, m_handlerCb);
        m_handlerCb = nullptr;
    }
}
}  // namespace Location
}  // namespace OHOS
