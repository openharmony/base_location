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
#include "cached_locations_callback_napi.h"

#include "ipc_skeleton.h"
#include "napi/native_common.h"

#include "common_utils.h"
#include "location_log.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
static std::mutex g_mutex;
CachedLocationsCallbackNapi::CachedLocationsCallbackNapi()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    remoteDied_ = false;
}

CachedLocationsCallbackNapi::~CachedLocationsCallbackNapi()
{
}

int CachedLocationsCallbackNapi::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackNapi::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "invalid token.");
        return -1;
    }
    if (remoteDied_) {
        LBSLOGD(CACHED_LOCATIONS_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }

    switch (code) {
        case RECEIVE_CACHED_LOCATIONS_EVENT: {
            int size = data.ReadInt32();
            if (size > 0 && size < MAXIMUM_CACHE_LOCATIONS) {
                std::vector<std::unique_ptr<Location>> locations(size);
                for (int i = 0; i < size; i++) {
                    locations.push_back(Location::Unmarshalling(data));
                }
                Send(locations);
            }
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool CachedLocationsCallbackNapi::IsRemoteDied()
{
    return remoteDied_;
}

bool CachedLocationsCallbackNapi::Send(std::vector<std::unique_ptr<Location>>& locations)
{
    std::unique_lock<std::mutex> guard(g_mutex);
    uv_loop_s *loop = nullptr;
    NAPI_CALL_BASE(env_, napi_get_uv_event_loop(env_, &loop), false);
    if (loop == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "loop == nullptr.");
        return false;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "handler is nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "work == nullptr.");
        return false;
    }
    CachedLocationAsyncContext *context = new (std::nothrow) CachedLocationAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "context == nullptr.");
        delete work;
        return false;
    }
    if (!InitContext(context)) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "InitContext fail");
        return false;
    }
    for (std::unique_ptr<Location>& location : locations) {
        context->locationList.emplace_back(std::move(location));
    }
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}

void CachedLocationsCallbackNapi::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
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
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(CACHED_LOCATIONS_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            std::unique_lock<std::mutex> guard(g_mutex);
            napi_value jsEvent = nullptr;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_object(context->env, &jsEvent),
                scope, context, work);
            LocationsToJs(context->env, context->locationList, jsEvent);
            if (context->callback[0] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                    scope, context, work);
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_get_reference_value(context->env, context->callback[0], &handler), scope, context, work);
                if (napi_call_function(context->env, nullptr, handler, 1,
                    &jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(CACHED_LOCATIONS_CALLBACK, "Report event failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            uint32_t refCount = INVALID_REF_COUNT;
            napi_reference_unref(context->env, context->callback[0], &refCount);
            if (refCount == 0) {
                NAPI_CALL_RETURN_VOID(context->env, napi_delete_reference(context->env, context->callback[0]));
            }
            delete context;
            delete work;
    });
}

void CachedLocationsCallbackNapi::OnCacheLocationsReport(const std::vector<std::unique_ptr<Location>>& locations)
{
    LBSLOGD(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackNapi::OnCacheLocationsReport");
}

void CachedLocationsCallbackNapi::DeleteHandler()
{
    std::unique_lock<std::mutex> guard(g_mutex);
    if (handlerCb_ == nullptr || env_ == nullptr) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "handler or env is nullptr.");
        return;
    }
    uint32_t refCount = INVALID_REF_COUNT;
    napi_reference_unref(env_, handlerCb_, &refCount);
    if (refCount == 0) {
        NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, handlerCb_));
    }
    handlerCb_ = nullptr;
}
}  // namespace Location
}  // namespace OHOS
