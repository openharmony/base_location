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

#include "location_mock_config.h"

#include <parcel.h>

#include "constant_definition.h"

namespace OHOS {
namespace Location {
LocationMockConfig::LocationMockConfig()
{
    scenario_ = SCENE_UNSET;
    timeInterval_ = 0;
}

LocationMockConfig::LocationMockConfig(const int scenario) : scenario_(scenario)
{
    timeInterval_ = 0;
}

void LocationMockConfig::Set(LocationMockConfig& mockConfig)
{
    scenario_ = mockConfig.GetScenario();
    timeInterval_ = mockConfig.GetTimeInterval();
}

bool LocationMockConfig::IsSame(LocationMockConfig& mockConfig)
{
    if (scenario_ != mockConfig.GetScenario()) {
        return false;
    }
    return true;
}

void LocationMockConfig::ReadFromParcel(Parcel& parcel)
{
    scenario_ = parcel.ReadInt32();
    timeInterval_ = parcel.ReadInt32();
}

std::unique_ptr<LocationMockConfig> LocationMockConfig::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<LocationMockConfig> mockConfig = std::make_unique<LocationMockConfig>();
    mockConfig->ReadFromParcel(parcel);
    return mockConfig;
}

bool LocationMockConfig::Marshalling(Parcel& parcel) const
{
    return parcel.WriteInt32(scenario_) &&
           parcel.WriteInt32(timeInterval_);
}
} // namespace Location
} // namespace OHOS