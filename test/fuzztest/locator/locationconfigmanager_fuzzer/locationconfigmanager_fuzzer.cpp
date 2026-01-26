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
    const int MIN_DATA_LEN = 7;
    bool LocationConfigManagerFuzzerTest(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        int index = 0;
        LocationConfigManager::GetInstance()->Init();
        LocationConfigManager::GetInstance()->GetLocationSwitchState();
        LocationConfigManager::GetInstance()->SetLocationSwitchState(data[index++]);
        std::string fileName(reinterpret_cast<const char*>(data + index), size - index);
        index++;
        LocationConfigManager::GetInstance()->IsExistFile(fileName);
        std::string fileData(reinterpret_cast<const char*>(data + index), size - index);
        index++;
        LocationConfigManager::GetInstance()->CreateFile(fileName, fileData);
        LocationConfigManager::GetInstance()->GetPrivacyTypeConfigPath(data[index++]);
        bool isConfirmed = false;
        LocationConfigManager::GetInstance()->GetPrivacyTypeState(data[index++], isConfirmed);
        LocationConfigManager::GetInstance()->SetPrivacyTypeState(data[index++], true);
        LocationConfigManager::GetInstance()->SetPrivacyTypeState(data[index++], false);
        std::string name;
        LocationConfigManager::GetInstance()->GetSettingsBundleName(name);
        LocationConfigManager::GetInstance()->GetNlpServiceName(name);
        LocationConfigManager::GetInstance()->GetNlpAbilityName(name);
        LocationConfigManager::GetInstance()->GetSuplMode();
        LocationConfigManager::GetInstance()->GetAgnssServerAddr(name);
        LocationConfigManager::GetInstance()->GetAgnssServerPort();
        LocationConfigManager::GetInstance()->GetGnssEnableState();
        LocationConfigManager::GetInstance()->SetCachePrivacyType(0);
        LocationConfigManager::GetInstance()->GenerateStartCommand();
        LocationConfigManager::GetInstance()->OpenPrivacyDialog();
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