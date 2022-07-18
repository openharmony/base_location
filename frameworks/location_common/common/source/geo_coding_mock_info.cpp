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

#include "geo_coding_mock_info.h"
#include "constant_definition.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
GeocodingMockInfo::GeocodingMockInfo()
{
    location_ = std::make_shared<ReverseGeocodeRequest>();
    geoAddress_ = std::make_shared<GeoAddress>();
}

std::shared_ptr<ReverseGeocodeRequest> GeocodingMockInfo::GetLocation()
{
    return location_;
}

std::shared_ptr<GeoAddress> GeocodingMockInfo::GetGeoAddressInfo()
{
    return geoAddress_;
}

void GeocodingMockInfo::SetLocation(std::shared_ptr<ReverseGeocodeRequest> request)
{
    location_ = request;
}

void GeocodingMockInfo::SetGeoAddressInfo(std::shared_ptr<GeoAddress> geoAddress)
{
    geoAddress_ = geoAddress;
}

void GeocodingMockInfo::ReadFromParcel(Parcel& parcel)
{
    location_->locale = Str16ToStr8(parcel.ReadString16());
    location_->latitude = parcel.ReadDouble();
    location_->longitude = parcel.ReadDouble();
    location_->maxItems = parcel.ReadInt32();
    geoAddress_ = GeoAddress::Unmarshalling(parcel);
}

std::unique_ptr<GeocodingMockInfo> GeocodingMockInfo::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<GeocodingMockInfo> mockInfo = std::make_unique<GeocodingMockInfo>();
    mockInfo->ReadFromParcel(parcel);
    return mockInfo;
}

bool GeocodingMockInfo::Marshalling(Parcel& parcel) const
{
    return parcel.WriteString16(Str8ToStr16(location_->locale)) &&
           parcel.WriteDouble(location_->latitude) &&
           parcel.WriteDouble(location_->longitude) &&
           parcel.WriteInt32(location_->maxItems) &&
           geoAddress_->Marshalling(parcel);
}
} // namespace Location
} // namespace OHOS