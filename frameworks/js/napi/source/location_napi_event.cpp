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

#include "callback_manager.h"
#include "common_utils.h"
#include "location_log.h"
#include "location_napi_errcode.h"
#include "country_code_callback_host.h"
#include "locator.h"
#include "napi_util.h"
#ifdef SUPPORT_JSSTACK
#include "xpower_event_js.h"
#endif

namespace OHOS {
namespace Location {
CallbackManager<LocationSwitchCallbackHost> g_switchCallbacks;
CallbackManager<LocatorCallbackHost> g_locationCallbacks;
CallbackManager<GnssStatusCallbackHost> g_gnssStatusInfoCallbacks;
CallbackManager<NmeaMessageCallbackHost> g_nmeaCallbacks;
CallbackManager<CachedLocationsCallbackHost> g_cachedLocationCallbacks;
CallbackManager<CountryCodeCallbackHost> g_countryCodeCallbacks;
std::vector<GeoFenceState*> mFences;

std::unique_ptr<CachedGnssLocationsRequest> g_cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
std::unique_ptr<GeofenceRequest> g_fenceRequest = std::make_unique<GeofenceRequest>();
std::unique_ptr<RequestConfig> g_requestConfig = std::make_unique<RequestConfig>();
std::shared_ptr<CallbackResumeManager> g_callbackResumer = std::make_shared<CallbackResumeManager>();
auto g_locatorProxy = Locator::GetInstance();

std::map<std::string, bool(*)(const napi_env &)> g_offAllFuncMap;
std::map<std::string, bool(*)(const napi_env &, const napi_value &)> g_offFuncMap;
std::map<std::string, bool(*)(const napi_env &, const size_t, const napi_value *)> g_onFuncMap;
std::map<std::string, void(CallbackResumeManager::*)()> g_resumeFuncMap;

void InitOnFuncMap()
{
    if (g_onFuncMap.size() != 0) {
        return;
    }
#ifdef ENABLE_NAPI_MANAGER
    g_onFuncMap.insert(std::make_pair("locationEnabledChange", &OnLocationServiceStateCallback));
    g_onFuncMap.insert(std::make_pair("cachedGnssLocationsChange", &OnCachedGnssLocationsReportingCallback));
    g_onFuncMap.insert(std::make_pair("satelliteStatusChange", &OnGnssStatusChangeCallback));
    g_onFuncMap.insert(std::make_pair("gnssFenceStatusChange", &OnFenceStatusChangeCallback));
    g_onFuncMap.insert(std::make_pair("nmeaMessage", &OnNmeaMessageChangeCallback));
#else
    g_onFuncMap.insert(std::make_pair("locationServiceState", &OnLocationServiceStateCallback));
    g_onFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OnCachedGnssLocationsReportingCallback));
    g_onFuncMap.insert(std::make_pair("gnssStatusChange", &OnGnssStatusChangeCallback));
    g_onFuncMap.insert(std::make_pair("fenceStatusChange", &OnFenceStatusChangeCallback));
    g_onFuncMap.insert(std::make_pair("nmeaMessageChange", &OnNmeaMessageChangeCallback));
#endif
    g_onFuncMap.insert(std::make_pair("locationChange", &OnLocationChangeCallback));
    g_onFuncMap.insert(std::make_pair("countryCodeChange", &OnCountryCodeChangeCallback));
}

