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

#ifndef LOCATION_BEACON_FENCE_REQUEST_H
#define LOCATION_BEACON_FENCE_REQUEST_H

#include <parcel.h>
#include <string>
#include "string_ex.h"

namespace OHOS {
namespace Location {
class BeaconFenceRequest : public Parcelable {
public:
    BeaconFenceRequest();
    explicit BeaconFenceRequest(BeaconFenceRequest &beaconFenceRequest);
    ~BeaconFenceRequest() override;

    expor enum BeaconFenceInfoType {
        BEACON_MANUFACTURE_DATA = 1;
    }

    typedef struct {
        int32_t manufactureId;
        std::vector<uint8_t> manufactureData;
        std::vector<uint8_t> manufactureDataMask;
    } BeaconManufactureData;

    typedef struct {
        std::string identifier;
        BeaconFenceInfoType type;
        BeaconManufactureData manufactureData;
    } BeaconFence;

    inline BeaconFence GetBeaconFence() const
    {
        return beacon_;
    }

    inline void SetBeaconFence(BeaconFence beacon) const
    {
        beacon_ = beacon;
    }

    inline sptr<IRemoteObject> GetBeaconFenceTransitionCallback() const
    {
        return callback_;
    }

    inline void SetBeaconFenceTransitionCallback(const sptr<IRemoteObject>& callback)
    {
        callback_ = callback;
    }

    inline std::string GetFenceExtensionAbilityName() const
    {
        return fenceExtensionAbilityName_;
    }

    inline void SetFenceExtensionAbilityName(std::string fenceExtensionAbilityName)
    {
        fenceExtensionAbilityName_ = fenceExtensionAbilityName;
    }

    void ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    static std::shared_ptr<BeaconFenceRequest> UnmarshallingShared(Parcel& parcel);
    static BeaconFenceRequest* Unmarshalling(Parcel& parcel);

private:
    BeaconFence beacon_;
    sptr<IRemoteObject> callback_;
    std::string fenceExtensionAbilityName_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_BEACON_FENCE_REQUEST_H