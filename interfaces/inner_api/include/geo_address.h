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

#ifndef GEO_ADDRESS_H
#define GEO_ADDRESS_H

#include <map>
#include <parcel.h>
#include <string>

namespace OHOS {
namespace Location {
class GeoAddress : public Parcelable {
public:
    GeoAddress();
    ~GeoAddress() override = default;
    bool Marshalling(Parcel& parcel) const override;
    static std::unique_ptr<GeoAddress> Unmarshalling(Parcel& parcel);
    std::string GetDescriptions(int index);
    double GetLatitude();
    double GetLongitude();
    void ReadFromParcel(Parcel& in);

    double m_latitude;
    double m_longitude;
    std::string m_localeLanguage;
    std::string m_localeCountry;
    std::string m_placeName;
    std::string m_countryCode;
    std::string m_countryName;
    std::string m_administrativeArea;
    std::string m_subAdministrativeArea;
    std::string m_locality;
    std::string m_subLocality;
    std::string m_roadName;
    std::string m_subRoadName;
    std::string m_premises;
    std::string m_postalCode;
    std::string m_phoneNumber;
    std::string m_addressUrl;
    std::map<int, std::string> m_descriptions;
    int m_descriptionsSize = 0;
    bool m_hasLatitude = false;
    bool m_hasLongitude = false;
    bool m_isFromMock = false;
    static constexpr double PARCEL_INT_SIZE = 64.0;
    static constexpr int MAX_RESULT = 10;
};
} // namespace Location
} // namespace OHOS
#endif // GEO_ADDRESS_H
