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

#ifndef LOCATION_GEOFENCE_H
#define LOCATION_GEOFENCE_H

#include <map>
#include <parcel.h>
#include "geofence_definition.h"

namespace OHOS {
namespace Location {
class Geofence : public Parcelable {
public:
    Geofence();
    Geofence(const Geofence &Geofence);
    ~Geofence();

    inline double GetLatitude() const
    {
        return latitude_;
    }

    inline void SetLatitude(double latitude)
    {
        latitude_ = latitude;
    }

    inline double GetLongitude() const
    {
        return longitude_;
    }

    inline void SetLongitude(double longitude)
    {
        longitude_ = longitude;
    }

    inline double GetRadius() const
    {
        return radius_;
    }

    inline void SetRadius(double radius)
    {
        radius_ = radius;
    }

    inline double GetExpiration() const
    {
        return expiration_;
    }

    inline void SetExpiration(double expiration)
    {
        expiration_ = expiration;
    }

    inline CoordinateSystemType GetCoordinateSystemType() const
    {
        return coordinateSystemType_;
    }

    inline void SetCoordinateSystemType(CoordinateSystemType coordinateSystemType)
    {
        coordinateSystemType_ = coordinateSystemType;
    }

    void ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static std::shared_ptr<Geofence> UnmarshallingShared(Parcel& parcel);
    static std::unique_ptr<Geofence> UnmarshallingMakeUnique(Parcel& parcel);
    static Geofence* Unmarshalling(Parcel& parcel);
    
    double latitude_;
    double longitude_;
    double radius_;
    double expiration_;
    CoordinateSystemType coordinateSystemType_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_GEOFENCE_H