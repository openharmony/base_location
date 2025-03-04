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
    additionSize_ = 0;
    isFromMock_ = false;
    isSystemApp_ = 0;
    floorNo_ = 0;
    floorAccuracy_ = 0.0;
    altitudeAccuracy_ = 0.0;
    speedAccuracy_ = 0.0;
    directionAccuracy_ = 0.0;
    uncertaintyOfTimeSinceBoot_ = 0;
    locationSourceType_ = 0;
    uuid_ = "";
    fieldValidity_ = 0;
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
    floorNo_ = location.GetFloorNo();
    floorAccuracy_ = location.GetFloorAccuracy();
    additions_ = location.GetAdditions();
    additionsMap_ = location.GetAdditionsMap();
    additionSize_ = location.GetAdditionSize();
    isFromMock_ = location.GetIsFromMock();
    isSystemApp_ = location.GetIsSystemApp();
    altitudeAccuracy_ = location.GetAltitudeAccuracy();
    speedAccuracy_ = location.GetSpeedAccuracy();
    directionAccuracy_ = location.GetDirectionAccuracy();
    uncertaintyOfTimeSinceBoot_ = location.GetUncertaintyOfTimeSinceBoot();
    locationSourceType_ = location.GetLocationSourceType();
    uuid_ = location.GetUuid();
    fieldValidity_ = location.GetFieldValidity();
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
    std::vector<std::u16string> additions;
    parcel.ReadString16Vector(&additions);
    isFromMock_ = parcel.ReadBool();
    isSystemApp_ = parcel.ReadInt32();
    altitudeAccuracy_ = parcel.ReadDouble();
    speedAccuracy_ = parcel.ReadDouble();
    directionAccuracy_ = parcel.ReadDouble();
    uncertaintyOfTimeSinceBoot_ = parcel.ReadInt64();
    locationSourceType_ = parcel.ReadInt32();
    uuid_ = Str16ToStr8(parcel.ReadString16());
    fieldValidity_ = parcel.ReadInt32();
    VectorString16ToVectorString8(additions);
}

void Location::VectorString16ToVectorString8(const std::vector<std::u16string>& additions)
{
    for (auto &addition : additions) {
        auto additionString = Str16ToStr8(addition);
        if (additionString.size() == 0) {
            continue;
        }
        additions_.push_back(additionString);
        auto pos = additionString.find(":");
        auto key = additionString.substr(0, pos);
        auto value = additionString.substr(pos + 1, additionString.size() - 1);
        additionsMap_[key] = value;
    }
}

std::shared_ptr<Location> Location::UnmarshallingShared(Parcel& parcel)
{
    std::shared_ptr<Location> location = std::make_shared<Location>();
    location->ReadFromParcel(parcel);
    return location;
}

std::unique_ptr<Location> Location::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    return location;
}

std::unique_ptr<Location> Location::UnmarshallingMakeUnique(Parcel& parcel)
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    return location;
}

bool Location::Marshalling(Parcel& parcel) const
{
    auto additions = VectorString8ToVectorString16();
    return parcel.WriteDouble(latitude_) &&
           parcel.WriteDouble(longitude_) &&
           parcel.WriteDouble(altitude_) &&
           parcel.WriteDouble(accuracy_) &&
           parcel.WriteDouble(speed_) &&
           parcel.WriteDouble(direction_) &&
           parcel.WriteInt64(timeStamp_) &&
           parcel.WriteInt64(timeSinceBoot_) &&
           parcel.WriteInt64(additionSize_) &&
           parcel.WriteString16Vector(additions) &&
           parcel.WriteBool(isFromMock_) &&
           parcel.WriteInt32(isSystemApp_) &&
           parcel.WriteDouble(altitudeAccuracy_) &&
           parcel.WriteDouble(speedAccuracy_) &&
           parcel.WriteDouble(directionAccuracy_) &&
           parcel.WriteDouble(uncertaintyOfTimeSinceBoot_) &&
           parcel.WriteInt32(locationSourceType_) &&
           parcel.WriteString16(Str8ToStr16(uuid_)) &&
           parcel.WriteInt32(fieldValidity_);
}

std::vector<std::u16string> Location::VectorString8ToVectorString16() const
{
    std::vector<std::u16string> additions;
    for (auto &addition : additions_) {
        auto additionString = Str8ToStr16(addition);
        additions.push_back(additionString);
    }
    return additions;
}

std::string Location::ToString() const
{
    std::string str =
        ", altitude : " + std::to_string(altitude_) +
        ", accuracy : " + std::to_string(accuracy_) +
        ", speed : " + std::to_string(speed_) +
        ", direction : " + std::to_string(direction_) +
        ", timeStamp : " + std::to_string(timeStamp_) +
        ", timeSinceBoot : " + std::to_string(timeSinceBoot_) +
        ", additionSize : " + std::to_string(additionSize_) +
        ", isFromMock : " + std::to_string(isFromMock_) +
        ", isSystemApp : " + std::to_string(isSystemApp_) +
        ", altitudeAccuracy : " + std::to_string(altitudeAccuracy_) +
        ", speedAccuracy : " + std::to_string(speedAccuracy_) +
        ", directionAccuracy : " + std::to_string(directionAccuracy_) +
        ", uncertaintyOfTimeSinceBoot : " + std::to_string(uncertaintyOfTimeSinceBoot_) +
        ", locationSourceType : " + std::to_string(locationSourceType_) +
        ", uuid : " + uuid_ +
        ", fieldValidity : " + std::to_string(fieldValidity_);
    return str;
}

bool Location::LocationEqual(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return false;
    }
    if (this->GetLatitude() == location->GetLatitude() &&
        this->GetLongitude() == location->GetLongitude() &&
        this->GetAltitude() == location->GetAltitude() &&
        this->GetAccuracy() == location->GetAccuracy() &&
        this->GetSpeed() == location->GetSpeed() &&
        this->GetDirection() == location->GetDirection() &&
        this->GetTimeStamp() == location->GetTimeStamp() &&
        this->GetTimeSinceBoot() == location->GetTimeSinceBoot() &&
        this->AdditionEqual(location) &&
        this->GetAdditionSize() == location->GetAdditionSize() &&
        this->GetIsFromMock() == location->GetIsFromMock()) {
        return true;
    }
    return false;
}

bool Location::AdditionEqual(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return false;
    }
    std::vector<std::string> additionA = this->GetAdditions();
    std::vector<std::string> additionB = location->GetAdditions();
    if (additionA.size() != additionB.size()) {
        return false;
    }
    for (size_t i = 0; i < additionA.size(); i++) {
        if (additionA[i].compare(additionB[i]) != 0) {
            return false;
        }
    }
    return true;
}
} // namespace Location
} // namespace OHOS