/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "notification_napi.h"
#include "ans_inner_errors.h"
#include "location_log.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "napi_common.h"
#include "napi_common_util.h"
#include "notification_action_button.h"
#include "notification_capsule.h"
#include "notification_constant.h"
#include "notification_progress.h"
#include "notification_time.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace Location {
namespace {
static const std::unordered_map<int32_t, std::string> ERROR_CODE_MESSAGE {
    {ERROR_PERMISSION_DENIED, "Permission denied"},
    {ERROR_NOT_SYSTEM_APP, "The application isn't system application"},
    {ERROR_PARAM_INVALID, "Invalid parameter"},
    {ERROR_SYSTEM_CAP_ERROR, "SystemCapability not found"},
    {ERROR_INTERNAL_ERROR, "Internal error"},
    {ERROR_IPC_ERROR, "Marshalling or unmarshalling error"},
    {ERROR_SERVICE_CONNECT_ERROR, "Failed to connect service"},
    {ERROR_NOTIFICATION_CLOSED, "Notification is not enabled"},
    {ERROR_SLOT_CLOSED, "Notification slot is not enabled"},
    {ERROR_NOTIFICATION_UNREMOVABLE, "Notification is not allowed to remove"},
    {ERROR_NOTIFICATION_NOT_EXIST, "The notification is not exist"},
    {ERROR_USER_NOT_EXIST, "The user is not exist"},
    {ERROR_OVER_MAX_NUM_PER_SECOND, "Over max number notifications per second"},
    {ERROR_DISTRIBUTED_OPERATION_FAILED, "Distributed operation failed"},
    {ERROR_READ_TEMPLATE_CONFIG_FAILED, "Read template config failed"},
    {ERROR_NO_MEMORY, "No memory space"},
    {ERROR_BUNDLE_NOT_FOUND, "The specified bundle name was not found"},
    {ERROR_NO_AGENT_SETTING, "There is no corresponding agent relationship configuration"},
};
}

napi_value NotificationNapi::NapiGetBoolean(napi_env env, const bool &isValue)
{
    napi_value result = nullptr;
    napi_get_boolean(env, isValue, &result);
    return result;
}

napi_value NotificationNapi::NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value NotificationNapi::NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NotificationNapi::CreateErrorValue(napi_env env, int32_t errCode, bool newType)
{
    LBSLOGI(NAPI_UTILS, "enter, errorCode[%{public}d]", errCode);
    napi_value error = NotificationNapi::NapiGetNull(env);
    if (errCode == ERR_OK && newType) {
        return error;
    }

    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    auto iter = ERROR_CODE_MESSAGE.find(errCode);
    std::string errMsg = iter != ERROR_CODE_MESSAGE.end() ? iter->second : "";
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    return error;
}

void NotificationNapi::NapiThrow(napi_env env, int32_t errCode)
{
    LBSLOGD(NAPI_UTILS, "enter");

    napi_throw(env, CreateErrorValue(env, errCode, true));
}

napi_value NotificationNapi::GetCallbackErrorValue(napi_env env, int32_t errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

void NotificationNapi::PaddingCallbackPromiseInfo(
    const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise)
{
    LBSLOGD(NAPI_UTILS, "enter");

    if (callback) {
        LBSLOGD(NAPI_UTILS, "Callback is not nullptr.");
        info.callback = callback;
        info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        info.deferred = deferred;
        info.isCallback = false;
    }
}

void NotificationNapi::ReturnCallbackPromise(
    const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result)
{
    LBSLOGD(NAPI_UTILS, "enter errorCode=%{public}d", info.errorCode);
    if (info.isCallback) {
        SetCallback(env, info.callback, info.errorCode, result, false);
    } else {
        SetPromise(env, info.deferred, info.errorCode, result, false);
    }
    LBSLOGD(NAPI_UTILS, "end");
}

void NotificationNapi::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, const int32_t &errorCode, const napi_value &result, bool newType)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[ARGS_TWO] = {nullptr};
    results[PARAM0] = CreateErrorValue(env, errorCode, newType);
    results[PARAM1] = result;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &results[PARAM0], &resultout));
    LBSLOGD(NAPI_UTILS, "end");
}

void NotificationNapi::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, const napi_value &result)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_ONE, &result, &resultout));
    LBSLOGD(NAPI_UTILS, "end");
}

void NotificationNapi::SetCallbackArg2(
    const napi_env &env, const napi_ref &callbackIn, const napi_value &result0, const napi_value &result1)
{
    LBSLOGD(NAPI_UTILS, "enter");
    napi_value result[ARGS_TWO] = {result0, result1};
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, result, &resultout));
    LBSLOGD(NAPI_UTILS, "end");
}

void NotificationNapi::SetPromise(const napi_env &env,
    const napi_deferred &deferred, const int32_t &errorCode, const napi_value &result, bool newType)
{
    LBSLOGD(NAPI_UTILS, "enter");
    if (errorCode == ERR_OK) {
        napi_resolve_deferred(env, deferred, result);
    } else {
        napi_reject_deferred(env, deferred, CreateErrorValue(env, errorCode, newType));
    }
    LBSLOGD(NAPI_UTILS, "end");
}

napi_value NotificationNapi::JSParaError(const napi_env &env, const napi_ref &callback)
{
    if (callback) {
        return NotificationNapi::NapiGetNull(env);
    }
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);
    SetPromise(env, deferred, ERROR, NotificationNapi::NapiGetNull(env), false);
    return promise;
}
}  // namespace Location
}  // namespace OHOS
