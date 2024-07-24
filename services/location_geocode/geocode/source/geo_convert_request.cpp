/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "geo_convert_request.h"

namespace OHOS {
namespace Location {
GeoConvertRequest::GeoConvertRequest() {}

GeoConvertRequest::~GeoConvertRequest() {}

std::string GeoConvertRequest::GetLocale()
{
    return locale_;
}

void GeoConvertRequest::SetLocale(std::string locale)
{
    locale_ = locale;
}

double GeoConvertRequest::GetLatitude()
{
    return latitude_;
}

void GeoConvertRequest::SetLatitude(double latitude)
{
    latitude_ = latitude;
}

double GeoConvertRequest::GetLongitude()
{
    return longitude_;
}

void GeoConvertRequest::SetLongitude(double longitude)
{
    longitude_ = longitude;
}

int32_t GeoConvertRequest::GetMaxItems()
{
    return maxItems_;
}

void GeoConvertRequest::SetMaxItems(int32_t maxItems)
{
    maxItems_ = maxItems;
}

std::string GeoConvertRequest::GetDescription()
{
    return description_;
}

void GeoConvertRequest::SetDescription(std::string description)
{
    description_ = description;
}

double GeoConvertRequest::GetMaxLatitude()
{
    return maxLatitude_;
}

void GeoConvertRequest::SetMaxLatitude(double maxLatitude)
{
    maxLatitude_ = maxLatitude;
}

double GeoConvertRequest::GetMaxLongitude()
{
    return maxLongitude_;
}

void GeoConvertRequest::SetMaxLongitude(double maxLongitude)
{
    maxLongitude_ = maxLongitude;
}

double GeoConvertRequest::GetMinLatitude()
{
    return minLatitude_;
}

void GeoConvertRequest::SetMinLatitude(double minLatitude)
{
    minLatitude_ = minLatitude;
}

double GeoConvertRequest::GetMinLongitude()
{
    return minLongitude_;
}

void GeoConvertRequest::SetMinLongitude(double minLongitude)
{
    minLongitude_ = minLongitude;
}

std::string GeoConvertRequest::GetBundleName()
{
    return bundleName_;
}

void GeoConvertRequest::SetBundleName(std::string bundleName)
{
    bundleName_ = bundleName;
}

sptr<IRemoteObject> GeoConvertRequest::GetCallback()
{
    return callback_;
}

void GeoConvertRequest::SetCallback(sptr<IRemoteObject> callback)
{
    callback_ = callback;
}

std::string GeoConvertRequest::GetTransId()
{
    return transId_;
}

void GeoConvertRequest::SetTransId(std::string transId)
{
    transId_ = transId;
}

std::string GeoConvertRequest::GetCountry()
{
    return country_;
}

void GeoConvertRequest::SetCountry(std::string country)
{
    country_ = country;
}

GeoCodeType GeoConvertRequest::GetRequestType()
{
    return requestType_;
}

void GeoConvertRequest::SetRequestType(GeoCodeType requestType)
{
    requestType_ = requestType;
}

bool GeoConvertRequest::Marshalling(MessageParcel& parcel) const
{
    if (requestType_ == GeoCodeType::REQUEST_REVERSE_GEOCODE) {
        parcel.WriteString16(Str8ToStr16(locale_)); // locale
        parcel.WriteDouble(latitude_); // latitude
        parcel.WriteDouble(longitude_); // longitude
        parcel.WriteInt32(maxItems_); // maxItems
    } else {
        parcel.WriteString16(Str8ToStr16(locale_)); // locale
        parcel.WriteString16(Str8ToStr16(description_)); // description
        parcel.WriteInt32(maxItems_); // maxItems
        parcel.WriteDouble(minLatitude_); // minLatitude
        parcel.WriteDouble(minLongitude_); // minLongitude
        parcel.WriteDouble(maxLatitude_); // maxLatitude
        parcel.WriteDouble(maxLongitude_); // maxLongitude
    }
    parcel.WriteString16(Str8ToStr16(bundleName_)); // bundleName
    parcel.WriteRemoteObject(callback_);
    parcel.WriteString16(Str8ToStr16(transId_)); // transId
    parcel.WriteString16(Str8ToStr16(country_)); // country
    return true;
}

std::unique_ptr<GeoConvertRequest> GeoConvertRequest::Unmarshalling(MessageParcel& parcel, GeoCodeType requestType)
{
    std::unique_ptr<GeoConvertRequest> geoConvertRequest = std::make_unique<GeoConvertRequest>();
    geoConvertRequest->SetRequestType(requestType);
    geoConvertRequest->ReadFromParcel(parcel);
    return geoConvertRequest;
}

void GeoConvertRequest::ReadFromParcel(MessageParcel& parcel)
{
    if (requestType_ == GeoCodeType::REQUEST_REVERSE_GEOCODE) {
        locale_ = Str16ToStr8(parcel.ReadString16()); // locale
        latitude_ = parcel.ReadDouble(); // latitude
        longitude_ =  parcel.ReadDouble(); // longitude
        maxItems_ = parcel.ReadInt32(); // maxItems
    } else {
        locale_ = Str16ToStr8(parcel.ReadString16()); // locale
        description_ = Str16ToStr8(parcel.ReadString16()); // description
        maxItems_ = parcel.ReadInt32(); // maxItems
        minLatitude_ = parcel.ReadDouble(); // minLatitude
        minLongitude_ = parcel.ReadDouble(); // minLongitude
        maxLatitude_ = parcel.ReadDouble(); // maxLatitude
        maxLongitude_ = parcel.ReadDouble(); // maxLongitude
    }
    bundleName_ = Str16ToStr8(parcel.ReadString16()); // bundleName
    callback_ = parcel.ReadRemoteObject();
    transId_ = Str16ToStr8(parcel.ReadString16()); // transId
    country_ = Str16ToStr8(parcel.ReadString16()); // country
}

void GeoConvertRequest::OrderParcel(
    MessageParcel& in, MessageParcel& out, GeoCodeType requestType, std::string bundleName)
{
    if (requestType == GeoCodeType::REQUEST_REVERSE_GEOCODE) {
        out.WriteString16(in.ReadString16()); // locale
        out.WriteDouble(in.ReadDouble()); // latitude
        out.WriteDouble(in.ReadDouble()); // longitude
        out.WriteInt32(in.ReadInt32()); // maxItems
    } else {
        out.WriteString16(in.ReadString16()); // locale
        out.WriteString16(in.ReadString16()); // description
        out.WriteInt32(in.ReadInt32()); // maxItems
        out.WriteDouble(in.ReadDouble()); // minLatitude
        out.WriteDouble(in.ReadDouble()); // minLongitude
        out.WriteDouble(in.ReadDouble()); // maxLatitude
        out.WriteDouble(in.ReadDouble()); // maxLongitude
    }
    auto transId = in.ReadString16();
    auto country = in.ReadString16();
    auto callback = in.ReadRemoteObject();
    out.WriteString16(Str8ToStr16(bundleName)); // bundleName
    out.WriteRemoteObject(callback); // callback
    out.WriteString16(transId); // transId
    out.WriteString16(country); // country
}
} // namespace Location
} // namespace OHOS
#endif
