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

#include "fence_extension_stub_impl.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
FenceExtensionStubImpl::FenceExtensionStubImpl(const std::shared_ptr<JsFenceExtension> &extension)
    : extension_(extension)
{}

FenceExtensionStubImpl::~FenceExtensionStubImpl()
{}

FenceExtensionErrCode FenceExtensionStubImpl::OnFenceStatusChange(std::map<std::string, std::string> extraData)
{
    LBSLOGI(FENCE_EXTENSION, "stub impl OnFenceStatusChange");
    auto extension = extension_.lock();
    if (extension != nullptr) {
        extension->OnFenceStatusChange(extraData);
    } else {
        return FenceExtensionErrCode::EXTENSION_REMOTE_STUB_IS_NULL;
    }
    LBSLOGI(FENCE_EXTENSION, "stub impl end");
    return FenceExtensionErrCode::EXTENSION_SUCCESS;
}
}  // namespace Location
}  // namespace OHOS