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
#include "lbs_log.h"
#include "location_napi_adapter.h"
#include "location_util.h"
#include "locator.h"
#include "request_config.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
std::unique_ptr<Locator> g_locatorNapiPtr = Locator::GetInstance(LOCATION_LOCATOR_SA_ID);

EventManager::EventManager(napi_env env, napi_value thisVar) : m_env(env)
{
    m_thisVarRef = nullptr;
    napi_create_reference(env, thisVar, 1, &m_thisVarRef);
    m_locatorCallbackHost = sptr<LocatorCallbackHost>(new LocatorCallbackHost());
    m_locatorCallback = sptr<ILocatorCallback>(m_locatorCallbackHost);
    m_singleLocatorCallbackHost = sptr<LocatorCallbackHost>(new LocatorCallbackHost());
    m_singleLocatorCallback = sptr<ILocatorCallback>(m_singleLocatorCallbackHost);
    m_switchCallbackHost = sptr<LocationSwitchCallbackHost>(new LocationSwitchCallbackHost());
    m_switchCallback = sptr<ISwitchCallback>(m_switchCallbackHost);
    m_gnssStatusCallbackHost = sptr<GnssStatusCallbackHost>(new GnssStatusCallbackHost());
    m_nmeaMessageCallbackHost = sptr<NmeaMessageCallbackHost>(new NmeaMessageCallbackHost());
    m_cachedCallbackHost = sptr<CachedLocationsCallbackHost>(new CachedLocationsCallbackHost());
    m_cachedCallback = sptr<ICachedLocationsCallback>(m_cachedCallbackHost);
}

EventManager::~EventManager()
{
    if (m_locatorCallbackHost != nullptr) {
        delete m_locatorCallbackHost;
    }
    if (m_singleLocatorCallbackHost != nullptr) {
        delete m_singleLocatorCallbackHost;
    }
    if (m_switchCallbackHost != nullptr) {
        delete m_switchCallbackHost;
    }
    if (m_gnssStatusCallbackHost != nullptr) {
        delete m_gnssStatusCallbackHost;
    }
    if (m_nmeaMessageCallbackHost != nullptr) {
        delete m_nmeaMessageCallbackHost;
    }
    if (m_cachedCallbackHost != nullptr) {
        delete m_cachedCallbackHost;
    }
}

void EventManager::SubscribeLocationServiceState(const std::string& name, napi_value& handler)
{
    napi_ref handlerRef = nullptr;

    if (m_switchCallbackHost->m_handlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeLocationServiceState");
        UnSubscribeLocationServiceState();
        m_switchCallbackHost->DeleteHandler();
    }

    napi_create_reference(m_env, handler, 1, &handlerRef);
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", name.c_str());

    m_switchCallbackHost->m_env = m_env;
    m_switchCallbackHost->m_handlerCb = handlerRef;
    m_switchCallbackHost->m_thisVarRef = m_thisVarRef;
    g_locatorNapiPtr->RegisterSwitchCallback(m_switchCallbackHost->AsObject(), 10001);
}

void EventManager::SubscribeGnssStatus(const std::string& name, napi_value& handler)
{
    napi_ref handlerRef = nullptr;

    if (m_gnssStatusCallbackHost->m_handlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeGnssStatus");
        UnSubscribeGnssStatus();
        m_gnssStatusCallbackHost->DeleteHandler();
    }

    napi_create_reference(m_env, handler, 1, &handlerRef);
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", name.c_str());

    m_gnssStatusCallbackHost->m_env = m_env;
    m_gnssStatusCallbackHost->m_handlerCb = handlerRef;
    m_gnssStatusCallbackHost->m_thisVarRef = m_thisVarRef;
    g_locatorNapiPtr->RegisterGnssStatusCallback(m_gnssStatusCallbackHost->AsObject(), 10001);
}

