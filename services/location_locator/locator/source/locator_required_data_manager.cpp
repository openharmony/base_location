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
#include "locator_required_data_manager.h"
#include "location_log.h"
#ifdef WIFI_ENABLE
#include "wifi_errcode.h"
#include "wifi_device.h"
#endif
#include "iservice_registry.h"

namespace OHOS {
namespace Location {
const uint32_t EVENT_START_SCAN = 0x0100;
const uint32_t EVENT_STOP_SCAN = 0x0200;
const uint32_t EVENT_GET_WIFI_LIST = 0x0300;
const uint32_t EVENT_REGISTER_WIFI_CALLBACK = 0x0400;
const uint32_t EVENT_UNREGISTER_WIFI_CALLBACK = 0x0500;
const int32_t DEFAULT_TIMEOUT_4S = 4000;
LocatorRequiredDataManager::LocatorRequiredDataManager()
{
#ifdef WIFI_ENABLE
    WifiInfoInit();
#endif
    scanHandler_ = std::make_shared<ScanHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    wifiSdkHandler_ =
        std::make_shared<WifiSdkHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
}

LocatorRequiredDataManager* LocatorRequiredDataManager::GetInstance()
{
    static LocatorRequiredDataManager data;
    return &data;
}

LocatorRequiredDataManager::~LocatorRequiredDataManager()
{
}

__attribute__((no_sanitize("cfi"))) LocationErrCode LocatorRequiredDataManager::RegisterCallback(
    std::shared_ptr<LocatingRequiredDataConfig>& config, const sptr<IRemoteObject>& callback)
{
    sptr<ILocatingRequiredDataCallback> dataCallback = iface_cast<ILocatingRequiredDataCallback>(callback);
    if (dataCallback == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s iface_cast ILocatingRequiredDataCallback failed!", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    if (config->GetType() == LocatingRequiredDataType::WIFI) {
#ifdef WIFI_ENABLE
        std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
        lock.lock();
        callbacks_.push_back(dataCallback);
        LBSLOGD(LOCATOR, "after RegisterCallback, callback size:%{public}s", std::to_string(callbacks_.size()).c_str());
        bool needScan = false;
        if (config->GetNeedStartScan() && (callbacks_.size() == 1 || !IsWifiCallbackRegistered())) {
            needScan = true;
        }
        lock.unlock();
        if (needScan) {
            if (wifiSdkHandler_ != nullptr) {
                wifiSdkHandler_->SendEvent(EVENT_REGISTER_WIFI_CALLBACK, 0, 0);
            }
            timeInterval_ = config->GetScanIntervalMs();
            if (scanHandler_ != nullptr) {
                AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
                    EVENT_START_SCAN, config->GetFixNumber());
                scanHandler_->SendEvent(event);
            }
        }
#endif
    } else if (config->GetType() == LocatingRequiredDataType::BLUE_TOOTH) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorRequiredDataManager::UnregisterCallback(const sptr<IRemoteObject>& callback)
{
    sptr<ILocatingRequiredDataCallback> dataCallback = iface_cast<ILocatingRequiredDataCallback>(callback);
    if (dataCallback == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s iface_cast ILocatingRequiredDataCallback failed!", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
#ifdef WIFI_ENABLE
    std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
    lock.lock();
    size_t i = 0;
    for (; i < callbacks_.size(); i++) {
        sptr<IRemoteObject> remoteObject = callbacks_[i]->AsObject();
        if (remoteObject == callback) {
            break;
        }
    }
    if (callbacks_.size() <= i) {
        LBSLOGD(GNSS, "scan callback is not in vector");
        return ERRCODE_SUCCESS;
    }
    if (callbacks_.size() > 0) {
        callbacks_.erase(callbacks_.begin() + i);
    }
    LBSLOGD(LOCATOR, "after UnregisterCallback,  callback size:%{public}s", std::to_string(callbacks_.size()).c_str());
    if (callbacks_.size() > 0) {
        return ERRCODE_SUCCESS;
    }
    lock.unlock();
    if (wifiSdkHandler_ != nullptr) {
        wifiSdkHandler_->SendEvent(EVENT_UNREGISTER_WIFI_CALLBACK, 0, 0);
    }
    if (scanHandler_ != nullptr) {
        scanHandler_->SendEvent(EVENT_STOP_SCAN, 0, 0);
    }
#endif
    return ERRCODE_SUCCESS;
}

#ifdef BLUETOOTH_ENABLE

std::vector<std::shared_ptr<LocatingRequiredData>> LocatorBluetoothHost::GetLocatingRequiredDataByBtHost(
    const Bluetooth::BluetoothRemoteDevice &device)
{
    std::vector<std::shared_ptr<LocatingRequiredData>> result;
    std::shared_ptr<LocatingRequiredData> info = std::make_shared<LocatingRequiredData>();
    std::shared_ptr<BluetoothScanInfo> btData = std::make_shared<BluetoothScanInfo>();
    btData->SetMac(device.GetDeviceAddr());
    btData->SetDeviceName(device.GetDeviceName());
    info->SetType(LocatingRequiredDataType::BLUE_TOOTH);
    info->SetBluetoothScanInfo(btData);
    result.push_back(info);
    return result;
}

std::vector<std::shared_ptr<LocatingRequiredData>> LocatorBleCallbackWapper::GetLocatingRequiredDataByBle(
    const Bluetooth::BleScanResult &result)
{
    std::vector<std::shared_ptr<LocatingRequiredData>> res;
    std::shared_ptr<LocatingRequiredData> info = std::make_shared<LocatingRequiredData>();
    std::shared_ptr<BluetoothScanInfo> btData = std::make_shared<BluetoothScanInfo>();
    btData->SetMac(result.GetPeripheralDevice().GetDeviceAddr());
    btData->SetDeviceName(result.GetPeripheralDevice().GetDeviceName());
    btData->SetRssi(result.GetRssi());
    info->SetType(LocatingRequiredDataType::BLUE_TOOTH);
    info->SetBluetoothScanInfo(btData);
    res.push_back(info);
    return res;
}

void LocatorBluetoothHost::OnStateChanged(const int transport, const int status) {}

void LocatorBluetoothHost::OnDiscoveryStateChanged(int status) {}

void LocatorBluetoothHost::OnDiscoveryResult(const Bluetooth::BluetoothRemoteDevice &device, int rssi,
    const std::string deviceName, int deviceClass)
{
    std::vector<std::shared_ptr<LocatingRequiredData>> result = GetLocatingRequiredDataByBtHost(device);
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    dataManager->ReportData(result);
}

void LocatorBluetoothHost::OnPairRequested(const Bluetooth::BluetoothRemoteDevice &device) {}

void LocatorBluetoothHost::OnPairConfirmed(const Bluetooth::BluetoothRemoteDevice &device, int reqType, int number) {}

void LocatorBluetoothHost::OnScanModeChanged(int mode) {}

void LocatorBluetoothHost::OnDeviceNameChanged(const std::string &deviceName) {}

void LocatorBluetoothHost::OnDeviceAddrChanged(const std::string &address) {}

void LocatorBleCallbackWapper::OnScanCallback(const Bluetooth::BleScanResult &result)
{
    std::vector<std::shared_ptr<LocatingRequiredData>> res = GetLocatingRequiredDataByBle(result);
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    dataManager->ReportData(res);
}

void LocatorBleCallbackWapper::OnFoundOrLostCallback(const Bluetooth::BleScanResult &result, uint8_t callbackType) {}

void LocatorBleCallbackWapper::OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results) {}

void LocatorBleCallbackWapper::OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan) {}

void LocatorBleCallbackWapper::OnNotifyMsgReportFromLpDevice(const Bluetooth::UUID &btUuid, int msgType,
    const std::vector<uint8_t> &value) {}
#endif

#ifdef WIFI_ENABLE
void LocatorRequiredDataManager::WifiInfoInit()
{
    wifiScanPtr_ = Wifi::WifiScan::GetInstance(WIFI_SCAN_ABILITY_ID);
}

bool LocatorRequiredDataManager::IsWifiCallbackRegistered()
{
    std::unique_lock<std::mutex> lock(wifiRegisteredMutex_);
    return isWifiCallbackRegistered_;
}

void LocatorRequiredDataManager::SetIsWifiCallbackRegistered(bool isWifiCallbackRegistered)
{
    std::unique_lock<std::mutex> lock(wifiRegisteredMutex_);
    isWifiCallbackRegistered_ = isWifiCallbackRegistered;
}

__attribute__((no_sanitize("cfi"))) bool LocatorRequiredDataManager::RegisterWifiCallBack()
{
    LBSLOGD(LOCATOR, "%{public}s enter", __func__);
    wifiScanEventCallback_.OnWifiScanStateChanged = LocatorWifiScanEventCallback::OnWifiScanStateChanged;
    int32_t ret = RegisterWifiEvent(&wifiScanEventCallback_);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "%{public}s WifiScan RegisterCallBack failed!", __func__);
        SetIsWifiCallbackRegistered(false);
        return false;
    }
    SetIsWifiCallbackRegistered(true);
    return true;
}

