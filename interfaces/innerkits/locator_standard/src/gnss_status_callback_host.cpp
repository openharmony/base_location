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
#include "gnss_status_callback_host.h"

#include "common_utils.h"
#include "ipc_skeleton.h"
#include "lbs_log.h"
#include "location_util.h"

namespace OHOS {
namespace Location {
GnssStatusCallbackHost::GnssStatusCallbackHost()
{
    m_env = nullptr;
    m_handlerCb = nullptr;
    m_remoteDied = false;
    m_lastCallingPid = 0;
    m_lastCallingUid = 0;
}

GnssStatusCallbackHost::~GnssStatusCallbackHost()
{
}

int GnssStatusCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(GNSS_STATUS_CALLBACK, "GnssStatusCallbackHost::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "invalid token.");
        return -1;
    }
    if (m_remoteDied) {
        LBSLOGD(GNSS_STATUS_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }
    int uid = IPCSkeleton::GetCallingUid();
    if (uid > SYSTEM_UID) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "invalid uid!");
        return false;
    }
    switch (code) {
        case RECEIVE_STATUS_INFO_EVENT: {
            std::unique_ptr<SatelliteStatus> statusInfo = SatelliteStatus::Unmarshalling(data);
            OnStatusChange(statusInfo);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool GnssStatusCallbackHost::IsRemoteDied()
{
    return m_remoteDied;
}

bool GnssStatusCallbackHost::Send(const std::unique_ptr<SatelliteStatus>& statusInfo)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(m_env, &scope);

    napi_value jsEvent = nullptr;
    if (statusInfo != nullptr) {
        napi_create_object(m_env, &jsEvent);
        SatelliteStatusToJs(m_env, statusInfo, jsEvent);
    }

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "work == nullptr.");
        return false;
    }
    JsContext *context = new (std::nothrow) JsContext(m_env);
    if (context == nullptr) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "context == nullptr.");
        return false;
    }
    context->m_env = m_env;
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
                LBSLOGE(GNSS_STATUS_CALLBACK, "scope is nullptr");
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
                    LBSLOGE(GNSS_STATUS_CALLBACK, "Report event failed");
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

void GnssStatusCallbackHost::OnStatusChange(const std::unique_ptr<SatelliteStatus>& statusInfo)
{
    LBSLOGD(GNSS_STATUS_CALLBACK, "GnssStatusCallbackHost::OnStatusChange");
    Send(statusInfo);
}

void GnssStatusCallbackHost::DeleteHandler()
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_handlerCb);
    m_handlerCb = nullptr;
}
}  // namespace Location
}  // namespace OHOS
