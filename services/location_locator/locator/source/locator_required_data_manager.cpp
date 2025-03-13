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
#include "common_utils.h"
#include "permission_manager.h"
#include "locator_ability.h"
#include "location_data_rdb_manager.h"
#ifdef LOCATION_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace Location {
const uint32_t EVENT_START_SCAN = 0x0100;
const uint32_t EVENT_GET_WIFI_LIST = 0x0300;
const uint32_t EVENT_REGISTER_WIFI_CALLBACK = 0x0400;
const uint32_t EVENT_UNREGISTER_WIFI_CALLBACK = 0x0500;
const uint32_t EVENT_START_BLUETOOTH_SCAN = 0x0600;
const uint32_t EVENT_STOP_BLUETOOTH_SCAN = 0x0700;
const int32_t DEFAULT_TIMEOUT_4S = 4000;
const int32_t DEFAULT_TIMEOUT_MS = 1500;
const int64_t DEFAULT_TIMEOUT_30_MIN = 30 * 60 * MILLI_PER_SEC * MICRO_PER_MILLI;
const int64_t DEFAULT_INVALID_10_SECONDS = 10 * MILLI_PER_SEC * MICRO_PER_MILLI;
const int64_t DEFAULT_NOT_RETRY_TIME_10_SECONDS = 10 * MILLI_PER_SEC * MICRO_PER_MILLI; //10s
const int64_t DEFAULT_INVALID_12_HOURS = 12 * 60 * 60 * MILLI_PER_SEC;
const int TIMEOUT_WATCHDOG = 60; // s
const int32_t MAX_CALLBACKS_MAP_NUM = 1000;

const std::string TYPE_WHITE_LIST_BLE = "ble";
LocatorRequiredDataManager::LocatorRequiredDataManager()
{
#ifdef BLUETOOTH_ENABLE
    std::shared_ptr<LocatorBleCallbackWapper> bleCallback = std::make_shared<LocatorBleCallbackWapper>();
    bleCentralManager_ = std::make_shared<Bluetooth::BleCentralManager>(bleCallback);
#endif
    scanHandler_ = std::make_shared<ScanHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    wifiSdkHandler_ =
        std::make_shared<WifiSdkHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
    if (wifiSdkHandler_ != nullptr) {
        wifiSdkHandler_->SendEvent(EVENT_REGISTER_WIFI_CALLBACK, 0, 0);
    }
}

LocatorRequiredDataManager* LocatorRequiredDataManager::GetInstance()
{
    static LocatorRequiredDataManager data;
    return &data;
}

LocatorRequiredDataManager::~LocatorRequiredDataManager()
{
    if (wifiSdkHandler_ != nullptr) {
        wifiSdkHandler_->SendEvent(EVENT_UNREGISTER_WIFI_CALLBACK, 0, 0);
    }
}

void LocatorRequiredDataManager::SyncStillMovementState(bool state)
{
    std::unique_lock<std::mutex> lock(lastStillTimeMutex_);
    if (state) {
        lastStillTime_ = CommonUtils::GetSinceBootTime();
    } else {
        lastStillTime_ = 0;
    }
}

int64_t LocatorRequiredDataManager::GetlastStillTime()
{
    std::unique_lock<std::mutex> lock(lastStillTimeMutex_);
    return lastStillTime_;
}

bool LocatorRequiredDataManager::IsStill()
{
    std::unique_lock<std::mutex> lock(lastStillTimeMutex_);
    return lastStillTime_ > 0;
}

void LocatorRequiredDataManager::SendStartBluetoothScanEvent()
{
    if (scanHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            EVENT_START_BLUETOOTH_SCAN);
        scanHandler_->SendEvent(event);
    }
}

void LocatorRequiredDataManager::SendStopBluetoothScanEvent()
{
    if (scanHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            EVENT_STOP_BLUETOOTH_SCAN);
        scanHandler_->SendEvent(event);
    }
}

void LocatorRequiredDataManager::SendWifiScanEvent()
{
    if (scanHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            EVENT_START_SCAN, 1);
        scanHandler_->SendEvent(event);
    }
}

void LocatorRequiredDataManager::SendGetWifiListEvent(int timeout, bool needRetryScan)
{
    if (timeout > 0 && wifiSdkHandler_->HasInnerEvent(EVENT_GET_WIFI_LIST)) {
        return;
    }
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_GET_WIFI_LIST, needRetryScan);
    if (wifiSdkHandler_ != nullptr) {
        wifiSdkHandler_->SendHighPriorityEvent(event, timeout);
    }
}

