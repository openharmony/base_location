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

#ifndef OHOS_ABILITY_RUNTIME_FENCE_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_FENCE_EXTENSION_H

#include "extension_base.h"
#include "fence_extension_context.h"
#include "constant_definition.h"
#include <map>

namespace OHOS {
namespace Location {
using namespace AbilityRuntime;

class FenceExtensionContext;

class FenceExtension;

using CreatorFunc = std::function<FenceExtension *(const std::unique_ptr<Runtime> &runtime)>;

/**
 * @brief Basic vpn components.
 */
class FenceExtension : public ExtensionBase<FenceExtensionContext> {
public:
    FenceExtension() = default;

    virtual ~FenceExtension() = default;

    /**
     * @brief Create and init fence context.
     *
     * @param record the fence extension record.
     * @param application the application info.
     * @param handler the fence extension handler.
     * @param token the remote token.
     * @return The created fence context.
     */
    virtual std::shared_ptr<FenceExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord> &record, const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

    /**
     * @brief Init the fence extension.
     *
     * @param record the fence extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Create Extension.
     *
     * @param runtime The runtime.
     * @return The VpnExtension instance.
     */
    static FenceExtension *Create(const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime);

    /**
     * @brief Set a creator function.
     *
     * @param creator The function for create a vpn-extension ability.
     */
    static void SetCreator(const CreatorFunc &creator);

    /**
     * @brief called back when geofence status is change.
     *
     * @param fenceId enter fence id
     * @param fenceType enter fence type
     * @param extraData other extra data
     */
    FenceExtensionErrCode OnFenceStatusChange(std::map<std::string, std::string> extraData);

    /**
     * @brief destroy this extension
     */
    void OnDestroy();
};
}  // namespace Location
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_FENCE_EXTENSION_H
