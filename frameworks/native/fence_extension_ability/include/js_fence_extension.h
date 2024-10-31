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

#ifndef FENCEEXTENSION_JS_FENCE_EXTENSION_H
#define FENCEEXTENSION_JS_FENCE_EXTENSION_H

#include "async_context.h"
#include "js_runtime.h"
#include "napi/native_api.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "runtime.h"
#include "fence_extension.h"
#include <map>

namespace OHOS {
namespace Location {
class JsFenceExtension : public FenceExtension {
public:
    explicit JsFenceExtension(AbilityRuntime::JsRuntime &jsRuntime);

    virtual ~JsFenceExtension() override;

    /**
     * @brief Create JsFenceExtension.
     *
     * @param runtime The runtime.
     * @return The JsFenceExtension instance.
     */
    static JsFenceExtension *Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this remoteLocation extension is started. You must override this function if you want to
     * perform some initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this remoteLocation extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Notification
     * extension.
     * @return Returns a pointer to the <b>sid</b> of the connected remoteLocation extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this remoteLocation extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when this remoteLocation extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop() override;

    /**
     * @brief called back when geofence status is change.
     *
     * @param fenceId enter fence id
     * @param fenceType enter fence type
     * @param extraData other extra data
     */
    FenceExtensionErrCode OnFenceStatusChange(std::map<std::string, std::string> extraData);

    void OnDestroy();

private:
    AbilityRuntime::JsRuntime &jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_ = nullptr;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
    /**
     * @brief BindContext
     * @param [IN] &env napi_env
     * @param [IN] &obj napi_value
     */
    void BindContext(const napi_env &env, const napi_value &obj);
    /**
     * @brief get extension src
     * @param [IN] &srcPath
     */
    void GetSrcPath(std::string &srcPath);
    std::string GetAndDeleteFromMap(std::map<std::string, std::string> &param, std::string key);
    ::napi_value GetMethod(
        AbilityRuntime::JsRuntime &jsRuntime, const std::unique_ptr<NativeReference> &jsObj, const std::string &name);
    napi_status SetValueUtf8String(const napi_env &env, const char *fieldStr, const char *str, napi_value &result);
    napi_status SetValueInt32(const napi_env &env, const char *fieldStr, const int intValue, napi_value &result);
    FenceExtensionErrCode CallToUiThread(std::map<std::string, std::string> extraData);
};
}  // namespace Location
}  // namespace OHOS

#endif  // FENCEEXTENSION_JS_FENCE_EXTENSION_H