void LocatorRequiredDataManager::RemoveGetWifiListEvent()
{
    if (wifiSdkHandler_ != nullptr) {
        wifiSdkHandler_->RemoveEvent(EVENT_GET_WIFI_LIST);
    }
}

__attribute__((no_sanitize("cfi"))) LocationErrCode LocatorRequiredDataManager::RegisterCallback(
    AppIdentity &identity, std::shared_ptr<LocatingRequiredDataConfig>& config, const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr || config == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s nullptr.", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    if (config->GetType() == LocatingRequiredDataType::WIFI) {
#ifdef WIFI_ENABLE
        std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
        lock.lock();
        if (callbacksMap_.size() <= MAX_CALLBACKS_MAP_NUM) {
            callbacksMap_[callback] = identity;
        } else {
            LBSLOGE(LOCATOR, "LocatorRequiredDataManager::RegisterCallback fail,Exceeded the maximum number limit");
            lock.unlock();
            return ERRCODE_SCAN_FAIL;
        }
        LBSLOGD(LOCATOR, "after RegisterCallback, callback size:%{public}s",
            std::to_string(callbacksMap_.size()).c_str());
        if (!IsWifiCallbackRegistered() && wifiSdkHandler_ != nullptr) {
            wifiSdkHandler_->SendEvent(EVENT_REGISTER_WIFI_CALLBACK, 0, 0);
        }
        bool needScan = false;
        if (config->GetNeedStartScan()) {
            needScan = true;
        }
        lock.unlock();
        if (needScan) {
            SendWifiScanEvent();
            SendGetWifiListEvent(DEFAULT_TIMEOUT_4S >= config->GetScanTimeoutMs() ?
                config->GetScanTimeoutMs() : DEFAULT_TIMEOUT_4S, false);
        } else {
            SendGetWifiListEvent(0, false);
        }
#endif
    } else if (config->GetType() == LocatingRequiredDataType::BLUE_TOOTH) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorRequiredDataManager::UnregisterCallback(const sptr<IRemoteObject>& callback)
{
#ifdef WIFI_ENABLE
    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = callbacksMap_.find(callback);
    if (iter != callbacksMap_.end()) {
        callbacksMap_.erase(iter);
    }
    LBSLOGD(LOCATOR, "after UnregisterCallback,  callback size:%{public}s",
        std::to_string(callbacksMap_.size()).c_str());
#endif
    return ERRCODE_SUCCESS;
}

void LocatorRequiredDataManager::StartScanBluetoohDevice(sptr<IBluetoohScanResultCallback> callback,
    AppIdentity identity)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s.callback == nullptr", __func__);
        return;
    }
    if (!CheckScanWhiteList(identity.GetBundleName(), TYPE_WHITE_LIST_BLE)) {
        return;
    }
#ifdef BLUETOOTH_ENABLE
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow)
        BluetoohScanCallbackDeathRecipient());
    if (callback->AsObject() != nullptr) {
        callback->AsObject()->AddDeathRecipient(death);
    }
    {
        std::lock_guard<std::mutex> lock(bluetoohcallbacksMapMutex_);
        if (bluetoohcallbacksMap_.size() <= MAX_CALLBACKS_MAP_NUM) {
            bluetoohcallbacksMap_.insert(std::make_pair(callback->AsObject(), std::make_pair(identity, death)));
        } else {
            LBSLOGE(LOCATOR, "%{public}s fail,Exceeded the maximum number limit", __func__);
            return;
        }
        LBSLOGI(LOCATOR, "after StartScanBluetoohDevice, callback size:%{public}zu",
            bluetoohcallbacksMap_.size());
    }
    SendStartBluetoothScanEvent();
#endif
}

void LocatorRequiredDataManager::StopScanBluetoohDevice(sptr<IRemoteObject> callbackObj)
{
#ifdef BLUETOOTH_ENABLE
    LBSLOGE(LOCATOR, "%{public}s.", __func__);
    RemoveBluetoohScanCallbackDeathRecipientByCallback(callbackObj);
    RemoveBluetoohScanCallback(callbackObj);
    SendStopBluetoothScanEvent();
#endif
}

