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

#include "bluetooth_scan_result_callback_taihe.h"

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "message_option.h"
#include "message_parcel.h"

#include "constant_definition.h"
#include "util.h"

namespace OHOS {
namespace Location {
BluetoothScanResultCallbackTaihe::BluetoothScanResultCallbackTaihe()
{
}

BluetoothScanResultCallbackTaihe::~BluetoothScanResultCallbackTaihe()
{
}

int BluetoothScanResultCallbackTaihe::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(LOCATOR_CALLBACK, "BluetoothScanResultCallbackTaihe::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }
    switch (code) {
        case RECEIVE_INFO_EVENT: {
            std::unique_ptr<BluetoothScanResult> res = BluetoothScanResult::Unmarshalling(data);
            OnBluetoothScanResultChange(res);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void BluetoothScanResultCallbackTaihe::OnBluetoothScanResultChange(
    const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult)
{
    ::ohos::geoLocationManager::BluetoothScanResult bluetoothScanResultTaihe =
        ::ohos::geoLocationManager::BluetoothScanResult{};
    Util::BluetoothScanResultToTaihe(bluetoothScanResultTaihe, bluetoothScanResult);
    if (callback_) {
        (*callback_)(bluetoothScanResultTaihe);
    }
}
} // namespace Location
} // namespace OHOS
