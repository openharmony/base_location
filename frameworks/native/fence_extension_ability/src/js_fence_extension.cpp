/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "js_fence_extension.h"
#include "js_fence_extension_context.h"
#include "fence_extension_stub_impl.h"

#include "ability_handler.h"
#include "ability_info.h"
#include "hilog_wrapper.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "runtime.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "display_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "location_log.h"
#include "geofence_definition.h"
#include <thread>
#include "napi/native_api.h"
#include "uv.h"

using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace Location {
const size_t ARGC_ONE = 1;
const size_t ARGC_TWO = 2;
const size_t LOAD_SYSTEM_MODULE_ARGC = 1;
const std::string CONTEXT_MODULE_PATH = "app.ability.FenceExtensionContext";
using namespace OHOS::AppExecFwk;

napi_value AttachFenceExtensionContext(napi_env env, void *value, void *)
{
    LBSLOGI(FENCE_EXTENSION, "AttachFenceExtensionContext");
    if (value == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "invalid parameter");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<FenceExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "invalid context");
        return nullptr;
    }

    napi_value object = JsFenceExtensionContext::CreateJsFenceExtensionContext(env, ptr);
    auto napiContextObj =
        AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env, CONTEXT_MODULE_PATH, &object, LOAD_SYSTEM_MODULE_ARGC)
            ->GetNapiValue();

    napi_coerce_to_native_binding_object(
        env, napiContextObj, AbilityRuntime::DetachCallbackFunc, AttachFenceExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<FenceExtensionContext>(ptr);
    if (workContext == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "invalid extension context");
        return nullptr;
    }
    auto retStatus = ::napi_wrap(
        env,
        napiContextObj,
        workContext,
        [](napi_env, void *data, void *) {
            LBSLOGI(FENCE_EXTENSION, "Finalizer for weak_ptr extension context is called");
            delete static_cast<std::weak_ptr<FenceExtensionContext> *>(data);
        },
        nullptr,
        nullptr);
    if (retStatus != ::napi_status::napi_ok) {
        LBSLOGE(FENCE_EXTENSION, "Napi wrap context error");
        return nullptr;
    }
    LBSLOGI(FENCE_EXTENSION, "AttachFenceExtensionContext end");
    return napiContextObj;
}

JsFenceExtension *JsFenceExtension::Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    return new (std::nothrow) JsFenceExtension(static_cast<AbilityRuntime::JsRuntime &>(*runtime));
}

JsFenceExtension::JsFenceExtension(AbilityRuntime::JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{}

JsFenceExtension::~JsFenceExtension()
{
    LBSLOGD(FENCE_EXTENSION, "Js extension destructor");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsFenceExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    FenceExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_->srcEntrance.empty()) {
        LBSLOGE(FENCE_EXTENSION, "srcEntrance of abilityInfo is empty");
        return;
    }
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        LBSLOGE(FENCE_EXTENSION, "Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    LBSLOGW(FENCE_EXTENSION,
        "JsFenceExtension::Init module:%{public}s,srcPath:%{public}s",
        moduleName.c_str(),
        srcPath.c_str());
    AbilityRuntime::HandleScope handleScope(jsRuntime_);

    jsObj_ = jsRuntime_.LoadModule(moduleName,
        srcPath,
        abilityInfo_->hapPath,
        abilityInfo_->compileMode == CompileMode::ES_MODULE,
        false,
        abilityInfo_->srcEntrance);
    if (jsObj_ == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "Failed to load ability module");
        return;
    }
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "Failed to get extension object");
        return;
    }
    napi_env env = jsRuntime_.GetNapiEnv();

    LBSLOGI(FENCE_EXTENSION, "Init end");
    BindContext(env, obj);
}