void LocatorRequiredDataManager::RemoveBluetoohScanCallback(sptr<IRemoteObject> callbackObj)
{
    if (callbackObj == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s, callbackObj is nullptr", __func__);
        return;
    }
    std::unique_lock<std::mutex> lock(bluetoohcallbacksMapMutex_);
    auto iter = bluetoohcallbacksMap_.find(callbackObj);
    if (iter != bluetoohcallbacksMap_.end()) {
        bluetoohcallbacksMap_.erase(iter);
    }
    LBSLOGI(LOCATOR, "after RemoveBluetoohScanCallback, callback size:%{public}zu",
        bluetoohcallbacksMap_.size());
}

void LocatorRequiredDataManager::RemoveBluetoohScanCallbackDeathRecipientByCallback(sptr<IRemoteObject> callbackObj)
{
    if (callbackObj == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s, callbackObj is nullptr", __func__);
        return;
    }
    {
        std::unique_lock<std::mutex> lock(bluetoohcallbacksMapMutex_);
        for (auto iter = bluetoohcallbacksMap_.begin(); iter != bluetoohcallbacksMap_.end();) {
            auto callback = iter->first;
            auto deathRecipientPair = iter->second;
            auto death = deathRecipientPair.second;
            if (callbackObj == callback) {
                callback->RemoveDeathRecipient(death);
                break;
            } else {
                iter++;
            }
        }
    }
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

std::unique_ptr<BluetoothScanResult> LocatorBleCallbackWapper::GetBluetoohScanResultByBle(
    const Bluetooth::BleScanResult &result)
{
    std::unique_ptr<BluetoothScanResult> res = std::make_unique<BluetoothScanResult>();
    Bluetooth::BluetoothRemoteDevice peripheralDevice = result.GetPeripheralDevice();
    res->SetDeviceId(result.GetPeripheralDevice().GetDeviceAddr());
    res->SetDeviceName(result.GetPeripheralDevice().GetDeviceName());
    res->SetRssi(result.GetRssi());
    res->SetConnectable(result.IsConnectable());
    res->SetData(result.GetPayload());
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
    std::unique_ptr<BluetoothScanResult> res = GetBluetoohScanResultByBle(result);
    LocatorRequiredDataManager::GetInstance()->ReportBluetoohScanResult(res);
}

void LocatorBleCallbackWapper::OnFoundOrLostCallback(const Bluetooth::BleScanResult &result, uint8_t callbackType) {}

void LocatorBleCallbackWapper::OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results) {}

void LocatorBleCallbackWapper::OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan) {}

void LocatorBleCallbackWapper::OnNotifyMsgReportFromLpDevice(const Bluetooth::UUID &btUuid, int msgType,
    const std::vector<uint8_t> &value) {}
#endif

#ifdef WIFI_ENABLE
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
}

bool LocatorRequiredDataManager::GetLocatingRequiredDataByWifi(
    std::vector<std::shared_ptr<LocatingRequiredData>>& requiredData,
    const std::vector<Wifi::WifiScanInfo>& wifiScanInfo)
{
    auto deltaMis = (CommonUtils::GetSinceBootTime() - GetWifiScanCompleteTimestamp()) / NANOS_PER_MICRO;
    int validTimes = 0;
    for (size_t i = 0; i < wifiScanInfo.size(); i++) {
        std::shared_ptr<LocatingRequiredData> info = std::make_shared<LocatingRequiredData>();
        std::shared_ptr<WifiScanInfo> wifiData = std::make_shared<WifiScanInfo>();
        wifiData->SetSsid(wifiScanInfo[i].ssid);
        wifiData->SetBssid(wifiScanInfo[i].bssid);
        wifiData->SetRssi(wifiScanInfo[i].rssi);
        wifiData->SetFrequency(wifiScanInfo[i].frequency);
        if (!IsStill()) {
            wifiData->SetTimestamp(wifiScanInfo[i].timestamp);
        } else {
            wifiData->SetTimestamp(wifiScanInfo[i].timestamp + deltaMis);
        }
        if (((CommonUtils::GetSinceBootTime() / NANOS_PER_MICRO) - wifiData->GetTimestamp()) <=
            DEFAULT_INVALID_10_SECONDS) {
            validTimes ++;
        }
        info->SetType(LocatingRequiredDataType::WIFI);
        info->SetWifiScanInfo(wifiData);
        requiredData.push_back(info);
    }
    if (validTimes > 0) {
        return true;
    }
    return false;
}

void LocatorRequiredDataManager::UpdateWifiScanCompleteTimestamp()
{
    std::unique_lock<std::mutex> lock(wifiScanCompleteTimestampMutex_);
    wifiScanCompleteTimestamp_ = CommonUtils::GetSinceBootTime();
}

