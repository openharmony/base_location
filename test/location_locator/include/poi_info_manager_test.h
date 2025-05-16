/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#ifndef POI_INFO_MANAGER_TEST_H
#define POI_INFO_MANAGER_TEST_H
 
#include <gtest/gtest.h>
 
#include "poi_info_manager.h"
 
namespace OHOS {
namespace Location {
class PoiInfoManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    PoiInfoManager* poiInfoManager_;
    std::unique_ptr<Location> MockLocation();
    std::string MockPoiString(uint64_t delayTimeMilSec);
};
}
}
#endif // POI_INFO_MANAGER_TEST_H