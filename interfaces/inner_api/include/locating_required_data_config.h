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
class ArfcnInfo  : public Parcelable {
public:
    ArfcnInfo()
    {
        arfcnCount_ = 0;
        arfcnArray_ = {};
        plmnParamArray_ = {};
    }

    explicit ArfcnInfo(ArfcnInfo& arfcnInfo)
    {
        SetArfcnCount(arfcnInfo.GetArfcnCount());
        SetArfcnArray(arfcnInfo.GetArfcnArray());
        SetPlmnParamArray(arfcnInfo.GetPlmnParamArray());
    }

    ~ArfcnInfo() override = default;

    inline void SetArfcnCount(int32_t arfcnCount)
    {
        arfcnCount_ = arfcnCount;
    }

    inline int32_t GetArfcnCount()
    {
        return arfcnCount_;
    }

    inline void SetArfcnArray(std::vector<int32_t> arfcnArray)
    {
        arfcnArray_ = arfcnArray;
    }

    inline std::vector<int32_t> GetArfcnArray()
    {
        return arfcnArray_;
    }

    inline void SetPlmnParamArray(std::vector<int32_t> plmnParamArray)
    {
        plmnParamArray_ = plmnParamArray;
    }

    inline std::vector<int32_t> GetPlmnParamArray()
    {
        return plmnParamArray_;
    }

    void ReadFromParcel(Parcel& parcel)
    {
        arfcnCount_ = parcel.ReadInt32();
        parcel.ReadInt32Vector(&arfcnArray_);
        parcel.ReadInt32Vector(&plmnParamArray_);
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteInt32(arfcnCount_) &&
            parcel.WriteInt32Vector(arfcnArray_) &&
            parcel.WriteInt32Vector(plmnParamArray_);
    }

    static std::shared_ptr<ArfcnInfo> Unmarshalling(Parcel& parcel)
    {
        auto arfcnInfo = std::make_shared<ArfcnInfo>();
        arfcnInfo->ReadFromParcel(parcel);
        return arfcnInfo;
    }
private:
    int32_t arfcnCount_;
    std::vector<int> arfcnArray_;
    std::vector<int32_t> plmnParamArray_;
};

class LocatingRequiredDataConfig : public Parcelable {
public:
    LocatingRequiredDataConfig()
    {
        type_ = 0;
        needStartScan_ = false;
        scanIntervalMs_ = 0;
        scanTimeoutMs_ = DEFAULT_TIMEOUT_30S;
        fixNumber_ = 0;
        isWlanMatchCalled_ = false;
        rssiThreshold_ = 0;
        arfcnInfo_ = nullptr;
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

    inline bool GetIsWlanMatchCalled() const
    {
        return isWlanMatchCalled_;
    }

    inline void SetIsWlanMatchCalled(bool isWlanMatchCalled)
    {
        isWlanMatchCalled_ = isWlanMatchCalled;
    }

    inline int GetRssiThreshold() const
    {
        return rssiThreshold_;
    }

    inline void SetRssiThreshold(int rssiThreshold)
    {
        rssiThreshold_ = rssiThreshold;
    }

    inline std::vector<int32_t> GetSlotIdArray() const
    {
        return slotIdArray_;
    }

    inline void SetSlotIdArray(std::vector<int32_t> slotIdArray)
    {
        slotIdArray_ = slotIdArray;
    }

    inline std::shared_ptr<ArfcnInfo> GetArfcnInfo() const
    {
        return arfcnInfo_;
    }

    inline void SetArfcnInfo(std::shared_ptr<ArfcnInfo> arfcnInfo)
    {
        arfcnInfo_ = arfcnInfo;
    }

    inline std::vector<std::string> GetWlanBssidArray() const
    {
        return wlanBssidArray_;
    }

    inline void SetWlanBssidArray(std::vector<std::string> wlanBssidArray)
    {
        wlanBssidArray_ = wlanBssidArray;
    }

    void ReadFromParcel(Parcel& parcel)
    {
        type_ =  parcel.ReadInt32();
        needStartScan_ =  parcel.ReadBool();
        scanIntervalMs_ = parcel.ReadInt32();
        scanTimeoutMs_ = parcel.ReadInt32();
        fixNumber_ = parcel.ReadInt32();
        isWlanMatchCalled_ = parcel.ReadBool();
        rssiThreshold_ = parcel.ReadInt32();
        arfcnInfo_ = ArfcnInfo::Unmarshalling(parcel);
        int wlanArraySize = parcel.ReadInt32();
        if (wlanArraySize > INPUT_WIFI_LIST_MAX_SIZE) {
            wlanArraySize = INPUT_WIFI_LIST_MAX_SIZE;
        }
        for (int i = 0; i < wlanArraySize; ++i) {
            std::string wlanBssid = parcel.ReadString();
            wlanBssidArray_.push_back(wlanBssid);
        }
    }

    bool MarshallingWlanBssidArray(Parcel& parcel, std::vector<std::string> wlanBssidArray) const
    {
        bool marshallingState = false;
        size_t arraySize = wlanBssidArray.size();
        if (arraySize > INPUT_WIFI_LIST_MAX_SIZE) {
            arraySize = INPUT_WIFI_LIST_MAX_SIZE;
        }
        marshallingState = parcel.WriteInt32(arraySize);
        for (size_t i = 0; i < arraySize; ++i) {
            marshallingState = parcel.WriteString(wlanBssidArray[i]);
        }
        return marshallingState;
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteInt32(type_) &&
            parcel.WriteBool(needStartScan_) &&
            parcel.WriteInt32(scanIntervalMs_) &&
            parcel.WriteInt32(scanTimeoutMs_) &&
            parcel.WriteInt32(fixNumber_) &&
            parcel.WriteBool(isWlanMatchCalled_) &&
            parcel.WriteInt32(rssiThreshold_) &&
            parcel.WriteInt32Vector(slotIdArray_) &&
            arfcnInfo_->Marshalling(parcel) &&
            MarshallingWlanBssidArray(parcel, wlanBssidArray_);
    }

    static LocatingRequiredDataConfig* Unmarshalling(Parcel& parcel)
    {
        auto locatingRequiredDataConfig = new LocatingRequiredDataConfig();
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
    bool isWlanMatchCalled_;
    int rssiThreshold_;
    std::vector<std::string> wlanBssidArray_;
    std::vector<int32_t> slotIdArray_;
    std::shared_ptr<ArfcnInfo> arfcnInfo_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_LOCATING_REQUIRED_DATA_CONFIG_H