void InitOffFuncMap()
{
    if (g_offAllFuncMap.size() != 0 || g_offFuncMap.size() != 0) {
        return;
    }
#ifdef ENABLE_NAPI_MANAGER
    g_offAllFuncMap.insert(std::make_pair("locationEnabledChange", &OffAllLocationServiceStateCallback));
    g_offAllFuncMap.insert(std::make_pair("cachedGnssLocationsChange", &OffAllCachedGnssLocationsReportingCallback));
    g_offAllFuncMap.insert(std::make_pair("satelliteStatusChange", &OffAllGnssStatusChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("nmeaMessage", &OffAllNmeaMessageChangeCallback));
#else
    g_offAllFuncMap.insert(std::make_pair("locationServiceState", &OffAllLocationServiceStateCallback));
    g_offAllFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OffAllCachedGnssLocationsReportingCallback));
    g_offAllFuncMap.insert(std::make_pair("gnssStatusChange", &OffAllGnssStatusChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("nmeaMessageChange", &OffAllNmeaMessageChangeCallback));
#endif
    g_offAllFuncMap.insert(std::make_pair("locationChange", &OffAllLocationChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("countryCodeChange", &OffAllCountryCodeChangeCallback));

#ifdef ENABLE_NAPI_MANAGER
    g_offFuncMap.insert(std::make_pair("locationEnabledChange", &OffLocationServiceStateCallback));
    g_offFuncMap.insert(std::make_pair("cachedGnssLocationsChange", &OffCachedGnssLocationsReportingCallback));
    g_offFuncMap.insert(std::make_pair("satelliteStatusChange", &OffGnssStatusChangeCallback));
    g_offFuncMap.insert(std::make_pair("nmeaMessage", &OffNmeaMessageChangeCallback));
#else
    g_offFuncMap.insert(std::make_pair("locationServiceState", &OffLocationServiceStateCallback));
    g_offFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OffCachedGnssLocationsReportingCallback));
    g_offFuncMap.insert(std::make_pair("gnssStatusChange", &OffGnssStatusChangeCallback));
    g_offFuncMap.insert(std::make_pair("nmeaMessageChange", &OffNmeaMessageChangeCallback));
#endif
    g_offFuncMap.insert(std::make_pair("locationChange", &OffLocationChangeCallback));
    g_offFuncMap.insert(std::make_pair("countryCodeChange", &OffCountryCodeChangeCallback));
}

