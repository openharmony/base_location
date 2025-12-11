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

#include "location_napi_adapter.h"
#include "location_log.h"
#include "location_napi_errcode.h"
#include "constant_definition.h"
#include "geofence_sdk.h"
#include "geofence_napi.h"
#include "geofence_async_context.h"
#include "beacon_fence_request.h"
#include "beacon_fence_napi.h"
#include "location_hiappevent.h"
#include "poi_info_callback_napi.h"

namespace OHOS {
namespace Location {
auto g_locatorClient = Locator::GetInstance();
auto g_geofenceClient = GeofenceManager::GetInstance();
auto g_hiAppEventClient = LocationHiAppEvent::GetInstance();
std::map<int, sptr<LocationGnssGeofenceCallbackNapi>> g_gnssGeofenceCallbackHostMap;
std::map<std::shared_ptr<BeaconFence>, sptr<LocationGnssGeofenceCallbackNapi>> g_beaconFenceRequestMap;
std::mutex g_gnssGeofenceCallbackHostMutex;
std::mutex g_beaconFenceRequestMutex;

napi_value GetLastLocation(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

#ifdef ENABLE_NAPI_MANAGER
    return HandleGetCachedLocation(env);
#else
    auto asyncContext = new (std::nothrow) LocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "getLastLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<LocationAsyncContext*>(data);
        context->loc = g_locatorClient->IsLocationEnabled() ? g_locatorClient->GetCachedLocation() : nullptr;
        if (context->loc != nullptr) {
            context->errCode = SUCCESS;
        } else {
            context->errCode = LAST_KNOWN_LOCATION_ERROR;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<LocationAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "getLastLocation");
        if (context->loc != nullptr) {
            LocationToJs(context->env, context->loc, context->result[PARAM1]);
        } else {
            LBSLOGE(LOCATOR_STANDARD, "loc is nullptr!");
        }
        LBSLOGI(LOCATOR_STANDARD, "Push last location result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
#endif
}

#ifdef ENABLE_NAPI_MANAGER
napi_value HandleGetCachedLocation(napi_env env)
{
    napi_value res;
    NAPI_CALL(env, napi_create_object(env, &res));
    std::unique_ptr<Location> loc;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->GetCachedLocationV9(loc);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "getLastLocation");
    if (loc != nullptr && errorCode == ERRCODE_SUCCESS) {
        LocationToJs(env, loc, res);
        return res;
    }
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
}
#endif

napi_value IsLocationEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
#ifdef ENABLE_NAPI_MANAGER
    napi_value res;
    bool isEnabled = false;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->IsLocationEnabledV9(isEnabled);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "isLocationEnabled");
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &res));
    return res;
#else
    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "isLocationEnabled", NAPI_AUTO_LENGTH,
        &asyncContext->resourceName) != napi_ok) {
        LBSLOGE(LOCATOR_STANDARD, "copy string failed");
    }
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        context->enable = g_locatorClient->IsLocationEnabled();
        context->errCode = SUCCESS;
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push IsLocationEnabled result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
#endif
}

#ifdef ENABLE_NAPI_MANAGER
napi_value IsLocationEnabledByUserId(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    napi_value res;
    bool isEnabled = false;
    int userId;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &userId));
    LocationErrCode errorCode = g_locatorClient->IsLocationEnabledForUser(isEnabled, userId);
    if (errorCode != ERRCODE_SUCCESS) {
        ThrowBusinessError(env, errorCode);
    }
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &res));
    return res;
}

napi_value EnableLocationByUserId(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    int userId;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &userId));
    LocationErrCode errorCode = g_locatorClient->EnableAbilityForUser(true, userId);
    if (errorCode != ERRCODE_SUCCESS) {
        ThrowBusinessError(env, errorCode);
    }
    return UndefinedNapiValue(env);
}

napi_value DisableLocationByUserId(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    int userId;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &userId));
    LocationErrCode errorCode = g_locatorClient->EnableAbilityForUser(false, userId);
    if (errorCode != ERRCODE_SUCCESS) {
        ThrowBusinessError(env, errorCode);
    }
    return UndefinedNapiValue(env);
}
#endif

#ifdef ENABLE_NAPI_MANAGER
napi_value GetCurrentWifiBssidForLocating(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    napi_value res;
    std::string bssid;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->GetCurrentWifiBssidForLocating(bssid);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "getCurrentWifiBssidForLocating");
    if (errorCode != ERRCODE_SUCCESS) {
        ThrowBusinessError(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_create_string_utf8(env, bssid.c_str(), NAPI_AUTO_LENGTH, &res));
    return res;
}

napi_value GetDistanceBetweenLocations(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (g_locatorClient == nullptr) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
        return UndefinedNapiValue(env);
    }
    // 2 arguement is necessary
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    napi_valuetype valueType1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType1));
    if (valueType != napi_object || valueType1 != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    Location location1;
    Location location2;
    if (!JsObjToLocation(env, argv[0], location1) || !JsObjToLocation(env, argv[1], location2)) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    if (!location1.isValidLatitude(location1.GetLatitude()) || !location1.isValidLongitude(location1.GetLongitude()) ||
        !location1.isValidLatitude(location2.GetLatitude()) || !location1.isValidLongitude(location2.GetLongitude())) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_value res;
    double distance;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->GetDistanceBetweenLocations(location1, location2, distance);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "getDistanceBetweenLocations");
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_create_double(env, distance, &res));
    return res;
}
#endif

#ifdef ENABLE_NAPI_MANAGER
napi_value IsPoiServiceSupported(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    napi_value res;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    bool poiServiceSupportState = g_locatorClient->IsPoiServiceSupported();
    NAPI_CALL(env, napi_get_boolean(env, poiServiceSupportState, &res));
    g_hiAppEventClient->WriteEndEvent(beginTime, 0, ERRCODE_SUCCESS, "isPoiServiceSupported");
    return res;
}
#endif

