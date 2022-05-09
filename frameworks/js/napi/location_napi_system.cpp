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
std::unique_ptr<Locator> g_locatorPtr1 = Locator::GetInstance(LOCATION_LOCATOR_SA_ID);

napi_value GetLocationOnce(const napi_env& env,
                           const napi_ref& successHandlerRef,
                           const napi_ref& failHandlerRef,
                           const napi_ref& completeHandlerRef,
                           int fixNumber)
{
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(g_systemSingleLocatorCallbackHost);
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
        g_locatorPtr1->StopLocating(locatorCallback);
        if (g_systemSingleLocatorCallbackHost->m_successHandlerCb != nullptr) {
            g_systemSingleLocatorCallbackHost->DeleteSuccessHandler();
        }
        if (g_systemSingleLocatorCallbackHost->m_failHandlerCb != nullptr) {
            g_systemSingleLocatorCallbackHost->DeleteFailHandler();
        }
        if (g_systemSingleLocatorCallbackHost->m_completeHandlerCb != nullptr) {
            g_systemSingleLocatorCallbackHost->DeleteCompleteHandler();
        }
    }
    g_systemSingleLocatorCallbackHost->m_env = env;
    g_systemSingleLocatorCallbackHost->m_fixNumber = fixNumber;
    g_systemSingleLocatorCallbackHost->m_successHandlerCb = successHandlerRef;
    g_systemSingleLocatorCallbackHost->m_failHandlerCb = failHandlerRef;
    g_systemSingleLocatorCallbackHost->m_completeHandlerCb = completeHandlerRef;
    g_locatorPtr1->StartLocating(requestConfig, locatorCallback);
    return UndefinedNapiValue(env);
}

void GetAllCallback(const napi_env &env, const napi_value &argv, napi_ref &successHandlerRef,
    napi_ref &failHandlerRef, napi_ref &completeHandlerRef)
{
    bool hasProperty = false;
    napi_value nVsuccessCallback, nVfailCallback, nVcompleteCallback;
    napi_has_named_property(env, argv, "success", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv, "success", &nVsuccessCallback);
        napi_create_reference(env, nVsuccessCallback, 1, &successHandlerRef);
    }
    hasProperty = false;
    napi_has_named_property(env, argv, "fail", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv, "fail", &nVfailCallback);
        napi_create_reference(env, nVfailCallback, 1, &failHandlerRef);
    }
    hasProperty = false;
    napi_has_named_property(env, argv, "complete", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv, "complete", &nVcompleteCallback);
        napi_create_reference(env, nVcompleteCallback, 1, &completeHandlerRef);
    }
}

napi_value GetLocation(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0}, thisVar = 0, result = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, argc == 1, "number of parameters is error");
    NAPI_ASSERT(env, valueType == napi_object, "type of parameters is error");
    NAPI_ASSERT(env, g_locatorPtr1 != nullptr, "locator instance is null.");
    napi_value nVtimeout, nVcoordType;
    int32_t timeout = 0;
    std::string coordType = "";
    napi_ref successHandlerRef = nullptr, failHandlerRef = nullptr, completeHandlerRef = nullptr;
    bool hasProperty = false;
    napi_has_named_property(env, argv[0], "timeout", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv[0], "timeout", &nVtimeout);
        napi_get_value_int32(env, nVtimeout, &timeout);
    }
    hasProperty = false;
    napi_has_named_property(env, argv[0], "coordType", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv[0], "coordType", &nVcoordType);
        char type[64] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, nVcoordType, type, sizeof(type), &typeLen);
        coordType = type;
        if (coordType != "wgs84") {
            napi_get_undefined(env, &result);
            return result;
        }
    }
    GetAllCallback(env, argv[0], successHandlerRef, failHandlerRef, completeHandlerRef);
    int fixnumber = 1;
    GetLocationOnce(env, successHandlerRef, failHandlerRef, completeHandlerRef, fixnumber);
    napi_get_undefined(env, &result);
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
    napi_create_array(env, &arrString);
    napi_create_string_utf8(env, "gps", NAPI_AUTO_LENGTH, &value);
    napi_set_element(env, arrString, arrIndex, value);
    arrIndex++;
    napi_create_string_utf8(env, "network", NAPI_AUTO_LENGTH, &value);
    napi_set_element(env, arrString, arrIndex, value);
    napi_create_object(env, &jsEvent);
    napi_set_named_property(env, jsEvent, "types", arrString);
    napi_value undefine;
    napi_get_undefined(env, &undefine);
    napi_call_function(env, nullptr, successHandler, 1, &jsEvent, &undefine);
    return true;
}

