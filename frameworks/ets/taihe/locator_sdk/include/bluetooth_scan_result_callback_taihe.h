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

#ifndef BLUETOOTH_SCAN_RESULT_CALLBACK_TAIHE_H
#define BLUETOOTH_SCAN_RESULT_CALLBACK_TAIHE_H

#include "iremote_stub.h"

#include "constant_definition.h"
#include "location_log.h"
#include "ibluetooth_scan_result_callback.h"
#include "bluetooth_scan_result.h"

#include "ohos.geoLocationManager.proj.hpp"
#include "ohos.geoLocationManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Location {
class BluetoothScanResultCallbackTaihe : public IRemoteStub<IBluetoothScanResultCallback> {
public:
    BluetoothScanResultCallbackTaihe();
    virtual ~BluetoothScanResultCallbackTaihe();
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnBluetoothScanResultChange(const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult) override;
    ::taihe::optional<::taihe::callback<void(::ohos::geoLocationManager::BluetoothScanResult const&)>> callback_;
};
} // namespace Location
} // namespace OHOS
#endif // BLUETOOTH_SCAN_RESULT_CALLBACK_NAPI_H