#ifdef ENABLE_NAPI_MANAGER
void CreateGetPoiInfoAsyncContext(PoiInfoAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) ->void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<PoiInfoAsyncContext*>(data);
        if (context->callbackHost_ == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "callbackHost_ is nullptr!");
            return;
        }
        auto callbackPtr = sptr<IPoiInfoCallback>(context->callbackHost_);
        context->errCode = g_locatorClient->GetPoiInfo(callbackPtr);
        if (context->errCode == SUCCESS) {
            context->callbackHost_->Wait();
            context->errCode = context->callbackHost_->GetErrorCode();
        }
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<PoiInfoAsyncContext*>(data);
        if (context->callbackHost_ == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "callbackHost_ is nullptr!");
            NAPI_CALL_RETURN_VOID(
                context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
            return;
        }
        if (context->errCode == SUCCESS) {
            auto poiInfo = context->callbackHost_->GetResult();
            if (poiInfo != nullptr) {
                PoiInfo poi = *poiInfo;
                napi_value poiObj = CreatePoiInfoJsObj(context->env, poi);
                context->result[PARAM1] = poiObj;
            } else {
                NAPI_CALL_RETURN_VOID(
                    context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
            }
        }
        if (context->callbackHost_) {
            context->callbackHost_ = nullptr;
        }
    };
}

napi_value GetPoiInfo(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    auto asyncContext = new PoiInfoAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "getPoiInfo", NAPI_AUTO_LENGTH, &asyncContext->resourceName)
        != napi_ok) {
        delete asyncContext;
        return nullptr;
    }
    asyncContext->callbackHost_ = sptr<PoiInfoCallbackNapi>(new PoiInfoCallbackNapi());
    CreateGetPoiInfoAsyncContext(asyncContext);
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}
#endif

napi_value EnableLocation(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
#ifdef ENABLE_NAPI_MANAGER
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsFunctionType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "enableLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        context->errCode = g_locatorClient->EnableAbilityV9(true);
#else
        g_locatorClient->EnableAbility(true);
        context->errCode = SUCCESS;
#endif
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        NAPI_CALL_RETURN_VOID(context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
#else
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
#endif
        LBSLOGI(LOCATOR_STANDARD, "Push EnableLocation result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value DisableLocation(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = g_locatorClient->EnableAbilityV9(false);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
#else
    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "disableLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        g_locatorClient->EnableAbility(false);
        context->errCode = SUCCESS;
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push DisableLocation result to client");
    };
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
#endif
}

napi_value RequestEnableLocation(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "enableLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        if (!g_locatorClient->IsLocationEnabled()) {
            g_locatorClient->ShowNotification();
        }
        g_locatorClient->EnableAbility(true);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push RequestEnableLocation result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value IsGeoServiceAvailable(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
#ifdef ENABLE_NAPI_MANAGER
    napi_value res;
    bool isAvailable = false;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->IsGeoServiceAvailableV9(isAvailable);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "isGeoServiceAvailable");
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_get_boolean(env, isAvailable, &res));
    return res;
#else
    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "isGeoServiceAvailable", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        bool isAvailable = g_locatorClient->IsGeoServiceAvailable();
        context->enable = isAvailable;
        context->errCode = SUCCESS;
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push isGeoServiceAvailable result to client");
    };
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
#endif
}

#ifdef ENABLE_NAPI_MANAGER
napi_value SetLocationSwitchIgnored(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    bool isIgnored;
    NAPI_CALL(env, napi_get_value_bool(env, argv[0], &isIgnored));
    LocationErrCode errorCode = g_locatorClient->SetLocationSwitchIgnored(isIgnored);
    if (errorCode != ERRCODE_SUCCESS) {
        ThrowBusinessError(env, errorCode);
    }
    return UndefinedNapiValue(env);
}
#endif

void CreateReverseGeocodeAsyncContext(ReverseGeoCodeAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        if (context->errCode != ERRCODE_SUCCESS) {
#else
        if (context->errCode != SUCCESS) {
#endif
            return;
        }
#ifdef ENABLE_NAPI_MANAGER
        bool isAvailable = false;
        LocationErrCode errorCode = g_locatorClient->IsGeoServiceAvailableV9(isAvailable);
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
        if (!isAvailable) {
            context->errCode = ERRCODE_REVERSE_GEOCODING_FAIL;
            return;
        }
        context->beginTime = CommonUtils::GetCurrentTimeMilSec();
        errorCode = g_locatorClient->GetAddressByCoordinateV9(context->reverseGeoCodeRequest, context->replyList);
        if (context->replyList.empty() || errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
        }
#else
        if (!g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
            return;
        }
        g_locatorClient->GetAddressByCoordinate(context->reverseGeoCodeRequest, context->replyList);
        if (context->replyList.empty()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
        }
#endif
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == SUCCESS ? 0 : 1, context->errCode, "getAddressesFromLocation");
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
    };
}

void CreateGeocodeAsyncContext(GeoCodeAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        if (context->errCode != SUCCESS) {
            return;
        }
#ifdef ENABLE_NAPI_MANAGER
        bool isAvailable = false;
        LocationErrCode errorCode = g_locatorClient->IsGeoServiceAvailableV9(isAvailable);
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
        if (!isAvailable) {
            context->errCode = ERRCODE_GEOCODING_FAIL;
            return;
        }
        errorCode = g_locatorClient->GetAddressByLocationNameV9(context->geoCodeRequest, context->replyList);
        if (context->replyList.empty() || errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
        }
#else
        if (!g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = GEOCODE_ERROR;
            return;
        }
        g_locatorClient->GetAddressByLocationName(context->geoCodeRequest, context->replyList);
        if (context->replyList.empty()) {
            context->errCode = GEOCODE_ERROR;
        }
#endif
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        g_hiAppEventClient->WriteEndEvent(context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1,
            context->errCode, "getAddressesFromLocationName");
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push GetAddressesFromLocationName result to client");
    };
}