void SubscribeLocationServiceState(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackHost>& switchCallbackHost)
{
    switchCallbackHost->SetEnv(env);
    switchCallbackHost->SetHandleCb(handlerRef);
    g_locatorProxy->RegisterSwitchCallback(switchCallbackHost->AsObject(), DEFAULT_UID);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeLocationServiceStateV9(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackHost>& switchCallbackHost)
{
    switchCallbackHost->SetEnv(env);
    switchCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->RegisterSwitchCallbackV9(switchCallbackHost->AsObject());
}
#endif

void SubscribeGnssStatus(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost)
{
    gnssStatusCallbackHost->SetEnv(env);
    gnssStatusCallbackHost->SetHandleCb(handlerRef);
    g_locatorProxy->RegisterGnssStatusCallback(gnssStatusCallbackHost->AsObject(), DEFAULT_UID);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeGnssStatusV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost)
{
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    gnssStatusCallbackHost->SetEnv(env);
    gnssStatusCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->RegisterGnssStatusCallbackV9(gnssStatusCallbackHost->AsObject());
}
#endif

void SubscribeNmeaMessage(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost)
{
    nmeaMessageCallbackHost->SetEnv(env);
    nmeaMessageCallbackHost->SetHandleCb(handlerRef);
    g_locatorProxy->RegisterNmeaMessageCallback(nmeaMessageCallbackHost->AsObject(), DEFAULT_UID);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeNmeaMessageV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost)
{
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    nmeaMessageCallbackHost->SetEnv(env);
    nmeaMessageCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->RegisterNmeaMessageCallbackV9(nmeaMessageCallbackHost->AsObject());
}
#endif

void UnSubscribeLocationServiceState(sptr<LocationSwitchCallbackHost>& switchCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationServiceState");
    g_locatorProxy->UnregisterSwitchCallback(switchCallbackHost->AsObject());
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnSubscribeLocationServiceStateV9(sptr<LocationSwitchCallbackHost>& switchCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationServiceStateV9");
    return g_locatorProxy->UnregisterSwitchCallbackV9(switchCallbackHost->AsObject());
}
#endif

void UnSubscribeGnssStatus(sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeGnssStatus");
    g_locatorProxy->UnregisterGnssStatusCallback(gnssStatusCallbackHost->AsObject());
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnSubscribeGnssStatusV9(sptr<GnssStatusCallbackHost>& gnssStatusCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeGnssStatusV9");
    return g_locatorProxy->UnregisterGnssStatusCallbackV9(gnssStatusCallbackHost->AsObject());
}
#endif

void UnSubscribeNmeaMessage(sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeNmeaMessage");
    g_locatorProxy->UnregisterNmeaMessageCallback(nmeaMessageCallbackHost->AsObject());
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnSubscribeNmeaMessageV9(sptr<NmeaMessageCallbackHost>& nmeaMessageCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeNmeaMessageV9");
    return g_locatorProxy->UnregisterNmeaMessageCallbackV9(nmeaMessageCallbackHost->AsObject());
}
#endif

void SubscribeLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackHost>& locatorCallbackHost)
{
    auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
    locatorCallbackHost->SetFixNumber(0);
    locatorCallbackHost->SetEnv(env);
    locatorCallbackHost->SetHandleCb(handlerRef);
    JsObjToLocationRequest(env, object, g_requestConfig);
    g_locatorProxy->StartLocating(g_requestConfig, locatorCallback);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackHost>& locatorCallbackHost)
{
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
    locatorCallbackHost->SetFixNumber(0);
    locatorCallbackHost->SetEnv(env);
    locatorCallbackHost->SetHandleCb(handlerRef);
    JsObjToLocationRequest(env, object, g_requestConfig);
    return g_locatorProxy->StartLocatingV9(g_requestConfig, locatorCallback);
}
#endif

void SubscribeCountryCodeChange(const napi_env& env,
    const napi_ref& handlerRef, sptr<CountryCodeCallbackHost>& callbackHost)
{
    auto callbackPtr = sptr<ICountryCodeCallback>(callbackHost);
    callbackHost->SetEnv(env);
    callbackHost->SetCallback(handlerRef);
    g_locatorProxy->RegisterCountryCodeCallback(callbackPtr->AsObject(), DEFAULT_UID);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeCountryCodeChangeV9(const napi_env& env,
    const napi_ref& handlerRef, sptr<CountryCodeCallbackHost>& callbackHost)
{
    auto callbackPtr = sptr<ICountryCodeCallback>(callbackHost);
    callbackHost->SetEnv(env);
    callbackHost->SetCallback(handlerRef);
    return g_locatorProxy->RegisterCountryCodeCallbackV9(callbackPtr->AsObject());
}
#endif

void UnsubscribeCountryCodeChange(sptr<CountryCodeCallbackHost>& callbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnsubscribeCountryCodeChange");
    g_locatorProxy->UnregisterCountryCodeCallback(callbackHost->AsObject());
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnsubscribeCountryCodeChangeV9(sptr<CountryCodeCallbackHost>& callbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnsubscribeCountryCodeChangeV9");
    return g_locatorProxy->UnregisterCountryCodeCallbackV9(callbackHost->AsObject());
}
#endif

void SubscribeCacheLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackHost>& cachedCallbackHost)
{
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    cachedCallbackHost->SetEnv(env);
    cachedCallbackHost->SetHandleCb(handlerRef);
    JsObjToCachedLocationRequest(env, object, g_cachedRequest);
    g_locatorProxy->RegisterCachedLocationCallback(g_cachedRequest, cachedCallback);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeCacheLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackHost>& cachedCallbackHost)
{
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    cachedCallbackHost->SetEnv(env);
    cachedCallbackHost->SetHandleCb(handlerRef);
    JsObjToCachedLocationRequest(env, object, g_cachedRequest);
    g_locatorProxy->RegisterCachedLocationCallbackV9(g_cachedRequest, cachedCallback);
    return ERRCODE_NOT_SUPPORTED;
}
#endif

void SubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler)
{
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_RETURN_VOID(env, napi_unwrap(env, handler, (void **)&wantAgent));
    JsObjToGeoFenceRequest(env, object, g_fenceRequest);
    auto state = new (std::nothrow) GeoFenceState(g_fenceRequest->geofence, wantAgent);
    if (state != nullptr) {
        mFences.push_back(state);
        g_locatorProxy->AddFence(g_fenceRequest);
    }
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeFenceStatusChangeV9(const napi_env& env, const napi_value& object, const napi_value& handler)
{
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_BASE(env, napi_unwrap(env, handler, (void **)&wantAgent), ERRCODE_NOT_SUPPORTED);
    JsObjToGeoFenceRequest(env, object, g_fenceRequest);
    auto state = new (std::nothrow) GeoFenceState(g_fenceRequest->geofence, wantAgent);
    if (state != nullptr) {
        mFences.push_back(state);
        g_locatorProxy->AddFenceV9(g_fenceRequest);
    }
    return ERRCODE_NOT_SUPPORTED;
}
#endif

void UnSubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler)
{
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_RETURN_VOID(env, napi_unwrap(env, handler, (void **)&wantAgent));
    JsObjToGeoFenceRequest(env, object, g_fenceRequest);
    if (mFences.size() > 0) {
        mFences.erase(mFences.begin());
        g_locatorProxy->RemoveFence(g_fenceRequest);
    }
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnSubscribeFenceStatusChangeV9(const napi_env& env, const napi_value& object, const napi_value& handler)
{
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    auto wantAgent = AbilityRuntime::WantAgent::WantAgent();
    NAPI_CALL_BASE(env, napi_unwrap(env, handler, (void **)&wantAgent), ERRCODE_NOT_SUPPORTED);
    JsObjToGeoFenceRequest(env, object, g_fenceRequest);
    if (mFences.size() > 0) {
        mFences.erase(mFences.begin());
        g_locatorProxy->RemoveFenceV9(g_fenceRequest);
    }
    return ERRCODE_NOT_SUPPORTED;
}
#endif

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
                context->errCode = ERRCODE_LOCATING_FAIL;
            }
            callbackHost->SetCount(1);
#ifndef ENABLE_NAPI_MANAGER
        } else {
            context->errCode = LOCATION_SWITCH_ERROR;
#endif
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

std::unique_ptr<RequestConfig> CreateRequestConfig(const napi_env& env,
    const napi_value* argv, const size_t& objectArgsNum)
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

#ifdef ENABLE_NAPI_MANAGER
napi_value RequestLocationOnceV9(const napi_env& env, const size_t argc, const napi_value* argv)
{
    size_t objectArgsNum = 0;
    objectArgsNum = static_cast<size_t>(GetObjectArgsNum(env, argc, argv));
    auto requestConfig = CreateRequestConfig(env, argv, objectArgsNum);
    if (requestConfig == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    if (singleLocatorCallbackHost == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    auto callbackPtr = sptr<ILocatorCallback>(singleLocatorCallbackHost);
    errorCode = g_locatorProxy->StartLocatingV9(requestConfig, callbackPtr);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }

    auto asyncContext = CreateSingleLocationAsyncContext(env, requestConfig, singleLocatorCallbackHost);
    if (asyncContext == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}
#endif

void UnSubscribeLocationChange(sptr<ILocatorCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationChange");
    g_locatorProxy->StopLocating(callback);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnSubscribeLocationChangeV9(sptr<ILocatorCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationChangeV9");
    return g_locatorProxy->StopLocatingV9(callback);
}
#endif

void UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeCacheLocationChange");
    g_locatorProxy->UnregisterCachedLocationCallback(callback);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnSubscribeCacheLocationChangeV9(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeCacheLocationChangeV9");
    g_locatorProxy->UnregisterCachedLocationCallbackV9(callback);
    return ERRCODE_NOT_SUPPORTED;
}
#endif

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

bool OnLocationServiceStateCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", INPUT_PARAMS_ERROR);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", INPUT_PARAMS_ERROR);
#endif
    if (g_switchCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackHost>(new (std::nothrow) LocationSwitchCallbackHost());
    if (switchCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = SubscribeLocationServiceStateV9(env, handlerRef, switchCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeLocationServiceState(env, handlerRef, switchCallbackHost);
#endif
        g_switchCallbacks.AddCallback(env, handlerRef, switchCallbackHost);
    }
    return true;
}

bool OnCachedGnssLocationsReportingCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM3) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, argv[PARAM1], &valueType), false);
    if (valueType != napi_object || !CheckIfParamIsFunctionType(env, argv[PARAM2])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM3, "number of parameters is wrong", INPUT_PARAMS_ERROR);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM2]),
        "callback should be function, mismatch for param.", INPUT_PARAMS_ERROR);
