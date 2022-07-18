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
#include "location_log.h"
#include "location_napi_adapter.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
CachedLocationsCallbackHost::CachedLocationsCallbackHost()
{
    m_env = nullptr;
    m_handlerCb = nullptr;
    m_remoteDied = false;
}

CachedLocationsCallbackHost::~CachedLocationsCallbackHost()
{
}

int CachedLocationsCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackHost::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "invalid token.");
        return -1;
    }
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
            std::vector<std::shared_ptr<Location>> locations(size);
            for (int i = 0; i < size; i++) {
                locations.push_back(Location::UnmarshallingShared(data));
            }
            Send(locations);
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

bool CachedLocationsCallbackHost::Send(std::vector<std::shared_ptr<Location>>& locations)
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(m_env, &loop);
    if (loop == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "work == nullptr.");
        return false;
    }
    CachedLocationAsyncContext *context = new (std::nothrow) CachedLocationAsyncContext(m_env);
    if (context == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "context == nullptr.");
        return false;
    }
    context->env = m_env;
    context->callback[SUCCESS_CALLBACK] = m_handlerCb;
    context->locationList = locations;
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}

void CachedLocationsCallbackHost::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            CachedLocationAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<CachedLocationAsyncContext *>(work->data);
            if (context == nullptr) {
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            napi_value jsEvent = nullptr;
            napi_create_object(context->env, &jsEvent);
            LocationsToJs(context->env, context->locationList, jsEvent);
            if (context->callback[0] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                napi_get_undefined(context->env, &undefine);
                napi_get_reference_value(context->env, context->callback[0], &handler);
                if (napi_call_function(context->env, nullptr, handler, 1,
                    &jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(CACHED_LOCATIONS_CALLBACK, "Report event failed");
                }
            }
            napi_close_handle_scope(context->env, scope);
            delete context;
            delete work;
    });
}

void CachedLocationsCallbackHost::OnCacheLocationsReport(const std::vector<std::unique_ptr<Location>>& locations)
{
    LBSLOGD(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackHost::OnCacheLocationsReport");
}

void CachedLocationsCallbackHost::DeleteHandler()
{
    std::shared_lock<std::shared_mutex> guard(m_mutex);
    if (m_handlerCb) {
        napi_delete_reference(m_env, m_handlerCb);
        m_handlerCb = nullptr;
    }
}
}  // namespace Location
}  // namespace OHOS