int64_t LocatorRequiredDataManager::GetWifiScanCompleteTimestamp()
{
    std::unique_lock<std::mutex> lock(wifiScanCompleteTimestampMutex_);
    return wifiScanCompleteTimestamp_;
}

void LocatorRequiredDataManager::UpdateQueryScanWhitListTimestamp()
{
    queryScanWhitListTimestamp_ = CommonUtils::GetSinceBootTime();
}

int64_t LocatorRequiredDataManager::GetQueryScanWhitListTimestamp()
{
    return queryScanWhitListTimestamp_;
}

void LocatorWifiScanEventCallback::OnWifiScanStateChanged(int state, int size)
{
    LBSLOGD(LOCATOR, "OnWifiScanStateChanged state=%{public}d", state);
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    if (state == 0) {
        LBSLOGE(LOCATOR, "OnWifiScanStateChanged false");
        dataManager->SendGetWifiListEvent(0, false);
    } else {
        dataManager->UpdateWifiScanCompleteTimestamp();
        dataManager->SendGetWifiListEvent(0, true);
    }
    return;
}
#endif

void LocatorRequiredDataManager::ReportData(const std::vector<std::shared_ptr<LocatingRequiredData>>& result)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (const auto& pair : callbacksMap_) {
        auto callback = pair.first;
        sptr<ILocatingRequiredDataCallback> locatingRequiredDataCallback =
            iface_cast<ILocatingRequiredDataCallback>(callback);
        if (locatingRequiredDataCallback == nullptr) {
            LBSLOGW(LOCATOR, "ReportData nullptr callback.");
            continue;
        }
        AppIdentity identity = pair.second;
        if (CommonUtils::IsAppBelongCurrentAccount(identity)) {
            locatingRequiredDataCallback->OnLocatingDataChange(result);
        }
    }
}

void LocatorRequiredDataManager::ReportBluetoohScanResult(
    const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult)
{
    std::unique_lock<std::mutex> lock(bluetoohcallbacksMapMutex_);
    for (const auto& pair : bluetoohcallbacksMap_) {
        auto callback = pair.first;
        sptr<IBluetoohScanResultCallback> bluetoohScanResultCallback =
            iface_cast<IBluetoohScanResultCallback>(callback);
        if (bluetoohScanResultCallback == nullptr) {
            LBSLOGW(LOCATOR, "ReportBluetoohScanResult nullptr callback.");
            continue;
        }
        auto deathRecipientPair = pair.second;
        AppIdentity identity = deathRecipientPair.first;
        if (CommonUtils::IsAppBelongCurrentAccount(identity) && CheckLocationPermission(identity) &&
            CheckScanWhiteList(identity.GetBundleName(), TYPE_WHITE_LIST_BLE) &&
            !LocatorAbility::GetInstance()->IsProxyPid(identity.GetPid())) {
            bluetoohScanResultCallback->OnBluetoohScanResultChange(bluetoothScanResult);
        }
    }
}

bool LocatorRequiredDataManager::CheckLocationPermission(AppIdentity &identity)
{
    uint32_t callingTokenId = identity.GetTokenId();
    uint32_t callingFirstTokenid = identity.GetFirstTokenId();
    if (!PermissionManager::CheckLocationPermission(callingTokenId, callingFirstTokenid) &&
        !PermissionManager::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        LBSLOGE(LOCATOR, "%{public}d %{public}s failed", callingTokenId, __func__);
        return false;
    } else {
        return true;
    }
}

bool LocatorRequiredDataManager::CheckScanWhiteList(const std::string& bundleName, const std::string& type)
{
    std::string scanWhiteList = GetScanWhiteListStr();
    if (scanWhiteList.empty()) {
        LBSLOGE(LOCATOR, "%{public}s, can not Query Scan White List", __func__);
        return false;
    }
    nlohmann::json whiteList = nlohmann::json::parse(scanWhiteList);
    if (whiteList.contains(type) && whiteList[type].is_array()) {
        for (const auto& item : whiteList[type]) {
            if (bundleName.compare(item.get<std::string>()) == 0) {
                return true;
            }
        }
    }
    return false;
}

