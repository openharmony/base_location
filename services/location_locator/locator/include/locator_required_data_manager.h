/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef LOCATOR_REQUIRED_DATA_MANAGER_H
#define LOCATOR_REQUIRED_DATA_MANAGER_H

#include <map>
#include <mutex>
#include <singleton.h>
#include <string>

#include "bluetooth_ble_central_manager.h"
#include "bluetooth_host.h"
#include "common_event_subscriber.h"
#include "constant_definition.h"
#include "event_handler.h"
#include "event_runner.h"
#include "iremote_stub.h"
#include "i_locating_required_data_callback.h"
#include "locating_required_data_config.h"
#include "wifi_scan.h"

namespace OHOS {
namespace Location {
class LocatorWifiScanEventCallback : public Wifi::IWifiScanCallback {
public:
    explicit LocatorWifiScanEventCallback() {}
    ~LocatorWifiScanEventCallback() {}
    void OnWifiScanStateChanged(int state) override;
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
    std::vector<std::shared_ptr<LocatingRequiredData>> GetLocatingRequiredDataByWifi(
        const std::vector<Wifi::WifiScanInfo>& wifiScanInfo);
};

class LocatorBleCallbackWapper : public Bluetooth::BleCentralManagerCallback {
public:
    /**
     * @brief Scan result callback.
     *
     * @param result Scan result.
     * @since 6
     */
    void OnScanCallback(const Bluetooth::BleScanResult &result) override;
    /**
     * @brief Batch scan results event callback.
     *
     * @param results Scan results.
     * @since 6
     */
    void OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results) override;
    /**
     * @brief Start or Stop scan event callback.
     *
     * @param resultCode Scan result code.
     * @param isStartScan true->start scan, false->stop scan.
     * @since 6
     */
    void OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan) override;
    /**
     * @brief Notify low power device msg callback.
     *
     * @param btUuid uuid.
     * @param msgType notify msgType.
     * @param value notify msg value.
     * @since 6
     */
    void OnNotifyMsgReportFromLpDevice(const Bluetooth::UUID &btUuid,
        int msgType, const std::vector<uint8_t> &value) override;

    std::vector<std::shared_ptr<LocatingRequiredData>> GetLocatingRequiredDataByBle(
        const Bluetooth::BleScanResult &result);
};

class LocatorBluetoothHost : public Bluetooth::BluetoothHostObserver {
public:
    /**
     * @brief Adapter state change function.
     *
     * @param transport Transport type when state change.
     *        BTTransport::ADAPTER_BREDR : classic;
     *        BTTransport::ADAPTER_BLE : ble.
     * @param state Change to the new state.
     *        BTStateID::STATE_TURNING_ON;
     *        BTStateID::STATE_TURN_ON;
     *        BTStateID::STATE_TURNING_OFF;
     *        BTStateID::STATE_TURN_OFF.
     * @since 6
     */
    void OnStateChanged(const int transport, const int status) override;
    /**
     * @brief Discovery state changed observer.
     *
     * @param status Device discovery status.
     * @since 6
     */
    void OnDiscoveryStateChanged(int status) override;
    /**
     * @brief Discovery result observer.
     *
     * @param device Remote device.
     * @since 6
     */
    void OnDiscoveryResult(const Bluetooth::BluetoothRemoteDevice &device) override;
    /**
     * @brief Pair request observer.
     *
     * @param device Remote device.
     * @since 6
     */
    void OnPairRequested(const Bluetooth::BluetoothRemoteDevice &device) override;
    /**
     * @brief Pair confirmed observer.
     *
     * @param device Remote device.
     * @param reqType Pair type.
     * @param number Paired passkey.
     * @since 6
     */
    void OnPairConfirmed(const Bluetooth::BluetoothRemoteDevice &device, int reqType, int number) override;
    /**
     * @brief Scan mode changed observer.
     *
     * @param mode Device scan mode.
     * @since 6
     */
    void OnScanModeChanged(int mode) override;
    /**
     * @brief Device name changed observer.
     *
     * @param deviceName Device name.
     * @since 6
     */
    void OnDeviceNameChanged(const std::string &deviceName) override;
    /**
     * @brief Device address changed observer.
     *
     * @param address Device address.
     * @since 6
     */
    void OnDeviceAddrChanged(const std::string &address) override;

    std::vector<std::shared_ptr<LocatingRequiredData>> GetLocatingRequiredDataByBtHost(
        const Bluetooth::BluetoothRemoteDevice &device);
};

class ScanHandler : public AppExecFwk::EventHandler {
public:
    explicit ScanHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~ScanHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
};

class LocatorRequiredDataManager : public DelayedSingleton<LocatorRequiredDataManager> {
public:
    LocatorRequiredDataManager();
    ~LocatorRequiredDataManager();
    LocationErrCode RegisterCallback(std::shared_ptr<LocatingRequiredDataConfig>& config,
        const sptr<IRemoteObject>& callback);
    LocationErrCode UnregisterCallback(const sptr<IRemoteObject>& callback);
    void ReportData(const std::vector<std::shared_ptr<LocatingRequiredData>>& result);
    void StartWifiScan(bool flag);
    bool IsConnecting();
private:
    int timeInterval_ = 0;
    std::unique_ptr<Wifi::WifiScan> wifiScanPtr_;
    sptr<LocatorWifiScanEventCallback> wifiScanEventCallback_;
    std::shared_ptr<Bluetooth::BleCentralManager> bleCentralManager_;
    Bluetooth::BluetoothHost *bluetoothHost_;
    LocatorBluetoothHost locatorBluetoothHost_;
    std::mutex mutex_;
    std::vector<sptr<ILocatingRequiredDataCallback>> callbacks_;
    std::shared_ptr<ScanHandler> scanHandler_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_REQUIRED_DATA_MANAGER_H
