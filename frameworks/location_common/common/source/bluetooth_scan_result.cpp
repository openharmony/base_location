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

#include "request.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "bluetooth_scan_result.h"

namespace OHOS {
namespace Location {
BluetoothScanResult::BluetoothScanResult()
{
    deviceId_ = "";
    deviceName_ = "";
    rssi_ = 0;
    connectable_ = false;
}

BluetoothScanResult::BluetoothScanResult(BluetoothScanResult& bluetoothScanResult)
{
    deviceId_ = bluetoothScanResult.GetDeviceId();
    deviceName_ = bluetoothScanResult.GetDeviceName();
    rssi_ = bluetoothScanResult.GetRssi();
    connectable_ = bluetoothScanResult.GetConnectable();
    data_ = bluetoothScanResult.GetData();
}

BluetoothScanResult::~BluetoothScanResult() {}

void BluetoothScanResult::ReadFromParcel(Parcel& parcel)
{
    deviceId_ = Str16ToStr8(parcel.ReadString16());
    deviceName_ = Str16ToStr8(parcel.ReadString16());
    rssi_ = parcel.ReadInt64();
    connectable_ = parcel.ReadBool();
    parcel.ReadUInt8Vector(&data_);
}

std::shared_ptr<BluetoothScanResult> BluetoothScanResult::UnmarshallingShared(Parcel& parcel)
{
    std::shared_ptr<BluetoothScanResult> bluetoothScanResult = std::make_shared<BluetoothScanResult>();
    bluetoothScanResult->ReadFromParcel(parcel);
    return bluetoothScanResult;
}

std::unique_ptr<BluetoothScanResult> BluetoothScanResult::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<BluetoothScanResult> bluetoothScanResult = std::make_unique<BluetoothScanResult>();
    bluetoothScanResult->ReadFromParcel(parcel);
    return bluetoothScanResult;
}

bool BluetoothScanResult::Marshalling(Parcel& parcel) const
{
    return parcel.WriteString16(Str8ToStr16(deviceId_)) &&
           parcel.WriteString16(Str8ToStr16(deviceName_)) &&
           parcel.WriteInt64(rssi_) &&
           parcel.WriteBool(connectable_) &&
           parcel.WriteUInt8Vector(data_);
}
} // namespace Location
} // namespace OHOS