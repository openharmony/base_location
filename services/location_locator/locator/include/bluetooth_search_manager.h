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

#ifndef BLUETOOTH_SEARCH_MANAGER_H
#define BLUETOOTH_SEARCH_MANAGER_H

#include <map>
#include <memory>
#include <mutex>

#ifdef BLUETOOTH_ENABLE
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_host.h"
#include "ohos_bt_gatt.h"
#endif

#include "app_identity.h"
#include "bluetooth_scan_result.h"
#include "bluetooth_search_request_params.h"
#include "constant_definition.h"
#include "ibluetooth_scan_result_callback.h"
#include "iremote_stub.h"
#include "singleton.h"

namespace OHOS {
namespace Location {

class BluetoothSearchManager : public Singleton<BluetoothSearchManager> {
public:
    BluetoothSearchManager();
    ~BluetoothSearchManager();

    void StartBluetoothSearch(sptr<IBluetoothScanResultCallback> callback,
        AppIdentity identity, const BluetoothSearchRequestParams& params);
    void StopBluetoothSearch(IRemoteObject* remote);
    void ReportBluetoothScanResult(const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult);

public:
    class BluetoothSearchCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };

private:
    bool MatchFilter(const BluetoothScanResult& result, const BluetoothSearchRequestParams& params);
    void InitBleManager();
    void UninitBleManager();

#ifdef BLUETOOTH_ENABLE
    class BluetoothSearchScanCallback : public Bluetooth::BleCentralManagerCallback {
    public:
        void OnScanCallback(const Bluetooth::BleScanResult& result) override;
        void OnFoundOrLostCallback(const Bluetooth::BleScanResult& result, uint8_t callbackType) override;
        void OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult>& results) override;
        void OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan) override;
    };
#endif
    std::mutex callbacksMapMutex_;
    std::map<sptr<IRemoteObject>, std::pair<AppIdentity, std::pair<BluetoothSearchRequestParams,
        sptr<IRemoteObject::DeathRecipient>>>> bluetoothSearchCallbacksMap_;

    std::mutex bluetoothSearchScanStatusMutex_;
    bool bluetoothSearchScanStatus_;

#ifdef BLUETOOTH_ENABLE
    std::shared_ptr<Bluetooth::BleCentralManager> bleCentralManager_;
    std::shared_ptr<BluetoothSearchScanCallback> scanCallback_;
#endif
};

} // namespace Location
} // namespace OHOS

#endif // BLUETOOTH_SEARCH_MANAGER_H