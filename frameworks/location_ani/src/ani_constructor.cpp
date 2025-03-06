/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <ani.h>
#include <array>
#include <iostream>
#include <unistd.h>
#include "locator_callback_ani.h"
#include "locator.h"
#include "request_config.h"
#include "common_utils.h"

auto g_locatorProxy = OHOS::Location::Locator::GetInstance();
static constexpr int LASTLOCATION_CACHED_TIME = 10 * 60;

OHOS::sptr<OHOS::Location::LocatorCallbackAni> CreateSingleLocationCallbackHost()
{
    auto singleLocatorCallbackHost =
        OHOS::sptr<OHOS::Location::LocatorCallbackAni>(new (std::nothrow) OHOS::Location::LocatorCallbackAni());
    singleLocatorCallbackHost->SetFixNumber(1);
    return singleLocatorCallbackHost;
}

int GetCurrentLocationType(std::unique_ptr<OHOS::Location::RequestConfig>& config)
{
    if (config->GetPriority() == OHOS::Location::LOCATION_PRIORITY_ACCURACY ||
        (config->GetScenario() == OHOS::Location::SCENE_UNSET &&
        config->GetPriority() == OHOS::Location::PRIORITY_ACCURACY) ||
        config->GetScenario() == OHOS::Location::SCENE_NAVIGATION ||
        config->GetScenario() == OHOS::Location::SCENE_TRAJECTORY_TRACKING ||
        config->GetScenario() == OHOS::Location::SCENE_CAR_HAILING) {
        return OHOS::Location::LOCATION_PRIORITY_ACCURACY;
    } else {
        return OHOS::Location::LOCATION_PRIORITY_LOCATING_SPEED;
    }
}

bool IsRequestConfigValid(std::unique_ptr<OHOS::Location::RequestConfig>& config)
{
    if (config == nullptr) {
        return false;
    }
    if ((config->GetScenario() > OHOS::Location::SCENE_NO_POWER ||
        config->GetScenario() < OHOS::Location::SCENE_UNSET) &&
        (config->GetScenario() > OHOS::Location::LOCATION_SCENE_RIDE ||
        config->GetScenario() < OHOS::Location::LOCATION_SCENE_NAVIGATION) &&
        (config->GetScenario() > OHOS::Location::LOCATION_SCENE_NO_POWER_CONSUMPTION ||
        config->GetScenario() < OHOS::Location::LOCATION_SCENE_HIGH_POWER_CONSUMPTION)) {
        return false;
    }
    if ((config->GetPriority() > OHOS::Location::PRIORITY_FAST_FIRST_FIX ||
        config->GetPriority() < OHOS::Location::PRIORITY_UNSET) &&
        (config->GetPriority() > OHOS::Location::LOCATION_PRIORITY_LOCATING_SPEED ||
        config->GetPriority() < OHOS::Location::LOCATION_PRIORITY_ACCURACY)) {
        return false;
    }
    if (config->GetTimeOut() < 1) {
        return false;
    }
    if (config->GetTimeInterval() < 0) {
        return false;
    }
    if (config->GetDistanceInterval() < 0) {
        return false;
    }
    if (config->GetMaxAccuracy() < 0) {
        return false;
    }
    return true;
}

int JsObjectToInt(ani_env* env, ani_object request, std::string property, int32_t& value)
{
    ani_int int_value;
    if (ANI_OK != env->Object_GetPropertyByName_Int(request, property.c_str(), &int_value)) {
        std::cerr << "Object_GetFieldByName_Ref optionField Failed" << std::endl;
        return -1;
    }
    std::cout << "optionField is:" << int_value << std::endl;
    value = int_value;
    return OHOS::Location::SUCCESS;
}

int JsObjectToDouble(ani_env* env, ani_object request, std::string property, double& value)
{
    ani_double double_value;
    if (ANI_OK != env->Object_GetPropertyByName_Double(request, property.c_str(), &double_value)) {
        std::cerr << "Object_GetFieldByName_Ref optionField Failed" << std::endl;
        return -1;
    }
    std::cout << "optionField is:" << double_value << std::endl;
    value = double_value;
    return OHOS::Location::SUCCESS;
}

