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

#include "geo_address.h"
#include "string_ex.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
GeoAddress::GeoAddress()
{
    latitude_ = 0.0;
    longitude_ = 0.0;
}

std::string GeoAddress::GetDescriptions(int index)
{
    if (index < 0) {
        return "";
    }
    if (descriptionsSize_ <= 0) {
        return "";
    }

    std::unique_lock<std::mutex> lock(mutex_);
    std::map<int, std::string>::iterator it = descriptions_.find(index);
    if (it == descriptions_.end()) {
        return "";
    }

    return it->second;
}

double GeoAddress::GetLatitude()
{
    return latitude_;
}

double GeoAddress::GetLongitude()
{
    return longitude_;
}

bool GeoAddress::GetIsSystemApp()
{
    return isSystemApp_;
}

void GeoAddress::SetIsSystemApp(bool isSystemApp)
{
    isSystemApp_ = isSystemApp;
}

std::unique_ptr<GeoAddress> GeoAddress::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    geoAddress->ReadFromParcel(parcel);
    return geoAddress;
}

void GeoAddress::ReadFromParcel(Parcel& in)
{
    latitude_ = in.ReadDouble();
    longitude_ = in.ReadDouble();
    locale_ = Str16ToStr8(in.ReadString16());
    placeName_ = Str16ToStr8(in.ReadString16());
    countryCode_ = Str16ToStr8(in.ReadString16());
    countryName_ = Str16ToStr8(in.ReadString16());
    administrativeArea_ = Str16ToStr8(in.ReadString16());
    subAdministrativeArea_ = Str16ToStr8(in.ReadString16());
    locality_ = Str16ToStr8(in.ReadString16());
    subLocality_ = Str16ToStr8(in.ReadString16());
    roadName_ = Str16ToStr8(in.ReadString16());
    subRoadName_ = Str16ToStr8(in.ReadString16());
    premises_ = Str16ToStr8(in.ReadString16());
    postalCode_ = Str16ToStr8(in.ReadString16());
    phoneNumber_ = Str16ToStr8(in.ReadString16());
    addressUrl_ = Str16ToStr8(in.ReadString16());
    int size = in.ReadInt32(); // descriptionsSize
    if (size > 0 && size < MAXIMUM_INTERATION) {
        for (int i = 0; i < size; i++) {
            int index = in.ReadInt32();
            if (index < 0 || index >= MAXIMUM_INTERATION) {
                continue;
            }
            std::string line = Str16ToStr8(in.ReadString16());
            std::unique_lock<std::mutex> lock(mutex_);
            descriptions_.insert(std::pair<int, std::string>(index, line));
            descriptionsSize_ = std::max(descriptionsSize_, index + 1);
        }
    } else {
        descriptionsSize_ = 0;
    }
    isFromMock_ = in.ReadBool();
}

bool GeoAddress::Marshalling(Parcel& parcel) const
{
    parcel.WriteDouble(latitude_);
    parcel.WriteDouble(longitude_);
    parcel.WriteString16(Str8ToStr16(locale_));
    parcel.WriteString16(Str8ToStr16(placeName_));
    parcel.WriteString16(Str8ToStr16(countryCode_));
    parcel.WriteString16(Str8ToStr16(countryName_));
    parcel.WriteString16(Str8ToStr16(administrativeArea_));
    parcel.WriteString16(Str8ToStr16(subAdministrativeArea_));
    parcel.WriteString16(Str8ToStr16(locality_));
    parcel.WriteString16(Str8ToStr16(subLocality_));
    parcel.WriteString16(Str8ToStr16(roadName_));
    parcel.WriteString16(Str8ToStr16(subRoadName_));
    parcel.WriteString16(Str8ToStr16(premises_));
    parcel.WriteString16(Str8ToStr16(postalCode_));
    parcel.WriteString16(Str8ToStr16(phoneNumber_));
    parcel.WriteString16(Str8ToStr16(addressUrl_));
    if (descriptions_.size() == 0) {
        parcel.WriteInt32(0);
    } else {
        parcel.WriteInt32(descriptions_.size());
        for (auto iter = descriptions_.begin(); iter != descriptions_.end(); iter++) {
            parcel.WriteInt32(iter->first);
            parcel.WriteString16(Str8ToStr16(iter->second));
        }
    }
    parcel.WriteBool(isFromMock_);
    return true;
}
} // namespace Location
} // namespace OHOS