napi_value GetAddressesFromLocation(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
#ifdef ENABLE_NAPI_MANAGER
    if (argc < PARAM1 || argc > PARAM2 || (argc == PARAM2 && !CheckIfParamIsFunctionType(env, argv[1]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
#endif

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
#ifdef ENABLE_NAPI_MANAGER
    if (valueType != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");
#endif
    auto asyncContext = new ReverseGeoCodeAsyncContext(env);
    if (napi_create_string_latin1(env, "getAddressesFromLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    int ret = JsObjToReverseGeoCodeRequest(env, argv[0], asyncContext->reverseGeoCodeRequest);
#ifdef ENABLE_NAPI_MANAGER
    asyncContext->errCode = (ret == SUCCESS) ? ERRCODE_SUCCESS : ERRCODE_INVALID_PARAM;
#else
    asyncContext->errCode = (ret == SUCCESS) ? SUCCESS : INPUT_PARAMS_ERROR;
#endif
#ifdef ENABLE_NAPI_MANAGER
    if (asyncContext->errCode != SUCCESS) {
        int code = asyncContext->errCode;
        delete asyncContext;
        asyncContext = nullptr;
        HandleSyncErrCode(env, code);
        return UndefinedNapiValue(env);
    }
#endif
    CreateReverseGeocodeAsyncContext(asyncContext);

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value GetAddressesFromLocationName(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
#ifdef ENABLE_NAPI_MANAGER
    if (argc < PARAM1 || argc > PARAM2 || (argc == PARAM2 && !CheckIfParamIsFunctionType(env, argv[1]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
#endif

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
#ifdef ENABLE_NAPI_MANAGER
    if (valueType != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");
#endif
    auto asyncContext = new (std::nothrow) GeoCodeAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "GetAddressesFromLocationName",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->errCode = JsObjToGeoCodeRequest(env, argv[0], asyncContext->geoCodeRequest);
#ifdef ENABLE_NAPI_MANAGER
    if (asyncContext->errCode == INPUT_PARAMS_ERROR) {
        delete asyncContext;
        asyncContext = nullptr;
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    CreateGeocodeAsyncContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

#ifdef ENABLE_NAPI_MANAGER
napi_value IsLocationPrivacyConfirmed(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (g_locatorClient == nullptr) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
        return UndefinedNapiValue(env);
    }
    // 1 arguement is necessary
    if (argc != PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    if (valueType != napi_number) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    napi_value res;
    bool isEnabled = false;
    LocationErrCode errorCode = g_locatorClient->IsLocationPrivacyConfirmedV9(type, isEnabled);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &res));
    return res;
}

napi_value SetLocationPrivacyConfirmStatus(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (g_locatorClient == nullptr) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
        return UndefinedNapiValue(env);
    }
    // 2 arguement is necessary
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_valuetype valueType1;
    napi_valuetype valueType2;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType1));
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType2));
    if (valueType1 != napi_number || valueType2 != napi_boolean) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    bool isConfirmed;
    NAPI_CALL(env, napi_get_value_bool(env, argv[1], &isConfirmed));
    LocationErrCode errorCode = g_locatorClient->SetLocationPrivacyConfirmStatusV9(type, isConfirmed);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
}
#endif

napi_value GetCachedGnssLocationsSize(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
#ifdef ENABLE_NAPI_MANAGER
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsFunctionType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    auto asyncContext = new (std::nothrow) CachedAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "GetCachedGnssLocationsSize",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForGetCachedGnssLocationsSizeContext(asyncContext);
    SetCompleteFuncForGetCachedGnssLocationsSizeContext(asyncContext);
    return DoAsyncWork(env, asyncContext, argc, argv, 0);
}

void SetExecuteFuncForGetCachedGnssLocationsSizeContext(CachedAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = CheckLocationSwitchState();
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
#endif

#ifdef ENABLE_NAPI_MANAGER
        int size = -1;
        context->errCode = g_locatorClient->GetCachedGnssLocationsSizeV9(size);
        context->locationSize = size;
#else
        context->locationSize = g_locatorClient->GetCachedGnssLocationsSize();
        context->errCode = (context->locationSize >= 0) ? SUCCESS : NOT_SUPPORTED;
#endif
    };
}

void SetCompleteFuncForGetCachedGnssLocationsSizeContext(CachedAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
        g_hiAppEventClient->WriteEndEvent(context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1,
            context->errCode, "getCachedGnssLocationsSize");
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_int32(context->env, context->locationSize, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push GetCachedGnssLocationsSize result to client");
    };
}

napi_value FlushCachedGnssLocations(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
#ifdef ENABLE_NAPI_MANAGER
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsFunctionType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    auto asyncContext = new CachedAsyncContext(env);
    if (napi_create_string_latin1(env, "FlushCachedGnssLocations",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForFlushCachedGnssLocationsContext(asyncContext);
    SetCompleteFuncForFlushCachedGnssLocationsContext(asyncContext);
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void SetExecuteFuncForFlushCachedGnssLocationsContext(CachedAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = CheckLocationSwitchState();
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
        context->errCode = g_locatorClient->FlushCachedGnssLocationsV9();
#else
        if (g_locatorClient->IsLocationEnabled()) {
            g_locatorClient->FlushCachedGnssLocations();
        }
        context->errCode = NOT_SUPPORTED;
#endif
    };
}

void SetCompleteFuncForFlushCachedGnssLocationsContext(CachedAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        g_hiAppEventClient->WriteEndEvent(context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1,
            context->errCode, "flushCachedGnssLocations");
        NAPI_CALL_RETURN_VOID(context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
#else
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
#endif
        LBSLOGI(LOCATOR_STANDARD, "Push FlushCachedGnssLocations result to client");
    };
}

void CreateCommandAsyncContext(CommandAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CommandAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        if (context->command != nullptr) {
            context->errCode = g_locatorClient->SendCommandV9(context->command);
        }
#else
        if (context->command != nullptr) {
            g_locatorClient->SendCommand(context->command);
        }
        context->errCode = NOT_SUPPORTED;
#endif
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CommandAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "sendCommand");
        NAPI_CALL_RETURN_VOID(context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
#else
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env,
            context->enable, &context->result[PARAM1]));
#endif
        LBSLOGI(LOCATOR_STANDARD, "Push SendCommand result to client");
    };
}

napi_value SendCommand(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
#ifdef ENABLE_NAPI_MANAGER
    if (argc < PARAM1 || argc > PARAM2 || (argc == PARAM2 && !CheckIfParamIsFunctionType(env, argv[1]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
#endif

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
#ifdef ENABLE_NAPI_MANAGER
    if (valueType != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");
#endif

#ifdef ENABLE_NAPI_MANAGER
    if (argc == PARAM2 && !CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    auto asyncContext = new CommandAsyncContext(env);
    asyncContext->command = std::make_unique<LocationCommand>();
    if (napi_create_string_latin1(env, "SendCommand",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    int errCode = JsObjToCommand(env, argv[0], asyncContext->command);
#ifdef ENABLE_NAPI_MANAGER
    if (errCode == INPUT_PARAMS_ERROR) {
        delete asyncContext;
        asyncContext = nullptr;
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, errCode != INPUT_PARAMS_ERROR, "The input params should be checked first.");
#endif
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    CreateCommandAsyncContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

#ifdef ENABLE_NAPI_MANAGER
napi_value GetIsoCountryCode(napi_env env, napi_callback_info info)
{
    LBSLOGD(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsFunctionType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    CountryCodeContext *asyncContext = new (std::nothrow) CountryCodeContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "CountryCodeContext", NAPI_AUTO_LENGTH,
        &asyncContext->resourceName) != napi_ok) {
        LBSLOGE(LOCATOR_STANDARD, "copy string failed");
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForCountryCodeContext(asyncContext);
    SetCompleteFuncForCountryCodeContext(asyncContext);
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void SetExecuteFuncForCountryCodeContext(CountryCodeContext* asyncContext)
{
    asyncContext->executeFunc = [&](void *data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "GetIsoCountryCode data == nullptr");
            return;
        }
        CountryCodeContext *context = static_cast<CountryCodeContext*>(data);
        std::shared_ptr<CountryCode> country = std::make_shared<CountryCode>();
        LocationErrCode errorCode = g_locatorClient->GetIsoCountryCodeV9(country);
        context->errCode = errorCode;
        if (errorCode == ERRCODE_SUCCESS) {
            context->country = country;
        }
    };
}

void SetCompleteFuncForCountryCodeContext(CountryCodeContext* asyncContext)
{
    asyncContext->completeFunc = [&](void *data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "GetIsoCountryCode data == nullptr");
            return;
        }
        CountryCodeContext *context = static_cast<CountryCodeContext *>(data);
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "getCountryCode");
        NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
        if (context->country) {
            CountryCodeToJs(context->env, context->country, context->result[PARAM1]);
        } else {
            LBSLOGE(LOCATOR_STANDARD, "country is nullptr!");
        }
        LBSLOGI(LOCATOR_STANDARD, "Push GetIsoCountryCode result to client, time = %{public}s",
            std::to_string(CommonUtils::GetCurrentTimeMilSec()).c_str());
    };
}

int ParseLocationMockParams(napi_env env, LocationMockAsyncContext *asyncContext, napi_value object)
{
    CHK_ERROR_CODE("timeInterval", JsObjectToInt(env, object, "timeInterval", asyncContext->timeInterval), true);
    bool result = false;
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, "locations", &result), false);
    if (result) {
        NAPI_CALL_BASE(env, napi_get_named_property(env, object, "locations", &value), false);
        bool isArray = false;
        NAPI_CALL_BASE(env, napi_is_array(env, value, &isArray), false);
        if (!isArray) {
            LBSLOGE(LOCATOR_STANDARD, "not an array!");
            return INPUT_PARAMS_ERROR;
        }
        GetLocationArray(env, asyncContext, value);
    }
    return SUCCESS;
}

napi_value EnableLocationMock(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    LocationErrCode errorCode = CheckLocationSwitchState();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    errorCode = g_locatorClient->EnableLocationMockV9();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
}

napi_value DisableLocationMock(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    LocationErrCode errorCode = CheckLocationSwitchState();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    errorCode = g_locatorClient->DisableLocationMockV9();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
}

napi_value SetMockedLocations(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    if (argc != PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    if (valueType != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }

    LocationMockAsyncContext *asyncContext = new (std::nothrow) LocationMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "SetMockedLocations",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->errCode = ParseLocationMockParams(env, asyncContext, argv[0]);
    if (asyncContext->errCode == INPUT_PARAMS_ERROR) {
        delete asyncContext;
        asyncContext = nullptr;
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    LocationErrCode errorCode = CheckLocationSwitchState();
    if (errorCode != ERRCODE_SUCCESS) {
        delete asyncContext;
        asyncContext = nullptr;
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    errorCode = g_locatorClient->SetMockedLocationsV9(asyncContext->timeInterval, asyncContext->LocationNapi);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    delete asyncContext;
    asyncContext = nullptr;
    return UndefinedNapiValue(env);
}

napi_value EnableReverseGeocodingMock(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    LocationErrCode errorCode = g_locatorClient->EnableReverseGeocodingMockV9();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
}

napi_value DisableReverseGeocodingMock(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    LocationErrCode errorCode = g_locatorClient->DisableReverseGeocodingMockV9();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
}

napi_value SetReverseGeocodingMockInfo(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    if (argc != PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[0], &isArray));
    if (!isArray) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    JsObjToRevGeocodeMock(env, argv[0], mockInfo);
    LocationErrCode errorCode = g_locatorClient->SetReverseGeocodingMockInfoV9(mockInfo);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
    }
    return UndefinedNapiValue(env);
}
#endif

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode CheckLocationSwitchState()
{
    bool isEnabled = false;
    LocationErrCode errorCode = g_locatorClient->IsLocationEnabledV9(isEnabled);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (!isEnabled) {
        return ERRCODE_SWITCH_OFF;
    }
    return ERRCODE_SUCCESS;
}

sptr<LocatingRequiredDataCallbackNapi> CreateSingleCallbackHost()
{
    auto callbackHost =
        sptr<LocatingRequiredDataCallbackNapi>(new (std::nothrow) LocatingRequiredDataCallbackNapi());
    if (callbackHost) {
        callbackHost->SetFixNumber(1);
    }
    return callbackHost;
}

SingleScanAsyncContext* CreateSingleScanAsyncContext(const napi_env& env,
    std::unique_ptr<LocatingRequiredDataConfig>& config, sptr<LocatingRequiredDataCallbackNapi> callback)
{
    auto asyncContext = new (std::nothrow) SingleScanAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "getLocatingRequiredDataOnce",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->timeout_ = config->GetScanTimeoutMs();
    asyncContext->callbackHost_ = callback;
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<SingleScanAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr) {
            callbackHost->Wait(context->timeout_);
            auto callbackPtr = sptr<ILocatingRequiredDataCallback>(callbackHost);
            g_locatorClient->UnRegisterLocatingRequiredDataCallback(callbackPtr);
            if (callbackHost->GetCount() != 0) {
                context->errCode = ERRCODE_SCAN_FAIL;
            }
            callbackHost->SetCount(1);
        }
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<SingleScanAsyncContext*>(data);

        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr) {
            std::vector<std::shared_ptr<LocatingRequiredData>> res = callbackHost->GetSingleResult();
            napi_create_array_with_length(context->env, res.size(), &context->result[PARAM1]);
            LocatingRequiredDataToJsObj(context->env, res, context->result[PARAM1]);
            callbackHost->ClearSingleResult();
        } else {
            LBSLOGE(LOCATOR_STANDARD, "m_singleLocation is nullptr!");
        }
        if (context->callbackHost_) {
            context->callbackHost_ = nullptr;
        }
        LBSLOGI(LOCATOR_STANDARD, "Push scan info to client");
    };
    return asyncContext;
}

napi_value GetLocatingRequiredData(napi_env env, napi_callback_info info)
{
    LBSLOGD(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsObjectType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    if (argc == PARAM1) {
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
        if (valueType != napi_object) {
            HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
            return UndefinedNapiValue(env);
        }
    }

    auto singleCallbackHost = CreateSingleCallbackHost();
    if (singleCallbackHost == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    std::unique_ptr<LocatingRequiredDataConfig> requestConfig = std::make_unique<LocatingRequiredDataConfig>();
    JsObjToLocatingRequiredDataConfig(env, argv[0], requestConfig);
    requestConfig->SetFixNumber(1);
    auto callbackPtr = sptr<ILocatingRequiredDataCallback>(singleCallbackHost);
    LocationErrCode errorCode = g_locatorClient->RegisterLocatingRequiredDataCallback(requestConfig, callbackPtr);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }

    auto asyncContext = CreateSingleScanAsyncContext(env, requestConfig, singleCallbackHost);
    if (asyncContext == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    return DoAsyncWork(env, asyncContext, argc, argv, 1);
}

bool IsMatched(sptr<LocatingRequiredDataCallbackNapi> callbackHost,
    SingleScanAsyncContext* context)
{
    bool isMatched = false;
    std::unordered_map<std::string, int> wifiResultMap;
    std::vector<std::shared_ptr<LocatingRequiredData>> res = callbackHost->GetSingleResult();
    for (auto &scanRes : res) {
        if (scanRes == nullptr || scanRes->GetWifiScanInfo() == nullptr) {
            return false;
        }
        wifiResultMap[scanRes->GetWifiScanInfo()->GetBssid()] = scanRes->GetWifiScanInfo()->GetRssi();
    }
    for (auto &requestWlanBssid : context->wlanBssidArray_) {
        if (wifiResultMap.count(requestWlanBssid) &&
            wifiResultMap[requestWlanBssid] >= context->rssiThreshold_) {
            isMatched = true;
            break;
        }
    }
    return isMatched;
}

SingleScanAsyncContext* CreateSingleWifiScanAsyncContext(const napi_env& env,
    std::unique_ptr<LocatingRequiredDataConfig>& config, sptr<LocatingRequiredDataCallbackNapi> callback)
{
    auto asyncContext = new (std::nothrow) SingleScanAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "isWlanBssidMatched",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->timeout_ = config->GetScanTimeoutMs();
    asyncContext->rssiThreshold_ = config->GetRssiThreshold();
    asyncContext->wlanBssidArray_ = config->GetWlanBssidArray();
    asyncContext->callbackHost_ = callback;
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<SingleScanAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr) {
            callbackHost->Wait(context->timeout_);
            auto callbackPtr = sptr<ILocatingRequiredDataCallback>(callbackHost);
            g_locatorClient->UnRegisterLocatingRequiredDataCallback(callbackPtr);
            if (callbackHost->GetCount() != 0) {
                context->errCode = ERRCODE_WIFI_SCAN_FAIL;
            }
            callbackHost->SetCount(1);
        }
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<SingleScanAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        bool isMatched = false;
        if (callbackHost != nullptr) {
            isMatched = IsMatched(callbackHost, context);
            callbackHost->ClearSingleResult();
        }
        NAPI_CALL_RETURN_VOID(
                    context->env, napi_get_boolean(context->env, isMatched, &context->result[PARAM1]));
        if (context->callbackHost_) {
            context->callbackHost_ = nullptr;
        }
    };
    return asyncContext;
}

bool GetWlanRequestConfig(napi_env env, napi_value argv[], WlanRequestConfig& requestConfig)
{
    // wlanBssidArray校验
    bool is_array;
    napi_is_array(env, argv[PARAM0], &is_array);
    if (!is_array) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s First argument must be an array of strings.", __func__);
        return false;
    }
    uint32_t length;
    napi_value element;
    napi_valuetype type;
    napi_get_array_length(env, argv[PARAM0], &length);
    if (length > INPUT_WIFI_LIST_MAX_SIZE) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s input wifi size over limited size.", __func__);
        return false;
    }
    for (uint32_t i = 0; i< length; ++i) {
        napi_get_element(env, argv[PARAM0], i, &element);
        napi_typeof(env, element, &type);
        if (type != napi_string) {
            LBSLOGE(LOCATOR_STANDARD, "%{public}s Array elements must be a string.", __func__);
            return false;
        }
        char mac[64] = {0};
        size_t macLen = 0;
        napi_get_value_string_utf8(env, element, mac, sizeof(mac), &macLen);
        std::string macAddr = mac;
        requestConfig.wlanBssidArray.push_back(macAddr);
    }
    // rssiThreshold校验
    napi_typeof(env, argv[PARAM1], &type);
    if (type != napi_number) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s Second argument must be a number.", __func__);
        return false;
    }
    napi_get_value_int32(env, argv[PARAM1], &requestConfig.rssiThreshold);
    // needStartScan校验
    napi_typeof(env, argv[PARAM2], &type);
    if (type != napi_boolean) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s Third argument must be a boolean.", __func__);
        return false;
    }
    napi_get_value_bool(env, argv[PARAM2], &requestConfig.needStartScan);
    return true;
}