napi_value GetLocationType(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = 0;
    napi_value result = nullptr;
    napi_status status = napi_generic_failure;
    napi_valuetype valueType = napi_undefined;
    status = napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    NAPI_ASSERT(env, status == napi_ok, "get parameters from Js error");
    NAPI_ASSERT(env, argc == 1, "number of parameters is error");
    status = napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, status == napi_ok, "get parameters type error");
    NAPI_ASSERT(env, valueType == napi_object, "type of parameters is error");
    napi_value nVsuccessCallback;
    napi_value nVfailCallback;
    napi_value nVcompleteCallback;
    bool hasProperty = false;
    napi_has_named_property(env, argv[0], "success", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv[0], "success", &nVsuccessCallback);
    }
    hasProperty = false;
    napi_has_named_property(env, argv[0], "fail", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv[0], "fail", &nVfailCallback);
    }
    hasProperty = false;
    napi_has_named_property(env, argv[0], "complete", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv[0], "complete", &nVcompleteCallback);
    }
    EmitSyncCallbackWork(env, nVsuccessCallback, nVfailCallback, nVcompleteCallback);
    napi_get_undefined(env, &result);
    return result;
}

void SubscribeSystemLocationChange(napi_env env,
                                   napi_ref& successHandlerRef,
                                   napi_ref& failHandlerRef,
                                   int fixNumber,
                                   sptr<LocatorCallbackHost>& locatorCallbackHost)
{
    if (g_locatorPtr1 == nullptr) {
        LBSLOGE(LOCATION_NAPI, "g_locatorPtr1 is nullptr, return.");
        return;
    }
    sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
    if (locatorCallbackHost->m_successHandlerCb != nullptr ||
        locatorCallbackHost->m_failHandlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeLocationChange");
        g_locatorPtr1->StopLocating(locatorCallback);
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
    g_locatorPtr1->StartLocating(requestConfig, locatorCallback);
}

napi_value Subscribe(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0}, thisVar = 0, result = nullptr;
    napi_value nVcoordType;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, argc == 1, "number of parameters is error");
    NAPI_ASSERT(env, valueType == napi_object, "type of parameters is error");
    std::string coordType = "";
    napi_ref successHandlerRef = nullptr, failHandlerRef = nullptr, completeHandlerRef = nullptr;
    bool hasProperty = false;
    napi_has_named_property(env, argv[0], "coordType", &hasProperty);
    if (hasProperty) {
        napi_get_named_property(env, argv[0], "coordType", &nVcoordType);
        char type[64] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, nVcoordType, type, sizeof(type), &typeLen);
        coordType = type;
        if (coordType != "wgs84") {
            napi_get_undefined(env, &result);
            return result;
        }
    }
    GetAllCallback(env, argv[0], successHandlerRef, failHandlerRef, completeHandlerRef);
    SubscribeSystemLocationChange(env, successHandlerRef, failHandlerRef, 0, g_systemSubcribeCallbackHost);
    napi_get_undefined(env, &result);
    return result;
}

napi_value Unsubscribe(napi_env env, napi_callback_info cbinfo)
{
    napi_value result = nullptr;
    sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(g_systemSubcribeCallbackHost);
    g_locatorPtr1->StopLocating(locatorCallback);
    g_systemSubcribeCallbackHost->DeleteHandler();
    g_systemSubcribeCallbackHost->DeleteSuccessHandler();
    g_systemSubcribeCallbackHost->DeleteFailHandler();
    g_systemSubcribeCallbackHost->DeleteCompleteHandler();
    napi_get_undefined(env, &result);
    return result;
}

napi_value GetSupportedCoordTypes(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = 0;
    napi_status status = napi_generic_failure;
    status = napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    NAPI_ASSERT(env, status == napi_ok, "get parameters from Js error");
    NAPI_ASSERT(env, argc == 0, "number of parameters is error");
    napi_value arrString = nullptr;
    napi_value value;
    int arrIndex = 0;
    napi_create_array(env, &arrString);
    napi_create_string_utf8(env, "wgs84", NAPI_AUTO_LENGTH, &value);
    napi_set_element(env, arrString, arrIndex, value);
    return arrString;
}
}  // namespace Location
}  // namespace OHOS
