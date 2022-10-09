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
#include "location_log.h"
#include "location_napi_adapter.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
GnssStatusCallbackHost::GnssStatusCallbackHost()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    remoteDied_ = false;
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
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != LOCATOR_UID) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "uid pid not match locationhub process.");
        return REPLY_CODE_EXCEPTION;
    }
    if (remoteDied_) {
        LBSLOGD(GNSS_STATUS_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }

    switch (code) {
        case RECEIVE_STATUS_INFO_EVENT: {
            std::unique_ptr<SatelliteStatus> statusInfo = SatelliteStatus::Unmarshalling(data);
            Send(statusInfo);
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
    return remoteDied_;
}

bool GnssStatusCallbackHost::Send(std::unique_ptr<SatelliteStatus>& statusInfo)
{
    std::shared_lock<std::shared_mutex> guard(mutex_);

    uv_loop_s *loop = nullptr;
    NAPI_CALL_BASE(env_, napi_get_uv_event_loop(env_, &loop), false);
    if (loop == nullptr) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "work == nullptr.");
        return false;
    }
    GnssStatusAsyncContext *context = new (std::nothrow) GnssStatusAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "context == nullptr.");
        return false;
    }
    context->env = env_;
    context->callback[SUCCESS_CALLBACK] = handlerCb_;
    context->statusInfo = std::move(statusInfo);
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}

void GnssStatusCallbackHost::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GnssStatusAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "work is nullptr!");
                return;
            }
            context = static_cast<GnssStatusAsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "context is nullptr!");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(GNSS_STATUS_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            napi_value jsEvent = nullptr;
            if (context->statusInfo != nullptr) {
                NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &jsEvent));
                SatelliteStatusToJs(context->env, context->statusInfo, jsEvent);
            }
            if (context->callback[0] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                NAPI_CALL_RETURN_VOID(context->env, napi_get_undefined(context->env, &undefine));
                NAPI_CALL_RETURN_VOID(context->env,
                    napi_get_reference_value(context->env, context->callback[0], &handler));
                if (napi_call_function(context->env, nullptr, handler, 1,
                    &jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(GNSS_STATUS_CALLBACK, "Report event failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

void GnssStatusCallbackHost::OnStatusChange(const std::unique_ptr<SatelliteStatus>& statusInfo)
{
    LBSLOGD(GNSS_STATUS_CALLBACK, "GnssStatusCallbackHost::OnStatusChange");
}

void GnssStatusCallbackHost::DeleteHandler()
{
    std::shared_lock<std::shared_mutex> guard(mutex_);
    auto context = new (std::nothrow) AsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(GNSS_STATUS_CALLBACK, "context == nullptr.");
        return;
    }
    context->env = env_;
    context->callback[SUCCESS_CALLBACK] = handlerCb_;
    if (handlerCb_ && env_) {
        DeleteQueueWork(context);
        handlerCb_ = nullptr;
    }
}
}  // namespace Location
}  // namespace OHOS
