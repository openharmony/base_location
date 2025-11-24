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

#ifndef LOCATION_WIFI_SCAN_INFO_H
#define LOCATION_WIFI_SCAN_INFO_H

#include <parcel.h>
#include <string>

namespace OHOS {
namespace Location {
class WifiScanInfo : public Parcelable {
public:
    WifiScanInfo()
    {
        ssid_ = "";
        bssid_ = "";
        rssi_ = 0;
        frequency_ = 0;
        timestamp_ = 0;
        securityType_ = -1;
    }

    explicit WifiScanInfo(WifiScanInfo& wifiScanInfo)
    {
        SetSsid(wifiScanInfo.GetSsid());
        SetBssid(wifiScanInfo.GetBssid());
        SetRssi(wifiScanInfo.GetRssi());
        SetFrequency(wifiScanInfo.GetFrequency());
        SetTimestamp(wifiScanInfo.GetTimestamp());
        SetSecurityType(wifiScanInfo.GetSecurityType());
    }

    ~WifiScanInfo() override = default;

    inline std::string GetSsid() const
    {
        return ssid_;
    }

    inline void SetSsid(std::string ssid)
    {
        ssid_ = ssid;
    }

    inline std::string GetBssid() const
    {
        return bssid_;
    }

    inline void SetBssid(std::string bssid)
    {
        bssid_ = bssid;
    }

    inline int GetRssi() const
    {
        return rssi_;
    }

    inline void SetRssi(int rssi)
    {
        rssi_ = rssi;
    }

    inline int GetFrequency() const
    {
        return frequency_;
    }

    inline void SetFrequency(int frequency)
    {
        frequency_ = frequency;
    }

    inline int64_t GetTimestamp() const
    {
        return timestamp_;
    }

    inline void SetTimestamp(int64_t timestamp)
    {
        timestamp_ = timestamp;
    }

    inline int32_t GetSecurityType() const
    {
        return securityType_;
    }

    inline void SetSecurityType(int32_t securityType)
    {
        securityType_ = securityType;
    }

    void ReadFromParcel(Parcel& parcel)
    {
        ssid_ =  Str16ToStr8(parcel.ReadString16());
        bssid_ =  Str16ToStr8(parcel.ReadString16());
        rssi_ = parcel.ReadInt32();
        frequency_ = parcel.ReadInt32();
        timestamp_ = parcel.ReadInt64();
        securityType_ = parcel.ReadInt32();
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteString16(Str8ToStr16(ssid_)) &&
            parcel.WriteString16(Str8ToStr16(bssid_)) &&
            parcel.WriteInt32(rssi_) &&
            parcel.WriteInt32(frequency_) &&
            parcel.WriteInt64(timestamp_) &&
            parcel.WriteInt32(securityType_);
    }

    static std::shared_ptr<WifiScanInfo> Unmarshalling(Parcel& parcel)
    {
        auto wifiScanInfo = std::make_shared<WifiScanInfo>();
        wifiScanInfo->ReadFromParcel(parcel);
        return wifiScanInfo;
    }

    std::string ToString()
    {
        std::string str = "ssid_ : " + ssid_ +
            ", bssid_ : " + bssid_ +
            ", rssi_ : " + std::to_string(rssi_) +
            ", timestamp_ : " + std::to_string(timestamp_) +
            ", securityType_ : " + std::to_string(securityType_);
        return str;
    }

private:
    std::string ssid_;
    std::string bssid_;
    int rssi_;
    int frequency_;
    int64_t timestamp_;
    int32_t securityType_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_WIFI_SCAN_INFO_H