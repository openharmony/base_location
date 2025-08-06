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
#include "beacon_fence_manager.h"
#include "location_log.h"
#include "common_utils.h"
#include "hook_utils.h"
#include <sstream>
#include <iomanip>
#include "location_data_rdb_manager.h"
#include "proxy_freeze_manager.h"
#include "locator_background_proxy.h"

namespace OHOS {
namespace Location {
const int32_t MAX_REQUEST_MAP_NUM = 16;
constexpr int32_t FENCE_MAX_ID = 1000000;
const int32_t STOI_BYTE_LIMIT = 7;
const int BEACON_FENCE_OPERATE_RESULT_ENTER = 1;
const int BEACON_FENCE_OPERATE_RESULT_EXIT = 2;
const int BEACON_FENCE_DATA_LENGTH = 18;
const int BEACON_FENCE_UUID_OFFSET_2 = 2;
const int BEACON_FENCE_UUID_OFFSET_6 = 6;
const int BEACON_FENCE_UUID_OFFSET_8 = 8;
const int BEACON_FENCE_UUID_OFFSET_10 = 10;
const int BEACON_FENCE_UUID_OFFSET_12 = 12;

BeaconFenceManager::BeaconFenceManager()
{
    beaconFenceId_ = 0;
#ifdef BLUETOOTH_ENABLE
    std::shared_ptr<BeaconBleCallbackWapper> bleCallback = std::make_shared<BeaconBleCallbackWapper>();
    bleCentralManager_ = std::make_shared<Bluetooth::BleCentralManager>(bleCallback);
#endif
}

BeaconFenceManager* BeaconFenceManager::GetInstance()
{
    static BeaconFenceManager data;
    return &data;
}

BeaconFenceManager::~BeaconFenceManager() {}

int32_t BeaconFenceManager::GenerateBeaconFenceId()
{
    beaconFenceId_++;
    std::int32_t id = beaconFenceId_;
    return id;
}

ErrCode BeaconFenceManager::AddBeaconFence(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest,
    const AppIdentity& identity)
{
    int beaconFenceId = GenerateBeaconFenceId();
    beaconFenceRequest->SetFenceId(std::to_string(beaconFenceId));

    // 解析请求数据
    std::shared_ptr<BeaconFence> beaconFence = beaconFenceRequest->GetBeaconFence();
    BeaconManufactureData beaconManufactureData = beaconFence->GetBeaconManufactureData();
    std::string uuid = ExtractiBeaconUUID(beaconManufactureData.manufactureData);
    std::string uuidMask = ExtractiBeaconUUID(beaconManufactureData.manufactureDataMask);

    beaconFenceRequest->SetServiceUuid(uuid);
    beaconFenceRequest->SetServiceUuidMask(uuidMask);
#ifdef BLUETOOTH_ENABLE
    std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
    if (beaconFenceRequestMap_.size() >= MAX_REQUEST_MAP_NUM) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, beaconfence request is exceed max number!", __func__);
        return ERRCODE_BEACONFENCE_EXCEED_MAXIMUM;
    }
    if (isBeaconFenceRequestExists(beaconFenceRequest)) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, Request is exists!", __func__);
        return ERRCODE_BEACONFENCE_DUPLICATE_INFORMATION;
    }
    RegisterBeaconFenceCallback(beaconFenceRequest, identity);
    StartAddBeaconFence(beaconFenceRequest, identity);
#endif
    return ERRCODE_SUCCESS;
}

void BeaconFenceManager::RegisterBeaconFenceCallback(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest,
    const AppIdentity& appIdentity)
{
    sptr<IRemoteObject> callback = beaconFenceRequest->GetBeaconFenceTransitionCallback();
    if (callback != nullptr) {
        sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) BeaconFenceCallbackDeathRecipient());
        callback->AddDeathRecipient(death);
    }
    beaconFenceRequestMap_.insert(std::make_pair(beaconFenceRequest, std::make_pair(callback, appIdentity)));
    LBSLOGI(BEACON_FENCE_MANAGER, "after AddBeaconFence, request size:%{public}zu",
        beaconFenceRequestMap_.size());
}

bool BeaconFenceManager::isBeaconFenceRequestExists(const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest)
{
    for (auto iter = beaconFenceRequestMap_.begin(); iter != beaconFenceRequestMap_.end(); iter++) {
        auto request = iter->first;
        if (CompareUUID(request->GetServiceUuid(), beaconFenceRequest->GetServiceUuid()) &&
            request->GetBundleName().compare(beaconFenceRequest->GetBundleName()) == 0) {
            return true;
        }
    }
    return false;
}

