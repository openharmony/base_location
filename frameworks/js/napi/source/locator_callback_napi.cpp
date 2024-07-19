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

#include "locator_callback_napi.h"

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "js_native_api.h"
#include "message_option.h"
#include "message_parcel.h"
#include "napi/native_common.h"
#include "uv.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator_callback.h"
#include "location.h"
#include "location_async_context.h"
#include "location_log.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
LocatorCallbackNapi::LocatorCallbackNapi()
{
    env_ = nullptr;
    handlerCb_ = nullptr;
    successHandlerCb_ = nullptr;
    failHandlerCb_ = nullptr;
    completeHandlerCb_ = nullptr;
    fixNumber_ = 0;
    inHdArea_ = true;
    singleLocation_ = nullptr;
    callbackValid_ = false;
	locationPriority_ = 0;
    InitLatch();
}

void LocatorCallbackNapi::InitLatch()
{
    latch_ = new CountDownLatch();
    latch_->SetCount(1);
}

LocatorCallbackNapi::~LocatorCallbackNapi()
{
    delete latch_;
}

int LocatorCallbackNapi::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_LOCATION_INFO_EVENT: {
            std::unique_ptr<Location> location = Location::Unmarshalling(data);
            OnLocationReport(location);
            if (location->GetLocationSourceType() == LocationSourceType::NETWORK_TYPE &&
                location->GetAdditionsMap()["inHdArea"] != "") {
                inHdArea_ = (location->GetAdditionsMap()["inHdArea"] == "true");
            }
            if (NeedSetSingleLocation(location)) {
                SetSingleLocation(location);
            }
            if (IfReportAccuracyLocation()) {
                CountDown();
            }
            break;
        }
        case RECEIVE_LOCATION_STATUS_EVENT: {
            int status = data.ReadInt32();
            OnLocatingStatusChange(status);
            break;
        }
        case RECEIVE_ERROR_INFO_EVENT: {
            int errorCode = data.ReadInt32();
            LBSLOGI(LOCATOR_STANDARD, "CallbackSutb receive ERROR_EVENT. errorCode:%{public}d", errorCode);
            if (errorCode == LOCATING_FAILED_INTERNET_ACCESS_FAILURE) {
                inHdArea_ = false;
                if (GetSingleLocation() != nullptr) {
                    CountDown();
                }
            } else {
                OnErrorReport(errorCode);
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

void LocatorCallbackNapi::DoSendWork(uv_loop_s*& loop, uv_work_t*& work)
{
    uv_queue_work(loop, work, [](uv_work_t* work) {}, [](uv_work_t* work, int status) {
        if (work == nullptr) {
            return;
        }
        napi_handle_scope scope = nullptr;
        auto context = static_cast<LocationAsyncContext*>(work->data);
        if (context == nullptr) {
            delete work;
            return;
        }
        if (context->env == nullptr || context->loc == nullptr || context->callbackValid == nullptr) {
            delete context;
            delete work;
            return;
        }
        napi_open_handle_scope(context->env, &scope);
        if (scope == nullptr) {
            DELETE_SCOPE_CONTEXT_WORK(context->env, scope, context, work);
            return;
        }
        napi_value jsEvent = nullptr;
        CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_create_object(context->env, &jsEvent), scope, context, work);
        if (context->callback[1]) {
            SystemLocationToJs(context->env, context->loc, jsEvent);
        } else {
            LocationToJs(context->env, context->loc, jsEvent);
        }
        if (context->callback[0] != nullptr && *(context->callbackValid)) {
            napi_value undefine = nullptr;
            napi_value handler = nullptr;
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                scope, context, work);
            CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                napi_get_reference_value(context->env, context->callback[0], &handler), scope, context, work);
            if (napi_call_function(context->env, nullptr, handler, 1, &jsEvent, &undefine) != napi_ok) {
                LBSLOGE(LOCATOR_CALLBACK, "Report location failed");
            }
        }
        DELETE_SCOPE_CONTEXT_WORK(context->env, scope, context, work);
    });
}

void LocatorCallbackNapi::DoSendErrorCode(uv_loop_s *&loop, uv_work_t *&work)
{
    uv_queue_work(loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            AsyncContext *context = nullptr;
            napi_handle_scope scope = nullptr;
            if (work == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "work is nullptr");
                return;
            }
            context = static_cast<AsyncContext *>(work->data);
            if (context == nullptr || context->env == nullptr || context->callbackValid == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "context is nullptr");
                delete work;
                return;
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_open_handle_scope(context->env, &scope));
            if (scope == nullptr) {
                LBSLOGE(LOCATOR_CALLBACK, "scope is nullptr");
                delete context;
                delete work;
                return;
            }
            if (context->callback[FAIL_CALLBACK] != nullptr && *(context->callbackValid)) {
                napi_value undefine;
                napi_value handler = nullptr;
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env, napi_get_undefined(context->env, &undefine),
                    scope, context, work);
                CHK_NAPI_ERR_CLOSE_SCOPE(context->env,
                    napi_get_reference_value(context->env, context->callback[FAIL_CALLBACK], &handler),
                    scope, context, work);
                std::string msg = GetErrorMsgByCode(context->errCode);
                CreateFailCallBackParams(*context, msg, context->errCode);
                if (napi_call_function(context->env, nullptr, handler, RESULT_SIZE,
                    context->result, &undefine) != napi_ok) {
                    LBSLOGE(LOCATOR_CALLBACK, "Report system error failed");
                }
            }
            NAPI_CALL_RETURN_VOID(context->env, napi_close_handle_scope(context->env, scope));
            delete context;
            delete work;
    });
}

