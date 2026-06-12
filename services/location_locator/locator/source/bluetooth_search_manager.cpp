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

#include "bluetooth_search_manager.h"

#include "location_log.h"
#include "common_utils.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {

BluetoothSearchManager::BluetoothSearchManager()
    : bluetoothSearchScanStatus_(false)
{
#ifdef BLUETOOTH_ENABLE
    scanCallback_ = nullptr;
    bleCentralManager_ = nullptr;
#endif
}

BluetoothSearchManager::~BluetoothSearchManager()
{
    UninitBleManager();
}

void BluetoothSearchManager::InitBleManager()
{
#ifdef BLUETOOTH_ENABLE
    if (bleCentralManager_ != nullptr) {
        return;
    }
    Bluetooth::BluetoothState state = Bluetooth::BluetoothHost::GetDefaultHost().GetBluetoothState();
    if (state != Bluetooth::BluetoothState::STATE_ON) {
        LBSLOGE(LOCATOR, "%{public}s BT_SEARCH_LOG bluetooth is off, state=%{public}d", __func__, state);
        return;
    }
    scanCallback_ = std::make_shared<BluetoothSearchScanCallback>();
    bleCentralManager_ = std::make_shared<Bluetooth::BleCentralManager>(scanCallback_);
    if (bleCentralManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s bleCentralManager_ is nullptr", __func__);
        scanCallback_ = nullptr;
        return;
    }
#endif
}

void BluetoothSearchManager::UninitBleManager()
{
#ifdef BLUETOOTH_ENABLE
    if (bleCentralManager_ != nullptr && bluetoothSearchScanStatus_) {
        bleCentralManager_->StopScan();
        bluetoothSearchScanStatus_ = false;
    }
    scanCallback_ = nullptr;
    bleCentralManager_ = nullptr;
#endif
}

bool BluetoothSearchManager::MatchFilter(const BluetoothScanResult& result,
    const BluetoothSearchRequestParams& params)
{
    int32_t rssi = result.GetRssi();
    if (rssi < params.rssiThreshold) {
        LBSLOGD(LOCATOR, "MatchFilter filtered by RSSI: rssi=%{public}d, threshold=%{public}d",
            rssi, params.rssiThreshold);
        return false;
    }

    if (!params.deviceIdArray.empty()) {
        std::string mac = result.GetDeviceId();
        bool found = false;
        for (const auto& allowedMac : params.deviceIdArray) {
            if (mac == allowedMac) {
                found = true;
                break;
            }
        }
        if (!found) {
            LBSLOGD(LOCATOR, "MatchFilter filtered by MAC: %{public}s not in whitelist", mac.c_str());
            return false;
        }
    }

    return true;
}

void BluetoothSearchManager::StartBluetoothSearch(sptr<IBluetoothScanResultCallback> callback,
    AppIdentity identity, const BluetoothSearchRequestParams& params)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s callback == nullptr", __func__);
        return;
    }

#ifdef BLUETOOTH_ENABLE
    InitBleManager();
    if (bleCentralManager_ == nullptr) {
        LBSLOGE(LOCATOR, "BT_SEARCH_LOG %{public}s bleCentralManager_ == nullptr", __func__);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(callbacksMapMutex_);
        if (bluetoothSearchCallbacksMap_.size() < 5) {
            auto callbackObj = callback->AsObject();
            auto deathRecipient = new (std::nothrow) BluetoothSearchCallbackDeathRecipient();
            if (deathRecipient != nullptr) {
                callbackObj->AddDeathRecipient(deathRecipient);
                bluetoothSearchCallbacksMap_.insert(std::make_pair(
                    callbackObj, std::make_pair(identity, std::make_pair(params, sptr<IRemoteObject::DeathRecipient>(deathRecipient)))));
            }
        } else {
            LBSLOGE(LOCATOR, "BT_SEARCH_LOG %{public}s fail,Exceeded the maximum number limit", __func__);
            return;
        }
    }
    Bluetooth::BleScanSettings settings;
    settings.SetScanMode(Bluetooth::SCAN_MODE::SCAN_MODE_LOW_POWER);
    std::vector<Bluetooth::BleScanFilter> filters;
    Bluetooth::BleScanFilter scanFilter;
    filters.push_back(scanFilter);
    int ret = bleCentralManager_->StartScan(settings, filters);
    if (ret == 0) {
        std::lock_guard<std::mutex> lock(bluetoothSearchScanStatusMutex_);
        bluetoothSearchScanStatus_ = true;
    } else {
        LBSLOGE(LOCATOR, "BT_SEARCH_LOG %{public}s StartScan failed, ret=%{public}d", __func__, ret);
        std::lock_guard<std::mutex> lock(callbacksMapMutex_);
        auto callbackObj = callback->AsObject();
        auto it = bluetoothSearchCallbacksMap_.find(callbackObj);
        if (it != bluetoothSearchCallbacksMap_.end()) {
            callbackObj->RemoveDeathRecipient(it->second.second.second.GetRefPtr());
            bluetoothSearchCallbacksMap_.erase(it);
        }
    }
#endif
}

