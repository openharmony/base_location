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

    auto asyncContext = new (std::nothrow) LocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "getLastLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

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
}

napi_value IsLocationEnabled(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "isLocationEnabled", NAPI_AUTO_LENGTH, &asyncContext->resourceName);

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
}

napi_value EnableLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
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
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push EnableLocation result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value DisableLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

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
}

napi_value RequestEnableLocation(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
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

    auto asyncContext = new (std::nothrow) ReverseGeoCodeAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "getAddressesFromLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->errCode =
        JsObjToReverseGeoCodeRequest(env, argv[0], asyncContext->reverseGeoCodeRequest) ?
        SUCCESS : INPUT_PARAMS_ERROR;

    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        if (context->errCode != SUCCESS) {
            return;
        }
        if (!g_locatorClient->IsLocationEnabled() ||
            !g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
            return;
        }
        g_locatorClient->GetAddressByCoordinate(context->reverseGeoCodeRequest, context->replyList);
        if (context->replyList.empty()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push GetAddressesFromLocation result to client");
    };

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
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    auto asyncContext = new (std::nothrow) GeoCodeAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "GetAddressesFromLocationName", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    asyncContext->errCode = JsObjToGeoCodeRequest(env, argv[0], asyncContext->geoCodeRequest) ?
        SUCCESS : INPUT_PARAMS_ERROR;
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        if (context->errCode != SUCCESS) {
            return;
        }
        if (!g_locatorClient->IsLocationEnabled() ||
            !g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = GEOCODE_ERROR;
            return;
        }
        g_locatorClient->GetAddressByLocationName(context->geoCodeRequest,
            context->replyList);
        if (context->replyList.empty()) {
            context->errCode = GEOCODE_ERROR;
        }
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push GetAddressesFromLocationName result to client");
    };

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value IsLocationPrivacyConfirmed(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    auto asyncContext = new (std::nothrow) PrivacyAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "isLocationPrivacyConfirmed", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    asyncContext->type = type;
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<PrivacyAsyncContext*>(data);
        context->isConfirmed = g_locatorClient->IsLocationPrivacyConfirmed(context->type);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<PrivacyAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->isConfirmed, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push IsLocationPrivacyConfirmed result to client");
    };

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value SetLocationPrivacyConfirmStatus(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    auto asyncContext = new (std::nothrow) PrivacyAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "SetLocationPrivacyConfirmStatus",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    int type;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &type));
    asyncContext->type = type;
    NAPI_CALL(env, napi_get_value_bool(env, argv[1], &asyncContext->isConfirmed));
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<PrivacyAsyncContext*>(data);
        g_locatorClient->SetLocationPrivacyConfirmStatus(context->type, context->isConfirmed);
        context->errCode = SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<PrivacyAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->isConfirmed, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push SetLocationPrivacyConfirmStatus result to client");
    };

    size_t objectArgsNum = 2;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

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
        context->locationSize = g_locatorClient->GetCachedGnssLocationsSize();
        context->errCode = (context->locationSize >= 0) ?
            SUCCESS : NOT_SUPPORTED;
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
        if (g_locatorClient->IsLocationEnabled()) {
            replyCode = g_locatorClient->FlushCachedGnssLocations();
        }
        context->errCode = replyCode == REPLY_CODE_UNSUPPORT ?
            NOT_SUPPORTED : SUCCESS;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push FlushCachedGnssLocations result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
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

    auto asyncContext = new (std::nothrow) CommandAsyncContext(env);
    asyncContext->command = std::make_unique<LocationCommand>();
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "SendCommand", NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    JsObjToCommand(env, argv[0], asyncContext->command);

    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CommandAsyncContext*>(data);
        if (g_locatorClient->IsLocationEnabled() && context->command != nullptr) {
            g_locatorClient->SendCommand(context->command);
        }
        context->errCode = NOT_SUPPORTED;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CommandAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push SendCommand result to client");
    };

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value GetIsoCountryCode(napi_env env, napi_callback_info info)
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
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type, object is expected for parameter 1.");

    GetIsoCountryCodeContext *asyncContext = new (std::nothrow) GetIsoCountryCodeContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "GetIsoCountryCodeContext", NAPI_AUTO_LENGTH, &asyncContext->resourceName);
    JsObjectToString(env, argv[0], "countryCode", asyncContext->buff, asyncContext->countryCode);

    asyncContext->executeFunc = [&](void *data) -> void {
        GetIsoCountryCodeContext *context = static_cast<GetIsoCountryCodeContext *>(data);
        int code = -1;
        code = g_locatorClient->GetIsoCountryCode(context->countryCode);
        if (code == 0) {
            context->errCode = SUCCESS;
        } else {
            context->errCode = REVERSE_GEOCODE_ERROR;
        }
    };
    asyncContext->completeFunc = [&](void *data) -> void {
        GetIsoCountryCodeContext *context = static_cast<GetIsoCountryCodeContext *>(data);
        napi_create_string_utf8(
            context->env, context->countryCode.c_str(), context->countryCode.size(), &context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push IsLocationEnabled result to client");
    };

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void ParameterConfiguration(napi_env env, EnableLocationMockAsyncContext *asyncContext, napi_value object)
{
    JsObjectToInt(env, object, "scenario", asyncContext->priority);
    JsObjectToInt(env, object, "scenario", asyncContext->scenario);
    JsObjectToInt(env, object, "timeInterval", asyncContext->timeInterval);
}

void ParameterConfiguration(napi_env env, DisableLocationMockAsyncContext *asyncContext, napi_value object)
{
    JsObjectToInt(env, object, "scenario", asyncContext->priority);
    JsObjectToInt(env, object, "scenario", asyncContext->scenario);
    JsObjectToInt(env, object, "timeInterval", asyncContext->timeInterval);
}

void ParameterConfiguration(napi_env env, SetMockedLocationsAsyncContext *asyncContext, napi_value object)
{
    JsObjectToInt(env, object, "scenario", asyncContext->priority);
    JsObjectToInt(env, object, "scenario", asyncContext->scenario);
    JsObjectToInt(env, object, "timeInterval", asyncContext->timeInterval);
}

napi_value EnableLocationMock(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    EnableLocationMockAsyncContext *asyncContext = new (std::nothrow) EnableLocationMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "EnableLocationMockAsyncContext", NAPI_AUTO_LENGTH, &asyncContext->resourceName);
    ParameterConfiguration(env, asyncContext, argv[0]);
    asyncContext->executeFunc = [&](void *data) -> void {
        EnableLocationMockAsyncContext *context = static_cast<EnableLocationMockAsyncContext *>(data);
        LocationMockConfig LocationMockInfo;
        LocationMockInfo.priority_ = context->priority;
        LocationMockInfo.scenario_ = context->scenario;
        LocationMockInfo.timeInterval_ = context->timeInterval;
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            context->enable = g_locatorClient->EnableLocationMock(LocationMockInfo);
            context->errCode = SUCCESS;
        }
    };
    asyncContext->completeFunc = [&](void *data) -> void {
        EnableLocationMockAsyncContext *context = static_cast<EnableLocationMockAsyncContext *>(data);
        napi_get_boolean(context->env, context->enable, &context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push SendCommand result to client");
    };

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value DisableLocationMock(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    DisableLocationMockAsyncContext *asyncContext = new (std::nothrow) DisableLocationMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "DisableLocationMockAsyncContext", NAPI_AUTO_LENGTH, &asyncContext->resourceName);
    ParameterConfiguration(env, asyncContext, argv[0]);
    asyncContext->executeFunc = [&](void *data) -> void {
        DisableLocationMockAsyncContext *context = static_cast<DisableLocationMockAsyncContext *>(data);
        LocationMockConfig LocationMockInfo;
        LocationMockInfo.priority_ = context->priority;
        LocationMockInfo.scenario_ = context->scenario;
        LocationMockInfo.timeInterval_ = context->timeInterval;
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            context->enable = g_locatorClient->DisableLocationMock(LocationMockInfo);
            context->errCode = SUCCESS;
        }
    };
    asyncContext->completeFunc = [&](void *data) -> void {
        DisableLocationMockAsyncContext *context = static_cast<DisableLocationMockAsyncContext *>(data);
        napi_get_boolean(context->env, context->enable, &context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push SendCommand result to client");
    };

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void ParameterConfiguration(
    napi_env env, uint32_t arrayLength, napi_value object, SetMockedLocationsAsyncContext *asyncContext)
{
    napi_get_array_length(env, object, &arrayLength);
    std::shared_ptr<Location> locationAdapter = std::make_shared<Location>();
    if (locationAdapter == nullptr) {
        LBSLOGI(LOCATOR_STANDARD, "location_adapter is nullptr");
        return;
    }
    napi_value elementValue = nullptr;
    for (uint32_t i = 0; i < arrayLength; i++) {
        napi_get_element(env, object, i, &elementValue);
        double latitude = 0.0;
        JsObjectToDouble(env, elementValue, "latitude", latitude);
        locationAdapter->SetLatitude(latitude);
        double longitude = 0.0;
        JsObjectToDouble(env, elementValue, "longitude", longitude);
        locationAdapter->SetLongitude(longitude);
        double altitude = 0.0;
        JsObjectToDouble(env, elementValue, "altitude", altitude);
        locationAdapter->SetAltitude(altitude);
        int32_t accuracy = 0;
        JsObjectToInt(env, elementValue, "accuracy", accuracy);
        locationAdapter->SetAccuracy(static_cast<float>(accuracy));
        int32_t speed = 0;
        JsObjectToInt(env, elementValue, "speed", speed);
        locationAdapter->SetSpeed(static_cast<float>(speed));
        double direction = 0.0;
        JsObjectToDouble(env, elementValue, "direction", direction);
        locationAdapter->SetDirection(direction);
        int32_t timeStamp = 0;
        JsObjectToInt(env, elementValue, "timeStamp", timeStamp);
        locationAdapter->SetTimeStamp(static_cast<int64_t>(accuracy));
        int32_t timeSinceBoot = 0;
        JsObjectToInt(env, elementValue, "timeSinceBoot", timeSinceBoot);
        locationAdapter->SetTimeSinceBoot(static_cast<int64_t>(timeSinceBoot));
        std::string additions = " ";
        int buff = 100;
        JsObjectToString(env, elementValue, "additions", buff, additions);
        locationAdapter->SetAdditions(additions);
        int32_t additionSize = 0;
        JsObjectToInt(env, elementValue, "additionSize", additionSize);
        locationAdapter->SetAdditionSize(static_cast<int64_t>(additionSize));
        bool isFromMock = false;
        JsObjectToBool(env, elementValue, "isFromMock", isFromMock);
        locationAdapter->SetIsFromMock(isFromMock);
        asyncContext->LocationNapi.push_back(locationAdapter);
    }
}

napi_value SetMockedLocations(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    bool filePathIsArray = false;
    napi_is_array(env, argv[1], &filePathIsArray);
    NAPI_ASSERT(env, filePathIsArray, "Wrong argument type, object is expected for parameter 2.");

    SetMockedLocationsAsyncContext *asyncContext = new (std::nothrow) SetMockedLocationsAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    napi_create_string_latin1(env, "SetMockedLocationsAsyncContext", NAPI_AUTO_LENGTH, &asyncContext->resourceName);
    uint32_t arrayLength = 0;
    ParameterConfiguration(env, asyncContext, argv[0]);
    ParameterConfiguration(env, arrayLength, argv[1], asyncContext);
    asyncContext->executeFunc = [&](void *data) -> void {
        SetMockedLocationsAsyncContext *context = static_cast<SetMockedLocationsAsyncContext *>(data);
        LocationMockConfig LocationMockInfo;
        LocationMockInfo.priority_ = context->priority;
        LocationMockInfo.scenario_ = context->scenario;
        LocationMockInfo.timeInterval_ = context->timeInterval;
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            context->enable = g_locatorClient->SetMockedLocations(LocationMockInfo, context->LocationNapi);
            context->errCode = SUCCESS;
        }
    };
    asyncContext->completeFunc = [&](void *data) -> void {
        SetMockedLocationsAsyncContext *context = static_cast<SetMockedLocationsAsyncContext *>(data);
        napi_get_boolean(context->env, context->enable, &context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push SendCommand result to client");
    };

    size_t objectArgsNum = 2;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}
} // namespace Location
} // namespace OHOS