__attribute__((no_sanitize("cfi"))) bool LocatorRequiredDataManager::UnregisterWifiCallBack()
{
    LBSLOGD(LOCATOR, "%{public}s enter", __func__);
    int ret = UnRegisterWifiEvent(&wifiScanEventCallback_);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "%{public}s WifiScan RegisterCallBack failed!", __func__);
        return false;
    }
    SetIsWifiCallbackRegistered(false);
    return true;
}

__attribute__((no_sanitize("cfi"))) void LocatorRequiredDataManager::GetWifiScanList(
    std::vector<Wifi::WifiScanInfo>& wifiScanInfo)
{
    std::shared_ptr<Wifi::WifiScan> ptrWifiScan = Wifi::WifiScan::GetInstance(WIFI_SCAN_ABILITY_ID);
    if (ptrWifiScan == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s WifiScan get instance failed", __func__);
        return;
    }
    int ret = ptrWifiScan->GetScanInfoList(wifiScanInfo);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "GetScanInfoList failed");
        return;
    }
    if (wifiSdkHandler_ != nullptr) {
        wifiSdkHandler_->RemoveEvent(EVENT_GET_WIFI_LIST);
    }
}

std::vector<std::shared_ptr<LocatingRequiredData>> LocatorRequiredDataManager::GetLocatingRequiredDataByWifi(
    const std::vector<Wifi::WifiScanInfo>& wifiScanInfo)
{
    std::vector<std::shared_ptr<LocatingRequiredData>> res;
    for (size_t i = 0; i < wifiScanInfo.size(); i++) {
        std::shared_ptr<LocatingRequiredData> info = std::make_shared<LocatingRequiredData>();
        std::shared_ptr<WifiScanInfo> wifiData = std::make_shared<WifiScanInfo>();
        wifiData->SetSsid(wifiScanInfo[i].ssid);
        wifiData->SetBssid(wifiScanInfo[i].bssid);
        wifiData->SetRssi(wifiScanInfo[i].rssi);
        wifiData->SetFrequency(wifiScanInfo[i].frequency);
        wifiData->SetTimestamp(wifiScanInfo[i].timestamp);
        info->SetType(LocatingRequiredDataType::WIFI);
        info->SetWifiScanInfo(wifiData);
        res.push_back(info);
    }
    return res;
}

