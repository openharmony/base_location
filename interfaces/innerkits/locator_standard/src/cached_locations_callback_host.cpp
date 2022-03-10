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
#include "cached_locations_callback_host.h"

#include "common_utils.h"
#include "ipc_skeleton.h"
#include "lbs_log.h"
#include "location_util.h"

namespace OHOS {
namespace Location {
CachedLocationsCallbackHost::CachedLocationsCallbackHost()
{
    m_env = nullptr;
    m_handlerCb = nullptr;
    m_thisVarRef = nullptr;
    m_remoteDied = false;
}

CachedLocationsCallbackHost::~CachedLocationsCallbackHost()
{
}

int CachedLocationsCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackHost::OnRemoteRequest!");
    if (m_remoteDied) {
        LBSLOGD(CACHED_LOCATIONS_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }
    int uid = IPCSkeleton::GetCallingUid();
    if (uid > SYSTEM_UID) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "invalid uid!");
        return false;
    }
    switch (code) {
        case RECEIVE_CACHED_LOCATIONS_EVENT: {
            int size = data.ReadInt32();
            std::vector<std::unique_ptr<Location>> locations(size);
            for (int i = 0; i < size; i++) {
                locations.push_back(Location::Unmarshalling(data));
            }
            OnCacheLocationsReport(locations);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool CachedLocationsCallbackHost::IsRemoteDied()
{
    return m_remoteDied;
}

bool CachedLocationsCallbackHost::Send(const std::vector<std::unique_ptr<Location>>& locations)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(m_env, &scope);
    napi_value jsEvent = nullptr;
    napi_create_object(m_env, &jsEvent);
    LocationsToJs(m_env, locations, jsEvent);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new uv_work_t;
    if (work == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "work == nullptr.");
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
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<JsContext *>(work->data);
            if (context == nullptr) {
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "context is nullptr");
                return;
            }
            napi_open_handle_scope(context->m_env, &scope);
            if (scope == nullptr) {
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "scope is nullptr");
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
                    LBSLOGE(CACHED_LOCATIONS_CALLBACK, "Report event failed");
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

void CachedLocationsCallbackHost::OnCacheLocationsReport(const std::vector<std::unique_ptr<Location>>& locations)
{
    LBSLOGD(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackHost::OnCacheLocationsReport");
    Send(locations);
}

void CachedLocationsCallbackHost::DeleteHandler()
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    napi_delete_reference(m_env, m_handlerCb);
    m_handlerCb = nullptr;
}
}  // namespace Location
}  // namespace OHOS
