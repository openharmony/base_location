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
#endif
#include "iservice_registry.h"
#include "common_utils.h"
#include "permission_manager.h"

namespace OHOS {
namespace Location {
const uint32_t EVENT_START_SCAN = 0x0100;
const uint32_t EVENT_STOP_SCAN = 0x0200;
const uint32_t EVENT_GET_WIFI_LIST = 0x0300;
const int32_t DEFAULT_TIMEOUT_4S = 4000;
LocatorRequiredDataManager::LocatorRequiredDataManager()
{
#ifdef WIFI_ENABLE
    WifiInfoInit();
#endif
    scanHandler_ = std::make_shared<ScanHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    scanListHandler_ =
        std::make_shared<ScanListHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
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
    AppIdentity &identity, std::shared_ptr<LocatingRequiredDataConfig>& config, const sptr<IRemoteObject>& callback)
{
    sptr<ILocatingRequiredDataCallback> dataCallback = iface_cast<ILocatingRequiredDataCallback>(callback);
    if (dataCallback == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s iface_cast ILocatingRequiredDataCallback failed!", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    if (config->GetType() == LocatingRequiredDataType::WIFI) {
#ifdef WIFI_ENABLE
        if (!isWifiCallbackRegistered()) {
            bool ret = RegisterWifiCallBack();
            if (!ret) {
                LBSLOGE(LOCATOR, "%{public}s WifiScan RegisterCallBack failed!", __func__);
                return ERRCODE_SERVICE_UNAVAILABLE;
            }
        }
        std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
        lock.lock();
        auto iter = callbacksMap_.find(dataCallback);
        if (iter != callbacksMap_.end()) {
            LBSLOGE(LOCATOR, "dataCallback has registered!");
            lock.unlock();
            return ERRCODE_SUCCESS;
        }
        callbacksMap_.insert[dataCallback] = identity;
        if (config->GetNeedStartScan() && callbacksMap_.size() == 1) {
            timeInterval_ = config->GetScanIntervalMs();
            if (scanHandler_ != nullptr) {
                scanHandler_->SendEvent(EVENT_START_SCAN, 0, 0);
            }
        }
        lock.unlock();
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
    std::unique_lock<std::mutex> lock(mutex_);
    if (callbacksMap_.size() <= 0) {
        LBSLOGD(GNSS, "scan callback is not in vector");
        return ERRCODE_SUCCESS;
    }
    std::map<sptr<ILocatingRequiredDataCallback>, AppIdentity>::iterator iter;
    for (iter = callbacksMap_.begin(); iter != callbacksMap_.end(); iter++) {
        sptr<IRemoteObject> remoteObject = iter->first->AsObject();
        if (remoteObject == callback) {
            break;
        }
    }
    if (iter != callbacksMap_.end()) {
        callbacksMap_.erase(iter);
    }
    LBSLOGD(LOCATOR, "after UnregisterCallback,  callback size:%{public}s",
        std::to_string(callbacksMap_.size()).c_str());
    if (scanHandler_ != nullptr && callbacksMap_.size() == 0) {
        scanHandler_->SendEvent(EVENT_STOP_SCAN, 0, 0);
    }
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
    wifiScanEventCallback_ =
        sptr<LocatorWifiScanEventCallback>(new (std::nothrow) LocatorWifiScanEventCallback());
    bool ret = RegisterWifiCallBack();
    if (!ret) {
        LBSLOGE(LOCATOR, "%{public}s WifiScan RegisterCallBack failed!", __func__);
    }
    int32_t result;
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s samgrProxy is nullptr!", __func__);
        return;
    }
    saStatusListener_ = sptr<WifiServiceStatusChange>(new WifiServiceStatusChange());
    if (saStatusListener_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s saStatusListener_ is nullptr!", __func__);
        return;
    }
    result = samgrProxy->SubscribeSystemAbility(static_cast<int32_t>(WIFI_SCAN_ABILITY_ID), saStatusListener_);
    LBSLOGI(LOCATOR, "%{public}s SubcribeSystemAbility result is %{public}d!", __func__, result);
}

bool LocatorRequiredDataManager::isWifiCallbackRegistered()
{
    std::unique_lock<std::mutex> lock(wifiRegisteredMutex_);
    return isWifiCallbackRegistered_;
}

void LocatorRequiredDataManager::ResetCallbackRegisteredStatus()
{
    std::unique_lock<std::mutex> lock(wifiRegisteredMutex_);
    isWifiCallbackRegistered_ = false;
}

__attribute__((no_sanitize("cfi"))) bool LocatorRequiredDataManager::RegisterWifiCallBack()
{
    LBSLOGD(LOCATOR, "%{public}s enter", __func__);
    if (wifiScanPtr_ == nullptr || wifiScanEventCallback_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s param unexpected.", __func__);
        return false;
    }
    std::vector<std::string> events = {EVENT_STA_SCAN_STATE_CHANGE};
    std::unique_lock<std::mutex> lock(wifiRegisteredMutex_);
    int ret = wifiScanPtr_->RegisterCallBack(wifiScanEventCallback_, events);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "%{public}s WifiScan RegisterCallBack failed!", __func__);
        return false;
    }
    isWifiCallbackRegistered_ = true;
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
    if (scanListHandler_ != nullptr) {
        scanListHandler_->RemoveEvent(EVENT_GET_WIFI_LIST);
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

void LocatorWifiScanEventCallback::OnWifiScanStateChanged(int state)
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
    for (const auto& pair : callbacksMap_) {
        auto locatingRequiredDataCallback = pair.first;
        AppIdentity identity = pair.second;
        if (CheckPermissionforUser(identity)) {
            locatingRequiredDataCallback->OnLocatingDataChange(result);
        }
    }
}

__attribute__((no_sanitize("cfi"))) void LocatorRequiredDataManager::StartWifiScan(bool flag)
{
    if (!flag) {
        if (scanHandler_ != nullptr) {
            scanHandler_->RemoveEvent(EVENT_START_SCAN);
        }
        if (scanListHandler_ != nullptr) {
            scanListHandler_->RemoveEvent(EVENT_GET_WIFI_LIST);
        }
        LBSLOGE(LOCATOR, "%{public}s no valid callback, return", __func__);
        return;
    }
    if (!IsConnecting()) {
        if (scanHandler_ != nullptr) {
            scanHandler_->RemoveEvent(EVENT_START_SCAN);
        }
        if (scanListHandler_ != nullptr) {
            scanListHandler_->RemoveEvent(EVENT_GET_WIFI_LIST);
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
        if (scanListHandler_ != nullptr) {
            scanListHandler_->SendHighPriorityEvent(EVENT_GET_WIFI_LIST, 0, 0);
        }
    } else {
        if (scanListHandler_ != nullptr) {
            scanListHandler_->SendHighPriorityEvent(EVENT_GET_WIFI_LIST, 0, DEFAULT_TIMEOUT_4S);
        }
    }
#endif
    LBSLOGD(LOCATOR, "StartWifiScan timeInterval_=%{public}d", timeInterval_);
    if (scanHandler_ != nullptr) {
        scanHandler_->SendHighPriorityEvent(EVENT_START_SCAN, 0, timeInterval_);
    }
}

bool LocatorRequiredDataManager::IsConnecting()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callbacksMap_.size() > 0) {
        return true;
    }
    return false;
}

