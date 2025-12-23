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

#ifdef BLUETOOTH_ENABLE
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_host.h"
#include "ohos_bt_gatt.h"
#endif
#include "common_event_subscriber.h"
#include "constant_definition.h"
#include "event_handler.h"
#include "event_runner.h"
#include "iremote_stub.h"
#include "i_locating_required_data_callback.h"
#include "ibluetooth_scan_result_callback.h"
#include "bluetooth_scan_result.h"
#include "locating_required_data_config.h"
#include "system_ability_status_change_stub.h"
#include "app_identity.h"
#ifdef WIFI_ENABLE
#include "wifi_scan.h"
#include "kits/c/wifi_device.h"
#endif

namespace OHOS {
namespace Location {
#ifdef WIFI_ENABLE
class LocatorWifiScanEventCallback {
public:
    static void OnWifiScanStateChanged(int state, int size);
};
#endif

class LocatorCellScanInfoCallback {
public:
    static void OnCellScanInfoReceived(std::vector<std::shared_ptr<LocatingRequiredData>> result);
};

#ifdef BLUETOOTH_ENABLE
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
     * @brief Scan result for found or lost callback type.
     *
     * @param result Scan result.
     * @param callbackType callback Type.
     * @since 12
     */
    void OnFoundOrLostCallback(const Bluetooth::BleScanResult &result, uint8_t callbackType) override;

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
    std::unique_ptr<BluetoothScanResult> GetBluetoothScanResultByBle(const Bluetooth::BleScanResult &result);
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
     * @param rssi Rssi of remote device.
     * @param deviceName Name of remote device.
     * @param deviceClass Class of remote device.
     * @since 6
     */
    void OnDiscoveryResult(const Bluetooth::BluetoothRemoteDevice &device, int rssi,
        const std::string deviceName, int deviceClass) override;
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
#endif

class ScanHandler : public AppExecFwk::EventHandler {
public:
    using ScanEventHandle = std::function<void(const AppExecFwk::InnerEvent::Pointer &)>;
    using ScanEventHandleMap = std::map<int, ScanEventHandle>;
    explicit ScanHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~ScanHandler() override;
private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    void InitScanHandlerEventMap();
    void StartBluetoothScanEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StopBluetoothScanEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StartScanEvent(const AppExecFwk::InnerEvent::Pointer& event);

    ScanEventHandleMap scanHandlerEventMap_;
};

class WifiSdkHandler : public AppExecFwk::EventHandler {
public:
    using WifiSdkEventHandle = std::function<void(const AppExecFwk::InnerEvent::Pointer &)>;
    using WifiSdkEventHandleMap = std::map<int, WifiSdkEventHandle>;
    explicit WifiSdkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~WifiSdkHandler() override;

private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    void InitWifiSdkHandlerEventMap();
    void GetWifiListEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void RegisterWifiCallbackEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void UnregisterWifiCallbackEvent(const AppExecFwk::InnerEvent::Pointer& event);

