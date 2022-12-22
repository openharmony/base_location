/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "fusioncontroller_fuzzer.h"

#include "fusion_controller.h"
#include "location.h"

namespace OHOS {
    using namespace OHOS::Location;
    bool FusionControllerFuzzerTest(const uint8_t* data, size_t size)
    {
        if (size == 0) {
            return true;
        }
        std::shared_ptr<FusionController> fusionController =
            std::make_shared<FusionController>();
        int index = 0;
        fusionController->ActiveFusionStrategies(data[index++]);
        std::string abilityName((const char*) data, size);
        fusionController->Process(abilityName);
        auto location = std::make_unique<OHOS::Location::Location>();
        fusionController->FuseResult(abilityName, location);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FusionControllerFuzzerTest(data, size);
    return 0;
}