void JsObjToRequestConfig(ani_env* env, ani_object request,
    std::unique_ptr<OHOS::Location::RequestConfig>& requestConfig)
{
    ani_class currentLocationRequest;
    env->FindClass("Llocation_ani/CurrentLocationRequest;", &currentLocationRequest);
    ani_boolean isCurrentLocationRequest;
    env->Object_InstanceOf(request, currentLocationRequest, &isCurrentLocationRequest);
    if (isCurrentLocationRequest) {
        int value = 0;
        double valueDouble = 0.0;
        if (JsObjectToInt(env, request, "priority", value) == OHOS::Location::SUCCESS) {
            requestConfig->SetPriority(value);
        }
        if (JsObjectToInt(env, request, "scenario", value) == OHOS::Location::SUCCESS) {
            requestConfig->SetScenario(value);
        }
        if (JsObjectToDouble(env, request, "maxAccuracy", valueDouble) == OHOS::Location::SUCCESS) {
            requestConfig->SetMaxAccuracy(valueDouble);
        }
        if (JsObjectToInt(env, request, "timeoutMs", value) == OHOS::Location::SUCCESS) {
            requestConfig->SetTimeOut(value);
        }
        std::cout << "Object is CurrentLocationRequest Object Content:" << std::endl;
        return;
    }

    ani_class singleLocationRequest;
    env->FindClass("Llocation_ani/SingleLocationRequest;", &singleLocationRequest);
    ani_boolean isSingleLocationRequest;
    env->Object_InstanceOf(request, singleLocationRequest, &isSingleLocationRequest);
    if (isSingleLocationRequest) {
        int value = 0;
        if (JsObjectToInt(env, request, "locatingPriority", value) == OHOS::Location::SUCCESS) {
            requestConfig->SetPriority(value);
        }
        if (JsObjectToInt(env, request, "locatingTimeoutMs", value) == OHOS::Location::SUCCESS) {
            requestConfig->SetTimeOut(value);
        }
        std::cout << "Object is singleLocationRequest Object Content:"  << std::endl;
        return;
    }
}

void SetValueInt32(ani_env* env, ani_class* cls, std::string property, int32_t value, ani_object& location_obj)
{
    ani_method method;
    if (ANI_OK != env->Class_FindMethod(*cls, property.c_str(), nullptr, &method)) {
        std::cerr << "Class_FindMethod Fail'" << std::endl;
    }
    if (ANI_OK != env->Object_CallMethod_Void(location_obj, method, ani_int(value))) {
        std::cerr << "Object_CallMethod_Void Fail'" << std::endl;
    }
}

void SetValueInt64(ani_env* env, ani_class* cls, std::string property, int64_t value, ani_object& location_obj)
{
    ani_method method;
    if (ANI_OK != env->Class_FindMethod(*cls, property.c_str(), nullptr, &method)) {
        std::cerr << "Class_FindMethod Fail'" << std::endl;
    }
    if (ANI_OK != env->Object_CallMethod_Void(location_obj, method, ani_long(value))) {
        std::cerr << "Object_CallMethod_Void Fail'" << std::endl;
    }
}

void SetValueDouble(ani_env* env, ani_class* cls, std::string property, double value, ani_object& location_obj)
{
    ani_method method;
    if (ANI_OK != env->Class_FindMethod(*cls, property.c_str(), nullptr, &method)) {
        std::cerr << "Class_FindMethod Fail'" << std::endl;
    }
    if (ANI_OK != env->Object_CallMethod_Void(location_obj, method, ani_double(value))) {
        std::cerr << "Object_CallMethod_Void Fail'" << std::endl;
    }
}