#endif
#ifndef ENABLE_NAPI_MANAGER
    if (!g_locatorProxy->IsLocationEnabled()) {
        LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
        return false;
    }
#endif
    // the third params should be handler
    if (g_cachedLocationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto cachedCallbackHost =
        sptr<CachedLocationsCallbackHost>(new (std::nothrow) CachedLocationsCallbackHost());
    if (cachedCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM2], PARAM1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = SubscribeCacheLocationChangeV9(env, argv[PARAM1], handlerRef, cachedCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeCacheLocationChange(env, argv[PARAM1], handlerRef, cachedCallbackHost);
#endif
        g_cachedLocationCallbacks.AddCallback(env, handlerRef, cachedCallbackHost);
    }
    return true;
}

bool OnGnssStatusChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", INPUT_PARAMS_ERROR);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", INPUT_PARAMS_ERROR);
#endif
    if (g_gnssStatusInfoCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackHost>(new (std::nothrow) GnssStatusCallbackHost());
    if (gnssCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = SubscribeGnssStatusV9(env, handlerRef, gnssCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeGnssStatus(env, handlerRef, gnssCallbackHost);
#endif
        g_gnssStatusInfoCallbacks.AddCallback(env, handlerRef, gnssCallbackHost);
    }
    return true;
}

bool OnLocationChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef SUPPORT_JSSTACK
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "GNSS_STATE");
#endif
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM3) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, argv[PARAM1], &valueType), false);
    if (valueType != napi_object || !CheckIfParamIsFunctionType(env, argv[PARAM2])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM3, "number of parameters is wrong", INPUT_PARAMS_ERROR);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM2]),
        "callback should be function, mismatch for param.", INPUT_PARAMS_ERROR);
    if (!g_locatorProxy->IsLocationEnabled()) {
        LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
        return false;
    }
