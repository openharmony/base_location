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

#include "location.h"

#include <parcel.h>
#include <string>
#include "string_ex.h"

namespace OHOS {
namespace Location {
static constexpr double MIN_LATITUDE = -90.0;
static constexpr double MIN_LONGITUDE = -180.0;
const int MAX_ADDITION_SIZE = 100;

Location::Location()
{
    latitude_ = MIN_LATITUDE - 1;
    longitude_ = MIN_LONGITUDE - 1;
    altitude_ = 0.0;
    accuracy_ = 0.0;
    speed_ = 0.0;
    direction_ = 0.0;
    timeStamp_ = 0;
    timeSinceBoot_ = 0;
    isFromMock_ = false;
    sourceType_ = 0;
    floorNo_ = 0;
    floorAccuracy_ = 0.0;
    additionSize_ = 0;
}

Location::Location(Location& location)
{
    latitude_ = location.GetLatitude();
    longitude_ = location.GetLongitude();
    altitude_ = location.GetAltitude();
    accuracy_ = location.GetAccuracy();
    speed_ = location.GetSpeed();
    direction_ = location.GetDirection();
    timeStamp_ = location.GetTimeStamp();
    timeSinceBoot_ = location.GetTimeSinceBoot();
    isFromMock_ = location.GetIsFromMock();
    sourceType_ = location.GetSourceType();
    floorNo_ = location.GetFloorNo();
    floorAccuracy_ = location.GetFloorAccuracy();
    additions_ = location.GetAdditions();
    additionSize_ = location.GetAdditionSize();
}

void Location::ReadFromParcel(Parcel& parcel)
{
    latitude_ = parcel.ReadDouble();
    longitude_ = parcel.ReadDouble();
    altitude_ = parcel.ReadDouble();
    accuracy_ = parcel.ReadDouble();
    speed_ = parcel.ReadDouble();
    direction_ = parcel.ReadDouble();
    timeStamp_ = parcel.ReadInt64();
    timeSinceBoot_ = parcel.ReadInt64();
    additionSize_ = parcel.ReadInt64();
    int64_t len = additionSize_ > MAX_ADDITION_SIZE ? MAX_ADDITION_SIZE : additionSize_;
    for (int64_t i = 0; i < len; i++) {
        additions_.push_back(Str16ToStr8(parcel.ReadString16()));
    }
    isFromMock_ = parcel.ReadBool();
    sourceType_ = parcel.ReadInt32();
}

void Location::ReadFromParcelV9(Parcel& parcel)
{
    latitude_ = parcel.ReadDouble();
    longitude_ = parcel.ReadDouble();
    altitude_ = parcel.ReadDouble();
    accuracy_ = parcel.ReadDouble();
    speed_ = parcel.ReadDouble();
    direction_ = parcel.ReadDouble();
    timeStamp_ = parcel.ReadInt64();
    timeSinceBoot_ = parcel.ReadInt64();
    additions_.push_back(Str16ToStr8(parcel.ReadString16()));
    additionSize_ = parcel.ReadInt64() >= 1 ? 1 : 0;
    isFromMock_ = parcel.ReadBool();
    sourceType_ = parcel.ReadInt32();
    floorNo_ = parcel.ReadInt32();
    floorAccuracy_ = parcel.ReadDouble();
}

std::shared_ptr<Location> Location::UnmarshallingShared(Parcel& parcel)
{
    std::shared_ptr<Location> location = std::make_shared<Location>();
    location->ReadFromParcel(parcel);
    return location;
}

std::shared_ptr<Location> Location::UnmarshallingSharedV9(Parcel& parcel)
{
    std::shared_ptr<Location> location = std::make_shared<Location>();
    location->ReadFromParcelV9(parcel);
    return location;
}

std::unique_ptr<Location> Location::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    return location;
}

std::unique_ptr<Location> Location::UnmarshallingV9(Parcel& parcel)
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcelV9(parcel);
    return location;
}

bool Location::Marshalling(Parcel& parcel) const
{
    parcel.WriteDouble(latitude_);
    parcel.WriteDouble(longitude_);
    parcel.WriteDouble(altitude_);
    parcel.WriteDouble(accuracy_);
    parcel.WriteDouble(speed_);
    parcel.WriteDouble(direction_);
    parcel.WriteInt64(timeStamp_);
    parcel.WriteInt64(timeSinceBoot_);
    additionSize_ = (additionSize_ > MAX_ADDITION_SIZE) ? MAX_ADDITION_SIZE : additionSize_;
    parcel.WriteInt64(additionSize_);
    for (int64_t i = 0; i < additionSize_; i++) {
        parcel.WriteString16(Str8ToStr16(additions_[i]));
    }
    parcel.WriteBool(isFromMock_);
    parcel.WriteInt32(sourceType_);
    return true;
}

bool Location::MarshallingV9(Parcel& parcel)
{
    std::string addition = additionSize_ > 0 ? additions_[0] : "";
    return parcel.WriteDouble(latitude_) &&
           parcel.WriteDouble(longitude_) &&
           parcel.WriteDouble(altitude_) &&
           parcel.WriteDouble(accuracy_) &&
           parcel.WriteDouble(speed_) &&
           parcel.WriteDouble(direction_) &&
           parcel.WriteInt64(timeStamp_) &&
           parcel.WriteInt64(timeSinceBoot_) &&
           parcel.WriteString16(Str8ToStr16(addition)) &&
           parcel.WriteInt64(additionSize_) &&
           parcel.WriteBool(isFromMock_) &&
           parcel.WriteInt32(sourceType_) &&
           parcel.WriteInt32(floorNo_) &&
           parcel.WriteDouble(floorAccuracy_);
}

std::string Location::ToString() const
{
    std::string str = "latitude : " + std::to_string(latitude_) +
        ", longitude : " + std::to_string(longitude_) +
        ", altitude : " + std::to_string(altitude_) +
        ", accuracy : " + std::to_string(accuracy_) +
        ", speed : " + std::to_string(speed_) +
        ", direction : " + std::to_string(direction_) +
        ", timeStamp : " + std::to_string(timeStamp_) +
        ", timeSinceBoot : " + std::to_string(timeSinceBoot_) +
        ", isFromMock : " + std::to_string(isFromMock_) +
        ", sourceType : " + std::to_string(sourceType_) +
        ", additionSize : " + std::to_string(additionSize_) +
        ", additions : [ ";
    int64_t len = (additionSize_ > MAX_ADDITION_SIZE) ? MAX_ADDITION_SIZE : additionSize_;
    for (int64_t i = 0; i < len; i++) {
        str = str + additions_[i] + " ";
    }
    str = str + "]";
    return str;
}
} // namespace Location
} // namespace OHOS