void BeaconFenceManager::StartAddBeaconFence(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest,
    const AppIdentity& identity)
{
    StopBluetoothScan();
    StartBluetoothScan();
    OnReportOperationResultByCallback(
        beaconFenceRequest, GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD,
        GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);
}

ErrCode BeaconFenceManager::RemoveBeaconFence(const std::shared_ptr<BeaconFence>& beaconFence)
{
    // 解析请求数据
    BeaconManufactureData beaconManufactureData = beaconFence->GetBeaconManufactureData();
    std::string uuid = ExtractiBeaconUUID(beaconManufactureData.manufactureData);
    std::string uuidMask = ExtractiBeaconUUID(beaconManufactureData.manufactureDataMask);

    // 判断是否在请求集合
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = GetBeaconFenceRequestByServiceUuid(uuid);
    if (beaconFenceRequest == nullptr) {
        LBSLOGE(BEACON_FENCE_MANAGER, "beaconFence is not registered");
        return ERRCODE_BEACONFENCE_INCORRECT_ID;
    }
    if (!CompareBeaconFence(beaconFence, beaconFenceRequest->GetBeaconFence())) {
        LBSLOGE(BEACON_FENCE_MANAGER, "CompareBeaconFence false");
        return ERRCODE_BEACONFENCE_INCORRECT_ID;
    }
    RemoveBeaconFenceRequestByBeacon(beaconFence);
#ifdef BLUETOOTH_ENABLE
    StopBluetoothScan();
    {
        std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
        LBSLOGI(BEACON_FENCE_MANAGER, "after RemoveBeaconFence, request size:%{public}zu",
            beaconFenceRequestMap_.size());
        if (beaconFenceRequestMap_.size() != 0) {
            StartBluetoothScan();
        }
    }
    OnReportOperationResultByCallback(
        beaconFenceRequest, GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE,
        GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);
#endif
    return ERRCODE_SUCCESS;
}

#ifdef BLUETOOTH_ENABLE
void BeaconFenceManager::StartBluetoothScan()
{
    if (bleCentralManager_ == nullptr) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, bleCentralManager_ == nullptr", __func__);
        return;
    }
    Bluetooth::BleScanSettings settings;
    settings.SetScanMode(Bluetooth::SCAN_MODE::SCAN_MODE_LOW_POWER);
    // 仅进入，或者丢失时上报,
    settings.SetReportMode(static_cast<int>(Bluetooth::REPORT_MODE::REPORT_MODE_FENCE_SENSITIVITY_LOW));
    settings.SetCallbackType(Bluetooth::BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH);

    // 添加过滤条件
    std::vector<Bluetooth::BleScanFilter> filters;
    ConstructFilter(filters);
    bleCentralManager_->StartScan(settings, filters);
}

void BeaconFenceManager::StopBluetoothScan()
{
    if (bleCentralManager_ == nullptr) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, bleCentralManager_ == nullptr", __func__);
        return;
    }
    bleCentralManager_->StopScan();
}

void BeaconFenceManager::ConstructFilter(std::vector<Bluetooth::BleScanFilter>& filters)
{
    std::vector<BeaconManufactureData> beaconManufactureDataVector = GetBeaconManufactureDataForFilter();
    for (const auto& data : beaconManufactureDataVector) {
        Bluetooth::BleScanFilter scanFilter;
        scanFilter.SetManufacturerId(data.manufactureId);
        scanFilter.SetManufactureData(data.manufactureData);
        scanFilter.SetManufactureDataMask(data.manufactureDataMask);
        filters.push_back(scanFilter);
    }
}
#endif

void BeaconFenceManager::OnReportOperationResultByCallback(
    const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest,
    GnssGeofenceOperateType type, GnssGeofenceOperateResult result)
{
    auto callback = beaconFenceRequest->GetBeaconFenceTransitionCallback();
    if (callback != nullptr) {
        std::string fenceId = beaconFenceRequest->GetFenceId();
        if (IsStrValidForStoi(fenceId)) {
            sptr<IGnssGeofenceCallback> beaconFenceCallback = iface_cast<IGnssGeofenceCallback>(callback);
            beaconFenceCallback->OnReportOperationResult(stoi(fenceId), type, result);
        }
    }
}

