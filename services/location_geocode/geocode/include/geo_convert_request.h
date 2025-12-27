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

#ifndef GEO_REQUEST_MESSAGE_H
#define GEO_REQUEST_MESSAGE_H
#ifdef FEATURE_GEOCODE_SUPPORT

#include <mutex>
#include <singleton.h>
#include <string>
#include <vector>

#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
enum class GeoCodeType {
    REQUEST_GEOCODE = 1,
    REQUEST_REVERSE_GEOCODE,
};

class GeoConvertRequest {
public:
    GeoConvertRequest();
    GeoConvertRequest(const GeoConvertRequest& geoConvertRequest);
    ~GeoConvertRequest();
    std::string GetLocale() const;
    void SetLocale(std::string locale);
    double GetLatitude() const;
    void SetLatitude(double latitude);
    double GetLongitude() const;
    void SetLongitude(double longitude);
    int32_t GetMaxItems() const;
    void SetMaxItems(int32_t maxItems);
    int32_t GetPriority() const;
    void SetPriority(int32_t priority);
    int64_t GetTimeStamp() const;
    void SetTimeStamp(int64_t timeStamp);
    std::string GetDescription() const;
    void SetDescription(std::string description);
    double GetMaxLatitude() const;
    void SetMaxLatitude(double maxLatitude);
    double GetMaxLongitude() const;
    void SetMaxLongitude(double maxLongitude);
    double GetMinLatitude() const;
    void SetMinLatitude(double minLatitude);
    double GetMinLongitude() const;
    void SetMinLongitude(double minLongitude);
    std::string GetBundleName() const;
    void SetBundleName(std::string bundleName);
    sptr<IRemoteObject> GetCallback() const;
    void SetCallback(sptr<IRemoteObject> callback);
    std::string GetTransId() const;
    void SetTransId(std::string transId);
    std::string GetCountry() const;
    void SetCountry(std::string country);
    GeoCodeType GetRequestType() const;
    void SetRequestType(GeoCodeType requestType);
    bool Marshalling(MessageParcel& parcel) const;
    static std::unique_ptr<GeoConvertRequest> Unmarshalling(MessageParcel& parcel, GeoCodeType requestType);
    static void OrderParcel(MessageParcel& in, MessageParcel& out,
        const sptr<IRemoteObject>& cb, GeoCodeType requestType, std::string bundleName);
    void ReadFromParcel(MessageParcel& parcel);
private:
    std::string locale_;
    double latitude_;
    double longitude_;
    int32_t maxItems_;
    std::string description_;
    double maxLatitude_;
    double maxLongitude_;
    double minLatitude_;
    double minLongitude_;
    std::string bundleName_;
    sptr<IRemoteObject> callback_;
    std::string transId_;
    std::string country_;
    GeoCodeType requestType_;
    int32_t priority_;
    int64_t timeStamp_;
};
} // namespace OHOS
} // namespace Location
#endif // FEATURE_GEOCODE_SUPPORT
#endif // GEO_REQUEST_MESSAGE_H
