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

#include "beacon_fence_request.h"

namespace OHOS {
namespace Location {
BeaconFenceRequest::BeaconFenceRequest()
{
    callback_ = nullptr;
    fenceId_ = "";
    fenceExtensionAbilityName_ = "";
    serviceUuid_ = "";
    serviceUuidMask_ = "";
}

BeaconFenceRequest::BeaconFenceRequest(BeaconFenceRequest& beaconFenceRequest)
{
    beacon_ =  beaconFenceRequest.GetBeaconFence();
    callback_ = beaconFenceRequest.GetBeaconFenceTransitionCallback();
    fenceExtensionAbilityName_ = beaconFenceRequest.GetFenceExtensionAbilityName();
    fenceId_ = beaconFenceRequest.GetFenceId();
}

BeaconFenceRequest::~BeaconFenceRequest() {}

std::shared_ptr<BeaconFence> BeaconFenceRequest::GetBeaconFence()
{
    return beacon_;
}

void BeaconFenceRequest::SetBeaconFence(std::shared_ptr<BeaconFence>& beacon)
{
    beacon_ = beacon;
}

void BeaconFenceRequest::ReadFromParcel(Parcel& parcel)
{
    std::unique_lock<std::mutex> lock(beaconFenceRequestMutex_);
    std::shared_ptr<BeaconFence> beacon = std::make_shared<BeaconFence>();
    beacon->SetIdentifier(Str16ToStr8(parcel.ReadString16()));
    beacon->SetBeaconFenceInfoType(static_cast<BeaconFenceInfoType>(parcel.ReadInt32()));
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = parcel.ReadInt32();
    parcel.ReadUInt8Vector(&manufactureData.manufactureData);
    parcel.ReadUInt8Vector(&manufactureData.manufactureDataMask);
    beacon->SetBeaconManufactureData(manufactureData);
    SetBeaconFence(beacon);
    callback_ = parcel.ReadObject<IRemoteObject>();
    fenceExtensionAbilityName_ = Str16ToStr8(parcel.ReadString16());
}

std::shared_ptr<BeaconFenceRequest> BeaconFenceRequest::UnmarshallingShared(Parcel& parcel)
{
    std::unique_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_unique<BeaconFenceRequest>();
    beaconFenceRequest->ReadFromParcel(parcel);
    return beaconFenceRequest;
}

BeaconFenceRequest* BeaconFenceRequest::Unmarshalling(Parcel& parcel)
{
    auto beaconFenceRequest = new BeaconFenceRequest();
    beaconFenceRequest->ReadFromParcel(parcel);
    return beaconFenceRequest;
}

bool BeaconFenceRequest::Marshalling(Parcel& parcel) const
{
    std::unique_lock<std::mutex> lock(beaconFenceRequestMutex_);
    return parcel.WriteString16(Str8ToStr16(beacon_->GetIdentifier())) &&
            parcel.WriteInt32(static_cast<int>(beacon_->GetBeaconFenceInfoType())) &&
            parcel.WriteInt32(beacon_->GetBeaconManufactureData().manufactureId) &&
            parcel.WriteUInt8Vector(beacon_->GetBeaconManufactureData().manufactureData) &&
            parcel.WriteUInt8Vector(beacon_->GetBeaconManufactureData().manufactureDataMask) &&
            parcel.WriteRemoteObject(callback_) &&
            parcel.WriteString16(Str8ToStr16(fenceExtensionAbilityName_));
}
} // namespace Location
} // namespace OHOS