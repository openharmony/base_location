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

namespace OHOS {
namespace Location {
std::unique_ptr<Locator> g_locatorClient = Locator::GetInstance();

napi_value GetLastLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    LocationAsyncContext* asyncContext = new (std::nothrow) LocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "getLastLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        LocationAsyncContext* context = static_cast<LocationAsyncContext*>(data);
        context->loc = g_locatorClient->IsLocationEnabled() ? g_locatorClient->GetCachedLocation() : nullptr;
        if (context->loc != nullptr) {
            context->errCode = SUCCESS;
        } else {
            context->errCode = LAST_KNOWN_LOCATION_ERROR;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        LocationAsyncContext* context = static_cast<LocationAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
        if (context->loc != nullptr) {
            LocationToJs(context->env, context->loc, context->result[PARAM1]);
        } else {
            LBSLOGE(LOCATOR_STANDARD, "loc is nullptr!");
        }
        LBSLOGI(LOCATOR_STANDARD, "Push last location result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value IsLocationEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    SwitchAsyncContext* asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "isLocationEnabled", NAPI_AUTO_LENGTH, &asyncContext->resourceName);

    asyncContext->executeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        context->enable = g_locatorClient->IsLocationEnabled();
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push IsLocationEnabled result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value EnableLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    SwitchAsyncContext* asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "enableLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        g_locatorClient->EnableAbility(true);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push EnableLocation result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value DisableLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    SwitchAsyncContext* asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "disableLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        g_locatorClient->EnableAbility(false);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push DisableLocation result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value RequestEnableLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    SwitchAsyncContext* asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "enableLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        if (!g_locatorClient->IsLocationEnabled()) {
            g_locatorClient->ShowNotification();
        }
        g_locatorClient->EnableAbility(true);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push RequestEnableLocation result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value IsGeoServiceAvailable(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    SwitchAsyncContext* asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "isGeoServiceAvailable", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->executeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        context->enable = g_locatorClient->IsLocationEnabled() && g_locatorClient->IsGeoServiceAvailable();
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        SwitchAsyncContext* context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push isGeoServiceAvailable result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value GetAddressesFromLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    ReverseGeoCodeAsyncContext* asyncContext = new (std::nothrow) ReverseGeoCodeAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "getAddressesFromLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    JsObjToReverseGeoCodeRequest(env, argv[0], asyncContext->reverseGeoCodeRequest);

    asyncContext->executeFunc = [&](void* data) -> void {
        ReverseGeoCodeAsyncContext* context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        if (g_locatorClient->IsLocationEnabled()) {
            g_locatorClient->GetAddressByCoordinate(context->reverseGeoCodeRequest, context->replyList);
        }
        if (context->replyList.empty()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
        } else {
            context->errCode = SUCCESS;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        ReverseGeoCodeAsyncContext* context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push GetAddressesFromLocation result to client");
    };

    size_t nonCallbackArgNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value GetAddressesFromLocationName(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    GeoCodeAsyncContext* asyncContext = new (std::nothrow) GeoCodeAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "GetAddressesFromLocationName", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    JsObjToGeoCodeRequest(env, argv[0], asyncContext->geoCodeRequest);

    asyncContext->executeFunc = [&](void* data) -> void {
        GeoCodeAsyncContext* context = static_cast<GeoCodeAsyncContext*>(data);
        if (g_locatorClient->IsLocationEnabled()) {
            g_locatorClient->GetAddressByLocationName(context->geoCodeRequest,
                context->replyList);
        }
        if (context->replyList.empty()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
        } else {
            context->errCode = SUCCESS;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        GeoCodeAsyncContext* context = static_cast<GeoCodeAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push GetAddressesFromLocationName result to client");
    };

    size_t nonCallbackArgNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value IsLocationPrivacyConfirmed(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    PrivacyAsyncContext* asyncContext = new (std::nothrow) PrivacyAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "isLocationPrivacyConfirmed", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    asyncContext->type = type;
    asyncContext->executeFunc = [&](void* data) -> void {
        PrivacyAsyncContext* context = static_cast<PrivacyAsyncContext*>(data);
        context->isConfirmed = g_locatorClient->IsLocationPrivacyConfirmed(context->type);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        PrivacyAsyncContext* context = static_cast<PrivacyAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->isConfirmed, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push IsLocationPrivacyConfirmed result to client");
    };

    size_t nonCallbackArgNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value SetLocationPrivacyConfirmStatus(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    PrivacyAsyncContext* asyncContext = new (std::nothrow) PrivacyAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "SetLocationPrivacyConfirmStatus",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    asyncContext->type = type;
    NAPI_CALL(env, napi_get_value_bool(env, argv[1], &asyncContext->isConfirmed));
    asyncContext->executeFunc = [&](void* data) -> void {
        PrivacyAsyncContext* context = static_cast<PrivacyAsyncContext*>(data);
        g_locatorClient->SetLocationPrivacyConfirmStatus(context->type, context->isConfirmed);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        PrivacyAsyncContext* context = static_cast<PrivacyAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push SetLocationPrivacyConfirmStatus result to client");
    };

    size_t nonCallbackArgNum = 2;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value GetCachedGnssLocationsSize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    CachedAsyncContext* asyncContext = new (std::nothrow) CachedAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "GetCachedGnssLocationsSize", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        CachedAsyncContext* context = static_cast<CachedAsyncContext*>(data);
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            context->locationSize = g_locatorClient->GetCachedGnssLocationsSize();
            context->errCode = SUCCESS;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        CachedAsyncContext* context = static_cast<CachedAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_int32(context->env, context->locationSize, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push GetCachedGnssLocationsSize result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value FlushCachedGnssLocations(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    CachedAsyncContext* asyncContext = new (std::nothrow) CachedAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "FlushCachedGnssLocations", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->executeFunc = [&](void* data) -> void {
        CachedAsyncContext* context = static_cast<CachedAsyncContext*>(data);
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            g_locatorClient->FlushCachedGnssLocations();
            context->errCode = SUCCESS;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        CachedAsyncContext* context = static_cast<CachedAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push FlushCachedGnssLocations result to client");
    };

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value SendCommand(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    CommandAsyncContext* asyncContext = new (std::nothrow) CommandAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "SendCommand", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    JsObjToCommand(env, argv[0], asyncContext->command);

    asyncContext->executeFunc = [&](void* data) -> void {
        CommandAsyncContext* context = static_cast<CommandAsyncContext*>(data);
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            context->enable = g_locatorClient->SendCommand(context->command);
            context->errCode = SUCCESS;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        CommandAsyncContext* context = static_cast<CommandAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push SendCommand result to client");
    };

    size_t nonCallbackArgNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}
}  // namespace Location
}  // namespace OHOS