void JsFenceExtension::BindContext(const ::napi_env &env, const ::napi_value &obj)
{
    LBSLOGI(FENCE_EXTENSION, "BindContext start");
    auto context = GetContext();
    if (context == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "Failed to get context");
        return;
    }
    napi_value contextObj = JsFenceExtensionContext::CreateJsFenceExtensionContext(env, context);
    auto shellContextRef_ =
        AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env, CONTEXT_MODULE_PATH, &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "Failed to get shell context from system module");
        return;
    }
    napi_value nativeObj = shellContextRef_->GetNapiValue();
    if (nativeObj == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "Failed to load context module");
        return;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<FenceExtensionContext>(context);
    if (workContext == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "invalid extension context");
        return;
    }
    napi_coerce_to_native_binding_object(
        env, nativeObj, AbilityRuntime::DetachCallbackFunc, AttachFenceExtensionContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.release());
    napi_set_named_property(env, obj, "context", nativeObj);

    napi_wrap(
        env,
        nativeObj,
        workContext,
        [](napi_env, void *data, void *) {
            LBSLOGI(FENCE_EXTENSION, "Finalizer for weak_ptr extension context is called");
            delete static_cast<std::weak_ptr<FenceExtensionContext> *>(data);
        },
        nullptr,
        nullptr);
    LBSLOGI(FENCE_EXTENSION, "BindContext end");
}
void JsFenceExtension::OnStart(const AAFwk::Want &want)
{
    AbilityRuntime::Extension::OnStart(want);
}

void JsFenceExtension::OnStop()
{
    LBSLOGI(FENCE_EXTENSION, "OnStop called");
    AbilityRuntime::HandleScope handleScope(jsRuntime_);
    if (jsObj_ == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "js fence extension obj is null");
        return;
    }
    ::napi_value method = GetMethod(jsRuntime_, jsObj_, "onDestroy");
    if (method == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "Call function method is null");
        return;
    }
    napi_value argv[0];
    ::napi_value undefined;
    ::napi_env env = jsRuntime_.GetNapiEnv();
    ::napi_value value = jsObj_->GetNapiValue();
    ::napi_status retStatus = ::napi_call_function(env, value, method, 0, argv, &undefined);
    if (retStatus != ::napi_status::napi_ok) {
        LBSLOGE(FENCE_EXTENSION, "Call function error");
    }
    LBSLOGI(FENCE_EXTENSION, "Call onDestroy end");
    AbilityRuntime::Extension::OnStop();
}

sptr<IRemoteObject> JsFenceExtension::OnConnect(const AAFwk::Want &want)
{
    LBSLOGI(FENCE_EXTENSION, "OnConnect");
    AbilityRuntime::Extension::OnConnect(want);
    sptr<FenceExtensionStubImpl> remoteObject =
        new (std::nothrow) FenceExtensionStubImpl(std::static_pointer_cast<JsFenceExtension>(shared_from_this()));
    if (remoteObject == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "failed to create FenceExtensionStubImpl");
        return nullptr;
    }
    return remoteObject->AsObject();
}

void JsFenceExtension::OnDisconnect(const AAFwk::Want &want)
{
    LBSLOGI(FENCE_EXTENSION, "OnDisconnect");
    Extension::OnDisconnect(want);
}

FenceExtensionErrCode JsFenceExtension::OnFenceStatusChange(std::map<std::string, std::string> extraData)
{
    LBSLOGI(FENCE_EXTENSION, "js fence extension:OnFenceStatusChange");
    if (jsObj_ == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "js fence extension obj is null");
        return FenceExtensionErrCode::EXTENSION_JS_OBJ_IS_NULL;
    }

    auto task = [=]() {
        LBSLOGI(FENCE_EXTENSION, "call js function start");
        JsFenceExtension::CallToUiThread(extraData);
    };
    if (handler_ == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "PostTask call js function start");
        return FenceExtensionErrCode::EXTENSION_JS_CALL_FAILED;
    }
    handler_->PostTask(task, "FenceExtension OnFenceStatusChange Task");
    LBSLOGI(FENCE_EXTENSION, "PostTask call js function start");
    return FenceExtensionErrCode::EXTENSION_SUCCESS;
}

void JsFenceExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName);
        srcPath.append("/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

FenceExtensionErrCode JsFenceExtension::CallToUiThread(std::map<std::string, std::string> extraData)
{
    LBSLOGI(FENCE_EXTENSION, "js fence extension:OnFenceStatusChange");
    if (jsObj_ == nullptr) {
        return FenceExtensionErrCode::EXTENSION_JS_OBJ_IS_NULL;
    }
    AbilityRuntime::HandleScope handleScope(jsRuntime_);
    ::napi_value method = GetMethod(jsRuntime_, jsObj_, "onFenceStatusChange");
    if (method == nullptr) {
        return FenceExtensionErrCode::EXTENSION_JS_NOT_FOUND_METHOD;
    }
    auto fenceId = std::atoi(GetAndDeleteFromMap(extraData, EXTENSION_PARAM_KEY_FENCE_ID).c_str());
    auto fenceEvent = std::atoi(GetAndDeleteFromMap(extraData, EXTENSION_PARAM_KEY_FENCE_EVENT).c_str());
    ::napi_value transitionObj;
    ::napi_env env = jsRuntime_.GetNapiEnv();
    napi_status retTransition = ::napi_create_object(env, &transitionObj);
    if (retTransition != napi_ok) {
        return FenceExtensionErrCode::EXTENSION_JS_CREATE_PARAM_ERROR;
    }
    SetValueInt32(env, "geofenceId", fenceId, transitionObj);
    SetValueInt32(env, "transitionEvent", static_cast<int>(fenceEvent), transitionObj);
    ::napi_value addtionsRecord;
    if (extraData.size() > 0) {
        ::napi_status status = napi_create_object(env, &addtionsRecord);
        for (const auto &pair : extraData) {
            napi_value key, value;
            status = napi_create_string_utf8(env, pair.first.c_str(), NAPI_AUTO_LENGTH, &key);
            if (status != napi_ok) {
                break;
            }
            status = napi_create_string_utf8(env, pair.second.c_str(), NAPI_AUTO_LENGTH, &value);
            if (status != napi_ok) {
                break;
            }
            status = napi_set_property(env, addtionsRecord, key, value);
            if (status != napi_ok) {
                break;
            }
        }
    } else {
        ::napi_status status = napi_get_undefined(env, &addtionsRecord);
    }
    ::napi_value argv[PARAM2];
    argv[PARAM0] = transitionObj;
    argv[PARAM1] = addtionsRecord;
    ::napi_value abilityObj = jsObj_->GetNapiValue();
    ::napi_value undefined;
    ::napi_status callStatus = ::napi_call_function(env, abilityObj, method, ARGC_TWO, argv, &undefined);
    return FenceExtensionErrCode::EXTENSION_SUCCESS;
}

napi_status JsFenceExtension::SetValueUtf8String(
    const napi_env &env, const char *fieldStr, const char *str, napi_value &result)
{
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &value), napi_generic_failure);
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}
napi_status JsFenceExtension::SetValueInt32(
    const napi_env &env, const char *fieldStr, const int intValue, napi_value &result)
{
    napi_value value = nullptr;
    NAPI_CALL_BASE(env, napi_create_int32(env, intValue, &value), napi_generic_failure);
    NAPI_CALL_BASE(env, napi_set_named_property(env, result, fieldStr, value), napi_generic_failure);
    return napi_ok;
}
std::string JsFenceExtension::GetAndDeleteFromMap(std::map<std::string, std::string> &param, std::string key)
{
    auto mapItr = param.find(key);
    std::string value;
    if (mapItr != param.end()) {
        value = mapItr->second;
        param.erase(key);
    }
    return value;
}

::napi_value JsFenceExtension::GetMethod(
    AbilityRuntime::JsRuntime &jsRuntime, const std::unique_ptr<NativeReference> &jsObj, const std::string &name)
{
    if (!jsObj) {
        LBSLOGE(FENCE_EXTENSION, "Not found Extension.js");
        return nullptr;
    }

    ::napi_value obj = jsObj->GetNapiValue();
    if (obj == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "Failed to get Extension object");
        return nullptr;
    }

    ::napi_env env = jsRuntime.GetNapiEnv();
    ::napi_value method;
    ::napi_status ret = ::napi_get_named_property(env, obj, name.c_str(), &method);
    if (ret != ::napi_status::napi_ok) {
        LBSLOGE(FENCE_EXTENSION, "napi get name fail(%{public}d)", ret);
        return nullptr;
    }
    ::napi_valuetype type = ::napi_valuetype::napi_undefined;
    ::napi_valuetype functionType = ::napi_valuetype::napi_function;
    ::napi_status retType = ::napi_typeof(env, method, &functionType);
    if (retType != ::napi_status::napi_ok) {
        LBSLOGE(FENCE_EXTENSION, "Parse napi object type fail(%{public}d)", retType);
        return nullptr;
    }
    return method;
}
}  // namespace Location
}  // namespace OHOS