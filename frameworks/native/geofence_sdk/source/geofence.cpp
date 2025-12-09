/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "geofence.h"
#include <parcel.h>
#include "common_utils.h"

namespace OHOS {
namespace Location {
Geofence::Geofence()
{
    latitude_ = 0.0;
    longitude_ = 0.0;
    radius_ = 0.0;
    expiration_ = 0.0;
    coordinateSystemType_ = WGS84;
}

Geofence::Geofence(const Geofence& geofence)
{
    this->SetLatitude(geofence.GetLatitude());
    this->SetLongitude(geofence.GetLongitude());
    this->SetRadius(geofence.GetRadius());
    this->SetExpiration(geofence.GetExpiration());
    this->SetCoordinateSystemType(geofence.GetCoordinateSystemType());
}

Geofence::~Geofence() {}

void Geofence::ReadFromParcel(Parcel& parcel)
{
    latitude_ = parcel.ReadDouble();
    longitude_ = parcel.ReadDouble();
    radius_ = parcel.ReadDouble();
    expiration_ = parcel.ReadDouble();
    coordinateSystemType_ = static_cast<CoordinateSystemType>(parcel.ReadInt32());
}

bool Geofence::Marshalling(Parcel& parcel) const
{
    parcel.WriteDouble(latitude_);
    parcel.WriteDouble(longitude_);
    parcel.WriteDouble(radius_);
    parcel.WriteDouble(expiration_);
    parcel.WriteInt32(static_cast<int>(coordinateSystemType_));
    return true;
}

std::shared_ptr<Geofence> Geofence::UnmarshallingShared(Parcel& parcel)
{
    std::shared_ptr<Geofence> geofence = std::make_shared<Geofence>();
    geofence->ReadFromParcel(parcel);
    return geofence;
}

std::unique_ptr<Geofence> Geofence::UnmarshallingMakeUnique(Parcel& parcel)
{
    std::unique_ptr<Geofence> geofence = std::make_unique<Geofence>();
    geofence->ReadFromParcel(parcel);
    return geofence;
}

Geofence* Geofence::Unmarshalling(Parcel& parcel)
{
    auto geofence = new Geofence();
    geofence->ReadFromParcel(parcel);
    return geofence;
}
} // namespace Location
} // namespace OHOS