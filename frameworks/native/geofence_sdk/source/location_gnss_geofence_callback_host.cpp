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
#include "location_gnss_geofence_callback_host.h"

#include "ipc_skeleton.h"
#include "napi/native_common.h"

#include "common_utils.h"
#include "location_log.h"
#include "napi_util.h"
#include "geofence_napi.h"
#include "location_async_context.h"
#include "geofence_async_context.h"

namespace OHOS {
namespace Location {
LocationGnssGeofenceCallbackHost::LocationGnssGeofenceCallbackHost()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    remoteDied_ = false;
    fenceId_ = -1;
    type_ = GNSS_GEOFENCE_OPT_TYPE_ADD;
    result_ = GNSS_GEOFENCE_OPERATION_SUCCESS;
    InitLatch();
}

LocationGnssGeofenceCallbackHost::~LocationGnssGeofenceCallbackHost()
{
    if (latch_ != nullptr) {
        delete latch_;
        latch_ = nullptr;
    }
}

void LocationGnssGeofenceCallbackHost::InitLatch()
{
    latch_ = new CountDownLatch();
    latch_->SetCount(1);
}

int LocationGnssGeofenceCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(LOCATION_GNSS_GEOFENCE_CALLBACK, "OnRemoteRequest enter");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "invalid token.");
        return -1;
    }
    if (remoteDied_) {
        LBSLOGD(LOCATION_GNSS_GEOFENCE_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }
    switch (code) {
        case RECEIVE_TRANSITION_STATUS_EVENT: {
            GeofenceTransition transition;
            transition.fenceId = data.ReadInt32();
            transition.event =
                static_cast<GeofenceTransitionEvent>(data.ReadInt32());
            OnTransitionStatusChange(transition);
            break;
        }
        case REPORT_OPERATION_RESULT_EVENT: {
            int fenceId = data.ReadInt32();
            int type = data.ReadInt32();
            int result = data.ReadInt32();
            OnReportOperationResult(fenceId, type, result);
            CountDown();
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocationGnssGeofenceCallbackHost::OnTransitionStatusChange(
    GeofenceTransition transition)
{
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    NAPI_CALL_RETURN_VOID(env_, napi_get_uv_event_loop(env_, &loop));
    if (loop == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "loop == nullptr.");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "work == nullptr.");
        return;
    }
    GnssGeofenceAsyncContext *context = new (std::nothrow) GnssGeofenceAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "context == nullptr.");
        delete work;
        return;
    }
    context->env = env_;
    context->callback[SUCCESS_CALLBACK] = handlerCb_;
    context->transition_ = transition;
    work->data = context;
    UvQueueWork(loop, work);
}

void LocationGnssGeofenceCallbackHost::OnReportOperationResult(int fenceId, int type, int result)
{
    int addValue = static_cast<int>(GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD);
    if ((type != addValue && fenceId == GetFenceId()) ||
        (type == addValue)) {
        GnssGeofenceOperateResult optResult = static_cast<GnssGeofenceOperateResult>(result);
        GnssGeofenceOperateType optType = static_cast<GnssGeofenceOperateType>(type);
        if (result == GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS &&
            optType == GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD) {
            SetFenceId(fenceId);
        }
        SetGeofenceOperationType(optType);
        SetGeofenceOperationResult(optResult);
    }
}

bool LocationGnssGeofenceCallbackHost::IsRemoteDied()
{
    return remoteDied_;
}

void LocationGnssGeofenceCallbackHost::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop, work, [](uv_work_t *work) {}, [](uv_work_t *work, int status) {
            GnssGeofenceAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<GnssGeofenceAsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            napi_value jsEvent[PARAM2];
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_object(context->env, &jsEvent[PARAM1]),
                scope, context, work);
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &jsEvent[PARAM0]),
                scope, context, work);
            GeofenceTransitionToJs(context->env, context->transition_, jsEvent[PARAM1]);
            if (context->callback[SUCCESS_CALLBACK] != nullptr) {
                napi_value undefine;
                napi_value handler = nullptr;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                    scope, context, work);
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_get_reference_value(context->env, context->callback[SUCCESS_CALLBACK], &handler),
                    scope, context, work);
                if (napi_call_function(context->env, nullptr, handler, RESULT_SIZE,
                    jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "Report event failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

void LocationGnssGeofenceCallbackHost::DeleteHandler()
{
    std::unique_lock<std::mutex> guard(mutex_);
    if (handlerCb_ == nullptr || env_ == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "handler or env is nullptr.");
        return;
    }
    auto context = new (std::nothrow) AsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "context == nullptr.");
        return;
    }
    context->env = env_;
    context->callback[SUCCESS_CALLBACK] = handlerCb_;
    DeleteQueueWork(context);
    handlerCb_ = nullptr;
}

void LocationGnssGeofenceCallbackHost::CountDown()
{
    if (latch_ != nullptr) {
        latch_->CountDown();
    }
}

void LocationGnssGeofenceCallbackHost::Wait(int time)
{
    if (latch_ != nullptr) {
        latch_->Wait(time);
    }
}

int LocationGnssGeofenceCallbackHost::GetCount()
{
    if (latch_ != nullptr) {
        return latch_->GetCount();
    }
    return 0;
}

void LocationGnssGeofenceCallbackHost::SetCount(int count)
{
    if (latch_ != nullptr) {
        return latch_->SetCount(count);
    }
}

void LocationGnssGeofenceCallbackHost::ClearFenceId()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    fenceId_ = -1;
}

int LocationGnssGeofenceCallbackHost::GetFenceId()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    return fenceId_;
}

void LocationGnssGeofenceCallbackHost::SetFenceId(int fenceId)
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    fenceId_ = fenceId;
}

GnssGeofenceOperateType LocationGnssGeofenceCallbackHost::GetGeofenceOperationType()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    return type_;
}

void LocationGnssGeofenceCallbackHost::SetGeofenceOperationType(GnssGeofenceOperateType type)
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    type_ = type;
}

GnssGeofenceOperateResult LocationGnssGeofenceCallbackHost::GetGeofenceOperationResult()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    return result_;
}

void LocationGnssGeofenceCallbackHost::SetGeofenceOperationResult(GnssGeofenceOperateResult result)
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    result_ = result;
}

LocationErrCode LocationGnssGeofenceCallbackHost::DealGeofenceOperationResult()
{
    std::unique_lock<std::mutex> guard(operationResultMutex_);
    LocationErrCode errCode = ERRCODE_SUCCESS;
    GnssGeofenceOperateResult result = result_;
    switch (result) {
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS:
            errCode = ERRCODE_SUCCESS;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_UNKNOWN:
            errCode = ERRCODE_SERVICE_UNAVAILABLE;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_TOO_MANY_GEOFENCES:
            errCode = ERRCODE_GEOFENCE_EXCEED_MAXIMUM;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_EXISTS:
            errCode = ERRCODE_SERVICE_UNAVAILABLE;
            break;
        case GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_PARAMS_INVALID:
            errCode = ERRCODE_SERVICE_UNAVAILABLE;
            break;
        default:
            break;
    }
    return errCode;
}
}  // namespace Location
}  // namespace OHOS
