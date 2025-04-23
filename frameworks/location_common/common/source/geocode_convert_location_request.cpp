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

#include "geocode_convert_location_request.h"

namespace OHOS {
namespace Location {
GeocodeConvertLocationRequest::GeocodeConvertLocationRequest()
{
    locale_ = "";
    latitude_ = 0.0;
    longitude_ = 0.0;
    maxItems_ = 0;
    transId_ = "";
    country_ = "";
}

GeocodeConvertLocationRequest::~GeocodeConvertLocationRequest() {}

std::string GeocodeConvertLocationRequest::GetLocale()
{
    return locale_;
}

void GeocodeConvertLocationRequest::SetLocale(std::string locale)
{
    locale_ = locale;
}

double GeocodeConvertLocationRequest::GetLatitude()
{
    return latitude_;
}

void GeocodeConvertLocationRequest::SetLatitude(double latitude)
{
    latitude_ = latitude;
}

double GeocodeConvertLocationRequest::GetLongitude()
{
    return longitude_;
}

void GeocodeConvertLocationRequest::SetLongitude(double longitude)
{
    longitude_ = longitude;
}

int32_t GeocodeConvertLocationRequest::GetMaxItems()
{
    return maxItems_;
}

void GeocodeConvertLocationRequest::SetMaxItems(int32_t maxItems)
{
    maxItems_ = maxItems;
}

std::string GeocodeConvertLocationRequest::GetTransId()
{
    return transId_;
}

void GeocodeConvertLocationRequest::SetTransId(std::string transId)
{
    transId_ = transId;
}

std::string GeocodeConvertLocationRequest::GetCountry()
{
    return country_;
}

void GeocodeConvertLocationRequest::SetCountry(std::string country)
{
    country_ = country;
}

bool GeocodeConvertLocationRequest::Marshalling(Parcel& parcel) const
{
    parcel.WriteString16(Str8ToStr16(locale_)); // locale
    parcel.WriteDouble(latitude_); // latitude
    parcel.WriteDouble(longitude_); // longitude
    parcel.WriteInt32(maxItems_); // maxItems
    parcel.WriteString16(Str8ToStr16(transId_)); // transId
    parcel.WriteString16(Str8ToStr16(country_)); // country
    return true;
}

GeocodeConvertLocationRequest* GeocodeConvertLocationRequest::Unmarshalling(Parcel& parcel)
{
    auto geoConvertRequest = new GeocodeConvertLocationRequest();
    geoConvertRequest->ReadFromParcel(parcel);
    return geoConvertRequest;
}

void GeocodeConvertLocationRequest::ReadFromParcel(Parcel& parcel)
{
    locale_ = Str16ToStr8(parcel.ReadString16()); // locale
    latitude_ = parcel.ReadDouble(); // latitude
    longitude_ =  parcel.ReadDouble(); // longitude
    maxItems_ = parcel.ReadInt32(); // maxItems
    transId_ = Str16ToStr8(parcel.ReadString16()); // transId
    country_ = Str16ToStr8(parcel.ReadString16()); // country
}

std::unique_ptr<GeocodeConvertLocationRequest> GeocodeConvertLocationRequest::UnmarshallingMessageParcel(
    MessageParcel& parcel)
{
    auto geoConvertRequest = std::make_unique<GeocodeConvertLocationRequest>();
    geoConvertRequest->ReadFromMessageParcel(parcel);
    return geoConvertRequest;
}

void GeocodeConvertLocationRequest::ReadFromMessageParcel(MessageParcel& parcel)
{
    locale_ = Str16ToStr8(parcel.ReadString16()); // locale
    latitude_ = parcel.ReadDouble(); // latitude
    longitude_ =  parcel.ReadDouble(); // longitude
    maxItems_ = parcel.ReadInt32(); // maxItems
    transId_ = Str16ToStr8(parcel.ReadString16()); // transId
    country_ = Str16ToStr8(parcel.ReadString16()); // country
}
} // namespace Location
} // namespace OHOS