void LocatorWifiScanEventCallback::OnWifiScanStateChanged(int state, int size)
{
    LBSLOGD(LOCATOR, "OnWifiScanStateChanged state=%{public}d", state);
    if (state == 0) {
        LBSLOGE(LOCATOR, "OnWifiScanStateChanged false");
    }
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    if (!dataManager->IsConnecting()) {
        LBSLOGE(LOCATOR, "%{public}s no valid callback, return", __func__);
        return;
    }
    std::vector<Wifi::WifiScanInfo> wifiScanInfo;
    dataManager->GetWifiScanList(wifiScanInfo);
    std::vector<std::shared_ptr<LocatingRequiredData>> result =
        dataManager->GetLocatingRequiredDataByWifi(wifiScanInfo);
    dataManager->ReportData(result);
    return;
}
#endif

void LocatorRequiredDataManager::ReportData(const std::vector<std::shared_ptr<LocatingRequiredData>>& result)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (size_t i = 0; i < callbacks_.size(); i++) {
        callbacks_[i]->OnLocatingDataChange(result);
    }
}

__attribute__((no_sanitize("cfi"))) void LocatorRequiredDataManager::StartWifiScan(int fixNumber, bool flag)
{
    if (!flag) {
        if (scanHandler_ != nullptr) {
            scanHandler_->RemoveEvent(EVENT_START_SCAN);
        }
        if (wifiSdkHandler_ != nullptr) {
            wifiSdkHandler_->RemoveEvent(EVENT_GET_WIFI_LIST);
        }
        LBSLOGE(LOCATOR, "%{public}s no valid callback, return", __func__);
        return;
    }
    if (!IsConnecting()) {
        if (scanHandler_ != nullptr) {
            scanHandler_->RemoveEvent(EVENT_START_SCAN);
        }
        if (wifiSdkHandler_ != nullptr) {
            wifiSdkHandler_->RemoveEvent(EVENT_GET_WIFI_LIST);
        }
        return;
    }
#ifdef WIFI_ENABLE
    if (wifiScanPtr_ == nullptr) {
        return;
    }
    int ret = wifiScanPtr_->Scan();
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "%{public}s WifiScan failed, ret=%{public}d", __func__, ret);
        if (wifiSdkHandler_ != nullptr) {
            wifiSdkHandler_->SendHighPriorityEvent(EVENT_GET_WIFI_LIST, 0, 0);
        }
    } else {
        if (wifiSdkHandler_ != nullptr) {
            wifiSdkHandler_->SendHighPriorityEvent(EVENT_GET_WIFI_LIST, 0, DEFAULT_TIMEOUT_4S);
        }
    }