std::string BeaconFenceManager::ExtractiBeaconUUID(const std::vector<uint8_t>& data)
{
    // 数据最小长度检查
    if (data.size() < BEACON_FENCE_DATA_LENGTH) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s manufactureData len invalid", __func__);
        return "";
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    // 提取UUID部分
    for (int i = BEACON_FENCE_UUID_OFFSET_2; i < BEACON_FENCE_DATA_LENGTH; ++i) {
        if (i == BEACON_FENCE_UUID_OFFSET_6 || i == BEACON_FENCE_UUID_OFFSET_8 ||
            i == BEACON_FENCE_UUID_OFFSET_10 || i == BEACON_FENCE_UUID_OFFSET_12) {
                oss << "-";
            }
        oss << std::setw(BEACON_FENCE_UUID_OFFSET_2) << static_cast<int>(data[i]);
    }
    std::string uuid = oss.str();
    return uuid;
}

#ifdef BLUETOOTH_ENABLE
void BeaconFenceManager::ReportFoundOrLost(const Bluetooth::BleScanResult &result, uint8_t type)
{
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = GetBeaconFenceRequestByScanResult(result);
    if (beaconFenceRequest == nullptr) {
        return;
    }
    AppIdentity identity = GetAppIdentityByBeaconFenceRequest(beaconFenceRequest);
    if (type == Bluetooth::BLE_SCAN_CALLBACK_TYPE_FIRST_MATCH) {
        // 首次进入围栏
        TransitionStatusChange(beaconFenceRequest, GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER, identity);
    } else if (type == Bluetooth::BLE_SCAN_CALLBACK_TYPE_LOST_MATCH) {
        // 首次退出围栏
        TransitionStatusChange(beaconFenceRequest, GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_EXIT, identity);
    }
}

std::shared_ptr<BeaconFenceRequest> BeaconFenceManager::GetBeaconFenceRequestByScanResult(
    const Bluetooth::BleScanResult &result)
{
    std::map<uint16_t, std::string> dataMap = result.GetManufacturerData();
    std::string scanData;
    for (const auto& pair : dataMap) {
        scanData = pair.second;
    }

    std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
    for (auto iter = beaconFenceRequestMap_.begin(); iter != beaconFenceRequestMap_.end(); iter++) {
        auto request = iter->first;
        if (MatchesData(request->GetBeaconFence()->GetBeaconManufactureData().manufactureData, scanData)) {
            return request;
        }
    }
    LBSLOGE(BEACON_FENCE_MANAGER, "can not get request by BleScanResult");
    return nullptr;
}

bool BeaconFenceManager::MatchesData(std::vector<uint8_t> fData, std::string scanData)
{
    if (scanData.empty()) {
        return false;
    }
    size_t len = fData.size();
    std::vector<uint8_t> vec(scanData.begin(), scanData.end());
    if (vec.size() < len) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (fData[i] != vec[i]) {
            return false;
        }
    }
    return true;
}
#endif

void BeaconFenceManager::TransitionStatusChange(std::shared_ptr<BeaconFenceRequest> beaconFenceRequest,
    GeofenceTransitionEvent event, const AppIdentity &identity)
{
    // 判断开关状态
    int state = DEFAULT_SWITCH_STATE;
    state = LocationDataRdbManager::QuerySwitchState();
    if (state == DISABLED) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s location switch is off", __func__);
        return;
    }
    // 判断应用是否卸载
    std::shared_ptr<BeaconFence> beacon = beaconFenceRequest->GetBeaconFence();
    if (!CommonUtils::CheckAppInstalled(beaconFenceRequest->GetBundleName())) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s service is not available", __func__);
        RemoveBeaconFence(beacon);
        return;
    }
    // 是否后台
    if (LocatorBackgroundProxy::GetInstance()->IsAppBackground(identity.GetUid(), identity.GetBundleName())) {
        if (beaconFenceRequest->GetFenceExtensionAbilityName().empty()) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s app is background", __func__);
            return;
        }
        if (!HookUtils::ExecuteHookWhenBeaconFenceTransitionStatusChange(identity.GetBundleName())) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s can not start extension", __func__);
            return;
        }
    }
    std::string fenceId = beaconFenceRequest->GetFenceId();
    // 通过callback回调事件
    auto callback = beaconFenceRequest->GetBeaconFenceTransitionCallback();
    if (callback != nullptr) {
        if (IsStrValidForStoi(fenceId)) {
            sptr<IGnssGeofenceCallback> gnssGeofenceCallback = iface_cast<IGnssGeofenceCallback>(callback);
            GeofenceTransition geofenceTransition;
            geofenceTransition.fenceId = stoi(fenceId);
            geofenceTransition.event = event;
            geofenceTransition.beaconFence = beacon;
            gnssGeofenceCallback->OnTransitionStatusChange(geofenceTransition);
        }
    }
    // 拉起fenceExtension
    if (!beaconFenceRequest->GetFenceExtensionAbilityName().empty()) {
        HookUtils::ExecuteHookWhenReportBeaconFenceOperateResult(fenceId, static_cast<int>(event),
            beaconFenceRequest->GetFenceExtensionAbilityName(), beaconFenceRequest->GetBundleName());
    }
}

