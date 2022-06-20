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
#include "location_napi_system.h"
#include "locator_callback_host.h"
#include "location_log.h"
#include "locator.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
sptr<LocatorCallbackHost> g_systemSingleLocatorCallbackHost =
    sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());
sptr<LocatorCallbackHost> g_systemSubcribeCallbackHost =
    sptr<LocatorCallbackHost>(new (std::nothrow)LocatorCallbackHost());;
std::unique_ptr<Locator> g_locatorImpl = Locator::GetInstance();

napi_value GetLocationOnce(const napi_env& env,
                           const napi_ref& successHandlerRef,
                           const napi_ref& failHandlerRef,
                           const napi_ref& completeHandlerRef,
                           int fixNumber)
{
    NAPI_ASSERT(env, g_locatorImpl != nullptr, "get locator SA failed");
    auto requestConfig = std::make_unique<RequestConfig>();
    auto locatorCallback = sptr<ILocatorCallback>(g_systemSingleLocatorCallbackHost);
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetScenario(SCENE_UNSET);
    requestConfig->SetTimeInterval(0);
    requestConfig->SetDistanceInterval(0);
    requestConfig->SetMaxAccuracy(0);
    requestConfig->SetFixNumber(fixNumber);
    if (g_systemSingleLocatorCallbackHost->m_successHandlerCb != nullptr ||
        g_systemSingleLocatorCallbackHost->m_failHandlerCb != nullptr ||
        g_systemSingleLocatorCallbackHost->m_completeHandlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "handlers is not nullptr, stop locating first");
        g_locatorImpl->StopLocating(locatorCallback);
        g_systemSingleLocatorCallbackHost->DeleteAllCallbacks();
    }
    g_systemSingleLocatorCallbackHost->m_env = env;
    g_systemSingleLocatorCallbackHost->m_fixNumber = fixNumber;
    g_systemSingleLocatorCallbackHost->m_successHandlerCb = successHandlerRef;
    g_systemSingleLocatorCallbackHost->m_failHandlerCb = failHandlerRef;
    g_systemSingleLocatorCallbackHost->m_completeHandlerCb = completeHandlerRef;
    g_locatorImpl->StartLocating(requestConfig, locatorCallback);
    return UndefinedNapiValue(env);
}

void GetAllCallback(const napi_env &env, const napi_value &argv, napi_ref &successHandlerRef,
    napi_ref &failHandlerRef, napi_ref &completeHandlerRef)
{
    bool hasProperty = false;
    napi_value nVsuccessCallback = nullptr, nVfailCallback = nullptr, nVcompleteCallback = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, argv, "success", &hasProperty));
    if (hasProperty) {
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "success", &nVsuccessCallback));
        NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, nVsuccessCallback, 1, &successHandlerRef));
    }
    hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, argv, "fail", &hasProperty));
    if (hasProperty) {
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "fail", &nVfailCallback));
        NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, nVfailCallback, 1, &failHandlerRef));
    }
    hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, argv, "complete", &hasProperty));
    if (hasProperty) {
        NAPI_CALL_RETURN_VOID(env, napi_get_named_property(env, argv, "complete", &nVcompleteCallback));
        NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, nVcompleteCallback, 1, &completeHandlerRef));
    }
}

napi_value GetLocation(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0}, thisVar = 0, result = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, argc == 1, "number of parameters is error");
    NAPI_ASSERT(env, valueType == napi_object, "type of parameters is error");
    NAPI_ASSERT(env, g_locatorImpl != nullptr, "get locator SA failed");
    napi_value nVtimeout, nVcoordType;
    int32_t timeout = 0;
    std::string coordType = "";
    napi_ref successHandlerRef = nullptr, failHandlerRef = nullptr, completeHandlerRef = nullptr;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, argv[0], "timeout", &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "timeout", &nVtimeout));
        NAPI_CALL(env, napi_get_value_int32(env, nVtimeout, &timeout));
    }
    hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, argv[0], "coordType", &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "coordType", &nVcoordType));
        char type[64] = {0};
        size_t typeLen = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, nVcoordType, type, sizeof(type), &typeLen));
        coordType = type;
        if (coordType != "wgs84") {
            NAPI_CALL(env, napi_get_undefined(env, &result));
            return result;
        }
    }
    GetAllCallback(env, argv[0], successHandlerRef, failHandlerRef, completeHandlerRef);
    int fixnumber = 1;
    GetLocationOnce(env, successHandlerRef, failHandlerRef, completeHandlerRef, fixnumber);
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

