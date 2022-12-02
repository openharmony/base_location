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

namespace OHOS {
namespace Location {
std::unique_ptr<Locator> g_locatorClient = Locator::GetInstance();

napi_value GetLastLocation(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) LocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "getLastLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
#ifdef ENABLE_NAPI_MANAGER
    napi_value res;
    NAPI_CALL(env, napi_create_object(env, &res));
    std::unique_ptr<Location> location = g_locatorClient->GetCachedLocation();
    if (location != nullptr) {
        LocationToJs(env, location, res);
        return res;
    } else {
        HandleSyncErrCode(env, ERRCODE_LOCATING_FAIL);
    }
    return UndefinedNapiValue(env);
#else
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

napi_value IsLocationEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "isLocationEnabled", NAPI_AUTO_LENGTH, &asyncContext->resourceName);

#ifdef ENABLE_NAPI_MANAGER
    napi_value res;
    bool isEnabled = g_locatorClient->IsLocationEnabled();
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &res));
    return res;
#else
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

napi_value EnableLocation(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "enableLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        g_locatorClient->EnableAbility(true);
        context->errCode = SUCCESS;
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
    size_t argc = PARAM1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "disableLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

#ifdef ENABLE_NAPI_MANAGER
    g_locatorClient->EnableAbility(false);
    return UndefinedNapiValue(env);
#else
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
    size_t argc = PARAM1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "enableLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

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
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "isGeoServiceAvailable", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
#ifdef ENABLE_NAPI_MANAGER
    napi_value res;
    bool isEnabled = g_locatorClient->IsGeoServiceAvailable();
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &res));
    return res;
#else
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        context->enable = g_locatorClient->IsLocationEnabled() && g_locatorClient->IsGeoServiceAvailable();
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

void CreateReverseGeocodeAsyncContext(ReverseGeoCodeAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void * data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        if (context->errCode != SUCCESS) {
            return;
        }
#ifdef ENABLE_NAPI_MANAGER
        if (!g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = ERRCODE_REVERSE_GEOCODING_FAIL;
            return;
        }
#else
        if (!g_locatorClient->IsLocationEnabled() ||
            !g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
            return;
        }
#endif
        g_locatorClient->GetAddressByCoordinate(context->reverseGeoCodeRequest, context->replyList);
        if (context->replyList.empty()) {
#ifdef ENABLE_NAPI_MANAGER
            context->errCode = ERRCODE_REVERSE_GEOCODING_FAIL;
#else
            context->errCode = REVERSE_GEOCODE_ERROR;
#endif
        }
    };
    asyncContext->completeFunc = [&](void * data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
    };
}

void CreateGeocodeAsyncContext(GeoCodeAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void * data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        if (context->errCode != SUCCESS) {
            return;
        }
#ifdef ENABLE_NAPI_MANAGER
        if (!g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = ERRCODE_GEOCODING_FAIL;
            return;
        }
#else
        if (!g_locatorClient->IsLocationEnabled() ||
            !g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = GEOCODE_ERROR;
            return;
        }
#endif
        g_locatorClient->GetAddressByLocationName(context->geoCodeRequest, context->replyList);
        if (context->replyList.empty()) {
#ifdef ENABLE_NAPI_MANAGER
            context->errCode = ERRCODE_GEOCODING_FAIL;
#else
            context->errCode = GEOCODE_ERROR;
#endif
        }
    };
    asyncContext->completeFunc = [&](void * data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push GetAddressesFromLocationName result to client");
    };
}

napi_value GetAddressesFromLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
#ifdef ENABLE_NAPI_MANAGER
    if (argc < 1) {
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
    auto asyncContext = new (std::nothrow) ReverseGeoCodeAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "getAddressesFromLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    bool ret = JsObjToReverseGeoCodeRequest(env, argv[0], asyncContext->reverseGeoCodeRequest);
    asyncContext->errCode = ret ? SUCCESS : INPUT_PARAMS_ERROR;
#ifdef ENABLE_NAPI_MANAGER
    if (asyncContext->errCode == INPUT_PARAMS_ERROR) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, asyncContext->errCode != INPUT_PARAMS_ERROR,
        "The input params should be checked first.");
