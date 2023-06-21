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

#ifndef LOCATION_H
#define LOCATION_H

#include <parcel.h>
#include <string>

namespace OHOS {
namespace Location {
class Location : public Parcelable {
public:
    Location();
    explicit Location(Location &location);
    ~Location() override = default;

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

    inline double GetAltitude() const
    {
        return altitude_;
    }

    inline void SetAltitude(double altitude)
    {
        altitude_ = altitude;
    }

    inline double GetAccuracy() const
    {
        return accuracy_;
    }

    inline void SetAccuracy(double accuracy)
    {
        accuracy_ = accuracy;
    }

    inline double GetSpeed() const
    {
        return speed_;
    }

    inline void SetSpeed(double speed)
    {
        speed_ = speed;
    }

    inline double GetDirection() const
    {
        return direction_;
    }

    inline void SetDirection(double direction)
    {
        direction_ = direction;
    }

    inline int64_t GetTimeStamp() const
    {
        return timeStamp_;
    }

    inline void SetTimeStamp(int64_t timeStamp)
    {
        timeStamp_ = timeStamp;
    }

    inline int64_t GetTimeSinceBoot() const
    {
        return timeSinceBoot_;
    }

    inline void SetTimeSinceBoot(int64_t timeStamp)
    {
        timeSinceBoot_ = timeStamp;
    }
    inline std::string GetAdditions() const
    {
        return additions_;
    }

    inline void SetAdditions(const std::string &additions)
    {
        additions_ = additions;
    }

    inline int64_t GetAdditionSize() const
    {
        return additionSize_;
    }

    inline void SetAdditionSize(int64_t size)
    {
        additionSize_ = size;
    }

    inline bool GetIsFromMock() const
    {
        return isFromMock_;
    }

    inline void SetIsFromMock(bool fromMock)
    {
        isFromMock_ = fromMock;
    }

    inline int32_t GetSourceType() const
    {
        return sourceType_;
    }

    inline void SetSourceType(int32_t sourceType)
    {
        sourceType_ = sourceType;
    }

    inline int32_t GetFloorNo() const
    {
        return floorNo_;
    }

    inline void SetFloorNo(int32_t floorNo)
    {
        floorNo_ = floorNo;
    }

    inline double GetFloorAccuracy() const
    {
        return floorAccuracy_;
    }

    inline void SetFloorAccuracy(double floorAccuracy)
    {
        floorAccuracy_ = floorAccuracy;
    }

    void ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    std::string ToString() const;
    static std::unique_ptr<Location> Unmarshalling(Parcel& parcel);
    static std::shared_ptr<Location> UnmarshallingShared(Parcel& parcel);
private:
    double latitude_;
    double longitude_;
    double altitude_;
    double accuracy_;
    double speed_;
    double direction_;
    int64_t timeStamp_;
    int64_t timeSinceBoot_;
    std::string additions_;
    int64_t additionSize_;
    bool isFromMock_;
    int32_t sourceType_;
    int32_t floorNo_;
    double floorAccuracy_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_H