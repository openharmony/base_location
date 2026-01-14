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

#ifndef LOCATION_REQUIRED_DATA_H
#define LOCATION_REQUIRED_DATA_H

#include "bluetooth_scan_info.h"
#include "wifi_scan_info.h"
#include "cell_info.h"

namespace OHOS {
namespace Location {
const int CELL_INFO_LIST_MAX_SIZE = 1000;

class LocatingRequiredData : public Parcelable {
public:
    LocatingRequiredData()
    {
        type_ = 0;
        wifiData_ = std::make_shared<WifiScanInfo>();
        slotId_ = -1;
        blueToothData_ = std::make_shared<BluetoothScanInfo>();
        campedCellInfo_ = std::make_shared<CellularInfo>();
    }

    ~LocatingRequiredData() override = default;

    inline int GetType() const
    {
        return type_;
    }

    inline void SetType(int type)
    {
        type_ = type;
    }

    inline std::shared_ptr<WifiScanInfo> GetWifiScanInfo() const
    {
        return wifiData_;
    }

    inline void SetWifiScanInfo(std::shared_ptr<WifiScanInfo> wifiData)
    {
        wifiData_ = wifiData;
    }

    inline std::shared_ptr<BluetoothScanInfo> GetBluetoothScanInfo() const
    {
        return blueToothData_;
    }

    inline void SetBluetoothScanInfo(std::shared_ptr<BluetoothScanInfo> blueToothData)
    {
        blueToothData_ = blueToothData;
    }

    inline void SetSlotId(int32_t slotId)
    {
        slotId_ = slotId;
    }

    inline int32_t GetSlotId() const
    {
        return slotId_;
    }

    inline std::shared_ptr<CellularInfo> GetCampedCellInfo() const
    {
        return campedCellInfo_;
    }

    inline void SetCampedCellInfo(std::shared_ptr<CellularInfo> campedCellInfo)
    {
        campedCellInfo_ = campedCellInfo;
    }

    inline std::vector<std::shared_ptr<CellularInfo>> GetNeighboringCellInfo() const
    {
        return neighboringCellInfo_;
    }

    inline void SetNeighboringCellInfo(std::vector<std::shared_ptr<CellularInfo>> neighboringCellInfo)
    {
        neighboringCellInfo_ = neighboringCellInfo;
    }

    void ReadFromParcel(Parcel& parcel)
    {
        type_ =  parcel.ReadInt32();
        wifiData_ = WifiScanInfo::Unmarshalling(parcel);
        blueToothData_ = BluetoothScanInfo::Unmarshalling(parcel);
        slotId_ =  parcel.ReadInt32();
        campedCellInfo_ = CellularInfo::Unmarshalling(parcel);
        auto size = parcel.ReadInt32();
        for (int i = 0; i < size && i < CELL_INFO_LIST_MAX_SIZE; i++) {
            neighboringCellInfo_.push_back(CellularInfo::Unmarshalling(parcel));
        }
    }

    bool Marshalling(Parcel& parcel) const override
    {
        parcel.WriteInt32(type_);
        wifiData_->Marshalling(parcel);
        blueToothData_->Marshalling(parcel);
        parcel.WriteInt32(slotId_);
        campedCellInfo_->Marshalling(parcel);
        parcel.WriteInt32(neighboringCellInfo_.size());
        for (size_t i = 0; i < neighboringCellInfo_.size(); i++) {
            neighboringCellInfo_[i]->Marshalling(parcel);
        }
        return true;
    }

    static std::shared_ptr<LocatingRequiredData> Unmarshalling(Parcel& parcel)
    {
        auto locatingRequiredData = std::make_shared<LocatingRequiredData>();
        locatingRequiredData->ReadFromParcel(parcel);
        return locatingRequiredData;
    }

    std::string ToString()
    {
        std::string str = "type_ : " + std::to_string(type_);
        return str + wifiData_->ToString() + blueToothData_->ToString();
    }

private:
    int type_;
    std::shared_ptr<WifiScanInfo> wifiData_;
    std::shared_ptr<BluetoothScanInfo> blueToothData_;
    int32_t slotId_;
    std::shared_ptr<CellularInfo> campedCellInfo_;
    std::vector<std::shared_ptr<CellularInfo>> neighboringCellInfo_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_REQUIRED_DATA_H