static ani_object LocationToJs(ani_env* env, const std::unique_ptr<OHOS::Location::Location>& locationInfo)
{
    ani_object location_obj = {};
    static const char *className = "Llocation_ani/LocationInner;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return location_obj;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        std::cerr << "get ctor Failed'" << className << "'" << std::endl;
        return location_obj;
    }
    if (ANI_OK != env->Object_New(cls, ctor, &location_obj)) {
        std::cerr << "Create Object Failed'" << className << "'" << std::endl;
        return location_obj;
    }

    SetValueDouble(env, &cls, "<set>latitude", locationInfo->GetLatitude(), location_obj);
    SetValueDouble(env, &cls, "<set>longitude", locationInfo->GetLongitude(), location_obj);
    SetValueDouble(env, &cls, "<set>altitude", locationInfo->GetAltitude(), location_obj);
    SetValueDouble(env, &cls, "<set>accuracy", locationInfo->GetAccuracy(), location_obj);
    SetValueDouble(env, &cls, "<set>speed", locationInfo->GetSpeed(), location_obj);
    SetValueInt64(env, &cls, "<set>timeStamp", locationInfo->GetTimeStamp(), location_obj);
    SetValueDouble(env, &cls, "<set>direction", locationInfo->GetDirection(), location_obj);
    SetValueInt64(env, &cls, "<set>timeSinceBoot", locationInfo->GetTimeSinceBoot(), location_obj);
    return location_obj;
}

static ani_object GetCurrentLocation([[maybe_unused]]ani_env *env,
    [[maybe_unused]]ani_object object, ani_object request)
{
    // request to capi request
    auto requestConfig = std::make_unique<OHOS::Location::RequestConfig>();
    JsObjToRequestConfig(env, request, requestConfig);
    //receive callback
    requestConfig->SetFixNumber(1);
    if (!IsRequestConfigValid(requestConfig)) {
        return {};
    }
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    if (singleLocatorCallbackHost == nullptr) {
        return {};
    }
    singleLocatorCallbackHost->SetLocationPriority(GetCurrentLocationType(requestConfig));

    auto callbackPtr = OHOS::sptr<OHOS::Location::ILocatorCallback>(singleLocatorCallbackHost);
    OHOS::Location::LocationErrCode errorCode = g_locatorProxy->StartLocatingV9(requestConfig, callbackPtr);
    if (errorCode != OHOS::Location::SUCCESS) {
        singleLocatorCallbackHost->SetCount(0);
    }
    if (requestConfig->GetTimeOut() > OHOS::Location::DEFAULT_TIMEOUT_30S) {
        singleLocatorCallbackHost->Wait(OHOS::Location::DEFAULT_TIMEOUT_30S);
        if (singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
            singleLocatorCallbackHost->Wait(requestConfig->GetTimeOut() - OHOS::Location::DEFAULT_TIMEOUT_30S);
        }
    } else {
        singleLocatorCallbackHost->Wait(requestConfig->GetTimeOut());
    }
    g_locatorProxy->StopLocatingV9(callbackPtr);
    if (singleLocatorCallbackHost->GetCount() != 0 && singleLocatorCallbackHost->GetSingleLocation() == nullptr) {
        std::unique_ptr<OHOS::Location::Location> location = nullptr;
        g_locatorProxy->GetCachedLocationV9(location);
        int64_t curTime = OHOS::Location::CommonUtils::GetCurrentTimeStamp();
        if (location != nullptr &&
            (curTime - location->GetTimeStamp() / OHOS::Location::MILLI_PER_SEC) <= LASTLOCATION_CACHED_TIME) {
            singleLocatorCallbackHost->SetSingleLocation(location);
        } else {
            return {};
        }
    }
    if (singleLocatorCallbackHost != nullptr && singleLocatorCallbackHost->GetSingleLocation() != nullptr) {
        std::unique_ptr<OHOS::Location::Location> location =
            std::make_unique<OHOS::Location::Location>(*singleLocatorCallbackHost->GetSingleLocation());
        return LocationToJs(env, location);
    } else {
        LBSLOGE(OHOS::Location::LOCATOR_STANDARD, "m_singleLocation is nullptr!");
    }

    return {};
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (vm == nullptr || ANI_OK != vm->GetEnv(ANI_VERSION_1, &env) || env == nullptr) {
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    static const char *className = "Llocation_ani/ETSGLOBAL";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return ANI_ERROR;
    }

    std::array methods = {
        ani_native_function {"getCurrentLocation", "Lstd/core/Object;:Llocation_ani/Location;",
            reinterpret_cast<void *>(GetCurrentLocation)},
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        std::cerr << "Cannot bind native methods to '" << className << "'" << std::endl;
        return ANI_ERROR;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}