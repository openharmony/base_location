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

namespace OHOS {
namespace Location {
CallbackManager<LocationSwitchCallbackHost> g_switchCallbacks;
CallbackManager<LocatorCallbackHost> g_locationCallbacks;
CallbackManager<GnssStatusCallbackHost> g_gnssStatusInfoCallbacks;
CallbackManager<NmeaMessageCallbackHost> g_nmeaCallbacks;
CallbackManager<CachedLocationsCallbackHost> g_cachedLocationCallbacks;
std::vector<GeoFenceState*> mFences;
sptr<LocatorCallbackHost> g_singleLocatorCallbackHost =
    sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
sptr<ILocatorCallback> g_singleLocatorCallback = sptr<ILocatorCallback>(g_singleLocatorCallbackHost);
std::unique_ptr<Locator> g_locatorProxy = Locator::GetInstance();

void SubscribeLocationServiceState(napi_env& env,
    napi_ref& handlerRef, sptr<LocationSwitchCallbackHost>& switchCallbackHost)
{
    switchCallbackHost->m_env = env;
    switchCallbackHost->m_handlerCb = handlerRef;
    g_locatorProxy->RegisterSwitchCallback(switchCallbackHost->AsObject(), DEFAULT_UID);
}

void SubscribeGnssStatus(napi_env& env, napi_ref& handlerRef,
    sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost)
{
    gnssStatusCallbackHost->m_env = env;
    gnssStatusCallbackHost->m_handlerCb = handlerRef;
    g_locatorProxy->RegisterGnssStatusCallback(gnssStatusCallbackHost->AsObject(), DEFAULT_UID);
}

void SubscribeNmeaMessage(napi_env& env, napi_ref& handlerRef,
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

void SubscribeLocationChange(napi_env& env, const napi_value& object,
    napi_ref& handlerRef, sptr<LocatorCallbackHost>& locatorCallbackHost)
{
    sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
    locatorCallbackHost->m_fixNumber = 0;
    locatorCallbackHost->m_env = env;
    locatorCallbackHost->m_handlerCb = handlerRef;
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    JsObjToLocationRequest(env, object, requestConfig);
    g_locatorProxy->StartLocating(requestConfig, locatorCallback);
}

void SubscribeCacheLocationChange(napi_env& env, const napi_value& object,
    napi_ref& handlerRef, sptr<CachedLocationsCallbackHost>& cachedCallbackHost)
{
    sptr<ICachedLocationsCallback> cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    cachedCallbackHost->m_env = env;
    cachedCallbackHost->m_handlerCb = handlerRef;
    std::unique_ptr<CachedGnssLocationsRequest> request = std::make_unique<CachedGnssLocationsRequest>();
    JsObjToCachedLocationRequest(env, object, request);
    g_locatorProxy->RegisterCachedLocationCallback(request, cachedCallback);
}

void SubscribeFenceStatusChange(napi_env& env, const napi_value& object, napi_value& handler)
{
    AbilityRuntime::WantAgent::WantAgent wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_RETURN_VOID(env, napi_unwrap(env, handler, (void **)&wantAgent));
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    JsObjToGeoFenceRequest(env, object, request);
    GeoFenceState* state = new (std::nothrow) GeoFenceState(request->geofence, wantAgent);
    if (state != nullptr) {
        mFences.push_back(state);
        g_locatorProxy->AddFence(request);
    }
}

void UnSubscribeFenceStatusChange(napi_env& env, const napi_value& object, napi_value& handler)
{
    AbilityRuntime::WantAgent::WantAgent wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_RETURN_VOID(env, napi_unwrap(env, handler, (void **)&wantAgent));
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    JsObjToGeoFenceRequest(env, object, request);
    if (mFences.size() > 0) {
        mFences.erase(mFences.begin());
        g_locatorProxy->RemoveFence(request);
    }
}

void GetCallbackType(napi_env& env, const size_t argc, const napi_value* argv, bool& isCallbackType,
    size_t& nonCallbackArgNum)
{
    napi_valuetype valueType = napi_undefined;

    if (argc == 0) {
        isCallbackType = false;
    } else if (argc == 1) {
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, argv[0], &valueType));
        if (valueType == napi_object) {
            isCallbackType = false;
            nonCallbackArgNum = 1;
        } else if (valueType == napi_function) {
            isCallbackType = true;
            nonCallbackArgNum = 0;
        }
    } else {
        isCallbackType = true;
        nonCallbackArgNum = 1;
    }
}

void GenRequestConfig(napi_env& env, const napi_value* argv,
    size_t& nonCallbackArgNum, std::unique_ptr<RequestConfig>& requestConfig)
{
    if (nonCallbackArgNum > 0) {
        JsObjToCurrentLocationRequest(env, argv[nonCallbackArgNum - 1], requestConfig);
    } else {
        requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
        requestConfig->SetScenario(SCENE_UNSET);
        requestConfig->SetMaxAccuracy(0);
    }
    requestConfig->SetTimeInterval(1);
    requestConfig->SetDistanceInterval(0);
    requestConfig->SetFixNumber(1);
}

void InitSingleLocatorCallback(napi_env& env, const size_t argc, const napi_value* argv)
{
    if (g_singleLocatorCallbackHost->m_handlerCb != nullptr || g_singleLocatorCallbackHost->m_deferred != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeLocationChange");
        UnSubscribeLocationChange(g_singleLocatorCallback);
        g_singleLocatorCallbackHost->DeleteAllNapiCb();
    }
    g_singleLocatorCallbackHost->m_env = env;
    g_singleLocatorCallbackHost->m_fixNumber = 1;

    napi_ref handlerRef = nullptr;
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    bool isCallbackType = false;
    size_t nonCallbackArgNum = 0;
    GetCallbackType(env, argc, argv, isCallbackType, nonCallbackArgNum);
    if (isCallbackType) {
        NAPI_CALL_RETURN_VOID(env, napi_create_reference(env, argv[nonCallbackArgNum], 1, &handlerRef));
        g_singleLocatorCallbackHost->m_handlerCb = handlerRef;
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        g_singleLocatorCallbackHost->m_deferred = deferred;
    }
}

napi_value RequestLocationOnce(napi_env& env, const size_t argc, const napi_value* argv)
{
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    LBSLOGI(LOCATION_NAPI, "RequestLocationOnce enter");
    if (g_singleLocatorCallbackHost == nullptr) {
        return UndefinedNapiValue(env);
    }
    size_t nonCallbackArgNum = 0;
    bool isCallbackType = false;
    GetCallbackType(env, argc, argv, isCallbackType, nonCallbackArgNum);
    InitSingleLocatorCallback(env, argc, argv);
    GenRequestConfig(env, argv, nonCallbackArgNum, requestConfig);

    CurrentLocationAsyncContext* asyncContext = new (std::nothrow) CurrentLocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env, napi_create_string_latin1(env, "GetCurrentLocation", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->timeout = requestConfig->GetTimeOut();
    if (g_locatorProxy->IsLocationEnabled()) {
        g_locatorProxy->StartLocating(requestConfig, g_singleLocatorCallback);
    }
    asyncContext->executeFunc = [&](void* data) -> void {
        CurrentLocationAsyncContext* context = static_cast<CurrentLocationAsyncContext*>(data);
        if (g_locatorProxy->IsLocationEnabled()) {
            g_singleLocatorCallbackHost->Wait(context->timeout);
            g_locatorProxy->StopLocating(g_singleLocatorCallback);
            if (g_singleLocatorCallbackHost->GetCount() != 0) {
                context->errCode = LOCATION_REQUEST_TIMEOUT_ERROR;
            } else {
                context->errCode = NO_DATA_TO_SEND;
            }
            g_singleLocatorCallbackHost->SetCount(1);
        } else {
            context->errCode = LOCATION_SWITCH_ERROR;
        }
    };
    asyncContext->completeFunc = [&](void* data) -> void {};
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
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

bool IsCallbackEquals(napi_env& env, napi_value& handler, napi_ref& savedCallback)
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
    napi_ref handlerRef = nullptr;
    if (event == "locationServiceState") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        // the second params should be handler
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef));
        if (g_switchCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto switchCallbackHost =
            sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
        if (switchCallbackHost != nullptr) {
            g_switchCallbacks.AddCallback(env, handlerRef, switchCallbackHost);
            SubscribeLocationServiceState(env, handlerRef, switchCallbackHost);
        }
    } else if (event == "locationChange") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM3, "number of parameters is wrong");
        if (!g_locatorProxy->IsLocationEnabled()) {
            LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
            return result;
        }
        // the third params should be handler
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM2], 1, &handlerRef));
        if (g_locationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto locatorCallbackHost =
            sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
        if (locatorCallbackHost != nullptr) {
            g_locationCallbacks.AddCallback(env, handlerRef, locatorCallbackHost);
            // argv[1]:request params, argv[2]:handler
            SubscribeLocationChange(env, argv[PARAM1], handlerRef, locatorCallbackHost);
        }
    } else if (event == "gnssStatusChange") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        // the second params should be handler
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef));
        if (g_gnssStatusInfoCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto gnssCallbackHost =
            sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
        if (gnssCallbackHost != nullptr) {
            g_gnssStatusInfoCallbacks.AddCallback(env, handlerRef, gnssCallbackHost);
            SubscribeGnssStatus(env, handlerRef, gnssCallbackHost);
        }
    } else if (event == "nmeaMessageChange") {
        // expect for 2 params
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        // the second params should be handler
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef));
        if (g_nmeaCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto nmeaCallbackHost =
            sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
        if (nmeaCallbackHost != nullptr) {
            g_nmeaCallbacks.AddCallback(env, handlerRef, nmeaCallbackHost);
            SubscribeNmeaMessage(env, handlerRef, nmeaCallbackHost);
        }
    } else if (event == "cachedGnssLocationsReporting") {
        // expect for 3 params
        NAPI_ASSERT(env, argc == PARAM3, "number of parameters is wrong");
        if (!g_locatorProxy->IsLocationEnabled()) {
            LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
            return result;
        }
        // the third params should be handler
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM2], PARAM1, &handlerRef));
        if (g_cachedLocationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
            LBSLOGE(LOCATION_NAPI, "This request already exists");
            return result;
        }
        auto cachedCallbackHost =
            sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
        if (cachedCallbackHost != nullptr) {
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
    }
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value Off(napi_env env, napi_callback_info cbinfo)
{
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
    bool isEqual = false;
    if (event == "locationServiceState") {
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        auto switchCallbackHost = g_switchCallbacks.GetCallbackPtr(env, argv[PARAM1]);
        if (switchCallbackHost) {
            UnSubscribeLocationServiceState(switchCallbackHost);
            switchCallbackHost->DeleteHandler();
            g_switchCallbacks.DeleteCallback(env, argv[PARAM1]);
        }
    } else if (event == "locationChange") {
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        auto locatorCallbackHost = g_locationCallbacks.GetCallbackPtr(env, argv[PARAM1]);
        if (locatorCallbackHost) {
            auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
            UnSubscribeLocationChange(locatorCallback);
            locatorCallbackHost->DeleteAllNapiCb();
            g_locationCallbacks.DeleteCallback(env, argv[PARAM1]);
        }
    } else if (event == "gnssStatusChange") {
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        auto gnssCallbackHost = g_gnssStatusInfoCallbacks.GetCallbackPtr(env, argv[PARAM1]);
        if (gnssCallbackHost) {
            UnSubscribeGnssStatus(gnssCallbackHost);
            gnssCallbackHost->DeleteHandler();
            g_gnssStatusInfoCallbacks.DeleteCallback(env, argv[PARAM1]);
        }
    } else if (event == "nmeaMessageChange") {
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        auto nmeaCallbackHost = g_nmeaCallbacks.GetCallbackPtr(env, argv[PARAM1]);
        if (nmeaCallbackHost) {
            UnSubscribeNmeaMessage(nmeaCallbackHost);
            nmeaCallbackHost->DeleteHandler();
            g_nmeaCallbacks.DeleteCallback(env, argv[PARAM1]);
        }
    } else if (event == "cachedGnssLocationsReporting") {
        NAPI_ASSERT(env, argc == PARAM2, "number of parameters is wrong");
        auto cachedCallbackHost = g_cachedLocationCallbacks.GetCallbackPtr(env, argv[PARAM1]);
        if (cachedCallbackHost) {
            auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
            UnSubscribeCacheLocationChange(cachedCallback);
            cachedCallbackHost->DeleteHandler();
            g_cachedLocationCallbacks.DeleteCallback(env, argv[PARAM1]);
        }
    } else if (event == "fenceStatusChange") {
        NAPI_ASSERT(env, argc == PARAM3, "number of parameters is wrong");
        UnSubscribeFenceStatusChange(env, argv[PARAM1], argv[PARAM2]);
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 0;
    size_t argc = PARAM3;
    napi_value argv[PARAM3] = {0};
    napi_value thisVar = 0;
    napi_value result = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= requireArgc, "number of parameters is error");

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
    if (!g_locatorProxy->IsLocationEnabled()) {
        LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
        return result;
    }
    return RequestLocationOnce(env, argc, argv);
}
}  // namespace Location
}  // namespace OHOS
