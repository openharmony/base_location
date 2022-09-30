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
#include "location_napi_event.h"
#include <unistd.h>
#include "common_utils.h"
#include "constant_definition.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "location_log.h"
#include "location_napi_adapter.h"
#include "napi_util.h"
#include "locator.h"
#include "request_config.h"
#include "system_ability_definition.h"
#include "callback_manager.h"
#include "country_code_callback_host.h"

namespace OHOS {
namespace Location {
CallbackManager<LocationSwitchCallbackHost> g_switchCallbacks;
CallbackManager<LocatorCallbackHost> g_locationCallbacks;
CallbackManager<GnssStatusCallbackHost> g_gnssStatusInfoCallbacks;
CallbackManager<NmeaMessageCallbackHost> g_nmeaCallbacks;
CallbackManager<CachedLocationsCallbackHost> g_cachedLocationCallbacks;
CallbackManager<CountryCodeCallbackHost> g_countryCodeCallbacks;
std::vector<GeoFenceState*> mFences;
auto g_locatorProxy = Locator::GetInstance();

std::map<std::string, bool(*)(const napi_env &)> g_offAllFuncMap;
std::map<std::string, bool(*)(const napi_env &, const napi_value &)> g_offFuncMap;

void InitOffFuncMap()
{
    if (g_offAllFuncMap.size() != 0 || g_offFuncMap.size() != 0) {
        return;
    }
    g_offAllFuncMap.insert(std::make_pair("locationServiceState", &OffAllLocationServiceStateCallback));
    g_offAllFuncMap.insert(std::make_pair("locationChange", &OffAllLocationChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("gnssStatusChange", &OffAllGnssStatusChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("nmeaMessageChange", &OffAllNmeaMessageChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OffAllCachedGnssLocationsReportingCallback));
    g_offAllFuncMap.insert(std::make_pair("countryCodeChange", &OffAllCountryCodeChangeCallback));

    g_offFuncMap.insert(std::make_pair("locationServiceState", &OffLocationServiceStateCallback));
    g_offFuncMap.insert(std::make_pair("locationChange", &OffLocationChangeCallback));
    g_offFuncMap.insert(std::make_pair("gnssStatusChange", &OffGnssStatusChangeCallback));
    g_offFuncMap.insert(std::make_pair("nmeaMessageChange", &OffNmeaMessageChangeCallback));
    g_offFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OffCachedGnssLocationsReportingCallback));
    g_offFuncMap.insert(std::make_pair("countryCodeChange", &OffCountryCodeChangeCallback));
}

void SubscribeLocationServiceState(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackHost>& switchCallbackHost)
{
    switchCallbackHost->m_env = env;
    switchCallbackHost->m_handlerCb = handlerRef;
    g_locatorProxy->RegisterSwitchCallback(switchCallbackHost->AsObject(), DEFAULT_UID);
}

void SubscribeGnssStatus(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost)
{
    gnssStatusCallbackHost->m_env = env;
    gnssStatusCallbackHost->m_handlerCb = handlerRef;
    g_locatorProxy->RegisterGnssStatusCallback(gnssStatusCallbackHost->AsObject(), DEFAULT_UID);
}

void SubscribeNmeaMessage(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost)
{
    nmeaMessageCallbackHost->m_env = env;
    nmeaMessageCallbackHost->m_handlerCb = handlerRef;
    g_locatorProxy->RegisterNmeaMessageCallback(nmeaMessageCallbackHost->AsObject(), DEFAULT_UID);
}

void UnSubscribeLocationServiceState(sptr<LocationSwitchCallbackHost>& switchCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationServiceState");
    g_locatorProxy->UnregisterSwitchCallback(switchCallbackHost->AsObject());
}

void UnSubscribeGnssStatus(sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeGnssStatus");
    g_locatorProxy->UnregisterGnssStatusCallback(gnssStatusCallbackHost->AsObject());
}

void UnSubscribeNmeaMessage(sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeNmeaMessage");
    g_locatorProxy->UnregisterNmeaMessageCallback(nmeaMessageCallbackHost->AsObject());
}

void SubscribeLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackHost>& locatorCallbackHost)
{
    auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
    locatorCallbackHost->SetFixNumber(0);
    locatorCallbackHost->SetEnv(env);
    locatorCallbackHost->SetHandleCb(handlerRef);
    auto requestConfig = std::make_unique<RequestConfig>();
    JsObjToLocationRequest(env, object, requestConfig);
    g_locatorProxy->StartLocating(requestConfig, locatorCallback);
}

void SubscribeCountryCodeChange(const napi_env& env,
    const napi_ref& handlerRef, sptr<CountryCodeCallbackHost>& CallbackHost)
{
    auto callbackPtr = sptr<ICountryCodeCallback>(CallbackHost);
    CallbackHost->SetEnv(env);
    CallbackHost->SetCallback(handlerRef);
    g_locatorProxy->RegisterCountryCodeCallback(callbackPtr->AsObject(), DEFAULT_UID);
}

void UnsubscribeCountryCodeChange(sptr<CountryCodeCallbackHost>& CallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnsubscribeCountryCodeChange");
    g_locatorProxy->UnregisterCountryCodeCallback(CallbackHost->AsObject());
}

void SubscribeCacheLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackHost>& cachedCallbackHost)
{
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    cachedCallbackHost->m_env = env;
    cachedCallbackHost->m_handlerCb = handlerRef;
    auto request = std::make_unique<CachedGnssLocationsRequest>();
    JsObjToCachedLocationRequest(env, object, request);
    g_locatorProxy->RegisterCachedLocationCallback(request, cachedCallback);
}

void SubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler)
{
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_RETURN_VOID(env, napi_unwrap(env, handler, (void **)&wantAgent));
    auto request = std::make_unique<GeofenceRequest>();
    JsObjToGeoFenceRequest(env, object, request);
    auto state = new (std::nothrow) GeoFenceState(request->geofence, wantAgent);
    if (state != nullptr) {
        mFences.push_back(state);
        g_locatorProxy->AddFence(request);
    }
}

void UnSubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler)
{
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_RETURN_VOID(env, napi_unwrap(env, handler, (void **)&wantAgent));
    auto request = std::make_unique<GeofenceRequest>();
    JsObjToGeoFenceRequest(env, object, request);
    if (mFences.size() > 0) {
        mFences.erase(mFences.begin());
        g_locatorProxy->RemoveFence(request);
    }
}

SingleLocationAsyncContext* CreateSingleLocationAsyncContext(const napi_env& env,
    std::unique_ptr<RequestConfig>& config, sptr<LocatorCallbackHost> callback)
{
    auto asyncContext = new (std::nothrow) SingleLocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "GetCurrentLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->timeout_ = config->GetTimeOut();
    asyncContext->callbackHost_ = callback;
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<SingleLocationAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (g_locatorProxy->IsLocationEnabled() && callbackHost != nullptr) {
            callbackHost->Wait(context->timeout_);
            auto callbackPtr = sptr<ILocatorCallback>(callbackHost);
            g_locatorProxy->StopLocating(callbackPtr);
            if (callbackHost->GetCount() != 0) {
                context->errCode = LOCATION_REQUEST_TIMEOUT_ERROR;
            }
            callbackHost->SetCount(1);
        } else {
            context->errCode = LOCATION_SWITCH_ERROR;
        }
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<SingleLocationAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr && callbackHost->GetSingleLocation() != nullptr) {
            std::unique_ptr<Location> location = std::make_unique<Location>(*callbackHost->GetSingleLocation());
            LocationToJs(context->env, location, context->result[PARAM1]);
        } else {
            LBSLOGE(LOCATOR_STANDARD, "m_singleLocation is nullptr!");
        }
        if (context->callbackHost_) {
            context->callbackHost_ = nullptr;
        }
        LBSLOGI(LOCATOR_STANDARD, "Push single location to client");
    };
    return asyncContext;
}