bool EmitSyncCallbackWork(const napi_env& env,
                          const napi_value& successHandler,
                          const napi_value& failHandler,
                          const napi_value& completeHandler)
{
    napi_value jsEvent = nullptr;
    napi_value arrString = nullptr;
    napi_value value;
    int arrIndex = 0;
    NAPI_CALL_BASE(env, napi_create_array(env, &arrString), false);
    NAPI_CALL_BASE(env, napi_create_string_utf8(env, "gps", NAPI_AUTO_LENGTH, &value), false);
    NAPI_CALL_BASE(env, napi_set_element(env, arrString, arrIndex, value), false);
    arrIndex++;
    NAPI_CALL_BASE(env, napi_create_string_utf8(env, "network", NAPI_AUTO_LENGTH, &value), false);
    NAPI_CALL_BASE(env, napi_set_element(env, arrString, arrIndex, value), false);
    NAPI_CALL_BASE(env, napi_create_object(env, &jsEvent), false);
    NAPI_CALL_BASE(env, napi_set_named_property(env, jsEvent, "types", arrString), false);
    napi_value undefine;
    NAPI_CALL_BASE(env, napi_get_undefined(env, &undefine), false);
    NAPI_CALL_BASE(env, napi_call_function(env, nullptr, successHandler, 1, &jsEvent, &undefine), false);
    return true;
}

napi_value GetLocationType(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = 0;
    napi_value result = nullptr;
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env,  napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc == 1, "number of parameters is error");
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "type of parameters is error");
    napi_value nVsuccessCallback;
    napi_value nVfailCallback;
    napi_value nVcompleteCallback;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, argv[0], "success", &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "success", &nVsuccessCallback));
    }
    hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, argv[0], "fail", &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "fail", &nVfailCallback));
    }
    hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, argv[0], "complete", &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "complete", &nVcompleteCallback));
    }
    EmitSyncCallbackWork(env, nVsuccessCallback, nVfailCallback, nVcompleteCallback);
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

void SubscribeSystemLocationChange(napi_env env,
                                   napi_ref& successHandlerRef,
                                   napi_ref& failHandlerRef,
                                   int fixNumber,
                                   sptr<LocatorCallbackHost>& locatorCallbackHost)
{
    auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
    if (locatorCallbackHost->m_successHandlerCb != nullptr ||
        locatorCallbackHost->m_failHandlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeLocationChange");
        g_locatorImpl->StopLocating(locatorCallback);
        if (locatorCallbackHost->m_successHandlerCb != nullptr) {
            locatorCallbackHost->DeleteSuccessHandler();
        }
        if (locatorCallbackHost->m_failHandlerCb != nullptr) {
            locatorCallbackHost->DeleteFailHandler();
        }
    }
    locatorCallbackHost->m_env = env;
    locatorCallbackHost->m_fixNumber = fixNumber;
    locatorCallbackHost->m_successHandlerCb = successHandlerRef;
    locatorCallbackHost->m_failHandlerCb = failHandlerRef;
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetScenario(SCENE_UNSET);
    requestConfig->SetTimeInterval(0);
    requestConfig->SetDistanceInterval(0);
    requestConfig->SetMaxAccuracy(0);
    requestConfig->SetFixNumber(fixNumber);
    g_locatorImpl->StartLocating(requestConfig, locatorCallback);
}

napi_value Subscribe(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0}, thisVar = 0, result = nullptr;
    napi_value nVcoordType;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, argc == 1, "number of parameters is error");
    NAPI_ASSERT(env, valueType == napi_object, "type of parameters is error");
    NAPI_ASSERT(env, g_locatorImpl != nullptr, "get locator SA failed");
    std::string coordType = "";
    napi_ref successHandlerRef = nullptr, failHandlerRef = nullptr, completeHandlerRef = nullptr;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, argv[0], "coordType", &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, argv[0], "coordType", &nVcoordType));
        char type[64] = {0};
        size_t typeLen = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, nVcoordType, type, sizeof(type), &typeLen));
        coordType = type;
        if (coordType != "wgs84") {
            NAPI_CALL(env, napi_get_undefined(env, &result));
            return result;
        }
    }
    GetAllCallback(env, argv[0], successHandlerRef, failHandlerRef, completeHandlerRef);
    SubscribeSystemLocationChange(env, successHandlerRef, failHandlerRef, 0, g_systemSubcribeCallbackHost);
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value Unsubscribe(napi_env env, napi_callback_info cbinfo)
{
    NAPI_ASSERT(env, g_locatorImpl != nullptr, "get locator SA failed");
    napi_value result = nullptr;
    auto locatorCallback = sptr<ILocatorCallback>(g_systemSubcribeCallbackHost);
    g_locatorImpl->StopLocating(locatorCallback);
    g_systemSubcribeCallbackHost->DeleteHandler();
    g_systemSubcribeCallbackHost->DeleteSuccessHandler();
    g_systemSubcribeCallbackHost->DeleteFailHandler();
    g_systemSubcribeCallbackHost->DeleteCompleteHandler();
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value GetSupportedCoordTypes(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = 0;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc == 0, "number of parameters is error");
    napi_value arrString = nullptr;
    napi_value value;
    int arrIndex = 0;
    NAPI_CALL(env, napi_create_array(env, &arrString));
    NAPI_CALL(env, napi_create_string_utf8(env, "wgs84", NAPI_AUTO_LENGTH, &value));
    NAPI_CALL(env, napi_set_element(env, arrString, arrIndex, value));
    return arrString;
}
}  // namespace Location
}  // namespace OHOS
