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

#ifndef NAPI_UTIL_H
#define NAPI_UTIL_H

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <list>
#include <mutex>
#include <string>

#include "constant_definition.h"
#include "geo_address.h"
#include "location.h"
#include "location_log.h"
#include "locator_proxy.h"
#include "message_parcel.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "request_config.h"
#include "satellite_status.h"
#include "uv.h"

namespace OHOS {
namespace Location {
class AsyncContext {
public:
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callback[3] = { 0 };
    std::function<void(void*)> executeFunc;
    std::function<void(void*)> completeFunc;
    napi_value resourceName;
    napi_value result[RESULT_SIZE];
    int errCode;

    AsyncContext(napi_env e, napi_async_work w = nullptr, napi_deferred d = nullptr)
    {
        env = e;
        work = w;
        deferred = d;
        executeFunc = nullptr;
        completeFunc = nullptr;
        resourceName = nullptr;
        result[PARAM0] = 0;
        result[PARAM1] = 0;
        errCode = 0;
    }

    AsyncContext() = delete;

    virtual ~AsyncContext()
    {
    }
};

napi_value UndefinedNapiValue(const napi_env& env);
void LocationToJs(const napi_env& env, const std::unique_ptr<Location>& locationInfo, napi_value& result);
void LocationsToJs(const napi_env& env, const std::vector<std::shared_ptr<Location>>& locations, napi_value& result);
void SatelliteStatusToJs(const napi_env& env, const std::shared_ptr<SatelliteStatus>& statusInfo, napi_value& result);
void SystemLocationToJs(const napi_env& env, const std::unique_ptr<Location>& locationInfo, napi_value& result);
bool GeoAddressesToJsObj(const napi_env& env,
    std::list<std::shared_ptr<GeoAddress>>& replyList, napi_value& arrayResult);
void JsObjToLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig);
void JsObjToCurrentLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<RequestConfig>& requestConfig);
void JsObjToCachedLocationRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<CachedGnssLocationsRequest>& request);
void JsObjToCommand(const napi_env& env, const napi_value& object,
    std::unique_ptr<LocationCommand>& commandConfig);
void JsObjToGeoFenceRequest(const napi_env& env, const napi_value& object,
    std::unique_ptr<GeofenceRequest>& request);
bool JsObjToGeoCodeRequest(const napi_env& env, const napi_value& object, MessageParcel& dataParcel);
bool JsObjToReverseGeoCodeRequest(const napi_env& env, const napi_value& object, MessageParcel& dataParcel);
napi_value JsObjectToString(const napi_env& env, const napi_value& object,
    const char* fieldStr, const int bufLen, std::string& fieldRef);
napi_value JsObjectToDouble(const napi_env& env, const napi_value& object, const char* fieldStr, double& fieldRef);
napi_value JsObjectToInt(const napi_env& env, const napi_value& object, const char* fieldStr, int& fieldRef);
napi_value JsObjectToBool(const napi_env& env, const napi_value& object, const char* fieldStr, bool& fieldRef);
napi_status SetValueUtf8String(const napi_env& env, const char* fieldStr, const char* str, napi_value& result);
napi_status SetValueStringArray(const napi_env& env, const char* fieldStr, napi_value& value, napi_value& result);
napi_status SetValueInt32(const napi_env& env, const char* fieldStr, const int intValue, napi_value& result);
napi_status SetValueInt64(const napi_env& env, const char* fieldStr, const int64_t intValue, napi_value& result);
napi_status SetValueDouble(const napi_env& env, const char* fieldStr, const double doubleValue, napi_value& result);
napi_status SetValueBool(const napi_env& env, const char* fieldStr, const bool boolvalue, napi_value& result);
napi_value DoAsyncWork(const napi_env& env, AsyncContext* asyncContext,
    const size_t argc, const napi_value* argv, const size_t nonCallbackArgNum);
napi_value CreateErrorMessage(napi_env env, std::string msg, int32_t errorCode);
void CreateFailCallBackParams(AsyncContext& context, std::string msg, int32_t errorCode);

#define GET_AND_THROW_NAPI_ERROR(env, message) \
        do { \
            const napi_extended_error_info* errorInfo = nullptr; \
            napi_get_last_error_info((env), &errorInfo); \
            bool isPending = false; \
            napi_is_exception_pending((env), &isPending); \
            if (!isPending && errorInfo != nullptr) { \
                std::string errDesc = std::string(__FUNCTION__) + ": " + #message + " fail. "; \
                std::string errorMessage = \
                    errorInfo->error_message != nullptr ? errorInfo->error_message : "empty error message"; \
                errDesc += errorMessage; \
                napi_throw_error((env), nullptr, errDesc.c_str()); \
            } \
        } while (0)

#define CHK_NAPIOK_RETVAL(env, state, message, retVal) \
        do { \
            if ((state) != napi_ok) { \
                LBSLOGE("(%{public}s) fail", #message); \
                GET_AND_THROW_NAPI_ERROR((env), (message)); \
                return retVal; \
            } \
        } while (0)

#define CHK_NAPIOK_RETNULL(env, state, message) \
        do { \
            if ((state) != napi_ok) { \
                LBSLOGE("(%{public}s) fail", #message); \
                GET_AND_THROW_NAPI_ERROR((env), (message)); \
                return nullptr; \
            } \
        } while (0)

#define CHK_NAPIOK_RETBOOL(env, state, message) \
        do { \
            if ((state) != napi_ok) { \
                LBSLOGE("(%{public}s) fail", #message); \
                GET_AND_THROW_NAPI_ERROR((env), (message)); \
                return false; \
            } \
        } while (0)

#define CHK_NAPIOK_RETVOID(env, state, message) \
        do { \
            if ((state) != napi_ok) { \
                LBSLOGE("(%{public}s) fail", #message); \
                GET_AND_THROW_NAPI_ERROR((env), (message)); \
                return; \
            } \
        } while (0)

#define CHK_NAPIOK_CONTINUE(env, state, message) \
        { \
            if ((state) != napi_ok) { \
                LBSLOGE("(%{public}s) fail", #message); \
                GET_AND_THROW_NAPI_ERROR((env), (message)); \
                continue; \
            } \
        }
}  // namespace Location
}  // namespace OHOS

#endif // NAPI_UTIL_H