#endif
    if (fixNumber) {
        return;
    }
    LBSLOGD(LOCATOR, "StartWifiScan timeInterval_=%{public}d", timeInterval_);
    if (scanHandler_ != nullptr) {
        scanHandler_->SendHighPriorityEvent(EVENT_START_SCAN, 0, timeInterval_);
    }
}

bool LocatorRequiredDataManager::IsConnecting()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callbacks_.size() > 0) {
        return true;
    }
    return false;
}

LocationErrCode LocatorRequiredDataManager::GetCurrentWifiBssidForLocating(std::string& bssid)
{
#ifdef WIFI_ENABLE
    auto wifiDeviceSharedPtr = Wifi::WifiDevice::GetInstance(WIFI_DEVICE_ABILITY_ID);
    Wifi::WifiDevice* wifiDevicePtr = wifiDeviceSharedPtr.get();
    if (wifiDevicePtr == nullptr) {
        LBSLOGE(LOCATOR, "Enter WifiEnhanceNewUtils:: wifiDevicePtr is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    OHOS::Wifi::WifiLinkedInfo linkedInfo;
    ErrCode ret = wifiDevicePtr->GetLinkedInfo(linkedInfo);
    if (ret == Wifi::WIFI_OPT_STA_NOT_OPENED) {
        LBSLOGE(LOCATOR, "Enter WifiEnhanceNewUtils::GetLinkedInfo fail: %{public}d", ret);
        return ERRCODE_WIFI_IS_NOT_CONNECTED;
    }
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "Enter WifiEnhanceNewUtils::GetLinkedInfo fail: %{public}d", ret);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (linkedInfo.bssid.size() == 0) {
        LBSLOGE(LOCATOR, "linkedInfo.bssid.size() is 0");
        return ERRCODE_WIFI_IS_NOT_CONNECTED;
    }
    bssid = linkedInfo.bssid;
    return ERRCODE_SUCCESS;
#else
    return ERRCODE_NOT_SUPPORTED;
#endif
}

ScanHandler::ScanHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

ScanHandler::~ScanHandler() {}

void ScanHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    int fixNumber = event->GetParam();
    LBSLOGD(LOCATOR, "ScanHandler ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_START_SCAN: {
            dataManager->StartWifiScan(fixNumber, true);
            break;
        }
        case EVENT_STOP_SCAN: {
            dataManager->StartWifiScan(fixNumber, false);
            break;
        }
        default:
            break;
    }
}

WifiSdkHandler::WifiSdkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

WifiSdkHandler::~WifiSdkHandler() {}

void WifiSdkHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(LOCATOR, "WifiSdkHandler ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_GET_WIFI_LIST: {
            std::vector<Wifi::WifiScanInfo> wifiScanInfo;
            dataManager->GetWifiScanList(wifiScanInfo);
            std::vector<std::shared_ptr<LocatingRequiredData>> result =
                dataManager->GetLocatingRequiredDataByWifi(wifiScanInfo);
            dataManager->ReportData(result);
            break;
        }
        case EVENT_REGISTER_WIFI_CALLBACK: {
            dataManager->RegisterWifiCallBack();
            break;
        }
        case EVENT_UNREGISTER_WIFI_CALLBACK: {
            dataManager->UnregisterWifiCallBack();
            break;
        }
        default:
            break;
    }
}
} // namespace Location
} // namespace OHOS
