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

#ifndef LOCATION_MOCK_CONFIG_H
#define LOCATION_MOCK_CONFIG_H

#include <parcel.h>
#include <string>

#include "location_mock_config.h"

namespace OHOS {
namespace Location {
class LocationMockConfig : public Parcelable {
public:
    LocationMockConfig();
    explicit LocationMockConfig(const int scenario);
    virtual ~LocationMockConfig() = default;

    inline int GetTimeInterval() const
    {
        return timeInterval_;
    }

    inline void SetTimeInterval(int timeInterval)
    {
        timeInterval_ = timeInterval;
    }

    inline int GetScenario() const
    {
        return scenario_;
    }

    inline void SetScenario(int scenario)
    {
        scenario_ = scenario;
    }

    void ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static std::unique_ptr<LocationMockConfig> Unmarshalling(Parcel& parcel);
    void Set(LocationMockConfig& requestConfig);
    bool IsSame(LocationMockConfig& requestConfig);

private:
    int scenario_;
    int timeInterval_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_MOCK_CONFIG_H