#endif
    CreateReverseGeocodeAsyncContext(asyncContext);

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value GetAddressesFromLocationName(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
#ifdef ENABLE_NAPI_MANAGER
    if (argc < 1) {
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
    NAPI_CALL(env,
        napi_create_string_latin1(env, "GetAddressesFromLocationName", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->errCode = JsObjToGeoCodeRequest(env, argv[0], asyncContext->geoCodeRequest);
#ifdef ENABLE_NAPI_MANAGER
    if (asyncContext->errCode == INPUT_PARAMS_ERROR) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, asyncContext->errCode != INPUT_PARAMS_ERROR,
        "The input params should be checked first.");
#endif
    CreateGeocodeAsyncContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

#ifdef ENABLE_NAPI_MANAGER
napi_value IsLocationPrivacyConfirmed(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (g_locatorClient == nullptr) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
        return UndefinedNapiValue(env);
    }
    // 1 arguement is necessary
    if (argc < 1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    napi_value res;
    bool isEnabled = g_locatorClient->IsLocationPrivacyConfirmed(type);
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &res));
    return res;
}

napi_value SetLocationPrivacyConfirmStatus(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (g_locatorClient == nullptr) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
        return UndefinedNapiValue(env);
    }
    // 2 arguement is necessary
    if (argc < 2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    bool isConfirmed;
    NAPI_CALL(env, napi_get_value_bool(env, argv[1], &isConfirmed));
    int errorCode = g_locatorClient->SetLocationPrivacyConfirmStatus(type, isConfirmed);
    if (errorCode != REPLY_CODE_NO_EXCEPTION) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
    }
    return UndefinedNapiValue(env);
}
#endif

napi_value GetCachedGnssLocationsSize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    auto asyncContext = new (std::nothrow) CachedAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "GetCachedGnssLocationsSize", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        if (!g_locatorClient->IsLocationEnabled()) {
            context->errCode = ERRCODE_SWITCH_OFF;
            return;
        }
#endif
        context->locationSize = g_locatorClient->GetCachedGnssLocationsSize();
#ifdef ENABLE_NAPI_MANAGER
        context->errCode = (context->locationSize >= 0) ? SUCCESS : ERRCODE_NOT_SUPPORTED;
#else
        context->errCode = (context->locationSize >= 0) ? SUCCESS : NOT_SUPPORTED;
