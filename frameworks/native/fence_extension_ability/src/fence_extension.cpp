/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "fence_extension.h"
#include "js_fence_extension.h"
#include "location_log.h"
#include "runtime.h"

namespace OHOS {
namespace Location {
using namespace OHOS::AppExecFwk;
FenceExtension *FenceExtension::Create(const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    if (!runtime) {
        return new FenceExtension();
    }

    LBSLOGI(FENCE_EXTENSION, "FenceExtension:Create");
    switch (runtime->GetLanguage()) {
        case OHOS::AbilityRuntime::Runtime::Language::JS:
            return JsFenceExtension::Create(runtime);
        default:
            return new FenceExtension();
    }
}

void FenceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<FenceExtensionContext>::Init(record, application, handler, token);
    LBSLOGI(FENCE_EXTENSION, "FenceExtension:Init");
}

std::shared_ptr<FenceExtensionContext> FenceExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record, const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    std::shared_ptr<FenceExtensionContext> context =
        ExtensionBase<FenceExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        LBSLOGE(FENCE_EXTENSION, "FenceExtension:context is null");
        return context;
    }
    LBSLOGI(FENCE_EXTENSION, "FenceExtension:CreateAndInitContext");
    return context;
}

FenceExtensionErrCode FenceExtension::OnFenceStatusChange(std::map<std::string, std::string> extraData)
{
    LBSLOGI(FENCE_EXTENSION, "FenceExtension:OnFenceStatusChange");
    return FenceExtensionErrCode::EXTENSION_SUCCESS;
}

void FenceExtension::OnDestroy()
{
    LBSLOGI(FENCE_EXTENSION, "FenceExtension:OnDestroy");
}
}  // namespace Location
}  // namespace OHOS