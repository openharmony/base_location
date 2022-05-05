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

#ifndef FUSION_CONTROLLER_H
#define FUSION_CONTROLLER_H

#include <cstdint>
#include <singleton.h>

#include "location.h"

namespace OHOS {
namespace Location {
class FusionController : public DelayedSingleton<FusionController> {
public:
    void ActiveFusionStrategies(int type);
    void Process(std::string abilityName);
    void FuseResult(std::string abilityName, const std::unique_ptr<Location>& location);
private:
    void RequestQuickFix(bool state);

    uint32_t fusedFlag_ = 0;
    bool needReset_ = true;
};
} // namespace Location
} // namespace OHOS
#endif // FUSION_CONTROLLER_H