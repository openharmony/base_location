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

namespace OHOS {
namespace Location {
Location::Location()
{
    latitude_ = 0.0;
    longitude_ = 0.0;
    altitude_ = 0.0;
    accuracy_ = 0.0;
    speed_ = 0.0;
    direction_ = 0.0;
    timeStamp_ = 0;
    timeSinceBoot_ = 0;
	additions_ = "";
	additionSize_ = 0;
	isFromMock_ = false;
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
	additions_ = location.GetAdditions();
	additionSize_ = location.GetAdditionSize();
	isFromMock_ = location.GetIsFromMock();
}

void Location::ReadFromParcel(Parcel& parcel)
{
    latitude_ = parcel.ReadDouble();
    longitude_ = parcel.ReadDouble();
    altitude_ = parcel.ReadDouble();
    accuracy_ = parcel.ReadFloat();
    speed_ = parcel.ReadFloat();
    direction_ = parcel.ReadDouble();
    timeStamp_ = parcel.ReadInt64();
    timeSinceBoot_ = parcel.ReadInt64();
	additions_ = parcel.ReadString();
	additionSize_ = parcel.ReadInt64();
	isFromMock_ = parcel.ReadBool();
}

void Location::ReadFromParcelLocation(Parcel& parcel)
{
    if (parcel.ReadInt32() == 0) {
        return;
    }
    parcel.ReadString16();               // read string
    timeStamp_ = parcel.ReadInt64();     // read time
    timeSinceBoot_ = parcel.ReadInt64(); // read mElapsedRealtimeNanos
    parcel.ReadDouble();                 // read mElapsedRealtimeUncertaintyNanos
    parcel.ReadInt32();                  // read mFieldsMask
    latitude_ = parcel.ReadDouble();     // read mLatitude
    longitude_ = parcel.ReadDouble();    // read mLongitude
    altitude_ = parcel.ReadDouble();     // read mAltitude
    speed_ = parcel.ReadFloat();         // read mSpeed
    direction_ = parcel.ReadFloat();     // read mBearing
    accuracy_ = parcel.ReadFloat();      // read mHorizontalAccuracyMeters
    parcel.ReadFloat();                  // read mVerticalAccuracyMeters
    parcel.ReadFloat();                  // read mSpeedAccuracyMetersPerSecond
    parcel.ReadFloat();                  // read mBearingAccuracyDegrees
}

std::unique_ptr<Location> Location::UnmarshallingLocation(Parcel& parcel)
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcelLocation(parcel);
    return location;
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

bool Location::Marshalling(Parcel& parcel) const
{
    return parcel.WriteDouble(latitude_) &&
           parcel.WriteDouble(longitude_) &&
           parcel.WriteDouble(altitude_) &&
           parcel.WriteFloat(accuracy_) &&
           parcel.WriteFloat(speed_) &&
           parcel.WriteDouble(direction_) &&
           parcel.WriteInt64(timeStamp_) &&
           parcel.WriteInt64(timeSinceBoot_) &&
           parcel.WriteString(additions_) &&
           parcel.WriteInt64(additionSize_) &&
           parcel.WriteBool(isFromMock_);
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
        ", timeSinceBoot : " + std::to_string(timeSinceBoot_);
    return str;
}
} // namespace Location
} // namespace OHOS