int GetObjectArgsNum(const napi_env& env, const size_t argc, const napi_value* argv)
{
    napi_valuetype valueType = napi_undefined;
    int objectArgsNum = PARAM0;
    if (argc == PARAM0) {
        objectArgsNum = PARAM0;
    } else if (argc == PARAM1) {
        NAPI_CALL_BASE(env, napi_typeof(env, argv[PARAM0], &valueType), objectArgsNum);
        if (valueType == napi_object) {
            objectArgsNum = PARAM1;
        } else if (valueType == napi_function) {
            objectArgsNum = PARAM0;
        }
    } else if (argc == PARAM2) {
        objectArgsNum = PARAM1;
    } else {
        LBSLOGI(LOCATION_NAPI, "argc of GetCurrentLocation is wrong.");
    }
    return objectArgsNum;
}

std::unique_ptr<RequestConfig> CreateRequestConfig(const napi_env& env, const napi_value* argv, const size_t& objectArgsNum)
{
    auto requestConfig = std::make_unique<RequestConfig>();
    if (objectArgsNum > 0) {
        JsObjToCurrentLocationRequest(env, argv[objectArgsNum - 1], requestConfig);
    } else {
        requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    }
    requestConfig->SetFixNumber(1);
    return requestConfig;
}

sptr<LocatorCallbackHost> CreateSingleLocationCallbackHost()
{
    auto callbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    if (callbackHost) {
        callbackHost->SetFixNumber(1);
    }
    return callbackHost;
}

napi_value RequestLocationOnce(const napi_env& env, const size_t argc, const napi_value* argv)
{
    size_t objectArgsNum = 0;

    objectArgsNum = static_cast<size_t>(GetObjectArgsNum(env, argc, argv));
    auto requestConfig = CreateRequestConfig(env, argv, objectArgsNum);
    NAPI_ASSERT(env, requestConfig != nullptr, "requestConfig is null.");
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    NAPI_ASSERT(env, singleLocatorCallbackHost != nullptr, "callbackHost is null.");

    if (g_locatorProxy->IsLocationEnabled()) {
        auto callbackPtr = sptr<ILocatorCallback>(singleLocatorCallbackHost);
        g_locatorProxy->StartLocating(requestConfig, callbackPtr);
    }

    auto asyncContext = CreateSingleLocationAsyncContext(env, requestConfig, singleLocatorCallbackHost);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void UnSubscribeLocationChange(sptr<ILocatorCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationChange");
    g_locatorProxy->StopLocating(callback);
}

void UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeCacheLocationChange");
    g_locatorProxy->UnregisterCachedLocationCallback(callback);
}

bool IsCallbackEquals(const napi_env& env, const napi_value& handler, const napi_ref& savedCallback)
{
    napi_value handlerTemp = nullptr;
    if (savedCallback == nullptr || handler == nullptr) {
        return false;
    }
    NAPI_CALL_BASE(env, napi_get_reference_value(env, savedCallback, &handlerTemp), false);
    bool isEqual = false;
    NAPI_CALL_BASE(env, napi_strict_equals(env, handlerTemp, handler, &isEqual), false);
    return isEqual;
}