bool BeaconFenceManager::IsStrValidForStoi(const std::string &str)
{
    if (str.length() > STOI_BYTE_LIMIT) {
        return false;
    }
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

AppIdentity BeaconFenceManager::GetAppIdentityByBeaconFenceRequest(
    const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest)
{
    std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
    AppIdentity appIdentity;
    for (auto iter = beaconFenceRequestMap_.begin(); iter != beaconFenceRequestMap_.end(); iter++) {
        auto request = iter->first;
        if (beaconFenceRequest->GetServiceUuid() == request->GetServiceUuid()) {
            auto callbackPair = iter->second;
            auto identity = callbackPair.second;
            return identity;
        }
    }
    return appIdentity;
}

std::shared_ptr<BeaconFenceRequest> BeaconFenceManager::GetBeaconFenceRequestByServiceUuid(std::string serviceUuid)
{
    std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
    for (auto iter = beaconFenceRequestMap_.begin(); iter != beaconFenceRequestMap_.end(); iter++) {
        auto request = iter->first;
        if (CompareUUID(request->GetServiceUuid(), serviceUuid)) {
            return request;
        }
    }
    LBSLOGE(BEACON_FENCE_MANAGER, "can not get request by serviceUuid");
    return nullptr;
}

std::shared_ptr<BeaconFenceRequest> BeaconFenceManager::GetBeaconFenceRequestByCallback(sptr<IRemoteObject> callbackObj)
{
    std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
    for (auto iter = beaconFenceRequestMap_.begin(); iter != beaconFenceRequestMap_.end(); iter++) {
        auto callbackPair = iter->second;
        auto callback = callbackPair.first;
        if (callback == callbackObj) {
            return iter->first;
        }
    }
    return nullptr;
}

std::shared_ptr<BeaconFenceRequest> BeaconFenceManager::GetBeaconFenceRequestByPackageName(std::string& packageName)
{
    std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
    for (auto iter = beaconFenceRequestMap_.begin(); iter != beaconFenceRequestMap_.end(); iter++) {
        auto callbackPair = iter->second;
        auto identity = callbackPair.second;
        if (packageName.compare(identity.GetBundleName()) == 0) {
            return iter->first;
        }
    }
    return nullptr;
}

void BeaconFenceManager::RemoveBeaconFenceRequestByBeacon(std::shared_ptr<BeaconFence> beaconFence)
{
    std::lock_guard<std::mutex> lock(beaconFenceRequestMapMutex_);
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = GetBeaconFenceRequestByBeacon(beaconFence);
    if (beaconFenceRequest != nullptr) {
        beaconFenceRequestMap_.erase(beaconFenceRequest);
    }
}

std::shared_ptr<BeaconFenceRequest> BeaconFenceManager::GetBeaconFenceRequestByBeacon(
    std::shared_ptr<BeaconFence> beaconFence)
{
    for (auto iter = beaconFenceRequestMap_.begin();
        iter != beaconFenceRequestMap_.end(); iter++) {
        auto request = iter->first;
        auto beacon = request->GetBeaconFence();
        if (CompareBeaconFence(beaconFence, beacon)) {
            return request;
        }
    }
    return nullptr;
}

void BeaconFenceManager::RemoveBeaconFenceRequestByCallback(sptr<IRemoteObject> callbackObj)
{
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = GetBeaconFenceRequestByCallback(callbackObj);
    if (beaconFenceRequest == nullptr) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, beaconFenceRequest == nullptr", __func__);
        return;
    }
    std::string fenceExtensionAbilityName = beaconFenceRequest->GetFenceExtensionAbilityName();
    if (fenceExtensionAbilityName.empty() ||
        !HookUtils::ExecuteHookWhenRemoveBeaconFenceByCallback(beaconFenceRequest->GetBundleName())) {
        std::shared_ptr<BeaconFence> beacon = beaconFenceRequest->GetBeaconFence();
        RemoveBeaconFence(beacon);
    }
}

