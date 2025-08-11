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

#include "beacon_fence.h"

namespace OHOS {
namespace Location {
BeaconFence::BeaconFence()
{
    identifier_ = "";
    type_ = BEACON_MANUFACTURE_DATA;
    manufactureData_.manufactureId = 0;
}

BeaconFence::BeaconFence(BeaconFence& beaconFence)
{
    identifier_ = beaconFence.GetIdentifier();
    type_ = beaconFence.GetBeaconFenceInfoType();
    manufactureData_ = beaconFence.GetBeaconManufactureData();
}

BeaconFence::~BeaconFence() {}

void BeaconFence::ReadFromParcel(Parcel& parcel)
{
    std::unique_lock<std::mutex> lock(beaconFenceMutex_);
    identifier_ = Str16ToStr8(parcel.ReadString16());
    type_ = static_cast<BeaconFenceInfoType>(parcel.ReadInt32());
    manufactureData_.manufactureId = parcel.ReadInt32();
    parcel.ReadUInt8Vector(&manufactureData_.manufactureData);
    parcel.ReadUInt8Vector(&manufactureData_.manufactureDataMask);
}

std::shared_ptr<BeaconFence> BeaconFence::UnmarshallingShared(Parcel& parcel)
{
    std::unique_ptr<BeaconFence> beaconFence = std::make_unique<BeaconFence>();
    beaconFence->ReadFromParcel(parcel);
    return beaconFence;
}

BeaconFence* BeaconFence::Unmarshalling(Parcel& parcel)
{
    auto beaconFence = new BeaconFence();
    beaconFence->ReadFromParcel(parcel);
    return beaconFence;
}

bool BeaconFence::Marshalling(Parcel& parcel) const
{
    std::unique_lock<std::mutex> lock(beaconFenceMutex_);
    return parcel.WriteString16(Str8ToStr16(identifier_)) &&
        parcel.WriteInt32(static_cast<int>(type_)) &&
        parcel.WriteInt32(manufactureData_.manufactureId) &&
        parcel.WriteUInt8Vector(manufactureData_.manufactureData) &&
        parcel.WriteUInt8Vector(manufactureData_.manufactureDataMask);
}
} // namespace Location
} // namespace OHOS