napi_value On(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = PARAM3;
    napi_value argv[PARAM3] = {0};
    napi_value thisVar = 0;
    napi_value result = nullptr;
    LBSLOGI(LOCATION_NAPI, "On function entry");
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    napi_valuetype eventName = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &eventName));
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");

    char type[64] = {0}; // max length
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[PARAM0], type, sizeof(type), &typeLen));
    std::string event = type;
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", event.c_str());
    if (event == "locationServiceState") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "callback should be function, mismatch for param.");
        // the second params should be handler
        if (g_switchCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        if (switchCallbackHost != nullptr) {
            napi_ref handlerRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef));
            g_switchCallbacks.AddCallback(env, handlerRef, switchCallbackHost);
            SubscribeLocationServiceState(env, handlerRef, switchCallbackHost);
        }
    } else if (event == "locationChange") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM3, "number of parameters is wrong");
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM2], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "callback should be function, mismatch for param.");
        if (!g_locatorProxy->IsLocationEnabled()) {
            LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
            return result;
        }
        // the third params should be handler
        if (g_locationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto locatorCallbackHost =
            sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
        if (locatorCallbackHost != nullptr) {
            napi_ref handlerRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, argv[PARAM2], 1, &handlerRef));
            g_locationCallbacks.AddCallback(env, handlerRef, locatorCallbackHost);
            // argv[1]:request params, argv[2]:handler
            SubscribeLocationChange(env, argv[PARAM1], handlerRef, locatorCallbackHost);
        }
    } else if (event == "gnssStatusChange") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "callback should be function, mismatch for param.");
        // the second params should be handler
        if (g_gnssStatusInfoCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        if (gnssCallbackHost != nullptr) {
            napi_ref handlerRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef));
            g_gnssStatusInfoCallbacks.AddCallback(env, handlerRef, gnssCallbackHost);
            SubscribeGnssStatus(env, handlerRef, gnssCallbackHost);
        }
    } else if (event == "nmeaMessageChange") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "callback should be function, mismatch for param.");
        // the second params should be handler
        if (g_nmeaCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        if (nmeaCallbackHost != nullptr) {
            napi_ref handlerRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef));
            g_nmeaCallbacks.AddCallback(env, handlerRef, nmeaCallbackHost);
            SubscribeNmeaMessage(env, handlerRef, nmeaCallbackHost);
        }
    } else if (event == "cachedGnssLocationsReporting") {
        // expect for 3 params
        NAPI_ASSERT(env, argc == PARAM3, "number of parameters is wrong");
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM2], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "callback should be function, mismatch for param.");
        if (!g_locatorProxy->IsLocationEnabled()) {
            LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
            return result;
        }
        // the third params should be handler
        if (g_cachedLocationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto cachedCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        if (cachedCallbackHost != nullptr) {
            napi_ref handlerRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, argv[PARAM2], PARAM1, &handlerRef));
            g_cachedLocationCallbacks.AddCallback(env, handlerRef, cachedCallbackHost);
            SubscribeCacheLocationChange(env, argv[PARAM1], handlerRef, cachedCallbackHost);
        }
    } else if (event == "fenceStatusChange") {
        // expect for 3 params
        NAPI_ASSERT(env, argc == PARAM3, "number of parameters is wrong");
        if (!g_locatorProxy->IsLocationEnabled()) {
            LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
            return result;
        }
        // the third params should be handler
        SubscribeFenceStatusChange(env, argv[PARAM1], argv[PARAM2]);
    } else if (event == "countryCodeChange") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "callback should be function, mismatch for param.");
        // the third params should be handler
        if (g_countryCodeCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto callbackHost =
            sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
        if (callbackHost) {
            napi_ref handlerRef = nullptr;
            NAPI_CALL(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef));
            g_countryCodeCallbacks.AddCallback(env, handlerRef, callbackHost);
            SubscribeCountryCodeChange(env, handlerRef, callbackHost);
        }
    }

    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

bool OffAllLocationServiceStateCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<LocationSwitchCallbackHost>>> callbackMap =
        g_switchCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        UnSubscribeLocationServiceState(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_switchCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllLocationChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<LocatorCallbackHost>>> callbackMap =
        g_locationCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        auto locatorCallback = sptr<ILocatorCallback>(callbackHost);
        UnSubscribeLocationChange(locatorCallback);
        callbackHost->DeleteAllCallbacks();
        callbackHost = nullptr;
    }
    g_locationCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllGnssStatusChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<GnssStatusCallbackHost>>> callbackMap =
        g_gnssStatusInfoCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        UnSubscribeGnssStatus(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_gnssStatusInfoCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllNmeaMessageChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<NmeaMessageCallbackHost>>> callbackMap =
        g_nmeaCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        UnSubscribeNmeaMessage(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_nmeaCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllCachedGnssLocationsReportingCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<CachedLocationsCallbackHost>>> callbackMap =
        g_cachedLocationCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        auto cachedCallback = sptr<ICachedLocationsCallback>(callbackHost);
        UnSubscribeCacheLocationChange(cachedCallback);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_cachedLocationCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllCountryCodeChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<CountryCodeCallbackHost>>> callbackMap =
        g_countryCodeCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        UnsubscribeCountryCodeChange(callbackHost);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_countryCodeCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffLocationServiceStateCallback(const napi_env& env, const napi_value& handler)
{
    auto switchCallbackHost = g_switchCallbacks.GetCallbackPtr(env, handler);
    if (switchCallbackHost) {
        UnSubscribeLocationServiceState(switchCallbackHost);
        g_switchCallbacks.DeleteCallback(env, handler);
        switchCallbackHost->DeleteHandler();
        switchCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffLocationChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto locatorCallbackHost = g_locationCallbacks.GetCallbackPtr(env, handler);
    if (locatorCallbackHost) {
        auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
        UnSubscribeLocationChange(locatorCallback);
        g_locationCallbacks.DeleteCallback(env, handler);
        locatorCallbackHost->DeleteAllCallbacks();
        locatorCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffGnssStatusChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto gnssCallbackHost = g_gnssStatusInfoCallbacks.GetCallbackPtr(env, handler);
    if (gnssCallbackHost) {
        UnSubscribeGnssStatus(gnssCallbackHost);
        g_gnssStatusInfoCallbacks.DeleteCallback(env, handler);
        gnssCallbackHost->DeleteHandler();
        gnssCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffNmeaMessageChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto nmeaCallbackHost = g_nmeaCallbacks.GetCallbackPtr(env, handler);
    if (nmeaCallbackHost) {
        UnSubscribeNmeaMessage(nmeaCallbackHost);
        g_nmeaCallbacks.DeleteCallback(env, handler);
        nmeaCallbackHost->DeleteHandler();
        nmeaCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffCachedGnssLocationsReportingCallback(const napi_env& env, const napi_value& handler)
{
    auto cachedCallbackHost = g_cachedLocationCallbacks.GetCallbackPtr(env, handler);
    if (cachedCallbackHost) {
        auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
        UnSubscribeCacheLocationChange(cachedCallback);
        g_cachedLocationCallbacks.DeleteCallback(env, handler);
        cachedCallbackHost->DeleteHandler();
        cachedCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffCountryCodeChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto callbackHost = g_countryCodeCallbacks.GetCallbackPtr(env, handler);
    if (callbackHost) {
        UnsubscribeCountryCodeChange(callbackHost);
        g_countryCodeCallbacks.DeleteCallback(env, handler);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
        return true;
    }
    return false;
}

napi_value Off(napi_env env, napi_callback_info cbinfo)
{
    InitOffFuncMap();
    size_t argc = PARAM2;
    napi_value argv[PARAM3] = {0};
    napi_value thisVar = 0;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");

    napi_valuetype eventName = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &eventName));
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");
    LBSLOGI(LOCATION_NAPI, "Off function entry");

    char type[64] = {0};
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[PARAM0], type, sizeof(type), &typeLen));
    std::string event = type;
    LBSLOGI(LOCATION_NAPI, "Unsubscribe event: %{public}s", event.c_str());
    if (argc == PARAM1) {
        auto offAllCallbackFunc = g_offAllFuncMap.find(event);
        if (offAllCallbackFunc != g_offAllFuncMap.end() && offAllCallbackFunc->second != nullptr) {
            auto memberFunc = offAllCallbackFunc->second;
            (*memberFunc)(env);
        }
    } else if (argc == PARAM2) {
        napi_valuetype valueType;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "callback should be function, mismatch for param.");
        auto offCallbackFunc = g_offFuncMap.find(event);
        if (offCallbackFunc != g_offFuncMap.end() && offCallbackFunc->second != nullptr) {
            auto singleMemberFunc = offCallbackFunc->second;
            (*singleMemberFunc)(env, argv[PARAM1]);
        }
    } else if (argc == PARAM3) {
        UnSubscribeFenceStatusChange(env, argv[PARAM1], argv[PARAM2]);
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = PARAM3;
    napi_value argv[PARAM3] = {0};
    napi_value thisVar = 0;

    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));

    napi_valuetype valueType = napi_undefined;
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");
    LBSLOGI(LOCATION_NAPI, "GetCurrentLocation enter");

    if (argc == PARAM1) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valueType));
        NAPI_ASSERT(env, valueType == napi_function || valueType == napi_object, "type mismatch for parameter 2");
    }
    if (argc == PARAM2) {
        napi_valuetype valueType1 = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valueType));
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valueType1));
        NAPI_ASSERT(env, valueType == napi_object, "type mismatch for parameter 1");
        NAPI_ASSERT(env, valueType1 == napi_function, "type mismatch for parameter 2");
    }
    return RequestLocationOnce(env, argc, argv);
}
}  // namespace Location
}  // namespace OHOS
