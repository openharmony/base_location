/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef LOCATION_MATCHING_WLAN_INFO_H
#define LOCATION_MATCHING_WLAN_INFO_H

#include <parcel.h>
#include <string>

namespace OHOS {
namespace Location {
class MatchingWlanInfo : public Parcelable {
public:
    MatchingWlanInfo()
    {
        index_ = 0;
        ssid_ = "";
        rssi_ = 0;
    }

    ~MatchingWlanInfo() override = default;

    inline int32_t GetIndex() const
    {
        return index_;
    }

    inline void SetIndex(int32_t index)
    {
        index_ = index;
    }

    inline std::string GetSsid() const
    {
        return ssid_;
    }

    inline void SetSsid(const std::string& ssid)
    {
        ssid_ = ssid;
    }

    inline int32_t GetRssi() const
    {
        return rssi_;
    }

    inline void SetRssi(int32_t rssi)
    {
        rssi_ = rssi;
    }

    bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteInt32(index_)) {
            return false;
        }
        if (!parcel.WriteString(ssid_)) {
            return false;
        }
        if (!parcel.WriteInt32(rssi_)) {
            return false;
        }
        return true;
    }

    static MatchingWlanInfo* Unmarshalling(Parcel& parcel)
    {
        auto matchingWlanInfo = new MatchingWlanInfo();
        if (matchingWlanInfo->ReadFromParcel(parcel)) {
            return matchingWlanInfo;
        } else {
            delete matchingWlanInfo;
            return nullptr;
        }
    }

    bool ReadFromParcel(Parcel& parcel)
    {
        if (!(parcel.ReadInt32(index_) && parcel.ReadString(ssid_) && parcel.ReadInt32(rssi_))) {
            return false;
        }
        return true;
    }

private:
    int32_t index_;
    std::string ssid_;
    int32_t rssi_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_MATCHING_WLAN_INFO_H