/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "fusion_fence_callback_napi.h"
#include "location_log.h"
#include "common_utils.h"
#include "fusion_fence_napi.h"
#include "napi_util.h"
#include "fusion_fence_async_context.h"

namespace OHOS {
namespace Location {
static std::mutex g_regCallbackMutex;
static std::vector<napi_ref> g_registerCallbacks;

static bool FindFusionFenceRegCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    auto iter = std::find(g_registerCallbacks.begin(), g_registerCallbacks.end(), cb);
    return iter != g_registerCallbacks.end();
}

static void DeleteFusionFenceRegCallback(napi_ref cb)
{
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    for (auto iter = g_registerCallbacks.begin(); iter != g_registerCallbacks.end(); iter++) {
        if (*iter == cb) {
            iter = g_registerCallbacks.erase(iter);
            break;
        }
    }
}

FusionFenceCallbackNapi::FusionFenceCallbackNapi()
    : env_(nullptr), handlerCb_(nullptr), remoteDied_(false),
      errorCode_(ERRCODE_SUCCESS), latch_(nullptr)
{
    LBSLOGI(FUSION_FENCE_CALLBACK, "%{public}s called", __func__);
    remoteDied_.store(false);
    InitLatch();
}

FusionFenceCallbackNapi::~FusionFenceCallbackNapi()
{
    LBSLOGI(FUSION_FENCE_CALLBACK, "%{public}s called", __func__);
    if (latch_ != nullptr) {
        delete latch_;
        latch_ = nullptr;
    }
}

bool FusionFenceCallbackNapi::IsRemoteDied()
{
    return remoteDied_;
}

void FusionFenceCallbackNapi::DeleteHandler()
{
    LBSLOGI(FUSION_FENCE_CALLBACK, "%{public}s called", __func__);
    std::unique_lock<std::mutex> lock(mutex_);
    if (env_ != nullptr && handlerCb_ != nullptr) {
        DeleteFusionFenceRegCallback(handlerCb_);
        napi_delete_reference(env_, handlerCb_);
        handlerCb_ = nullptr;
    }
}

std::string FusionFenceCallbackNapi::GetIdentifier()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return identifier_;
}

void FusionFenceCallbackNapi::SetIdentifier(const std::string& identifier)
{
    std::unique_lock<std::mutex> lock(mutex_);
    identifier_ = identifier;
}

LocationErrCode FusionFenceCallbackNapi::GetLastOperationErrorCode()
{
    std::unique_lock<std::mutex> lock(operationResultMutex_);
    if (errorCode_ == ERRCODE_SUCCESS) {
        LBSLOGI(FUSION_FENCE_CALLBACK, "Operation result is SUCCESS");
        return ERRCODE_SUCCESS;
    }
    LBSLOGE(FUSION_FENCE_CALLBACK, "Operation result is FAIL, errorCode: %{public}d", errorCode_);
    return errorCode_;
}

void FusionFenceCallbackNapi::OnTransitionStatusChange(FusionFenceTransition transition)
{
    LBSLOGI(FUSION_FENCE_CALLBACK, "identifier: %{public}s, transitionEvent: %{public}d",
        transition.identifier.c_str(), static_cast<int>(transition.transitionEvent));
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    NAPI_CALL_RETURN_VOID(env_, napi_get_uv_event_loop(env_, &loop));
    if (loop == nullptr) {
        LBSLOGE(FUSION_FENCE_CALLBACK, "loop == nullptr.");
        return;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(FUSION_FENCE_CALLBACK, "handler is nullptr.");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(FUSION_FENCE_CALLBACK, "work == nullptr.");
        return;
    }
    FusionFenceAsyncContext *context = new (std::nothrow) FusionFenceAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(FUSION_FENCE_CALLBACK, "context == nullptr.");
        delete work;
        return;
    }
    context->transition_ = transition;
    context->callbackHost_ = this;
    work->data = context;
    UvQueueWork(loop, work);
}

