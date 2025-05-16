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

#ifndef BULETOOH_SCAN_RESULT_CALLBACK_PROXY_H
#define BULETOOH_SCAN_RESULT_CALLBACK_PROXY_H

#include <vector>

#include "iremote_proxy.h"
#include "iremote_object.h"

#include "ibluetooth_scan_result_callback.h"
#include "bluetooth_scan_result.h"

namespace OHOS {
namespace Location {
class BluetoothScanResultCallbackProxy : public IRemoteProxy<IBluetoothScanResultCallback> {
public:
    explicit BluetoothScanResultCallbackProxy(const sptr<IRemoteObject> &impl);
    ~BluetoothScanResultCallbackProxy() = default;
    void OnBluetoothScanResultChange(const std::unique_ptr<BluetoothScanResult>& data) override;
private:
    static inline BrokerDelegator<BluetoothScanResultCallbackProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // BULETOOH_SCAN_RESULT_CALLBACK_PROXY_H