#endif
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_int32(context->env, context->locationSize, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push GetCachedGnssLocationsSize result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value FlushCachedGnssLocations(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    auto asyncContext = new (std::nothrow) CachedAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "FlushCachedGnssLocations", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
        int replyCode = 0;
#ifdef ENABLE_NAPI_MANAGER
        if (!g_locatorClient->IsLocationEnabled()) {
            context->errCode = ERRCODE_SWITCH_OFF;
            return;
        }
        replyCode = g_locatorClient->FlushCachedGnssLocations();
        context->errCode = replyCode == REPLY_CODE_UNSUPPORT ? ERRCODE_NOT_SUPPORTED : SUCCESS;
#else
        if (g_locatorClient->IsLocationEnabled()) {
            replyCode = g_locatorClient->FlushCachedGnssLocations();
        }
        context->errCode = replyCode == REPLY_CODE_UNSUPPORT ? NOT_SUPPORTED : SUCCESS;
#endif
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        NAPI_CALL_RETURN_VOID(context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
#else
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
#endif
        LBSLOGI(LOCATOR_STANDARD, "Push FlushCachedGnssLocations result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void CreateCommandAsyncContext(CommandAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void * data) -> void {
        auto context = static_cast<CommandAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        if (context->command != nullptr) {
            g_locatorClient->SendCommand(context->command);
        }
        context->errCode = ERRCODE_NOT_SUPPORTED;
#else
        if (g_locatorClient->IsLocationEnabled() && context->command != nullptr) {
            g_locatorClient->SendCommand(context->command);
        }
        context->errCode = NOT_SUPPORTED;
#endif
    };
    asyncContext->completeFunc = [&](void * data) -> void {
        auto context = static_cast<CommandAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
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
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
#ifdef ENABLE_NAPI_MANAGER
    if (argc < 1) {
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
    auto asyncContext = new (std::nothrow) CommandAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    asyncContext->command = std::make_unique<LocationCommand>();
    NAPI_CALL(env, napi_create_string_latin1(env, "SendCommand", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    int errCode = JsObjToCommand(env, argv[0], asyncContext->command);
#ifdef ENABLE_NAPI_MANAGER
    if (errCode == INPUT_PARAMS_ERROR) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, errCode != INPUT_PARAMS_ERROR, "The input params should be checked first.");
#endif
    CreateCommandAsyncContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

#ifdef ENABLE_NAPI_MANAGER
napi_value GetIsoCountryCode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    CountryCodeContext *asyncContext = new (std::nothrow) CountryCodeContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "CountryCodeContext", NAPI_AUTO_LENGTH, &asyncContext->resourceName);

    asyncContext->executeFunc = [&](void *data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "GetIsoCountryCode data == nullptr");
            return;
        }
        CountryCodeContext *context = static_cast<CountryCodeContext*>(data);
        std::shared_ptr<CountryCode> country = g_locatorClient->GetIsoCountryCode();
        if (country) {
            context->errCode = SUCCESS;
            context->country = country;
        } else {
            context->errCode = ERRCODE_COUNTRYCODE_FAIL;
        }
    };
    asyncContext->completeFunc = [&](void *data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "GetIsoCountryCode data == nullptr");
            return;
        }
        CountryCodeContext *context = static_cast<CountryCodeContext *>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
        if (context->country) {
            CountryCodeToJs(context->env, context->country, context->result[PARAM1]);
        } else {
            LBSLOGE(LOCATOR_STANDARD, "country is nullptr!");
        }
        LBSLOGI(LOCATOR_STANDARD, "Push GetIsoCountryCode result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
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
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    if (!g_locatorClient->IsLocationEnabled()) {
        HandleSyncErrCode(env, ERRCODE_SWITCH_OFF);
        return UndefinedNapiValue(env);
    }
    g_locatorClient->EnableLocationMock();
    return UndefinedNapiValue(env);
}

napi_value DisableLocationMock(napi_env env, napi_callback_info info)
{
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    if (!g_locatorClient->IsLocationEnabled()) {
        HandleSyncErrCode(env, ERRCODE_SWITCH_OFF);
        return UndefinedNapiValue(env);
    }
    g_locatorClient->DisableLocationMock();
    return UndefinedNapiValue(env);
}

napi_value SetMockedLocations(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    LocationMockAsyncContext *asyncContext = new (std::nothrow) LocationMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env,
        "SetMockedLocations", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->errCode = ParseLocationMockParams(env, asyncContext, argv[0]);
    if (asyncContext->errCode == INPUT_PARAMS_ERROR) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    if (!g_locatorClient->IsLocationEnabled()) {
        HandleSyncErrCode(env, ERRCODE_SWITCH_OFF);
        return UndefinedNapiValue(env);
    }
    bool ret = g_locatorClient->SetMockedLocations(asyncContext->timeInterval, asyncContext->LocationNapi);
    if (!ret) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
    }
    return UndefinedNapiValue(env);
}

napi_value EnableReverseGeocodingMock(napi_env env, napi_callback_info info)
{
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    g_locatorClient->EnableReverseGeocodingMock();
    return UndefinedNapiValue(env);
}

napi_value DisableReverseGeocodingMock(napi_env env, napi_callback_info info)
{
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    g_locatorClient->DisableReverseGeocodingMock();
    return UndefinedNapiValue(env);
}

napi_value SetReverseGeocodingMockInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[0], &isArray));
    if (!isArray) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    JsObjToRevGeocodeMock(env, argv[0], mockInfo);
    bool ret = g_locatorClient->SetReverseGeocodingMockInfo(mockInfo);
    if (!ret) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
    }
    return UndefinedNapiValue(env);
}
#endif
} // namespace Location
} // namespace OHOS
