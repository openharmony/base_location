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

#ifndef LOCATION_LOCATING_REQUIRED_DATA_CONFIG_H
#define LOCATION_LOCATING_REQUIRED_DATA_CONFIG_H

#include <parcel.h>
#include <string>

namespace OHOS {
namespace Location {
class LocatingRequiredDataConfig : public Parcelable {
public:
    LocatingRequiredDataConfig()
    {
        type_ = 0;
        needStartScan_ = false;
        scanIntervalMs_ = 0;
        scanTimeoutMs_ = DEFAULT_TIMEOUT_30S;
        fixNumber_ = 0;
    }

    explicit LocatingRequiredDataConfig(LocatingRequiredDataConfig& LocatingRequiredDataConfig)
    {
        SetType(LocatingRequiredDataConfig.GetType());
        SetNeedStartScan(LocatingRequiredDataConfig.GetNeedStartScan());
        SetScanIntervalMs(LocatingRequiredDataConfig.GetScanIntervalMs());
        SetScanTimeoutMs(LocatingRequiredDataConfig.GetScanTimeoutMs());
    }

    ~LocatingRequiredDataConfig() override = default;

    inline int GetType() const
    {
        return type_;
    }

    inline void SetType(int type)
    {
        type_ = type;
    }

    inline bool GetNeedStartScan() const
    {
        return needStartScan_;
    }

    inline void SetNeedStartScan(bool needStartScan)
    {
        needStartScan_ = needStartScan;
    }

    inline int GetScanIntervalMs() const
    {
        return scanIntervalMs_;
    }

    inline void SetScanIntervalMs(int scanIntervalMs)
    {
        scanIntervalMs_ = scanIntervalMs;
    }

    inline int GetScanTimeoutMs() const
    {
        return scanTimeoutMs_;
    }

    inline void SetScanTimeoutMs(int scanTimeoutMs)
    {
        scanTimeoutMs_ = scanTimeoutMs;
    }

    inline int GetFixNumber() const
    {
        return fixNumber_;
    }

    inline void SetFixNumber(int fixNumber)
    {
        fixNumber_ = fixNumber;
    }

    void ReadFromParcel(Parcel& parcel)
    {
        type_ =  parcel.ReadInt32();
        needStartScan_ =  parcel.ReadBool();
        scanIntervalMs_ = parcel.ReadInt32();
        scanTimeoutMs_ = parcel.ReadInt32();
        fixNumber_ = parcel.ReadInt32();
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteInt32(type_) &&
            parcel.WriteBool(needStartScan_) &&
            parcel.WriteInt32(scanIntervalMs_) &&
            parcel.WriteInt32(scanTimeoutMs_) &&
            parcel.WriteInt32(fixNumber_);
    }

    static LocatingRequiredDataConfig* Unmarshalling(Parcel& parcel)
    {
        auto locatingRequiredDataConfig = new (std::nothrow) LocatingRequiredDataConfig();
        locatingRequiredDataConfig->ReadFromParcel(parcel);
        return locatingRequiredDataConfig;
    }

    std::string ToString()
    {
        std::string str = "type_ : " + std::to_string(type_) +
            ", needStartScan_ : " + (needStartScan_ ? "true" : "false") +
            ", scanIntervalMs_ : " + std::to_string(scanIntervalMs_) +
            ", scanTimeoutMs_ : " + std::to_string(scanTimeoutMs_) +
            ", fixNumber : " + std::to_string(fixNumber_);
        return str;
    }

private:
    int type_;
    bool needStartScan_;
    int scanIntervalMs_;
    int scanTimeoutMs_;
    int fixNumber_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_LOCATING_REQUIRED_DATA_CONFIG_H