void EventManager::SubscribeNmeaMessage(const std::string& name, napi_value& handler)
{
    napi_ref handlerRef = nullptr;

    if (m_nmeaMessageCallbackHost->m_handlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeNmeaMessage");
        UnSubscribeNmeaMessage();
        m_nmeaMessageCallbackHost->DeleteHandler();
    }

    napi_create_reference(m_env, handler, 1, &handlerRef);
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", name.c_str());

    m_nmeaMessageCallbackHost->m_env = m_env;
    m_nmeaMessageCallbackHost->m_handlerCb = handlerRef;
    m_nmeaMessageCallbackHost->m_thisVarRef = m_thisVarRef;
    g_locatorNapiPtr->RegisterNmeaMessageCallback(m_nmeaMessageCallbackHost->AsObject(), 10001);
}

bool EventManager::IsCallbackEquals(napi_value& handler, napi_ref& savedCallback)
{
    napi_value handlerTemp = nullptr;
    if (savedCallback == nullptr || handler == nullptr) {
        return false;
    }
    napi_get_reference_value(m_env, savedCallback, &handlerTemp);
    bool isEqual = false;
    napi_strict_equals(m_env, handlerTemp, handler, &isEqual);
    return isEqual;
}

void EventManager::UnSubscribeLocationServiceState()
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationServiceState");
    g_locatorNapiPtr->UnregisterSwitchCallback(m_switchCallbackHost->AsObject());
}

void EventManager::UnSubscribeGnssStatus()
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeGnssStatus");
    g_locatorNapiPtr->UnregisterGnssStatusCallback(m_gnssStatusCallbackHost->AsObject());
}

void EventManager::UnSubscribeNmeaMessage()
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeNmeaMessage");
    g_locatorNapiPtr->UnregisterNmeaMessageCallback(m_nmeaMessageCallbackHost->AsObject());
}

void EventManager::SubscribeLocationChange(const std::string& name, const napi_value& object,
    napi_value& handler, int fixNumber)
{
    napi_ref handlerRef = nullptr;
    if (m_locatorCallbackHost->m_handlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeLocationChange");
        UnSubscribeLocationChange(m_locatorCallback);
        m_locatorCallbackHost->DeleteHandler();
    }

    napi_create_reference(m_env, handler, 1, &handlerRef);
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", name.c_str());

    m_locatorCallbackHost->m_fixNumber = fixNumber;
    m_locatorCallbackHost->m_env = m_env;
    m_locatorCallbackHost->m_handlerCb = handlerRef;
    m_locatorCallbackHost->m_thisVarRef = m_thisVarRef;

    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    JsObjToLocationRequest(m_env, object, requestConfig);
    g_locatorNapiPtr->StartLocating(requestConfig, m_locatorCallback);
}

void EventManager::SubscribeCacheLocationChange(const std::string& name, const napi_value& object,
    napi_value& handler)
{
    napi_ref handlerRef = nullptr;
    if (m_cachedCallbackHost->m_handlerCb != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeLocationChange");
        UnSubscribeCacheLocationChange(m_cachedCallback);
        m_cachedCallbackHost->DeleteHandler();
    }

    napi_create_reference(m_env, handler, 1, &handlerRef);
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", name.c_str());

    m_cachedCallbackHost->m_env = m_env;
    m_cachedCallbackHost->m_handlerCb = handlerRef;
    m_cachedCallbackHost->m_thisVarRef = m_thisVarRef;

    std::unique_ptr<CachedGnssLoactionsRequest> request = std::make_unique<CachedGnssLoactionsRequest>();
    JsObjToCachedLocationRequest(m_env, object, request);
    g_locatorNapiPtr->RegisterCachedLocationCallback(request, m_cachedCallback);
}

void EventManager::SubscribeFenceStatusChange(const std::string& name, const napi_value& object,
    napi_value& handler)
{
    napi_ref handlerRef = nullptr;

    napi_create_reference(m_env, handler, 1, &handlerRef);
    LBSLOGI(LOCATION_NAPI, "UnSubscribe event: %{public}s", name.c_str());

    AbilityRuntime::WantAgent::WantAgent wantAgent = AbilityRuntime::WantAgent::WantAgent();
    napi_unwrap(m_env, handler, (void **)&wantAgent);
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    JsObjToGeoFenceRequest(m_env, object, request);
    GeoFenceState* state = new GeoFenceState(request->geofence, wantAgent);
    mFences.push_back(state);
    g_locatorNapiPtr->AddFence(request);
}

