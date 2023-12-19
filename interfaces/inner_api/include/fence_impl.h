/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef FENCE_IMPL_H
#define FENCE_IMPL_H

#include <singleton.h>
#include "want_agent_helper.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
class FenceImpl : DelayedSingleton<FenceImpl> {
public:
    LocationErrCode AddFenceExt(
        std::unique_ptr<GeofenceRequest> &request, const AbilityRuntime::WantAgent::WantAgent wantAgent);
    LocationErrCode RemoveFenceExt(
        std::unique_ptr<GeofenceRequest> &request, const AbilityRuntime::WantAgent::WantAgent wantAgent);
private:
    bool InitLocationExt();
};
}  // namespace Location
}  // namespace OHOS
#endif  // FENCE_IMPL_H