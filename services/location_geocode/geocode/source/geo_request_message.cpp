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
#include "geo_request_message.h"

namespace OHOS {
namespace Location {
GeoRequestMessage::GeoRequestMessage() {}

GeoRequestMessage::~GeoRequestMessage() {}

std::string GeoRequestMessage::GetLocale()
{
    return locale_;
}

void GeoRequestMessage::SetLocale(std::string locale)
{
    locale_ = locale;
}

double GeoRequestMessage::GetLatitude()
{
    return latitude_;
}

void GeoRequestMessage::SetLatitude(double latitude)
{
    latitude_ = latitude;
}

double GeoRequestMessage::GetLongitude()
{
    return longitude_;
}

void GeoRequestMessage::SetLongitude(double longitude)
{
    longitude_ = longitude;
}

int32_t GeoRequestMessage::GetMaxItems()
{
    return maxItems_;
}

void GeoRequestMessage::SetMaxItems(int32_t maxItems)
{
    maxItems_ = maxItems;
}

std::string GeoRequestMessage::GetDescription()
{
    return description_;
}

void GeoRequestMessage::SetDescription(std::string description)
{
    description_ = description;
}

double GeoRequestMessage::GetMaxLatitude()
{
    return maxLatitude_;
}

void GeoRequestMessage::SetMaxLatitude(double maxLatitude)
{
    maxLatitude_ = maxLatitude;
}

double GeoRequestMessage::GetMaxLongitude()
{
    return maxLongitude_;
}

void GeoRequestMessage::SetMaxLongitude(double maxLongitude)
{
    maxLongitude_ = maxLongitude;
}

double GeoRequestMessage::GetMinLatitude()
{
    return minLatitude_;
}

void GeoRequestMessage::SetMinLatitude(double minLatitude)
{
    minLatitude_ = minLatitude;
}

double GeoRequestMessage::GetMinLongitude()
{
    return minLongitude_;
}

void GeoRequestMessage::SetMinLongitude(double minLongitude)
{
    minLongitude_ = minLongitude;
}

std::string GeoRequestMessage::GetBundleName()
{
    return bundleName_;
}

void GeoRequestMessage::SetBundleName(std::string bundleName)
{
    bundleName_ = bundleName;
}

sptr<IRemoteObject> GeoRequestMessage::GetCallback()
{
    return callback_;
}

void GeoRequestMessage::SetCallback(sptr<IRemoteObject> callback)
{
    callback_ = callback;
}

std::string GeoRequestMessage::GetTransId()
{
    return transId_;
}

void GeoRequestMessage::SetTransId(std::string transId)
{
    transId_ = transId;
}

std::string GeoRequestMessage::GetCountry()
{
    return country_;
}

void GeoRequestMessage::SetCountry(std::string country)
{
    country_ = country;
}

bool GeoRequestMessage::GetFlag()
{
    return flag_;
}

void GeoRequestMessage::SetFlag(bool flag)
{
    flag_ = flag;
}

int32_t GeoRequestMessage::GetCode()
{
    return code_;
}

void GeoRequestMessage::SetCode(int32_t code)
{
    code_ = code;
}

void GeoRequestMessage::WriteInfoToParcel(std::unique_ptr<GeoRequestMessage>& geoRequestMessage,
    MessageParcel &dataParcel, bool flag)
{
    if (flag) {
        dataParcel.WriteString16(Str8ToStr16(geoRequestMessage->GetLocale())); // locale
        dataParcel.WriteDouble(geoRequestMessage->GetLatitude()); // latitude
        dataParcel.WriteDouble(geoRequestMessage->GetLongitude()); // longitude
        dataParcel.WriteInt32(geoRequestMessage->GetMaxItems()); // maxItems
    } else {
        dataParcel.WriteString16(Str8ToStr16(geoRequestMessage->GetLocale())); // locale
        dataParcel.WriteString16(Str8ToStr16(geoRequestMessage->GetDescription())); // description
        dataParcel.WriteInt32(geoRequestMessage->GetMaxItems()); // maxItems
        dataParcel.WriteDouble(geoRequestMessage->GetMinLatitude()); // minLatitude
        dataParcel.WriteDouble(geoRequestMessage->GetMinLongitude()); // minLongitude
        dataParcel.WriteDouble(geoRequestMessage->GetMaxLatitude()); // maxLatitude
        dataParcel.WriteDouble(geoRequestMessage->GetMaxLongitude()); // maxLongitude
    }
    dataParcel.WriteString16(Str8ToStr16(geoRequestMessage->GetBundleName())); // bundleName
    dataParcel.WriteRemoteObject(geoRequestMessage->GetCallback());
    dataParcel.WriteString16(Str8ToStr16(geoRequestMessage->GetTransId())); // transId
    dataParcel.WriteString16(Str8ToStr16(geoRequestMessage->GetCountry())); // country
}

void GeoRequestMessage::WriteInfoToGeoRequestMessage(MessageParcel &data,
    std::unique_ptr<GeoRequestMessage>& geoRequestMessage, bool flag)
{
    if (flag) {
        geoRequestMessage->SetLocale(Str16ToStr8(data.ReadString16())); // locale
        geoRequestMessage->SetLatitude(data.ReadDouble()); // latitude
        geoRequestMessage->SetLongitude(data.ReadDouble()); // longitude
        geoRequestMessage->SetMaxItems(data.ReadInt32()); // maxItems
    } else {
        geoRequestMessage->SetLocale(Str16ToStr8(data.ReadString16())); // locale
        geoRequestMessage->SetDescription(Str16ToStr8(data.ReadString16())); // description
        geoRequestMessage->SetMaxItems(data.ReadInt32()); // maxItems
        geoRequestMessage->SetMinLatitude(data.ReadDouble()); // minLatitude
        geoRequestMessage->SetMinLongitude(data.ReadDouble()); // minLongitude
        geoRequestMessage->SetMaxLatitude(data.ReadDouble()); // maxLatitude
        geoRequestMessage->SetMaxLongitude(data.ReadDouble()); // maxLongitude
    }
    geoRequestMessage->SetBundleName(Str16ToStr8(data.ReadString16())); // bundleName
    geoRequestMessage->SetCallback(data.ReadRemoteObject());
    geoRequestMessage->SetTransId(Str16ToStr8(data.ReadString16())); // transId
    geoRequestMessage->SetCountry(Str16ToStr8(data.ReadString16())); // country
}
} // namespace Location
} // namespace OHOS
#endif