std::string LocatorRequiredDataManager::GetScanWhiteListStr()
{
    int64_t currentTime = CommonUtils::GetSinceBootTime();
    if (GetQueryScanWhitListTimestamp() == 0 ||
        (currentTime - GetQueryScanWhitListTimestamp()) / NANOS_PER_MICRO / MICRO_PER_MILLI >=
        DEFAULT_INVALID_12_HOURS) {
        std::string queryScanWhiteList = LocationDataRdbManager::QueryScanWhiteList();
        UpdateQueryScanWhitListTimestamp();
        scanWhiteListStr_ = queryScanWhiteList;
    }
    return scanWhiteListStr_;
}

__attribute__((no_sanitize("cfi"))) void LocatorRequiredDataManager::StartWifiScan(int fixNumber, bool flag)
{
#ifdef WIFI_ENABLE
    int64_t currentTime = CommonUtils::GetSinceBootTime();
    if (IsStill() && GetWifiScanCompleteTimestamp() > GetlastStillTime() &&
        (currentTime - GetWifiScanCompleteTimestamp()) / NANOS_PER_MICRO < DEFAULT_TIMEOUT_30_MIN) {
        SendGetWifiListEvent(0, true);
        return;
    }
    int ret = TriggerWifiScan();
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "%{public}s WifiScan failed, ret=%{public}d", __func__, ret);
        SendGetWifiListEvent(0, false);
    }
#endif
}

void LocatorRequiredDataManager::StartBluetoothScan()
{
    if (bleCentralManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s, bleCentralManager_ == nullptr", __func__);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(bluetoohcallbacksMapMutex_);
        if (bluetoohcallbacksMap_.size() > 1) {
            return;
        }
    }
    Bluetooth::BleScanSettings settings;
    settings.SetScanMode(Bluetooth::SCAN_MODE::SCAN_MODE_LOW_LATENCY);
    std::vector<Bluetooth::BleScanFilter> filters;
    Bluetooth::BleScanFilter scanFilter;
    filters.push_back(scanFilter);

    bleCentralManager_->StartScan(settings, filters);
}

void LocatorRequiredDataManager::StoptBluetoothScan()
{
    if (bleCentralManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s, bleCentralManager_ == nullptr", __func__);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(bluetoohcallbacksMapMutex_);
        if (bluetoohcallbacksMap_.size() != 0) {
            return;
        }
    }
    bleCentralManager_->StopScan();
}

#ifdef WIFI_ENABLE
int LocatorRequiredDataManager::TriggerWifiScan()
{
    wifiScanStartTimeStamp_ = CommonUtils::GetSinceBootTime() / NANOS_PER_MICRO;
    return Wifi::WifiScan::GetInstance(WIFI_SCAN_ABILITY_ID)->Scan();
}
#endif

bool LocatorRequiredDataManager::IsConnecting()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (callbacksMap_.size() > 0) {
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

ScanHandler::ScanHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitScanHandlerEventMap();
}

ScanHandler::~ScanHandler() {}

void ScanHandler::InitScanHandlerEventMap()
{
    if (scanHandlerEventMap_.size() != 0) {
        return;
    }
    scanHandlerEventMap_[EVENT_START_SCAN] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { StartScanEvent(event); };
    scanHandlerEventMap_[EVENT_START_BLUETOOTH_SCAN] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { StartBluetoothScanEvent(event); };
    scanHandlerEventMap_[EVENT_STOP_BLUETOOTH_SCAN] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { StopBluetoothScanEvent(event); };
}

void ScanHandler::StartBluetoothScanEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    dataManager->StartBluetoothScan();
}

void ScanHandler::StopBluetoothScanEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    dataManager->StoptBluetoothScan();
}

void ScanHandler::StartScanEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    int fixNumber = event->GetParam();
    dataManager->StartWifiScan(fixNumber, true);
}

void ScanHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(LOCATOR, "ScanHandler processEvent event:%{public}d, timestamp = %{public}s",
        eventId, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
    auto handleFunc = scanHandlerEventMap_.find(eventId);
    if (handleFunc != scanHandlerEventMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
#ifdef LOCATION_HICOLLIE_ENABLE
        int tid = gettid();
        std::string moduleName = "ScanHandler";
        XCollieCallback callbackFunc = [moduleName, eventId, tid](void *) {
            LBSLOGE(LOCATOR, "TimeoutCallback tid:%{public}d moduleName:%{public}s excute eventId:%{public}u timeout.",
                tid, moduleName.c_str(), eventId);
        };
        std::string dfxInfo = moduleName + "_" + std::to_string(eventId) + "_" + std::to_string(tid);
        int timerId = HiviewDFX::XCollie::GetInstance().SetTimer(dfxInfo, TIMEOUT_WATCHDOG, callbackFunc, nullptr,
            HiviewDFX::XCOLLIE_FLAG_LOG|HiviewDFX::XCOLLIE_FLAG_RECOVERY);
        memberFunc(event);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
#else
        memberFunc(event);
#endif
    }
}