napi_value IsWlanBssidMatched(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    WlanRequestConfig requestConfig;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    if (argc != PARAM3 || !GetWlanRequestConfig(env, argv, requestConfig)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s Expected 3 arguments.", __func__);
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }

    auto singleCallbackHost = CreateSingleCallbackHost();
    if (singleCallbackHost == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    std::unique_ptr<LocatingRequiredDataConfig> scanRequestConfig = std::make_unique<LocatingRequiredDataConfig>();
    scanRequestConfig->SetType(LocatingRequiredDataType::WIFI);
    scanRequestConfig->SetWlanBssidArray(requestConfig.wlanBssidArray);
    scanRequestConfig->SetRssiThreshold(requestConfig.rssiThreshold);
    scanRequestConfig->SetNeedStartScan(requestConfig.needStartScan);
    scanRequestConfig->SetScanTimeoutMs(DEFAULT_TIMEOUT_5S);
    scanRequestConfig->SetIsWlanMatchCalled(true);
    scanRequestConfig->SetFixNumber(1);
    auto callbackPtr = sptr<ILocatingRequiredDataCallback>(singleCallbackHost);
    LocationErrCode errorCode = g_locatorClient->RegisterLocatingRequiredDataCallback(scanRequestConfig, callbackPtr);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    auto asyncContext = CreateSingleWifiScanAsyncContext(env, scanRequestConfig, singleCallbackHost);
    if (asyncContext == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    size_t objectArgsNum = PARAM3;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value AddGnssGeofence(napi_env env, napi_callback_info info)
{
    LBSLOGD(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator ext SA failed");
    if (argc > PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    bool isValidParameter = ParseGnssGeofenceRequest(env, argv[0], gnssGeofenceRequest);
    if (!isValidParameter) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    auto locationGnssGeofenceCallbackHost =
        sptr<LocationGnssGeofenceCallbackNapi>(new LocationGnssGeofenceCallbackNapi());
    JsObjToGeofenceTransitionCallback(env, argv[0], locationGnssGeofenceCallbackHost);
    auto callbackPtr = sptr<IGnssGeofenceCallback>(locationGnssGeofenceCallbackHost);
    gnssGeofenceRequest->SetGeofenceTransitionCallback(callbackPtr->AsObject());
    auto asyncContext = new GnssGeofenceAsyncContext(env);
    if (napi_create_string_latin1(env, "addGnssGeofence",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->callbackHost_ = locationGnssGeofenceCallbackHost;
    asyncContext->request_ = gnssGeofenceRequest;
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForAddGnssGeofenceContext(asyncContext);
    SetCompleteFuncForAddGnssGeofenceContext(asyncContext);
    return DoAsyncWork(env, asyncContext, argc, argv, 1);
}

void SetExecuteFuncForAddGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        auto gnssGeofenceRequest = context->request_;
        if (callbackHost != nullptr && gnssGeofenceRequest != nullptr) {
            auto errCode = g_geofenceClient->AddGnssGeofence(gnssGeofenceRequest);
            if (errCode != ERRCODE_SUCCESS) {
                context->errCode = errCode;
                callbackHost->SetCount(0);
            }
            callbackHost->Wait(DEFAULT_CALLBACK_WAIT_TIME);
            if (callbackHost->GetCount() != 0) {
                context->errCode = ERRCODE_SERVICE_UNAVAILABLE;
            }
            callbackHost->SetCount(1);
        }
    };
}

void SetCompleteFuncForAddGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr && context->errCode == ERRCODE_SUCCESS &&
            callbackHost->GetGeofenceOperationType() == GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD) {
            LocationErrCode errCode = callbackHost->DealGeofenceOperationResult();
            if (errCode == ERRCODE_SUCCESS) {
                int fenceId = callbackHost->GetFenceId();
                napi_create_object(context->env, &context->result[PARAM1]);
                napi_create_int64(context->env, fenceId, &context->result[PARAM1]);
                AddCallbackToGnssGeofenceCallbackHostMap(fenceId, callbackHost);
            } else {
                context->errCode = errCode;
            }
        }
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "addGnssGeofence");
    };
}

napi_value RemoveGnssGeofence(napi_env env, napi_callback_info info)
{
    LBSLOGD(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    if (argc > PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    int fenceId = -1;
    napi_valuetype valueType1;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType1));
    if (valueType1 != napi_number) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &fenceId));
    auto asyncContext = new (std::nothrow) GnssGeofenceAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    asyncContext->fenceId_ = fenceId;
    asyncContext->callbackHost_ = FindCallbackInGnssGeofenceCallbackHostMap(fenceId);
    if (napi_create_string_latin1(env, "removeGnssGeofence",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForRemoveGnssGeofenceContext(asyncContext);
    SetCompleteFuncForRemoveGnssGeofenceContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void SetExecuteFuncForRemoveGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
        request->SetFenceId(context->fenceId_);
        context->errCode = g_geofenceClient->RemoveGnssGeofence(request);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr) {
            if (context->errCode != ERRCODE_SUCCESS) {
                callbackHost->SetCount(0);
            }
            callbackHost->Wait(DEFAULT_CALLBACK_WAIT_TIME);
            if (callbackHost->GetCount() != 0) {
                context->errCode = ERRCODE_SERVICE_UNAVAILABLE;
            }
            callbackHost->SetCount(1);
        } else {
            context->errCode = ERRCODE_GEOFENCE_INCORRECT_ID;
        }
    };
}

void SetCompleteFuncForRemoveGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr && context->errCode == ERRCODE_SUCCESS &&
            callbackHost->GetGeofenceOperationType() ==
            GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE) {
            LocationErrCode errCode = callbackHost->DealGeofenceOperationResult();
            if (errCode == ERRCODE_SUCCESS) {
                NAPI_CALL_RETURN_VOID(
                    context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
                RemoveCallbackToGnssGeofenceCallbackHostMap(context->fenceId_);
            } else {
                context->errCode = errCode;
            }
        }
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "remoceGnssGeofence");
        LBSLOGD(LOCATOR_STANDARD, "Push RemoveGnssGeofence result to client");
    };
}

napi_value AddBeaconFence(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator ext SA failed");
    if (argc > PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    bool isValidParameter = ParseBeaconFenceRequest(env, argv[0], beaconFenceRequest);
    if (!isValidParameter) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    auto asyncContext = new GnssGeofenceAsyncContext(env);
    if (napi_create_string_latin1(env, "addBeaconFence",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beaconRequest_ = beaconFenceRequest;
    auto beaconFenceCallbackHost =
        sptr<LocationGnssGeofenceCallbackNapi>(new LocationGnssGeofenceCallbackNapi());
    JsObjToBeaconFenceTransitionCallback(env, argv[0], beaconFenceCallbackHost);
    auto callbackPtr = sptr<IGnssGeofenceCallback>(beaconFenceCallbackHost);
    beaconFenceRequest->SetBeaconFenceTransitionCallback(callbackPtr->AsObject());
    asyncContext->callbackHost_ = beaconFenceCallbackHost;
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    CreateAsyncContextForAddBeaconFence(asyncContext);
    if (asyncContext == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    return DoAsyncWork(env, asyncContext, argc, argv, 1);
}

void CreateAsyncContextForAddBeaconFence(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        auto beaconFenceRequest = context->beaconRequest_;
        if (beaconFenceRequest == nullptr) {
            return;
        }
        context->errCode = g_locatorClient->AddBeaconFence(beaconFenceRequest);
        if (callbackHost != nullptr) {
            if (context->errCode != ERRCODE_SUCCESS) {
                callbackHost->SetCount(0);
            }
            callbackHost->Wait(DEFAULT_CALLBACK_WAIT_TIME);
            if (callbackHost->GetCount() != 0) {
                context->errCode = ERRCODE_SERVICE_UNAVAILABLE;
            }
            callbackHost->SetCount(1);
        }
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        auto beaconFenceRequest = context->beaconRequest_;
        if (callbackHost != nullptr && context->errCode == ERRCODE_SUCCESS &&
            callbackHost->GetGeofenceOperationType() == GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD) {
            LocationErrCode errCode = callbackHost->DealGeofenceOperationResult();
            if (errCode == ERRCODE_SUCCESS) {
                int fenceId = callbackHost->GetFenceId();
                napi_create_object(context->env, &context->result[PARAM1]);
                napi_create_int64(context->env, fenceId, &context->result[PARAM1]);
                AddBeaconFenceRequest(beaconFenceRequest, callbackHost);
            } else {
                context->errCode = errCode;
            }
            g_hiAppEventClient->WriteEndEvent(
                context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "addBeaconFence");
        }
    };
}

napi_value RemoveBeaconFence(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    if (argc > PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    auto asyncContext = new (std::nothrow) GnssGeofenceAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "removeBeaconFence",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }

    if (valueType == napi_undefined) {
        std::unique_lock<std::mutex> lock(g_beaconFenceRequestMutex);
        if (g_beaconFenceRequestMap.size() == 0) {
            HandleSyncErrCode(env, ERRCODE_BEACONFENCE_INCORRECT_ID);
            delete asyncContext;
            return UndefinedNapiValue(env);
        }
        asyncContext->clearBeaconFence_ = true;
    } else {
        std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
        bool isValidParameter = JsObjToBeaconFence(env, argv[0], beaconFence);
        if (!isValidParameter) {
            HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
            delete asyncContext;
            return UndefinedNapiValue(env);
        }
        asyncContext->beaconFence_ = beaconFence;
        asyncContext->callbackHost_ = FindRequestByBeaconFence(beaconFence);
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    CreateAsyncContextForRemoveBeaconFence(asyncContext);
    if (asyncContext == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void CreateAsyncContextForRemoveBeaconFence(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        if (context->clearBeaconFence_) {
            std::unique_lock<std::mutex> lock(g_beaconFenceRequestMutex);
            for (auto it = g_beaconFenceRequestMap.begin(); it != g_beaconFenceRequestMap.end(); ++it) {
                std::shared_ptr<BeaconFence> beaconFence = it->first;
                context->callbackHost_ = it->second;
                context->errCode = g_locatorClient->RemoveBeaconFence(beaconFence);
            }
        } else {
            context->errCode = g_locatorClient->RemoveBeaconFence(context->beaconFence_);
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (callbackHost == nullptr && context->errCode == ERRCODE_SUCCESS) {
            NAPI_CALL_RETURN_VOID(
                context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
        }
        if (callbackHost != nullptr && context->errCode == ERRCODE_SUCCESS &&
            callbackHost->GetGeofenceOperationType() ==
            GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE) {
            context->errCode = callbackHost->DealGeofenceOperationResult();
            if (context->errCode != ERRCODE_SUCCESS) {
                return;
            }
            if (context->clearBeaconFence_) {
                ClearBeaconFenceRequest();
            } else {
                RemoveBeaconFenceRequest(context->beaconFence_);
            }
            NAPI_CALL_RETURN_VOID(
                context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
        }
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "removeBeaconFence");
        LBSLOGI(LOCATOR_STANDARD, "Push RemoveBeaconFence result to client");
    };
}

napi_value IsBeaconFenceSupported(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    napi_value res;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    bool beaconFenceSupportedState = g_locatorClient->IsBeaconFenceSupported();
    g_hiAppEventClient->WriteEndEvent(
            beginTime, 0, ERRCODE_SUCCESS, "isBeaconFenceSupported");
    NAPI_CALL(env, napi_get_boolean(env, beaconFenceSupportedState, &res));
    return res;
}

napi_value GetGeofenceSupportedCoordTypes(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode =
        g_geofenceClient->GetGeofenceSupportedCoordTypes(coordinateSystemTypes);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "getGeofenceSupportedCoordTypes");
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    napi_value res;
    NAPI_CALL(env,
        napi_create_array_with_length(env, coordinateSystemTypes.size(), &res));
    uint32_t idx = 0;
    for (auto iter = coordinateSystemTypes.begin(); iter != coordinateSystemTypes.end(); ++iter) {
        auto coordType = *iter;
        napi_value eachObj;
        NAPI_CALL(env, napi_create_int32(env, static_cast<int>(coordType), &eachObj));
        NAPI_CALL(env, napi_set_element(env, res, idx++, eachObj));
    }
    return res;
}

void AddCallbackToGnssGeofenceCallbackHostMap(int fenceId, sptr<LocationGnssGeofenceCallbackNapi> callbackHost)
{
    std::unique_lock<std::mutex> lock(g_gnssGeofenceCallbackHostMutex);
    g_gnssGeofenceCallbackHostMap.insert(std::make_pair(fenceId, callbackHost));
}

void RemoveCallbackToGnssGeofenceCallbackHostMap(int fenceId)
{
    std::unique_lock<std::mutex> lock(g_gnssGeofenceCallbackHostMutex);
    auto iterForDelete = g_gnssGeofenceCallbackHostMap.find(fenceId);
    if (iterForDelete != g_gnssGeofenceCallbackHostMap.end()) {
        g_gnssGeofenceCallbackHostMap.erase(iterForDelete);
    }
}

sptr<LocationGnssGeofenceCallbackNapi> FindCallbackInGnssGeofenceCallbackHostMap(int fenceId)
{
    std::unique_lock<std::mutex> lock(g_gnssGeofenceCallbackHostMutex);
    auto iter = g_gnssGeofenceCallbackHostMap.find(fenceId);
    if (iter != g_gnssGeofenceCallbackHostMap.end()) {
        return iter->second;
    }
    return nullptr;
}

void AddBeaconFenceRequest(std::shared_ptr<BeaconFenceRequest>& request,
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost)
{
    std::unique_lock<std::mutex> lock(g_beaconFenceRequestMutex);
    std::shared_ptr<BeaconFence> beaconFence = request->GetBeaconFence();
    g_beaconFenceRequestMap.insert(std::make_pair(beaconFence, callbackHost));
}

void RemoveBeaconFenceRequest(std::shared_ptr<BeaconFence>& beaconFence)
{
    std::unique_lock<std::mutex> lock(g_beaconFenceRequestMutex);
    std::shared_ptr<BeaconFence> beaconFenceDelete;
    for (auto it = g_beaconFenceRequestMap.begin(); it != g_beaconFenceRequestMap.end(); ++it) {
        if (CompareBeaconFence(beaconFence, it->first)) {
            beaconFenceDelete = it->first;
        }
    }
    auto iterForDelete = g_beaconFenceRequestMap.find(beaconFenceDelete);
    if (iterForDelete != g_beaconFenceRequestMap.end()) {
        g_beaconFenceRequestMap.erase(iterForDelete);
    }
}

void ClearBeaconFenceRequest()
{
    std::unique_lock<std::mutex> lock(g_beaconFenceRequestMutex);
    g_beaconFenceRequestMap.clear();
}

sptr<LocationGnssGeofenceCallbackNapi> FindRequestByBeaconFence(std::shared_ptr<BeaconFence>& beaconFence)
{
    std::unique_lock<std::mutex> lock(g_beaconFenceRequestMutex);
    for (auto it = g_beaconFenceRequestMap.begin(); it != g_beaconFenceRequestMap.end(); ++it) {
        if (CompareBeaconFence(beaconFence, it->first)) {
            return it->second;
        }
    }
    LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, can not find beaconfence", __func__);
    return nullptr;
}

bool CompareBeaconFence(
    std::shared_ptr<BeaconFence> beaconFence1, std::shared_ptr<BeaconFence> beaconFence2)
{
    if (beaconFence1->GetIdentifier().compare(beaconFence2->GetIdentifier()) != 0) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare Identifier fail", __func__);
        return false;
    }
    if (static_cast<int>(beaconFence1->GetBeaconFenceInfoType()) !=
        static_cast<int>(beaconFence2->GetBeaconFenceInfoType())) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare BeaconFenceInfoType fail", __func__);
        return false;
    }
    if (beaconFence1->GetBeaconManufactureData().manufactureId !=
        beaconFence2->GetBeaconManufactureData().manufactureId) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare manufactureId fail", __func__);
        return false;
    }
    if (beaconFence1->GetBeaconManufactureData().manufactureData !=
        beaconFence2->GetBeaconManufactureData().manufactureData) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare manufactureData fail", __func__);
        return false;
    }
    if (beaconFence1->GetBeaconManufactureData().manufactureDataMask !=
        beaconFence2->GetBeaconManufactureData().manufactureDataMask) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare manufactureDataMask fail", __func__);
        return false;
    }
    LBSLOGD(BEACON_FENCE_MANAGER, "%{public}s res:true", __func__);
    return true;
}

void CreateGeofenceAsyncContext(GeofenceAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GeofenceAsyncContext*>(data);
        context->errCode = g_geofenceClient->GetActiveGeoFences(context->geofenceMap_);
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GeofenceAsyncContext*>(data);
        context->result[PARAM1] = CreateFenceMap(context->env, context->geofenceMap_);
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "getActiveGeoFences");
        LBSLOGI(LOCATOR_STANDARD, "Push GetActiveGeoFences result to client");
    };
}

napi_value GetActiveGeoFences(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    if (argc >= PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    auto asyncContext = new (std::nothrow) GeofenceAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "GetActiveGeoFences",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    CreateGeofenceAsyncContext(asyncContext);
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}
#endif
} // namespace Location
} // namespace OHOS