#endif
    // the third params should be handler
    if (g_locationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto locatorCallbackHost =
        sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    if (locatorCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM2], 1, &handlerRef), false);
        // argv[1]:request params, argv[2]:handler
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = SubscribeLocationChangeV9(env, argv[PARAM1], handlerRef, locatorCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeLocationChange(env, argv[PARAM1], handlerRef, locatorCallbackHost);
#endif
        g_locationCallbacks.AddCallback(env, handlerRef, locatorCallbackHost);
    }
    return true;
}

bool OnNmeaMessageChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", INPUT_PARAMS_ERROR);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", INPUT_PARAMS_ERROR);
#endif
    if (g_nmeaCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackHost>(new (std::nothrow) NmeaMessageCallbackHost());
    if (nmeaCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = SubscribeNmeaMessageV9(env, handlerRef, nmeaCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeNmeaMessage(env, handlerRef, nmeaCallbackHost);
#endif
        g_nmeaCallbacks.AddCallback(env, handlerRef, nmeaCallbackHost);
    }
    return true;
}

bool OnCountryCodeChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", INPUT_PARAMS_ERROR);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", INPUT_PARAMS_ERROR);
#endif
    if (g_countryCodeCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto callbackHost =
        sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    if (callbackHost) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = SubscribeCountryCodeChangeV9(env, handlerRef, callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeCountryCodeChange(env, handlerRef, callbackHost);
#endif
        g_countryCodeCallbacks.AddCallback(env, handlerRef, callbackHost);
    }
    return true;
}

bool OnFenceStatusChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM3) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM3, "number of parameters is wrong", INPUT_PARAMS_ERROR);
    if (!g_locatorProxy->IsLocationEnabled()) {
        LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
        return false;
    }
#endif
    // the third params should be handler
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = SubscribeFenceStatusChangeV9(env, argv[PARAM1], argv[PARAM2]);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#else
    SubscribeFenceStatusChange(env, argv[PARAM1], argv[PARAM2]);
#endif
    return true;
}

napi_value On(napi_env env, napi_callback_info cbinfo)
{
    InitOnFuncMap();
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS] = {0};
    napi_value thisVar = nullptr;
    LBSLOGI(LOCATION_NAPI, "On function entry");
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    napi_valuetype eventName = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &eventName));
#ifdef ENABLE_NAPI_MANAGER
    if (eventName != napi_string) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");