void BluetoothSearchManager::StopBluetoothSearch(IRemoteObject* remote)
{
    if (remote == nullptr) {
        LBSLOGE(LOCATOR, "BT_SEARCH_LOG StopBluetoothSearch remote is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(callbacksMapMutex_);
    auto it = bluetoothSearchCallbacksMap_.find(remote);
    if (it != bluetoothSearchCallbacksMap_.end()) {
        remote->RemoveDeathRecipient(it->second.second.second.GetRefPtr());
        bluetoothSearchCallbacksMap_.erase(it);
    }
    LBSLOGI(LOCATOR, "BT_SEARCH_LOG StopBluetoothSearch, size:%{public}zu, status = %{public}d",
            bluetoothSearchCallbacksMap_.size(), bluetoothSearchScanStatus_);
    if (bluetoothSearchCallbacksMap_.empty()) {
        std::lock_guard<std::mutex> lock(bluetoothSearchScanStatusMutex_);
        if (bluetoothSearchScanStatus_) {
#ifdef BLUETOOTH_ENABLE
            if (bleCentralManager_ != nullptr) {
                bleCentralManager_->StopScan();
            }
#endif
            bluetoothSearchScanStatus_ = false;
            LBSLOGI(LOCATOR, "BT_SEARCH_LOG %{public}s StopScan", __func__);
        }
    }
}

void BluetoothSearchManager::ReportBluetoothScanResult(
    const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult)
{
    if (bluetoothScanResult == nullptr) {
        LBSLOGW(LOCATOR, "BT_SEARCH_LOG ReportBluetoothScanResult bluetoothScanResult is nullptr");
        return;
    }
    std::unique_lock<std::mutex> lock(callbacksMapMutex_);
    for (const auto& pair : bluetoothSearchCallbacksMap_) {
        auto callback = pair.first;
        sptr<IBluetoothScanResultCallback> bluetoothScanResultCallback =
            iface_cast<IBluetoothScanResultCallback>(callback);
        if (bluetoothScanResultCallback == nullptr) {
            continue;
        }

        AppIdentity identity = pair.second.first;
        BluetoothSearchRequestParams params = pair.second.second.first;

        if (MatchFilter(*bluetoothScanResult, params)) {
            bluetoothScanResultCallback->OnBluetoothScanResultChange(bluetoothScanResult);
        }
    }
}

#ifdef BLUETOOTH_ENABLE
void BluetoothSearchManager::BluetoothSearchScanCallback::OnScanCallback(const Bluetooth::BleScanResult& result)
{
    std::unique_ptr<BluetoothScanResult> bluetoothScanResult = std::make_unique<BluetoothScanResult>();
    if (bluetoothScanResult == nullptr) {
        LBSLOGE(LOCATOR, "BT_SEARCH_LOG BluetoothSearchScanCallback::OnScanCallback bluetoothScanResult is nullptr");
        return;
    }
    bluetoothScanResult->SetDeviceId(result.GetPeripheralDevice().GetDeviceAddr());
    bluetoothScanResult->SetDeviceName(const_cast<Bluetooth::BleScanResult&>(result).GetName());
    bluetoothScanResult->SetRssi(result.GetRssi());
    bluetoothScanResult->SetConnectable(result.IsConnectable());
    bluetoothScanResult->SetData(result.GetPayload());
    BluetoothSearchManager::GetInstance().ReportBluetoothScanResult(bluetoothScanResult);
}

void BluetoothSearchManager::BluetoothSearchScanCallback::OnFoundOrLostCallback(
    const Bluetooth::BleScanResult& result, uint8_t callbackType)
{
    std::unique_ptr<BluetoothScanResult> bluetoothScanResult = std::make_unique<BluetoothScanResult>();
    if (bluetoothScanResult == nullptr) {
        return;
    }
    bluetoothScanResult->SetDeviceId(result.GetPeripheralDevice().GetDeviceAddr());
    bluetoothScanResult->SetDeviceName(const_cast<Bluetooth::BleScanResult&>(result).GetName());
    bluetoothScanResult->SetRssi(result.GetRssi());
    bluetoothScanResult->SetConnectable(result.IsConnectable());
    bluetoothScanResult->SetData(result.GetPayload());

    BluetoothSearchManager::GetInstance().ReportBluetoothScanResult(bluetoothScanResult);
}

void BluetoothSearchManager::BluetoothSearchScanCallback::OnBleBatchScanResultsEvent(
    const std::vector<Bluetooth::BleScanResult>& results)
{
    for (const auto& result : results) {
        std::unique_ptr<BluetoothScanResult> bluetoothScanResult = std::make_unique<BluetoothScanResult>();
        if (bluetoothScanResult == nullptr) {
            continue;
        }
        bluetoothScanResult->SetDeviceId(result.GetPeripheralDevice().GetDeviceAddr());
        bluetoothScanResult->SetDeviceName(const_cast<Bluetooth::BleScanResult&>(result).GetName());
        bluetoothScanResult->SetRssi(result.GetRssi());
        bluetoothScanResult->SetConnectable(result.IsConnectable());
        bluetoothScanResult->SetData(result.GetPayload());

        BluetoothSearchManager::GetInstance().ReportBluetoothScanResult(bluetoothScanResult);
    }
}

void BluetoothSearchManager::BluetoothSearchScanCallback::OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan)
{
}

void BluetoothSearchManager::BluetoothSearchCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    BluetoothSearchManager::GetInstance().StopBluetoothSearch(remote.promote());
}
#endif

} // namespace Location
} // namespace OHOS