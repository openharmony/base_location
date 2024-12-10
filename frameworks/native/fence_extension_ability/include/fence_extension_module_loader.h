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

#ifndef FENCEEXTENSION_FENCE_EXTENSION_MODULE_LOADER_H
#define FENCEEXTENSION_FENCE_EXTENSION_MODULE_LOADER_H
#include <map>
#include "extension_module_loader.h"

namespace OHOS {
namespace Location {
class FenceExtensionModuleLoader : public AbilityRuntime::ExtensionModuleLoader,
                                   public Singleton<FenceExtensionModuleLoader> {
    DECLARE_SINGLETON(FenceExtensionModuleLoader);

public:
    /**
     * @brief Create fence extension.
     *
     * @param runtime The runtime.
     * @return The fence extension instance.
     */
    virtual AbilityRuntime::Extension *Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime) const override;

    /**
     * @brief Get fence extension info.
     *
     * @return The fence extension info.
     */
    virtual std::map<std::string, std::string> GetParams() override;
};
}  // namespace Location
}  // namespace OHOS

#endif  // FENCEEXTENSION_FENCE_EXTENSION_MODULE_LOADER_H