#endif
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");

    char type[64] = {0}; // max length
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[PARAM0], type, sizeof(type), &typeLen));
    std::string event = type;
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", event.c_str());
    g_locatorProxy->SetResumer(g_callbackResumer);
    auto onCallbackFunc = g_onFuncMap.find(event);
    if (onCallbackFunc != g_onFuncMap.end() && onCallbackFunc->second != nullptr) {
        auto memberFunc = onCallbackFunc->second;
        (*memberFunc)(env, argc, argv);
    }
    return UndefinedNapiValue(env);
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
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnSubscribeLocationServiceStateV9(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationServiceState(callbackHost);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_switchCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllLocationChangeCallback(const napi_env& env)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
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
#ifdef ENABLE_NAPI_MANAGER
        errorCode = UnSubscribeLocationChangeV9(locatorCallback);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationChange(locatorCallback);
#endif
        callbackHost->DeleteAllCallbacks();
        callbackHost = nullptr;
    }
    g_locationCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllGnssStatusChangeCallback(const napi_env& env)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
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
#ifdef ENABLE_NAPI_MANAGER
        errorCode = UnSubscribeGnssStatusV9(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeGnssStatus(callbackHost);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_gnssStatusInfoCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllNmeaMessageChangeCallback(const napi_env& env)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
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
#ifdef ENABLE_NAPI_MANAGER
        errorCode = UnSubscribeNmeaMessageV9(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeNmeaMessage(callbackHost);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_nmeaCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllCachedGnssLocationsReportingCallback(const napi_env& env)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
    std::map<napi_env, std::map<napi_ref, sptr<CachedLocationsCallbackHost>>> callbackMap =
        g_cachedLocationCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
#ifdef ENABLE_NAPI_MANAGER
        HandleSyncErrCode(env, ERRCODE_NOT_SUPPORTED);
#endif
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        auto cachedCallback = sptr<ICachedLocationsCallback>(callbackHost);
#ifdef ENABLE_NAPI_MANAGER
        errorCode = UnSubscribeCacheLocationChangeV9(cachedCallback);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeCacheLocationChange(cachedCallback);
#endif
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
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnsubscribeCountryCodeChangeV9(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnsubscribeCountryCodeChange(callbackHost);
#endif
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
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnSubscribeLocationServiceStateV9(switchCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationServiceState(switchCallbackHost);
#endif
        g_switchCallbacks.DeleteCallback(env, handler);
        switchCallbackHost->DeleteHandler();
        switchCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffLocationChangeCallback(const napi_env& env, const napi_value& handler)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
    auto locatorCallbackHost = g_locationCallbacks.GetCallbackPtr(env, handler);
    if (locatorCallbackHost) {
        auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
#ifdef ENABLE_NAPI_MANAGER
        errorCode = UnSubscribeLocationChangeV9(locatorCallback);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationChange(locatorCallback);
#endif
        g_locationCallbacks.DeleteCallback(env, handler);
        locatorCallbackHost->DeleteAllCallbacks();
        locatorCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffGnssStatusChangeCallback(const napi_env& env, const napi_value& handler)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
    auto gnssCallbackHost = g_gnssStatusInfoCallbacks.GetCallbackPtr(env, handler);
    if (gnssCallbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        errorCode = UnSubscribeGnssStatusV9(gnssCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeGnssStatus(gnssCallbackHost);
#endif
        g_gnssStatusInfoCallbacks.DeleteCallback(env, handler);
        gnssCallbackHost->DeleteHandler();
        gnssCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffNmeaMessageChangeCallback(const napi_env& env, const napi_value& handler)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
    auto nmeaCallbackHost = g_nmeaCallbacks.GetCallbackPtr(env, handler);
    if (nmeaCallbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode ret = UnSubscribeNmeaMessageV9(nmeaCallbackHost);
        if (ret != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, ret);
            return false;
        }
#else
        UnSubscribeNmeaMessage(nmeaCallbackHost);
#endif
        g_nmeaCallbacks.DeleteCallback(env, handler);
        nmeaCallbackHost->DeleteHandler();
        nmeaCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffCachedGnssLocationsReportingCallback(const napi_env& env, const napi_value& handler)
{
#ifdef ENABLE_NAPI_MANAGER
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return false;
    }
#endif
    auto cachedCallbackHost = g_cachedLocationCallbacks.GetCallbackPtr(env, handler);
    if (cachedCallbackHost) {
        auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
#ifdef ENABLE_NAPI_MANAGER
        errorCode = UnSubscribeCacheLocationChangeV9(cachedCallback);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeCacheLocationChange(cachedCallback);
#endif
        g_cachedLocationCallbacks.DeleteCallback(env, handler);
        cachedCallbackHost->DeleteHandler();
        cachedCallbackHost = nullptr;
        return true;
    } else {
        LBSLOGI(LOCATION_NAPI, "%{public}s, the callback is not in the map", __func__);
#ifdef ENABLE_NAPI_MANAGER
        HandleSyncErrCode(env, ERRCODE_NOT_SUPPORTED);
#endif
    }
    return false;
}

bool OffCountryCodeChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto callbackHost = g_countryCodeCallbacks.GetCallbackPtr(env, handler);
    if (callbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnsubscribeCountryCodeChangeV9(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnsubscribeCountryCodeChange(callbackHost);
#endif
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
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");
#ifdef ENABLE_NAPI_MANAGER
    if (argc < PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    napi_valuetype eventName = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &eventName));
#ifdef ENABLE_NAPI_MANAGER
    if (eventName != napi_string) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");
#endif
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
#ifdef ENABLE_NAPI_MANAGER
        if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
            HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
            return UndefinedNapiValue(env);
        }
#else
        NAPI_ASSERT(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
            "callback should be function, mismatch for param.");
#endif
        auto offCallbackFunc = g_offFuncMap.find(event);
        if (offCallbackFunc != g_offFuncMap.end() && offCallbackFunc->second != nullptr) {
            auto singleMemberFunc = offCallbackFunc->second;
            (*singleMemberFunc)(env, argv[PARAM1]);
        }
#ifdef ENABLE_NAPI_MANAGER
    } else if (argc == PARAM3 && event == "gnssFenceStatusChange") {
        LocationErrCode errorCode = UnSubscribeFenceStatusChangeV9(env, argv[PARAM1], argv[PARAM2]);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
        }
    } else {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
#else
    } else if (argc == PARAM3 && event == "fenceStatusChange") {
        UnSubscribeFenceStatusChange(env, argv[PARAM1], argv[PARAM2]);
#endif
    }
    return UndefinedNapiValue(env);
}

napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));

    napi_valuetype valueType = napi_undefined;
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");
    LBSLOGI(LOCATION_NAPI, "GetCurrentLocation enter");
#ifdef SUPPORT_JSSTACK
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "GNSS_STATE");
#endif
    if (argc == PARAM1) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valueType));
