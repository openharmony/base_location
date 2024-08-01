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
#include "locating_required_data_callback_napi.h"

#include "ipc_skeleton.h"
#include "napi/native_common.h"

#include "common_utils.h"
#include "location_log.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
LocatingRequiredDataCallbackNapi::LocatingRequiredDataCallbackNapi()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    remoteDied_ = false;
    fixNumber_ = 0;
    InitLatch();
    callbackValid_ = false;
}

LocatingRequiredDataCallbackNapi::~LocatingRequiredDataCallbackNapi()
{
    if (latch_ != nullptr) {
        delete latch_;
        latch_ = nullptr;
    }
}

void LocatingRequiredDataCallbackNapi::InitLatch()
{
    latch_ = new CountDownLatch();
    latch_->SetCount(1);
}

int LocatingRequiredDataCallbackNapi::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(LOCATING_DATA_CALLBACK, "LocatingRequiredDataCallbackNapi::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "invalid token.");
        return -1;
    }
    if (remoteDied_) {
        LBSLOGD(LOCATING_DATA_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }

    switch (code) {
        case RECEIVE_INFO_EVENT: {
            int cnt = data.ReadInt32();
            if (cnt > 0 && cnt <= MAXIMUM_LOCATING_REQUIRED_DATAS) {
                std::vector<std::shared_ptr<LocatingRequiredData>> res;
                for (int i = 0; i < cnt; i++) {
                    res.push_back(LocatingRequiredData::Unmarshalling(data));
                }
                // update wifi info
                if (res[0]->GetType() == LocatingRequiredDataType::WIFI) {
                    SetSingleResult(res);
                }
                OnLocatingDataChange(res);
                CountDown();
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

bool LocatingRequiredDataCallbackNapi::IsRemoteDied()
{
    return remoteDied_;
}

bool LocatingRequiredDataCallbackNapi::Send(const std::vector<std::shared_ptr<LocatingRequiredData>>& data)
{
    if (IsSingleLocationRequest()) {
        return false;
    }
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    NAPI_CALL_BASE(env_, napi_get_uv_event_loop(env_, &loop), false);
    if (loop == nullptr) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "loop == nullptr.");
        return false;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "handler is nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "work == nullptr.");
        return false;
    }
    LocatingRequiredDataAsyncContext *context = new (std::nothrow) LocatingRequiredDataAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "context == nullptr.");
        delete work;
        return false;
    }
    if (!InitContext(context)) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "InitContext fail");
        return false;
    }
    context->locatingRequiredDataList_ = data;
    work->data = context;
    UvQueueWork(loop, work);
    return true;
}

void LocatingRequiredDataCallbackNapi::UvQueueWork(uv_loop_s* loop, uv_work_t* work)
{
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            LocatingRequiredDataAsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATING_DATA_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<LocatingRequiredDataAsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr || context->callbackValid == nullptr) {
                LBSLOGE(LOCATING_DATA_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(LOCATING_DATA_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            napi_value jsEvent = nullptr;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                napi_create_array_with_length(context->env, context->locatingRequiredDataList_.size(), &jsEvent),
                scope, context, work);
            LocatingRequiredDataToJsObj(context->env, context->locatingRequiredDataList_, jsEvent);
            if (context->callback[0] != nullptr && *(context->callbackValid)) {
                napi_value undefine;
                napi_value handler = nullptr;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                    scope, context, work);
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_get_reference_value(context->env, context->callback[0], &handler), scope, context, work);
                if (napi_call_function(context->env, nullptr, handler, 1,
                    &jsEvent, &undefine) != napi_ok) {
                    LBSLOGE(LOCATING_DATA_CALLBACK, "Report event failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

void LocatingRequiredDataCallbackNapi::OnLocatingDataChange(
    const std::vector<std::shared_ptr<LocatingRequiredData>>& data)
{
    LBSLOGD(LOCATING_DATA_CALLBACK, "LocatingRequiredDataCallbackNapi::OnLocatingDataChange");
    Send(data);
}

void LocatingRequiredDataCallbackNapi::DeleteHandler()
{
    std::unique_lock<std::mutex> guard(mutex_);
    if (handlerCb_ == nullptr || env_ == nullptr) {
        LBSLOGE(LOCATING_DATA_CALLBACK, "handler or env is nullptr.");
        return;
    }
    NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, handlerCb_));
    handlerCb_ = nullptr;
    callbackValid_ = false;
}

bool LocatingRequiredDataCallbackNapi::IsSingleLocationRequest()
{
    return (fixNumber_ == 1);
}

void LocatingRequiredDataCallbackNapi::CountDown()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->CountDown();
    }
}

void LocatingRequiredDataCallbackNapi::Wait(int time)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->Wait(time);
    }
}

int LocatingRequiredDataCallbackNapi::GetCount()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->GetCount();
    }
    return 0;
}

void LocatingRequiredDataCallbackNapi::SetCount(int count)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->SetCount(count);
    }
}

void LocatingRequiredDataCallbackNapi::ClearSingleResult()
{
    std::unique_lock<std::mutex> guard(singleResultMutex_);
    singleResult_.clear();
}

void LocatingRequiredDataCallbackNapi::SetSingleResult(
    std::vector<std::shared_ptr<LocatingRequiredData>> singleResult)
{
    std::unique_lock<std::mutex> guard(singleResultMutex_);
    singleResult_.assign(singleResult.begin(), singleResult.end());
}
}  // namespace Location
}  // namespace OHOS
