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

namespace OHOS {
namespace Location {
GeoAddress::GeoAddress()
{
    m_latitude = 0.0;
    m_longitude = 0.0;
}

std::string GeoAddress::GetDescriptions(int index)
{
    if (index < 0) {
        return "";
    }
    if (m_descriptionsSize <= 0) {
        return "";
    }
    std::map<int, std::string>::iterator it = m_descriptions.find(index);
    if (it == m_descriptions.end()) {
        return "";
    }

    return it->second;
}

double GeoAddress::GetLatitude()
{
    if (m_hasLatitude) {
        return m_latitude;
    }
    return 0.0;
}

double GeoAddress::GetLongitude()
{
    if (m_hasLongitude) {
        return m_longitude;
    }
    return 0.0;
}

std::unique_ptr<GeoAddress> GeoAddress::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<GeoAddress> geoAddress = std::make_unique<GeoAddress>();
    geoAddress->ReadFromParcel(parcel);
    return geoAddress;
}

void GeoAddress::ReadFromParcel(Parcel& in)
{
    m_localeLanguage = Str16ToStr8(in.ReadString16());
    m_localeCountry = Str16ToStr8(in.ReadString16());
    int size = in.ReadInt32();
    if (size > 0 && size < MAX_PARCEL_SIZE) {
        for (int i = 0; i < size; i++) {
            int index = in.ReadInt32();
            std::string line = Str16ToStr8(in.ReadString16());
            m_descriptions.insert(std::pair<int, std::string>(index, line));
            m_descriptionsSize = std::max(m_descriptionsSize, index + 1);
        }
    } else {
        m_descriptionsSize = 0;
    }
    m_placeName = Str16ToStr8(in.ReadString16());
    m_administrativeArea = Str16ToStr8(in.ReadString16());
    m_subAdministrativeArea = Str16ToStr8(in.ReadString16());
    m_locality = Str16ToStr8(in.ReadString16());
    m_subLocality = Str16ToStr8(in.ReadString16());
    m_roadName = Str16ToStr8(in.ReadString16());
    m_subRoadName = Str16ToStr8(in.ReadString16());
    m_premises = Str16ToStr8(in.ReadString16());
    m_postalCode = Str16ToStr8(in.ReadString16());
    m_countryCode = Str16ToStr8(in.ReadString16());
    m_countryName = Str16ToStr8(in.ReadString16());
    m_hasLatitude = (in.ReadInt32() != 0);
    if (m_hasLatitude) {
        m_latitude = in.ReadDouble();
    }
    m_hasLongitude = (in.ReadInt32() != 0);
    if (m_hasLongitude) {
        m_longitude = in.ReadDouble();
    }
    m_phoneNumber = Str16ToStr8(in.ReadString16());
    m_addressUrl = Str16ToStr8(in.ReadString16());
    m_isFromMock = in.ReadBool();
}

bool GeoAddress::Marshalling(Parcel& parcel) const
{
    parcel.WriteString16(Str8ToStr16(m_localeLanguage));
    parcel.WriteString16(Str8ToStr16(m_localeCountry));
    if (m_descriptions.size() == 0) {
        parcel.WriteInt32(0);
    } else {
        parcel.WriteInt32(m_descriptions.size());
        for (auto iter = m_descriptions.begin(); iter != m_descriptions.end(); iter++) {
            parcel.WriteInt32(iter->first);
            parcel.WriteString16(Str8ToStr16(iter->second));
        }
    }
    parcel.WriteString16(Str8ToStr16(m_placeName));
    parcel.WriteString16(Str8ToStr16(m_administrativeArea));
    parcel.WriteString16(Str8ToStr16(m_subAdministrativeArea));
    parcel.WriteString16(Str8ToStr16(m_locality));
    parcel.WriteString16(Str8ToStr16(m_subLocality));
    parcel.WriteString16(Str8ToStr16(m_roadName));
    parcel.WriteString16(Str8ToStr16(m_subRoadName));
    parcel.WriteString16(Str8ToStr16(m_premises));
    parcel.WriteString16(Str8ToStr16(m_postalCode));
    parcel.WriteString16(Str8ToStr16(m_countryCode));
    parcel.WriteString16(Str8ToStr16(m_countryName));
    parcel.WriteInt32(m_hasLatitude ? 1 : 0);
    if (m_hasLatitude) {
        parcel.WriteDouble(m_latitude);
    }
    parcel.WriteInt32(m_hasLongitude ? 1 : 0);
    if (m_hasLongitude) {
        parcel.WriteDouble(m_longitude);
    }
    parcel.WriteString16(Str8ToStr16(m_phoneNumber));
    parcel.WriteString16(Str8ToStr16(m_addressUrl));
    parcel.WriteBool(m_isFromMock);
    return true;
}
} // namespace Location
} // namespace OHOS