#ifdef ENABLE_NAPI_MANAGER
        if (valueType != napi_function && valueType != napi_object) {
            HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
            return UndefinedNapiValue(env);
        }
#else
        NAPI_ASSERT(env, valueType == napi_function || valueType == napi_object, "type mismatch for parameter 2");
#endif
    }
    if (argc == PARAM2) {
        napi_valuetype valueType1 = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valueType));
        NAPI_CALL(env, napi_typeof(env, argv[PARAM1], &valueType1));
#ifdef ENABLE_NAPI_MANAGER
        if (valueType != napi_object || valueType1 != napi_function) {
            HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
            return UndefinedNapiValue(env);
        }
#else
        NAPI_ASSERT(env, valueType == napi_object, "type mismatch for parameter 1");
        NAPI_ASSERT(env, valueType1 == napi_function, "type mismatch for parameter 2");
#endif
    }
#ifdef ENABLE_NAPI_MANAGER
    return RequestLocationOnceV9(env, argc, argv);
#else
    return RequestLocationOnce(env, argc, argv);
#endif
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode CheckLocationSwitchEnable()
{
    bool isEnabled = false;
    LocationErrCode errorCode = g_locatorProxy->IsLocationEnabledV9(isEnabled);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (!isEnabled) {
        return ERRCODE_SWITCH_OFF;
    }
    return ERRCODE_SUCCESS;
}
#endif

void CallbackResumeManager::InitResumeCallbackFuncMap()
{
    if (g_resumeFuncMap.size() != 0) {
        return;
    }
    g_resumeFuncMap.insert(std::make_pair("satelliteStatusChange", &CallbackResumeManager::ResumeGnssStatusCallback));
    g_resumeFuncMap.insert(std::make_pair("nmeaMessage", &CallbackResumeManager::ResumeNmeaMessageCallback));
    g_resumeFuncMap.insert(std::make_pair("locationChange", &CallbackResumeManager::ResumeLocating));
    g_resumeFuncMap.insert(std::make_pair("countryCodeChange", &CallbackResumeManager::ResumeCountryCodeCallback));
}

void CallbackResumeManager::ResumeCallback()
{
    LBSLOGI(LOCATION_NAPI, "%{public}s enter", __func__);
    InitResumeCallbackFuncMap();
    for (auto iter = g_resumeFuncMap.begin(); iter != g_resumeFuncMap.end(); iter++) {
        auto resumeFunc = iter->second;
        (this->*resumeFunc)();
    }
}

