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

#include "request_config.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
RequestConfig::RequestConfig()
{
    scenario_ = SCENE_UNSET;
    priority_ = PRIORITY_UNSET;
    timeInterval_ = 0; // no time interval limit for reporting location
    distanceInterval_ = 0; // no distance interval limit for reporting location
    maxAccuracy_ = 0.0; // no accuracy limit for reporting location
    fixNumber_ = 0; // no fix size limit for reporting location
}

RequestConfig::RequestConfig(const int scenario) : scenario_(scenario)
{
    priority_ = PRIORITY_UNSET;
    timeInterval_ = 0; // no time interval limit for reporting location
    distanceInterval_ = 0; // no distance interval limit for reporting location
    maxAccuracy_ = 0.0; // no accuracy limit for reporting location
    fixNumber_ = 0; // no fix size limit for reporting location
}

void RequestConfig::Set(RequestConfig& requestConfig)
{
    scenario_ = requestConfig.GetScenario();
    priority_ = requestConfig.GetPriority();
    timeInterval_ = requestConfig.GetTimeInterval();
    distanceInterval_ = requestConfig.GetDistanceInterval();
    maxAccuracy_ = requestConfig.GetMaxAccuracy();
    fixNumber_ = requestConfig.GetFixNumber();
}

bool RequestConfig::IsSame(RequestConfig& requestConfig)
{
    if (scenario_ != requestConfig.GetScenario()) {
        return false;
    }
    if (scenario_ != SCENE_UNSET) {
        return true;
    }
    return priority_ == requestConfig.GetPriority();
}

void RequestConfig::ReadFromParcel(Parcel& parcel)
{
    scenario_ = parcel.ReadInt32();
    priority_ = parcel.ReadInt32();
    timeInterval_ = parcel.ReadInt32();
    distanceInterval_ = parcel.ReadInt32();
    maxAccuracy_ = parcel.ReadFloat();
    fixNumber_ = parcel.ReadInt32();
}

std::unique_ptr<RequestConfig> RequestConfig::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    requestConfig->ReadFromParcel(parcel);
    return requestConfig;
}

bool RequestConfig::Marshalling(Parcel& parcel) const
{
    return parcel.WriteInt32(scenario_) &&
           parcel.WriteInt32(priority_) &&
           parcel.WriteInt32(timeInterval_) &&
           parcel.WriteInt32(distanceInterval_) &&
           parcel.WriteFloat(maxAccuracy_) &&
           parcel.WriteInt32(fixNumber_);
}

std::string RequestConfig::ToString() const
{
    std::stringstream str;
    str << "scenario : ";
    str << scenario_;
    str << ", location priority : ";
    str << priority_;
    str << ", timeInterval : ";
    str << timeInterval_;
    str << ", distanceInterval : ";
    str << distanceInterval_;
    str << ", maxAccuracy : ";
    str << maxAccuracy_;
    str << ", fixNumber : ";
    str << fixNumber_;
    return str.str();
}
} // Location
} // OHOS