bool LocatorCallbackNapi::SendErrorCode(const int& errorCode)
{
    std::unique_lock<std::mutex> guard(mutex_);
    if (!IsSystemGeoLocationApi() && !IsSingleLocationRequest()) {
        LBSLOGE(LOCATOR_CALLBACK, "this is Callback type,cant send error msg.");
        return false;
    }
    if (env_ == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "env_ is nullptr.");
        return false;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "handler is nullptr.");
        return false;
    }
    uv_loop_s *loop = nullptr;
    NAPI_CALL_BASE(env_, napi_get_uv_event_loop(env_, &loop), false);
    if (loop == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "loop == nullptr.");
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "work == nullptr.");
        return false;
    }
    AsyncContext *context = new (std::nothrow) AsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "context == nullptr.");
        delete work;
        return false;
    }
    if (!InitContext(context)) {
        LBSLOGE(LOCATOR_CALLBACK, "InitContext fail");
    }
    context->errCode = errorCode;
    work->data = context;
    DoSendErrorCode(loop, work);
    return true;
}

void LocatorCallbackNapi::OnLocationReport(const std::unique_ptr<Location>& location)
{
    std::unique_lock<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    if (env_ == nullptr) {
        LBSLOGD(LOCATOR_CALLBACK, "env_ is nullptr.");
        return;
    }
    if (handlerCb_ == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "handler is nullptr.");
        return;
    }
    NAPI_CALL_RETURN_VOID(env_, napi_get_uv_event_loop(env_, &loop));
    if (loop == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "loop == nullptr.");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "work == nullptr.");
        return;
    }
    auto context = new (std::nothrow) LocationAsyncContext(env_);
    if (context == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "context == nullptr.");
        delete work;
        return;
    }
    if (!InitContext(context)) {
        LBSLOGE(LOCATOR_CALLBACK, "InitContext fail");
    }
    context->loc = std::make_unique<Location>(*location);
    work->data = context;
    DoSendWork(loop, work);
}

void LocatorCallbackNapi::OnLocatingStatusChange(const int status)
{
}

void LocatorCallbackNapi::OnErrorReport(const int errorCode)
{
    SendErrorCode(errorCode);
}

void LocatorCallbackNapi::DeleteAllCallbacks()
{
    DeleteHandler();
}

void LocatorCallbackNapi::DeleteHandler()
{
    LBSLOGD(LOCATOR_CALLBACK, "before DeleteHandler");
    std::unique_lock<std::mutex> guard(mutex_);
    if (env_ == nullptr) {
        LBSLOGE(LOCATOR_CALLBACK, "env is nullptr.");
        return;
    }
    if (IsSystemGeoLocationApi()) {
        if (successHandlerCb_ != nullptr) {
            NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, successHandlerCb_));
            successHandlerCb_ = nullptr;
        }
        if (failHandlerCb_ != nullptr) {
            NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, failHandlerCb_));
            failHandlerCb_ = nullptr;
        }
        if (completeHandlerCb_ != nullptr) {
            NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, completeHandlerCb_));
            completeHandlerCb_ = nullptr;
        }
    } else {
        if (handlerCb_ != nullptr) {
            NAPI_CALL_RETURN_VOID(env_, napi_delete_reference(env_, handlerCb_));
            handlerCb_ = nullptr;
        }
    }
    callbackValid_ = false;
}

bool LocatorCallbackNapi::IsSystemGeoLocationApi()
{
    return (successHandlerCb_ != nullptr) ? true : false;
}

bool LocatorCallbackNapi::IsSingleLocationRequest()
{
    return (fixNumber_ == 1);
}

void LocatorCallbackNapi::CountDown()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->CountDown();
    }
}

void LocatorCallbackNapi::Wait(int time)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->Wait(time);
    }
}

int LocatorCallbackNapi::GetCount()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->GetCount();
    }
    return 0;
}

void LocatorCallbackNapi::SetCount(int count)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->SetCount(count);
    }
}

bool LocatorCallbackNapi::NeedSetSingleLocation(const std::unique_ptr<Location>& location)
{
    if (locationPriority_ == LOCATION_PRIORITY_ACCURACY &&
        singleLocation_ != nullptr &&
        location->GetLocationSourceType() == LocationSourceType::NETWORK_TYPE) {
        return false;
    } else {
        return true;
    }
}

bool LocatorCallbackNapi::IfReportAccuracyLocation()
{
    if (locationPriority_ == LOCATION_PRIORITY_ACCURACY &&
        (((singleLocation_->GetLocationSourceType() == LocationSourceType::GNSS_TYPE ||
        singleLocation_->GetLocationSourceType() == LocationSourceType::RTK_TYPE) && inHdArea_) ||
        singleLocation_->GetLocationSourceType() == LocationSourceType::NETWORK_TYPE)) {
        return false;
    } else {
        return true;
    }
}

void LocatorCallbackNapi::SetSingleLocation(const std::unique_ptr<Location>& location)
{
    std::unique_lock<std::mutex> guard(mutex_);
    singleLocation_ = std::make_shared<Location>(*location);
}
} // namespace Location
} // namespace OHOS