void CallbackResumeManager::ResumeSwitchCallback()
{
    auto callbackMap = g_switchCallbacks.GetCallbackMap();
    for (auto iter = callbackMap.begin(); iter != callbackMap.end(); iter++) {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
            auto switchCallbackHost = innerIter->second;
            if (switchCallbackHost == nullptr) {
                continue;
            }
#ifdef ENABLE_NAPI_MANAGER
            g_locatorProxy->RegisterSwitchCallbackV9(switchCallbackHost->AsObject());
#endif
        }
        LBSLOGI(LOCATION_NAPI, "%{public}s success", __func__);
    }
}

void CallbackResumeManager::ResumeGnssStatusCallback()
{
    auto callbackMap = g_gnssStatusInfoCallbacks.GetCallbackMap();
    for (auto iter = callbackMap.begin(); iter != callbackMap.end(); iter++) {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
            auto gnssStatusCallbackHost = innerIter->second;
            if (gnssStatusCallbackHost == nullptr) {
                continue;
            }
#ifdef ENABLE_NAPI_MANAGER
            g_locatorProxy->RegisterGnssStatusCallbackV9(gnssStatusCallbackHost->AsObject());
#endif
        }
        LBSLOGI(LOCATION_NAPI, "%{public}s success", __func__);
    }
}

void CallbackResumeManager::ResumeNmeaMessageCallback()
{
    auto callbackMap = g_nmeaCallbacks.GetCallbackMap();
    for (auto iter = callbackMap.begin(); iter != callbackMap.end(); iter++) {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
            auto nmeaCallbackHost = innerIter->second;
            if (nmeaCallbackHost == nullptr) {
                continue;
            }
#ifdef ENABLE_NAPI_MANAGER
            g_locatorProxy->RegisterNmeaMessageCallbackV9(nmeaCallbackHost->AsObject());
#endif
        }
        LBSLOGI(LOCATION_NAPI, "%{public}s success", __func__);
    }
}

void CallbackResumeManager::ResumeCountryCodeCallback()
{
    auto callbackMap = g_countryCodeCallbacks.GetCallbackMap();
    for (auto iter = callbackMap.begin(); iter != callbackMap.end(); iter++) {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
            auto countryCodeCallbackHost = innerIter->second;
            if (countryCodeCallbackHost == nullptr) {
                continue;
            }
#ifdef ENABLE_NAPI_MANAGER
            g_locatorProxy->RegisterCountryCodeCallbackV9(countryCodeCallbackHost->AsObject());
#endif
        }
        LBSLOGI(LOCATION_NAPI, "%{public}s success", __func__);
    }
}

void CallbackResumeManager::ResumeCachedLocationCallback()
{
    auto callbackMap = g_cachedLocationCallbacks.GetCallbackMap();
    for (auto iter = callbackMap.begin(); iter != callbackMap.end(); iter++) {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
            auto cachedCallbackHost = innerIter->second;
            if (cachedCallbackHost == nullptr) {
                continue;
            }
            auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
#ifdef ENABLE_NAPI_MANAGER
            g_locatorProxy->RegisterCachedLocationCallbackV9(g_cachedRequest, cachedCallback);
#endif
        }
        LBSLOGI(LOCATION_NAPI, "%{public}s success", __func__);
    }
}

void CallbackResumeManager::ResumeFence()
{
    if (mFences.size() == 0) {
        LBSLOGE(LOCATION_NAPI, "%{public}s, no fence need add", __func__);
        return;
    }
#ifdef ENABLE_NAPI_MANAGER
    g_locatorProxy->AddFenceV9(g_fenceRequest);
#endif
    LBSLOGI(LOCATION_NAPI, "%{public}s success", __func__);
}

void CallbackResumeManager::ResumeLocating()
{
    auto callbackMap = g_locationCallbacks.GetCallbackMap();
    for (auto iter = callbackMap.begin(); iter != callbackMap.end(); iter++) {
        for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
            auto locatorCallbackHost = innerIter->second;
            if (locatorCallbackHost == nullptr) {
                continue;
            }
            auto locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
#ifdef ENABLE_NAPI_MANAGER
            g_locatorProxy->StartLocatingV9(g_requestConfig, locatorCallback);
#endif
        }
        LBSLOGI(LOCATION_NAPI, "%{public}s success", __func__);
    }
}
}  // namespace Location
}  // namespace OHOS
