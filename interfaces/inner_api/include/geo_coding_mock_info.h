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

#ifdef FEATURE_GEOCODE_SUPPORT
#ifndef LOCATION_GEO_CODEING_MOCK_INFO_H
#define LOCATION_GEO_CODEING_MOCK_INFO_H

#include <parcel.h>

#include "constant_definition.h"
#include "geo_address.h"

namespace OHOS {
namespace Location {
class GeocodingMockInfo : public Parcelable {
public:
    GeocodingMockInfo();
    ~GeocodingMockInfo() override = default;

    void ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static std::unique_ptr<GeocodingMockInfo> Unmarshalling(Parcel& parcel);

    std::shared_ptr<ReverseGeocodeRequest> GetLocation();
    std::shared_ptr<GeoAddress> GetGeoAddressInfo();

    void SetLocation(std::shared_ptr<ReverseGeocodeRequest> request);
    void SetGeoAddressInfo(std::shared_ptr<GeoAddress> geoAddress);
private:
    std::shared_ptr<ReverseGeocodeRequest> location_;
    std::shared_ptr<GeoAddress> geoAddress_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_GEO_CODEING_MOCK_INFO_H
#endif // FEATURE_GEOCODE_SUPPORT