void FusionFenceCallbackNapi::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work_internal(
        loop, work, [](uv_work_t *work) {}, [](uv_work_t *work, int status) {
            FusionFenceAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(FUSION_FENCE_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<FusionFenceAsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr) {
                LBSLOGE(FUSION_FENCE_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(FUSION_FENCE_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            napi_ref handlerCb = context->callbackHost_->GetHandleCb();
            if (!FindFusionFenceRegCallback(handlerCb)) {
                LBSLOGE(FUSION_FENCE_CALLBACK, "no valid callback");
                NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
                delete context;
                delete work;
                return;
            }
            napi_value transitionObj = CreateFusionFenceTransitionJsObj(context->env, context->transition_);
            napi_value handler = nullptr;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_reference_value(
                context->env, handlerCb, &handler), scope, context, work);
            if (handler != nullptr) {
                napi_value result;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_call_function(
                    context->env, nullptr, handler, 1, &transitionObj, &result), scope, context, work);
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
            return;
        }, "fenceTransitionCallback");
}

napi_ref FusionFenceCallbackNapi::GetHandleCb()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return handlerCb_;
}

void FusionFenceCallbackNapi::SetHandleCb(const napi_ref& handlerCb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        handlerCb_ = handlerCb;
    }
    std::unique_lock<std::mutex> guard(g_regCallbackMutex);
    g_registerCallbacks.emplace_back(handlerCb);
}

napi_env FusionFenceCallbackNapi::GetEnv()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return env_;
}

void FusionFenceCallbackNapi::SetEnv(const napi_env& env)
{
    std::unique_lock<std::mutex> lock(mutex_);
    env_ = env;
}

int FusionFenceCallbackNapi::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGI(FUSION_FENCE_CALLBACK, "%{public}s called, code: %{public}d", __func__, code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(FUSION_FENCE_CALLBACK, "%{public}s: token error", __func__);
        return -1;
    }
    if (IsRemoteDied()) {
        LBSLOGE(FUSION_FENCE_CALLBACK, "%{public}s: remote died", __func__);
        return -1;
    }
    switch (code) {
        case static_cast<uint32_t>(FusionFenceInterfaceCode::ON_TRANSITION_STATUS_CHANGE): {
            auto transition = FusionFenceTransition::Unmarshalling(data);
            if (transition != nullptr) {
                OnTransitionStatusChange(*transition);
                delete transition;
            }
            break;
        }
        case static_cast<uint32_t>(FusionFenceInterfaceCode::REPORT_OPERATE_RESULT): {
            LBSLOGI(FUSION_FENCE_CALLBACK, "REPORT_OPERATE_RESULT received");
            std::string fenceId = data.ReadString();
            int32_t type = data.ReadInt32();
            int32_t errorCode = data.ReadInt32();
            OnReportOperateResult(fenceId, type, errorCode);
            CountDown();
            break;
        }
        default:
            LBSLOGE(FUSION_FENCE_CALLBACK,
                "%{public}s: unknown code: %{public}d, calling base OnRemoteRequest", __func__, code);
            return IRemoteStub<IFusionFenceCallback>::OnRemoteRequest(code, data, reply, option);
    }
    return 0;
}

void FusionFenceCallbackNapi::OnReportOperateResult(const std::string& fenceId, int type, int errorCode)
{
    LBSLOGI(FUSION_FENCE_CALLBACK, "fenceId: %{public}s, type: %{public}d, errorCode: %{public}d",
        fenceId.c_str(), type, errorCode);
    std::unique_lock<std::mutex> lock(operationResultMutex_);
    errorCode_ = static_cast<LocationErrCode>(errorCode);
}

void FusionFenceCallbackNapi::InitLatch()
{
    latch_ = new (std::nothrow) CountDownLatch();
    if (latch_ != nullptr) {
        latch_->SetCount(1);
    }
}

int FusionFenceCallbackNapi::GetCount() const
{
    if (latch_ != nullptr) {
        return latch_->GetCount();
    }
    return -1;
}

void FusionFenceCallbackNapi::SetCount(int count)
{
    if (latch_ != nullptr) {
        latch_->SetCount(count);
    }
}

void FusionFenceCallbackNapi::ResetLatch()
{
    if (latch_ != nullptr) {
        latch_->SetCount(1);
    }
}

void FusionFenceCallbackNapi::CountDown()
{
    if (latch_ != nullptr) {
        latch_->CountDown();
    }
}

void FusionFenceCallbackNapi::Wait(int timeMs)
{
    if (latch_ != nullptr) {
        latch_->Wait(timeMs);
    }
}
} // namespace Location
} // namespace OHOS