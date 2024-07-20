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
class GeoRequestMessage {
public:
    GeoRequestMessage();
    ~GeoRequestMessage();
    std::string GetLocale();
    void SetLocale(std::string locale);
    double GetLatitude();
    void SetLatitude(double latitude);
    double GetLongitude();
    void SetLongitude(double longitude);
    int32_t GetMaxItems();
    void SetMaxItems(int32_t maxItems);
    std::string GetDescription();
    void SetDescription(std::string description);
    double GetMaxLatitude();
    void SetMaxLatitude(double maxLatitude);
    double GetMaxLongitude();
    void SetMaxLongitude(double maxLongitude);
    double GetMinLatitude();
    void SetMinLatitude(double minLatitude);
    double GetMinLongitude();
    void SetMinLongitude(double minLongitude);
    std::string GetBundleName();
    void SetBundleName(std::string bundleName);
    sptr<IRemoteObject> GetCallback();
    void SetCallback(sptr<IRemoteObject> callback);
    std::string GetTransId();
    void SetTransId(std::string transId);
    std::string GetCountry();
    void SetCountry(std::string country);
    bool GetFlag();
    void SetFlag(bool flag);
    int32_t GetCode();
    void SetCode(int32_t code);
    void WriteInfoToParcel(std::unique_ptr<GeoRequestMessage>& geoRequestMessage, MessageParcel &dataParcel, bool flag);
    void WriteInfoToGeoRequestMessage(MessageParcel &data,
        std::unique_ptr<GeoRequestMessage>& geoRequestMessage, bool flag);
private:
    std::string locale_;
    double latitude_;
    double longitude_;
    int64_t maxItems_;
    std::string description_;
    double maxLatitude_;
    double maxLongitude_;
    double minLatitude_;
    double minLongitude_;
    std::string bundleName_;
    sptr<IRemoteObject> callback_ = nullptr;
    std::string transId_;
    std::string country_;
    bool flag_;
    int32_t code_;
};
} // namespace OHOS
} // namespace Location
#endif // FEATURE_GEOCODE_SUPPORT
#endif // GEO_REQUEST_MESSAGE_H
