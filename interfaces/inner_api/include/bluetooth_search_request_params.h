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

#ifndef BLUETOOTH_SEARCH_REQUEST_PARAMS_H
#define BLUETOOTH_SEARCH_REQUEST_PARAMS_H

#include <memory>
#include <parcel.h>
#include <string>
#include <vector>
#include "string_ex.h"

namespace OHOS {
namespace Location {

class BluetoothSearchRequestParams : public Parcelable {
public:
    BluetoothSearchRequestParams()
    {
        rssiThreshold = -1000;
    }
    ~BluetoothSearchRequestParams() override = default;

    void ReadFromParcel(Parcel& parcel)
    {
        rssiThreshold = parcel.ReadInt32();
        int32_t size = parcel.ReadInt32();
        for (int32_t i = 0; i < size; i++) {
            deviceIdArray.push_back(Str16ToStr8(parcel.ReadString16()));
        }
    }

    bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteInt32(rssiThreshold)) {
            return false;
        }
        if (!parcel.WriteInt32(deviceIdArray.size())) {
            return false;
        }
        for (const auto& deviceId : deviceIdArray) {
            if (!parcel.WriteString16(Str8ToStr16(deviceId))) {
                return false;
            }
        }
        return true;
    }

    static BluetoothSearchRequestParams* Unmarshalling(Parcel& parcel)
    {
        BluetoothSearchRequestParams* params = new (std::nothrow) BluetoothSearchRequestParams();
        if (params != nullptr) {
            params->ReadFromParcel(parcel);
        }
        return params;
    }

    std::vector<std::string> deviceIdArray;
    int32_t rssiThreshold;
};

} // namespace Location
} // namespace OHOS
#endif