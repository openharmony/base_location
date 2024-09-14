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

#ifndef OHOS_FENCEEXTENSION_FENCE_EXTENSION_STUB_IMPL_H
#define OHOS_FENCEEXTENSION_FENCE_EXTENSION_STUB_IMPL_H

#include <memory>
#include "js_fence_extension.h"
#include "fence_extension_stub.h"
#include "constant_definition.h"
#include <map>

namespace OHOS {
namespace Location {
class FenceExtensionStubImpl : public FenceExtensionStub {
public:
    explicit FenceExtensionStubImpl(const std::shared_ptr<JsFenceExtension> &extension);

    virtual ~FenceExtensionStubImpl();

    virtual FenceExtensionErrCode OnFenceStatusChange(std::map<std::string, std::string> extraData) override;

private:
    std::weak_ptr<JsFenceExtension> extension_;
};
}  // namespace Location
}  // namespace OHOS
#endif  // OHOS_FENCEEXTENSION_FENCE_EXTENSION_STUB_IMPL_H