    WifiSdkEventHandleMap wifiSdkHandlerEventMap_;
};

class LocatorRequiredInfo {
public:
    LocatorRequiredInfo() = default;
    LocatorRequiredInfo(const LocatorRequiredInfo& locatorRequiredInfo)
    {
        appIdentity_ = locatorRequiredInfo.appIdentity_;
        config_ = locatorRequiredInfo.config_;
    }
    ~LocatorRequiredInfo() = default;
    AppIdentity appIdentity_;
    LocatingRequiredDataConfig config_;
};

class LocatorRequiredDataManager {
public:
    LocatorRequiredDataManager();
    ~LocatorRequiredDataManager();
    __attribute__((no_sanitize("cfi"))) LocationErrCode RegisterCallback(AppIdentity &identity,
        std::shared_ptr<LocatingRequiredDataConfig>& config, const sptr<IRemoteObject>& callback);
    LocationErrCode UnregisterCallback(const sptr<IRemoteObject>& callback);
    void ReportData(const std::vector<std::shared_ptr<LocatingRequiredData>>& result, int type);
    void ReportBluetoothScanResult(const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult);
    void StartScanBluetoothDevice(sptr<IBluetoothScanResultCallback> callback, AppIdentity identity);
    void StopScanBluetoothDevice(sptr<IRemoteObject> callbackObj);
    void StartBluetoothScan();
    void StoptBluetoothScan();
    void HandleRefreshBluetoothRequest();
    bool GetBluetoothScanStatus();
    void SetBluetoothScanStatus(bool bluetoothScanStatus);
    void RemoveBluetoothScanCallback(sptr<IRemoteObject> callbackObj);
    void RemoveBluetoothScanCallbackDeathRecipientByCallback(sptr<IRemoteObject> callbackObj);
    __attribute__((no_sanitize("cfi"))) void StartWifiScan(int fixNumber, bool flag);
    bool IsWifiConnecting();
    bool IsBluetoothConnecting();
    static LocatorRequiredDataManager* GetInstance();
    void SyncStillMovementState(bool state);
    void SendStartBluetoothScanEvent();
    void SendStopBluetoothScanEvent();
    void SendWifiScanEvent();
    void SendGetWifiListEvent(int timeout, bool needRetryScan);
    void RemoveGetWifiListEvent();
    void UpdateWifiScanCompleteTimestamp();
    int64_t GetWifiScanCompleteTimestamp();
    int64_t GetlastStillTime();
    bool IsStill();
    LocationErrCode GetCurrentWifiBssidForLocating(std::string& bssid);
    int TriggerWifiScan();
    LocationErrCode AddScanCallback(AppIdentity &identity,
        std::shared_ptr<LocatingRequiredDataConfig>& config, const sptr<IRemoteObject>& callback);

private:
    int timeInterval_ = 0;
#ifdef WIFI_ENABLE
public:
    void ResetCallbackRegisteredStatus();
    __attribute__((no_sanitize("cfi"))) bool RegisterWifiCallBack();
    __attribute__((no_sanitize("cfi"))) bool UnregisterWifiCallBack();
    bool GetLocatingRequiredDataByWifi(std::vector<std::shared_ptr<LocatingRequiredData>>& requiredData,
        const std::vector<Wifi::WifiScanInfo>& wifiScanInfo);
    __attribute__((no_sanitize("cfi"))) void GetWifiScanList(std::vector<Wifi::WifiScanInfo>& wifiScanInfo);
    int64_t wifiScanStartTimeStamp_ = 0;
private:
    void WifiInfoInit();
    bool IsWifiCallbackRegistered();
    void SetIsWifiCallbackRegistered(bool isWifiCallbackRegistered);
    std::shared_ptr<Bluetooth::BleCentralManager> bleCentralManager_;
    bool isWifiCallbackRegistered_ = false;
    std::mutex wifiRegisteredMutex_;
    WifiEvent wifiScanEventCallback_ = {0};
#endif
    std::mutex mutex_;
    std::mutex bluetoothcallbacksMapMutex_;
    std::map<sptr<IRemoteObject>, LocatorRequiredInfo> callbacksMap_;
    std::map<sptr<IRemoteObject>,
        std::pair<AppIdentity, sptr<IRemoteObject::DeathRecipient>>> bluetoothcallbacksMap_;
    std::shared_ptr<ScanHandler> scanHandler_;
    std::shared_ptr<WifiSdkHandler> wifiSdkHandler_;
    std::mutex wifiScanCompleteTimestampMutex_;
    std::mutex bluetoothScanStatusMutex_;
    int64_t wifiScanCompleteTimestamp_ = 0;
    std::mutex lastStillTimeMutex_;
    int64_t lastStillTime_ = 0;
    bool bluetoothScanStatus_ = false;
};

class BluetoothScanCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    BluetoothScanCallbackDeathRecipient();
    ~BluetoothScanCallbackDeathRecipient() override;
 
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_REQUIRED_DATA_MANAGER_H