void EventManager::UnSubscribeFenceStatusChange(const std::string& name, const napi_value& object,
    napi_value& handler)
{
    napi_ref handlerRef = nullptr;

    napi_create_reference(m_env, handler, 1, &handlerRef);
    LBSLOGI(LOCATION_NAPI, "UnSubscribe event: %{public}s", name.c_str());

    AbilityRuntime::WantAgent::WantAgent wantAgent = AbilityRuntime::WantAgent::WantAgent();
    napi_unwrap(m_env, handler, (void **)&wantAgent);
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    JsObjToGeoFenceRequest(m_env, object, request);
    mFences.erase(mFences.begin());
    g_locatorNapiPtr->RemoveFence(request);
}

void EventManager::GetCallbackType(napi_env env, const size_t argc, const napi_value* argv, bool& isCallbackType,
    size_t& nonCallbackArgNum)
{
    napi_valuetype valueType = napi_undefined;

    if (argc == 0) {
        isCallbackType = false;
    } else if (argc == 1) {
        napi_typeof(env, argv[0], &valueType);
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

void EventManager::GenRequestConfig(napi_env env, const napi_value* argv,
    size_t& nonCallbackArgNum, std::unique_ptr<RequestConfig>& requestConfig)
{
    if (nonCallbackArgNum > 0) {
        JsObjToLocationRequest(env, argv[nonCallbackArgNum - 1], requestConfig);
    } else {
        requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
        requestConfig->SetScenario(SCENE_UNSET);
        requestConfig->SetTimeInterval(1);
        requestConfig->SetDistanceInterval(0);
        requestConfig->SetMaxAccuracy(0);
    }
    requestConfig->SetFixNumber(1);
}

void EventManager::GetTimeoutParam(napi_env env, const napi_value* argv,
    size_t& nonCallbackArgNum, int& timeout)
{
    if (nonCallbackArgNum > 0) {
        JsObjectToInt(env, argv[nonCallbackArgNum - 1], "timeoutMs", timeout);
    } else {
        timeout = 30000;
    }
}

napi_value EventManager::RequestLocationOnce(napi_env env, const size_t argc, const napi_value* argv)
{
    napi_ref handlerRef = nullptr;
    napi_deferred deferred;
    napi_value promise;
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    bool isCallbackType = false;
    size_t nonCallbackArgNum = 0;
    int timeout = 5000;
    LBSLOGI(LOCATION_NAPI, "RequestLocationOnce enter");

    GetCallbackType(env, argc, argv, isCallbackType, nonCallbackArgNum);

    GenRequestConfig(env, argv, nonCallbackArgNum, requestConfig);

    GetTimeoutParam(env, argv, nonCallbackArgNum, timeout);

    if (m_singleLocatorCallbackHost->m_handlerCb != nullptr || m_singleLocatorCallbackHost->m_deferred != nullptr) {
        LBSLOGI(LOCATION_NAPI, "GetHandlerCb() != nullptr, UnSubscribeLocationChange");
        UnSubscribeLocationChange(m_singleLocatorCallback);
        if (m_singleLocatorCallbackHost->m_handlerCb != nullptr) {
            m_singleLocatorCallbackHost->DeleteHandler();
        }
    }

    m_singleLocatorCallbackHost->m_env = m_env;
    m_singleLocatorCallbackHost->m_thisVarRef = m_thisVarRef;
    m_singleLocatorCallbackHost->m_fixNumber = 1;
    if (isCallbackType) {
        napi_create_reference(env, argv[nonCallbackArgNum], 1, &handlerRef);
        m_singleLocatorCallbackHost->m_handlerCb = handlerRef;
    } else {
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        m_singleLocatorCallbackHost->m_deferred = deferred;
    }
    g_locatorNapiPtr->StartLocating(requestConfig, m_singleLocatorCallback);
    if (isCallbackType) {
        return UndefinedNapiValue(env);
    } else {
        return promise;
    }
}

void EventManager::UnSubscribeLocationChange(sptr<ILocatorCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationChange");
    g_locatorNapiPtr->StopLocating(callback);
}

void EventManager::UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeCacheLocationChange");
    g_locatorNapiPtr->UnregisterCachedLocationCallback(callback);
}

void EventManager::DeleteHanderRef(napi_value& handler)
{
    if (IsCallbackEquals(handler, m_locatorCallbackHost->m_handlerCb)) {
        LBSLOGD(LOCATION_NAPI, "DeleteHanderRef locator");
        m_locatorCallbackHost->DeleteHandler();
    }
    if (IsCallbackEquals(handler, m_switchCallbackHost->m_handlerCb)) {
        LBSLOGD(LOCATION_NAPI, "DeleteHanderRef switch");
        m_switchCallbackHost->DeleteHandler();
    }
    if (IsCallbackEquals(handler, m_singleLocatorCallbackHost->m_handlerCb)) {
        LBSLOGD(LOCATION_NAPI, "DeleteHanderRef single locator callback");
        m_singleLocatorCallbackHost->DeleteHandler();
    }
}

napi_value On(napi_env env, napi_callback_info cbinfo)
{
    TRACE_FUNC_CALL;
    size_t argc = 3;
    napi_value argv[3] = {0};
    napi_value thisVar = 0;
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    napi_value result = nullptr;
    LBSLOGI(LOCATION_NAPI, "On function entry");
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    napi_valuetype eventName = napi_undefined;
    napi_typeof(env, argv[0], &eventName);
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");
    NAPI_ASSERT(env, g_locatorNapiPtr != nullptr, "locator instance is null.");
    if (argc == 2 || argc == 3) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, argv[argc - 1], &handler);
        NAPI_ASSERT(env, handler == napi_function, "type mismatch for parameter");
    }
    if (!CommonUtils::CheckLocationPermission(callingPid, callingUid)) {
        LBSLOGE(LOCATION_NAPI, "pid:%{public}d uid:%{public}d has no access permission to get cache location",
                callingPid, callingUid);
        g_locatorNapiPtr->RequestPermission(env);
        return result;
    }
    EventManager* manager = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&manager);
    if (status == napi_ok && manager != nullptr) {
        char type[64] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);
        std::string event = type;
        LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", event.c_str());

        if (event == "locationServiceState") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->SubscribeLocationServiceState(type, argv[1]);
        } else if (event == "locationChange") {
            NAPI_ASSERT(env, argc == 3, "number of parameters is wrong");
            manager->SubscribeLocationChange(type, argv[1], argv[2], 0);
        } else if (event == "gnssStatusChange") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->SubscribeGnssStatus(type, argv[1]);
        } else if (event == "nmeaMessageChange") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->SubscribeNmeaMessage(type, argv[1]);
        } else if (event == "cachedGnssLocationsReporting") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->SubscribeCacheLocationChange(type, argv[1], argv[2]);
        } else if (event == "fenceStatusChange") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->SubscribeFenceStatusChange(type, argv[1], argv[2]);
        }
    } else {
        LBSLOGE(LOCATION_NAPI, "On unwrap class failed");
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value Off(napi_env env, napi_callback_info cbinfo)
{
    TRACE_FUNC_CALL;
    size_t requireArgc = 2;
    size_t requireArgcWithCb = 2;
    size_t argc = 2;
    napi_value argv[3] = {0};
    napi_value thisVar = 0;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    NAPI_ASSERT(env, argc >= requireArgc, "requires at least 1 parameter");
    NAPI_ASSERT(env, g_locatorNapiPtr != nullptr, "locator instance is null.");

    napi_valuetype eventName = napi_undefined;
    napi_typeof(env, argv[0], &eventName);
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");
    LBSLOGI(LOCATION_NAPI, "Off function entry");

    if (argc >= requireArgcWithCb) {
        napi_valuetype handler = napi_undefined;
        napi_typeof(env, argv[1], &handler);
        NAPI_ASSERT(env, handler == napi_function, "type mismatch for parameter 2");
    }

    EventManager* manager = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&manager);
    if (status == napi_ok && manager != nullptr) {
        char type[64] = {0};
        size_t typeLen = 0;
        napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);
        std::string event = type;
        LBSLOGI(LOCATION_NAPI, "Unsubscribe event: %{public}s", event.c_str());

        if (event == "locationServiceState") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->UnSubscribeLocationServiceState();
        } else if (event == "locationChange") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->UnSubscribeLocationChange(manager->m_locatorCallback);
        } else if (event == "gnssStatusChange") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->UnSubscribeGnssStatus();
        } else if (event == "nmeaMessageChange") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->UnSubscribeNmeaMessage();
        } else if (event == "cachedGnssLocationsReporting") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->UnSubscribeCacheLocationChange(manager->m_cachedCallback);
        } else if (event == "fenceStatusChange") {
            NAPI_ASSERT(env, argc == 2, "number of parameters is wrong");
            manager->UnSubscribeFenceStatusChange(type, argv[1], argv[2]);
        }
        manager->DeleteHanderRef(argv[1]);
    } else {
        LBSLOGE(LOCATION_NAPI, "Off unwrap class failed");
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo)
{
    TRACE_FUNC_CALL;
    size_t requireArgc = 0;
    size_t argc = 3;
    napi_value argv[3] = {0};
    napi_value thisVar = 0;
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    napi_value result = nullptr;

    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);
    NAPI_ASSERT(env, argc >= requireArgc, "number of parameters is error");

    napi_valuetype valueType = napi_undefined;
    NAPI_ASSERT(env, g_locatorNapiPtr != nullptr, "locator instance is null.");
    LBSLOGI(LOCATION_NAPI, "GetCurrentLocation enter");

    if (argc == 1) {
        napi_typeof(env, argv[0], &valueType);
        NAPI_ASSERT(env, valueType == napi_function || valueType == napi_object, "type mismatch for parameter 2");
    }
    if (argc == 2) {
        napi_valuetype valueType1 = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        napi_typeof(env, argv[1], &valueType1);
        NAPI_ASSERT(env, valueType == napi_object, "type mismatch for parameter 1");
        NAPI_ASSERT(env, valueType1 == napi_function, "type mismatch for parameter 2");
    }
    if (!CommonUtils::CheckLocationPermission(callingPid, callingUid)) {
        LBSLOGE(LOCATION_NAPI, "pid:%{public}d uid:%{public}d has no access permission to get cache location",
                callingPid, callingUid);
        g_locatorNapiPtr->RequestPermission(env);
        return result;
    }

    EventManager* manager = nullptr;
    napi_status status = napi_unwrap(env, thisVar, (void**)&manager);
    if (status == napi_ok && manager != nullptr) {
        return manager->RequestLocationOnce(env, argc, argv);
    } else {
        LBSLOGE(LOCATION_NAPI, "On unwrap class failed");
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value LocationEventListenerConstructor(napi_env env, napi_callback_info cbinfo)
{
    LBSLOGI(LOCATION_NAPI, "Event listener constructor");
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    EventManager* eventManager = new EventManager(env, thisVar);
    if (eventManager == nullptr) {
        LBSLOGE(LOCATION_NAPI, "Init listener constructor failed");
        return nullptr;
    }
    napi_wrap(
        env, thisVar, eventManager,
        [](napi_env env, void* data, void* hint) {
            LBSLOGI(LOCATION_NAPI, "Event listener destructor");
            EventManager *eventManager = (EventManager *)data;
            delete eventManager;
            eventManager = nullptr;
        },
        nullptr, nullptr);
    return thisVar;
}
}  // namespace Location
}  // namespace OHOS
