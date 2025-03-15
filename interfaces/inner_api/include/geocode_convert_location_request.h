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

#ifndef GEOCODE_CONVERT_LOCATION_REQUEST_H
#define GEOCODE_CONVERT_LOCATION_REQUEST_H

#include <singleton.h>
#include <string>
#include <vector>

#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
class GeocodeConvertLocationRequest : public Parcelable {
public:
    GeocodeConvertLocationRequest();
    ~GeocodeConvertLocationRequest();
    std::string GetLocale();
    void SetLocale(std::string locale);
    double GetLatitude();
    void SetLatitude(double latitude);
    double GetLongitude();
    void SetLongitude(double longitude);
    int32_t GetMaxItems();
    void SetMaxItems(int32_t maxItems);
    std::string GetTransId();
    void SetTransId(std::string transId);
    std::string GetCountry();
    void SetCountry(std::string country);
    
    bool Marshalling(Parcel& parcel) const;
    static GeocodeConvertLocationRequest* Unmarshalling(Parcel& parcel);
    void ReadFromParcel(Parcel& parcel);
    static std::unique_ptr<GeocodeConvertLocationRequest> UnmarshallingMessageParcel(MessageParcel& parcel);
    void ReadFromMessageParcel(MessageParcel& parcel);
private:
    std::string locale_;
    double latitude_;
    double longitude_;
    int64_t maxItems_;
    std::string transId_;
    std::string country_;
};
} // namespace OHOS
} // namespace Location
#endif // GEOCODE_CONVERT_LOCATION_REQUEST_H
