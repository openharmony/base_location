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

#ifndef LOCATION_CELLULAR_INFO_H
#define LOCATION_CELLULAR_INFO_H

#include <parcel.h>
#include <string>
#include <map>
namespace OHOS {
namespace Location {
class CellularInfo : public Parcelable {
public:
    CellularInfo()
    {
        slotId_ = 0;
        timeSinceBoot_ = 0;
        cellId_ = 0;
        lac_ = 0;
        mcc_ = 0;
        mnc_ = 0;
        rat_ = 0;
        singalIntensity_ = 0;
        arfcn_ = 0;
        pci_ = 0;
        additionsMap_ = std::make_shared<std::map<std::string, std::string>>();
    }

    explicit CellularInfo(CellularInfo& cellularInfo)
    {
        SetSlotId(cellularInfo.GetSlotId());
        SetTimeSinceBoot(cellularInfo.GetTimeSinceBoot());
        SetCellId(cellularInfo.GetCellId());
        SetLat(cellularInfo.GetLat());
        SetMcc(cellularInfo.GetMcc());
        SetMnc(cellularInfo.GetMnc());
        SetRat(cellularInfo.GetRat());
        SetSingnalIntensity(cellularInfo.GetSingnalIntensity());
        SetArfcn(cellularInfo.GetArfcn());
        SetPci(cellularInfo.GetPci());
        SetAdditionsMap(cellularInfo.GetAdditionsMap());
    }

    ~CellularInfo() override = default;

    inline void SetSlotId(int32_t slotId)
    {
        slotId_ = slotId;
    }

    inline int32_t GetSlotId()
    {
        return slotId_;
    }

    inline void SetTimeSinceBoot(int64_t timeSinceBoot)
    {
        timeSinceBoot_ = timeSinceBoot;
    }

    inline int64_t GetTimeSinceBoot()
    {
        return timeSinceBoot_;
    }

    inline void SetCellId(int64_t cellId)
    {
        cellId_ = cellId;
    }

    inline int64_t GetCellId()
    {
        return cellId_;
    }

    inline void SetLat(int32_t lac)
    {
        lac_ = lac;
    }

    inline int32_t GetLat()
    {
        return lac_;
    }

    inline void SetMcc(int32_t mcc)
    {
        mcc_ = mcc;
    }

    inline int32_t GetMcc()
    {
        return mcc_;
    }

    inline void SetMnc(int32_t mnc)
    {
        mnc_ = mnc;
    }

    inline int32_t GetMnc()
    {
        return mnc_;
    }

    inline void SetRat(int32_t rat)
    {
        rat_ = rat;
    }

    inline int32_t GetRat()
    {
        return rat_;
    }

    inline void SetSingnalIntensity(int32_t singalIntensity)
    {
        singalIntensity_ = singalIntensity;
    }

    inline int32_t GetSingnalIntensity()
    {
        return singalIntensity_;
    }

    inline void SetArfcn(int32_t arfcn)
    {
        arfcn_ = arfcn;
    }

    inline int32_t GetArfcn()
    {
        return arfcn_;
    }

    inline void SetPci(int32_t pci)
    {
        pci_ = pci;
    }

    inline int32_t GetPci()
    {
        return pci_;
    }

    inline void SetAdditionsMap(std::shared_ptr<std::map<std::string, std::string>> additionsMap)
    {
        additionsMap_ = additionsMap;
    }

    inline std::shared_ptr<std::map<std::string, std::string>> GetAdditionsMap()
    {
        return additionsMap_;
    }

    void ReadFromParcel(Parcel& parcel)
    {
        slotId_ = parcel.ReadInt32();
        timeSinceBoot_ = parcel.ReadInt64();
        cellId_ = parcel.ReadInt64();
        lac_ = parcel.ReadInt32();
        mcc_ = parcel.ReadInt32();
        mnc_ = parcel.ReadInt32();
        rat_ = parcel.ReadInt32();
        singalIntensity_ = parcel.ReadInt32();
        arfcn_ = parcel.ReadInt32();
        pci_ = parcel.ReadInt32();
        size_t size = parcel.ReadUint32();
        for (size_t i = 0; i < size; i++) {
            std::string key, value;
            key = parcel.ReadString();
            value = parcel.ReadString();
            (*additionsMap_)[key] = value;
        }
    }

    bool Marshalling(Parcel& parcel) const override
    {
        parcel.WriteInt32(slotId_);
        parcel.WriteInt64(timeSinceBoot_);
        parcel.WriteInt64(cellId_);
        parcel.WriteInt32(lac_);
        parcel.WriteInt32(mcc_);
        parcel.WriteInt32(mnc_);
        parcel.WriteInt32(rat_);
        parcel.WriteInt32(singalIntensity_);
        parcel.WriteInt32(arfcn_);
        parcel.WriteInt32(pci_);
        if (additionsMap_ == nullptr) {
            return false;
        }
        size_t size = additionsMap_->size();
        if (!parcel.WriteUInt32(size)) {
            return false;
        }
        for (const auto& [key, value] : *additionsMap_) {
            parcel.WriteString(key);
            parcel.WriteString(value);
        }
        return true;
    }

    static std::shared_ptr<CellularInfo> Unmarshalling(Parcel& parcel)
    {
        auto cellularInfo = std::make_shared<CellularInfo>();
        cellularInfo->ReadFromParcel(parcel);
        return cellularInfo;
    }

private:
    int32_t slotId_;
    int64_t timeSinceBoot_;
    int64_t cellId_;
    int32_t lac_;
    int32_t mcc_;
    int32_t mnc_;
    int32_t rat_;
    int32_t singalIntensity_;
    int32_t arfcn_;
    int32_t pci_;
    std::shared_ptr<std::map<std::string, std::string>> additionsMap_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_CELLULAR_INFO_H