void BeaconFenceManager::RemoveBeaconFenceByPackageName(std::string& packageName)
{
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = GetBeaconFenceRequestByPackageName(packageName);
    if (beaconFenceRequest != nullptr) {
        std::shared_ptr<BeaconFence> beaconFence = beaconFenceRequest->GetBeaconFence();
        RemoveBeaconFence(beaconFence);
    }
}

bool BeaconFenceManager::CompareUUID(const std::string& uuid1, const std::string& uuid2)
{
    std::string lower1 = uuid1;
    std::string lower2 = uuid2;
    std::transform(lower1.begin(), lower1.end(), lower1.begin(), ::tolower);
    std::transform(lower2.begin(), lower2.end(), lower2.begin(), ::tolower);
    return lower1 == lower2;
}

bool BeaconFenceManager::CompareBeaconFence(
    std::shared_ptr<BeaconFence> beaconFence1, std::shared_ptr<BeaconFence> beaconFence2)
{
    if (beaconFence1->GetIdentifier().compare(beaconFence2->GetIdentifier()) != 0) {
        LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare Identifier fail", __func__);
        return false;
    }
    if (static_cast<int>(beaconFence1->GetBeaconFenceInfoType()) !=
        static_cast<int>(beaconFence2->GetBeaconFenceInfoType())) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare BeaconFenceInfoType fail", __func__);
        return false;
    }
    if (beaconFence1->GetBeaconManufactureData().manufactureId !=
        beaconFence2->GetBeaconManufactureData().manufactureId) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare manufactureId fail", __func__);
        return false;
    }
    if (beaconFence1->GetBeaconManufactureData().manufactureData !=
        beaconFence2->GetBeaconManufactureData().manufactureData) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare manufactureData fail", __func__);
        return false;
    }
    if (beaconFence1->GetBeaconManufactureData().manufactureDataMask !=
        beaconFence2->GetBeaconManufactureData().manufactureDataMask) {
            LBSLOGE(BEACON_FENCE_MANAGER, "%{public}s, compare manufactureDataMask fail", __func__);
        return false;
    }
    LBSLOGD(BEACON_FENCE_MANAGER, "%{public}s res:true", __func__);
    return true;
}

std::vector<BeaconManufactureData> BeaconFenceManager::GetBeaconManufactureDataForFilter()
{
    std::vector<BeaconManufactureData> filters;
    for (auto iter = beaconFenceRequestMap_.begin(); iter != beaconFenceRequestMap_.end(); iter++) {
        auto request = iter->first;
        BeaconManufactureData beaconManufactureData = request->GetBeaconFence()->GetBeaconManufactureData();
        filters.push_back(beaconManufactureData);
    }
    return filters;
}

#ifdef BLUETOOTH_ENABLE
void BeaconBleCallbackWapper::OnFoundOrLostCallback(const Bluetooth::BleScanResult &result, uint8_t callbackType)
{
    // 2进，4出  10s无设备扫描到认为出围栏
    LBSLOGI(BEACON_FENCE_MANAGER, "%{public}s, callbackType:%{public}hhu", __func__, callbackType);
    BeaconFenceManager::GetInstance()->ReportFoundOrLost(result, callbackType);
}

void BeaconBleCallbackWapper::OnScanCallback(const Bluetooth::BleScanResult &result) {}

void BeaconBleCallbackWapper::OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results) {}

void BeaconBleCallbackWapper::OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan) {}

void BeaconBleCallbackWapper::OnNotifyMsgReportFromLpDevice(const Bluetooth::UUID &btUuid, int msgType,
    const std::vector<uint8_t> &value) {}
#endif

void BeaconFenceRequestMessage::SetBeaconFenceRequest(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest)
{
    beaconFenceRequest_ = beaconFenceRequest;
}

std::shared_ptr<BeaconFenceRequest> BeaconFenceRequestMessage::GetBeaconFenceRequest()
{
    return beaconFenceRequest_;
}

void BeaconFenceRequestMessage::SetAppIdentity(AppIdentity& appIdentity)
{
    appIdentity_ = appIdentity;
}

AppIdentity BeaconFenceRequestMessage::GetAppIdentity()
{
    return appIdentity_;
}

BeaconFenceCallbackDeathRecipient::BeaconFenceCallbackDeathRecipient()
{
}
 
BeaconFenceCallbackDeathRecipient::~BeaconFenceCallbackDeathRecipient()
{
}
 
void BeaconFenceCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LBSLOGI(BEACON_FENCE_MANAGER, "beaconFence callback OnRemoteDied");
    BeaconFenceManager::GetInstance()->RemoveBeaconFenceRequestByCallback(remote.promote());
}
} // namespace Location
} // namespace OHOS