WifiSdkHandler::WifiSdkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitWifiSdkHandlerEventMap();
}

WifiSdkHandler::~WifiSdkHandler() {}

void WifiSdkHandler::InitWifiSdkHandlerEventMap()
{
    if (wifiSdkHandlerEventMap_.size() != 0) {
        return;
    }
    wifiSdkHandlerEventMap_[EVENT_GET_WIFI_LIST] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { GetWifiListEvent(event); };
    wifiSdkHandlerEventMap_[EVENT_REGISTER_WIFI_CALLBACK] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { RegisterWifiCallbackEvent(event); };
    wifiSdkHandlerEventMap_[EVENT_UNREGISTER_WIFI_CALLBACK] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { UnregisterWifiCallbackEvent(event); };
}

void WifiSdkHandler::GetWifiListEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    bool needRetryScan = event->GetParam();
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    if (!dataManager->IsConnecting()) {
        LBSLOGE(LOCATOR, "%{public}s no valid callback, return", __func__);
        return;
    }
    std::vector<Wifi::WifiScanInfo> wifiScanInfo;
    dataManager->GetWifiScanList(wifiScanInfo);
    std::vector<std::shared_ptr<LocatingRequiredData>> requiredData;
    bool requiredDataValid = dataManager->GetLocatingRequiredDataByWifi(requiredData, wifiScanInfo);
    if (needRetryScan && !requiredDataValid &&
        CommonUtils::GetSinceBootTime() / NANOS_PER_MICRO - dataManager->wifiScanStartTimeStamp_ >
        DEFAULT_NOT_RETRY_TIME_10_SECONDS) {
        LBSLOGI(LOCATOR, "%{public}s retry scan", __func__);
        int ret = dataManager->TriggerWifiScan();
        if (ret != Wifi::WIFI_OPT_SUCCESS) {
            LBSLOGE(LOCATOR, "%{public}s retry WifiScan failed, ret=%{public}d", __func__, ret);
            dataManager->ReportData(requiredData);
            dataManager->RemoveGetWifiListEvent();
        }
    } else {
        dataManager->ReportData(requiredData);
        dataManager->RemoveGetWifiListEvent();
    }
}

void WifiSdkHandler::RegisterWifiCallbackEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    dataManager->RegisterWifiCallBack();
}

void WifiSdkHandler::UnregisterWifiCallbackEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto dataManager = LocatorRequiredDataManager::GetInstance();
    dataManager->UnregisterWifiCallBack();
}

void WifiSdkHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(LOCATOR, "WifiSdkHandler processEvent event:%{public}d, timestamp = %{public}s",
        eventId, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
    auto handleFunc = wifiSdkHandlerEventMap_.find(eventId);
    if (handleFunc != wifiSdkHandlerEventMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
#ifdef LOCATION_HICOLLIE_ENABLE
        int tid = gettid();
        std::string moduleName = "WifiSdkHandler";
        XCollieCallback callbackFunc = [moduleName, eventId, tid](void *) {
            LBSLOGE(LOCATOR, "TimeoutCallback tid:%{public}d moduleName:%{public}s excute eventId:%{public}u timeout.",
                tid, moduleName.c_str(), eventId);
        };
        std::string dfxInfo = moduleName + "_" + std::to_string(eventId) + "_" + std::to_string(tid);
        int timerId = HiviewDFX::XCollie::GetInstance().SetTimer(dfxInfo, TIMEOUT_WATCHDOG, callbackFunc, nullptr,
            HiviewDFX::XCOLLIE_FLAG_LOG|HiviewDFX::XCOLLIE_FLAG_RECOVERY);
        memberFunc(event);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
#else
        memberFunc(event);
#endif
    }
}

BluetoohScanCallbackDeathRecipient::BluetoohScanCallbackDeathRecipient()
{
}
 
BluetoohScanCallbackDeathRecipient::~BluetoohScanCallbackDeathRecipient()
{
}
 
void BluetoohScanCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LBSLOGI(LOCATOR, "bluetooh scan callback OnRemoteDied");
    LocatorRequiredDataManager::GetInstance()->StopScanBluetoohDevice(remote.promote());
}
} // namespace Location
} // namespace OHOS
