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
#include "country_code.h"

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
    size_t argc = PARAM1;
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
        context->errCode = g_locatorClient->SetLocationPrivacyConfirmStatus(context->type, context->isConfirmed) ? SUCCESS : LOCATOR_ERROR;
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<PrivacyAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
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
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    NAPI_ASSERT(env, argc >= 0, "Wrong number of arguments");

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
            context->errCode = QUERY_COUNTRY_CODE_ERROR;
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

bool ParseLocationMockParams(napi_env env, LocationMockAsyncContext *asyncContext, napi_value object)
{
    JsObjectToInt(env, object, "scenario", asyncContext->scenario);
    JsObjectToInt(env, object, "timeInterval", asyncContext->timeInterval);
    bool result = false;
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, "locations", &result), false);
    if (result) {
        NAPI_CALL_BASE(env, napi_get_named_property(env, object, "locations", &value), false);
        bool isArray = false;
        NAPI_CALL_BASE(env, napi_is_array(env, value, &isArray), false);
        if (!isArray) {
            LBSLOGE(LOCATOR_STANDARD, "not an array!");
            return false;
        }
        GetLocationArray(env, asyncContext, value);
    }
    return true;
}

napi_value SetLocationMockState(napi_env env, napi_callback_info info, bool enable)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    LocationMockAsyncContext *asyncContext = new (std::nothrow) LocationMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env,
        "SetLocationMockState ", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    ParseLocationMockParams(env, asyncContext, argv[0]);
    asyncContext->enable = enable;
    asyncContext->executeFunc = [&](void *data) -> void {
        LocationMockAsyncContext *context = static_cast<LocationMockAsyncContext *>(data);
        LocationMockConfig mockInfo;
        mockInfo.SetScenario(context->scenario);
        mockInfo.SetTimeInterval(context->timeInterval);
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            bool ret = context->enable ? g_locatorClient->EnableLocationMock(mockInfo) :
                g_locatorClient->DisableLocationMock(mockInfo);
            context->errCode = ret ? SUCCESS : LOCATOR_ERROR;
        }
    };
    asyncContext->completeFunc = [&](void *data) -> void {};

    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value EnableLocationMock(napi_env env, napi_callback_info info)
{
    return SetLocationMockState(env, info, true);
}

napi_value DisableLocationMock(napi_env env, napi_callback_info info)
{
    return SetLocationMockState(env, info, false);
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
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");

    LocationMockAsyncContext *asyncContext = new (std::nothrow) LocationMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env,
        "SetMockedLocations", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    ParseLocationMockParams(env, asyncContext, argv[0]);
    asyncContext->executeFunc = [&](void *data) -> void {
        LocationMockAsyncContext *context = static_cast<LocationMockAsyncContext *>(data);
        LocationMockConfig mockInfo;
        mockInfo.SetScenario(context->scenario);
        mockInfo.SetTimeInterval(context->timeInterval);
        context->errCode = LOCATION_SWITCH_ERROR;
        if (g_locatorClient->IsLocationEnabled()) {
            bool ret = g_locatorClient->SetMockedLocations(mockInfo, context->LocationNapi);
            context->errCode = ret ? SUCCESS : LOCATOR_ERROR;
        }
    };
    asyncContext->completeFunc = [&](void *data) -> void {};

    size_t objectArgsNum = 2;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value SetReverseGeocodingMockState(napi_env env, napi_callback_info info, bool enable)
{
    size_t argc = 1;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");

    auto asyncContext = new (std::nothrow) ReverseGeocodeMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "SetReverseGeocodingMockState",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->enable = enable;
    asyncContext->executeFunc = [&](void *data) -> void {
        auto context = static_cast<ReverseGeocodeMockAsyncContext *>(data);
        bool ret = context->enable ? g_locatorClient->EnableReverseGeocodingMock() :
            g_locatorClient->DisableReverseGeocodingMock();
        context->errCode = ret ? SUCCESS : REVERSE_GEOCODE_ERROR;
    };
    asyncContext->completeFunc = [&](void *data) -> void {};

    size_t nonCallbackArgNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value EnableReverseGeocodingMock(napi_env env, napi_callback_info info)
{
    return SetReverseGeocodingMockState(env, info, true);
}

napi_value DisableReverseGeocodingMock(napi_env env, napi_callback_info info)
{
    return SetReverseGeocodingMockState(env, info, false);
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
    NAPI_ASSERT(env, isArray, "Wrong argument type, object is expected for parameter 1.");

    ReverseGeocodeMockAsyncContext *asyncContext = new (std::nothrow) ReverseGeocodeMockAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "SetReverseGeocodingMockInfo",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));

    JsObjToRevGeocodeMock(env, argv[0], asyncContext->mockInfo);

    asyncContext->executeFunc = [&](void *data) -> void {
        ReverseGeocodeMockAsyncContext *context = static_cast<ReverseGeocodeMockAsyncContext *>(data);
        bool ret = g_locatorClient->SetReverseGeocodingMockInfo(context->mockInfo);
        context->errCode = ret ? SUCCESS : REVERSE_GEOCODE_ERROR;
    };
    asyncContext->completeFunc = [&](void *data) -> void {};

    size_t nonCallbackArgNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}
} // namespace Location
} // namespace OHOS