ScanHandler::ScanHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

ScanHandler::~ScanHandler() {}

void ScanHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(LOCATOR, "ScanHandler ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_START_SCAN: {
            dataManager->StartWifiScan(true);
            break;
        }
        case EVENT_STOP_SCAN: {
            dataManager->StartWifiScan(false);
            break;
        }
        default:
            break;
    }
}

ScanListHandler::ScanListHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

ScanListHandler::~ScanListHandler() {}

void ScanListHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(LOCATOR, "ScanListHandler ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_GET_WIFI_LIST: {
            std::vector<Wifi::WifiScanInfo> wifiScanInfo;
            dataManager->GetWifiScanList(wifiScanInfo);
            std::vector<std::shared_ptr<LocatingRequiredData>> result =
                dataManager->GetLocatingRequiredDataByWifi(wifiScanInfo);
            dataManager->ReportData(result);
            break;
        }
        default:
            break;
    }
}

void WifiServiceStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGI(LOCATOR, "WifiServiceStatusChange::OnAddSystemAbility systemAbilityId :%{public}d", systemAbilityId);
    LocatorRequiredDataManager::GetInstance()->RegisterWifiCallBack();
}

void WifiServiceStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGI(LOCATOR, "WifiServiceStatusChange::OnRemoveSystemAbility systemAbilityId :%{public}d", systemAbilityId);
    LocatorRequiredDataManager::GetInstance()->ResetCallbackRegisteredStatus();
}
} // namespace Location
} // namespace OHOS
