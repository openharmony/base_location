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

#include "locationconfigmanager_fuzzer.h"
#include "location_config_manager.h"

namespace OHOS {
    using namespace OHOS::Location;

    bool LocationConfigManagerFuzzerTest(const uint8_t* data, size_t size)
    {
        LocationConfigManager::GetInstance().Init();
        LocationConfigManager::GetInstance().GetLocationSwitchState();
        int state = *reinterpret_cast<const int*>(data);
        LocationConfigManager::GetInstance().SetLocationSwitchState(state);
        std::string fileName((const char*) data, size);
        LocationConfigManager::GetInstance().IsExistFile(fileName);
        std::string fileData((const char*) data, size);
        LocationConfigManager::GetInstance().CreateFile(fileName, fileData);
        int type = *reinterpret_cast<const int*>(data);
        LocationConfigManager::GetInstance().GetPrivacyTypeConfigPath(type);
        LocationConfigManager::GetInstance().GetPrivacyTypeState(type);
        LocationConfigManager::GetInstance().SetPrivacyTypeState(type, true);
        LocationConfigManager::GetInstance().SetPrivacyTypeState(type, false);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::LocationConfigManagerFuzzerTest(data, size);
    return 0;
}