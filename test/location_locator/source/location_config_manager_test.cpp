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

#include "location_config_manager_test.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void LocationConfigManagerTest::SetUp()
{
}

void LocationConfigManagerTest::TearDown()
{
}

HWTEST_F(LocationConfigManagerTest, Test001, TestSize.Level1)
{
    LocationConfigManager::GetInstance().SetLocationSwitchState(modeValue);
    LocationConfigManager::GetInstance().GetLocationSwitchState();
    LocationConfigManager::GetInstance().IsExistFile(filename);
    LocationConfigManager::GetInstance().CreateFile(filename, filedata);
    LocationConfigManager::GetInstance().GetPrivacyTypeConfigPath(type);
    LocationConfigManager::GetInstance().GetPrivacyTypeState(type);
    LocationConfigManager::GetInstance().SetPrivacyTypeState(type, isConfirmed);
}


}  // namespace Location
}  // namespace OHOS