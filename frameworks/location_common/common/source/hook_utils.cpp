/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "hook_utils.h"
#include "location_log.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
static HOOK_MGR* locatorHookMgr_ = nullptr;

HOOK_MGR* HookUtils::GetLocationExtHookMgr()
{
    if (locatorHookMgr_ != nullptr) {
        return locatorHookMgr_;
    }

    locatorHookMgr_ = HookMgrCreate("locationHookMgr");
    return locatorHookMgr_;
}

void HookUtils::ExecuteHookWhenWifiScanStateChanged(
    const std::vector<std::shared_ptr<LocatingRequiredData>>& result)
{
    WifiScanResult scanResult;
    scanResult.result = result;
    ExecuteHook(LocationProcessStage::WIFI_SCAN_STATE_CHANGE, (void *)&scanResult, nullptr);
}

LocationErrCode HookUtils::RegisterHook(LocationProcessStage stage, int prio, OhosHook hook)
{
    auto ret = HookMgrAdd(GetLocationExtHookMgr(), static_cast<int>(stage), prio, hook);
    if (ret == 0) {
        return ERRCODE_SUCCESS;
    }
    LBSLOGE(LOCATOR, "%{public}s stage = %{public}d add failed ret = %{public}d",
        __func__, static_cast<int>(stage), ret);
    return ERRCODE_SERVICE_UNAVAILABLE;
}

void HookUtils::UnregisterHook(LocationProcessStage stage, OhosHook hook)
{
    HookMgrDel(GetLocationExtHookMgr(), static_cast<int>(stage), hook);
}

LocationErrCode HookUtils::ExecuteHook(
    LocationProcessStage stage, void *executionContext, const HOOK_EXEC_OPTIONS *options)
{
    auto ret = HookMgrExecute(GetLocationExtHookMgr(), static_cast<int>(stage), executionContext, options);
    if (ret == 0) {
        return ERRCODE_SUCCESS;
    }
    LBSLOGE(LOCATOR, "%{public}s stage = %{public}d execute failed ret = %{public}d",
        __func__, static_cast<int>(stage), ret);
    return ERRCODE_SERVICE_UNAVAILABLE;
}

void HookUtils::ExecuteHookWhenStartLocation(std::shared_ptr<Request> request)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.request = *request;
    reportStruct.retCode = true;
    ExecuteHook(LocationProcessStage::LOCATOR_SA_START_LOCATING, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenStopLocation(std::shared_ptr<Request> request)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.request = *request;
    reportStruct.retCode = true;
    ExecuteHook(LocationProcessStage::REQUEST_MANAGER_HANDLE_STOP, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenGetAddressFromLocation(std::string packageName)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.abilityName = packageName;
    reportStruct.retCode = true;
    ExecuteHook(
        LocationProcessStage::LOCATOR_SA_GET_ADDRESSES_FROM_LOCATION_PROCESS, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenGetAddressFromLocationName(std::string packageName)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.abilityName = packageName;
    reportStruct.retCode = true;
    ExecuteHook(
        LocationProcessStage::LOCATOR_SA_GET_ADDRESSES_FROM_LOCATIONNAME_PROCESS, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenReportInnerInfo(
    int32_t event, std::vector<std::string>& names, std::vector<std::string>& values)
{
    DfxInnerInfo innerInfo;
    innerInfo.eventId = event;
    innerInfo.names = names;
    innerInfo.values = values;
    ExecuteHook(LocationProcessStage::WRITE_DFX_INNER_EVENT_PROCESS, (void *)&innerInfo, nullptr);
}

bool HookUtils::ExecuteHookWhenAddWorkRecord(bool stillState, bool idleState, std::string abilityName,
    std::string bundleName)
{
    LocatorRequestStruct locatorRequestStruct;
    locatorRequestStruct.deviceStillState = stillState;
    locatorRequestStruct.deviceIdleState = idleState;
    locatorRequestStruct.abilityName = abilityName;
    locatorRequestStruct.bundleName = bundleName;
    locatorRequestStruct.result = false;
    ExecuteHook(
        LocationProcessStage::ADD_REQUEST_TO_WORK_RECORD, (void *)&locatorRequestStruct, nullptr);
    return locatorRequestStruct.result;
}

bool HookUtils::ExecuteHookWhenCheckAppForUser(std::string packageName)
{
    LocatorRequestStruct locatorRequestStruct;
    locatorRequestStruct.bundleName = packageName;
    locatorRequestStruct.result = false;
    ExecuteHook(
        LocationProcessStage::LOCATOR_SA_LOCATION_PERMISSION_CHECK, (void *)&locatorRequestStruct, nullptr);
    return locatorRequestStruct.result;
}

bool HookUtils::CheckGnssLocationValidity(const std::unique_ptr<Location>& location)
{
    GnssLocationValidStruct gnssLocationValidStruct;
    gnssLocationValidStruct.location = *location;
    gnssLocationValidStruct.result = true;
    HookUtils::ExecuteHook(
        LocationProcessStage::CHECK_GNSS_LOCATION_VALIDITY, (void *)&gnssLocationValidStruct, nullptr);
    return gnssLocationValidStruct.result;
}

int HookUtils::ExecuteHookReportManagerGetCacheLocation(std::string packageName, int nlpRequestType)
{
    LocatorRequestStruct locatorRequestStruct;
    locatorRequestStruct.bundleName = packageName;
    locatorRequestStruct.nlpRequestType = nlpRequestType;
    locatorRequestStruct.result = false;
    ExecuteHook(
        LocationProcessStage::REPORT_MANAGER_GET_CACHE_LOCATION_PROCESS, (void *)&locatorRequestStruct, nullptr);
    return locatorRequestStruct.cacheTime;
}

bool HookUtils::ExecuteHookEnableAbility(std::string packageName, bool isEnabled, int32_t userId)
{
    EnableAbilityStruct enableAbilityStruct;
    enableAbilityStruct.bundleName = packageName;
    enableAbilityStruct.isEnabled = isEnabled;
    enableAbilityStruct.userId = userId;
    enableAbilityStruct.result = true;
    ExecuteHook(
        LocationProcessStage::ENABLE_ABILITY_PROCESS, (void *)&enableAbilityStruct, nullptr);
    return enableAbilityStruct.result;
}

bool HookUtils::ExecuteHookWhenPreStartLocating(std::string packageName)
{
    LocatorRequestStruct locatorRequestStruct;
    locatorRequestStruct.bundleName = packageName;
    locatorRequestStruct.result = true;
    ExecuteHook(
        LocationProcessStage::PRE_START_LOCATING_PROCESS, (void *)&locatorRequestStruct, nullptr);
    return locatorRequestStruct.result;
}

bool HookUtils::ExecuteHookWhenAddNetworkRequest(std::string uuidTemp)
{
    NetworkRequestInfo info;
    info.uuid = uuidTemp;
    info.result = false;
    HookUtils::ExecuteHook(LocationProcessStage::NETWORK_SA_ADD_REQUEST_PROCESS, ((void *)(&info)), nullptr);
    return info.result;
}

bool HookUtils::ExecuteHookWhenRemoveNetworkRequest(std::string uuidTemp)
{
    NetworkRequestInfo info;
    info.uuid = uuidTemp;
    info.result = false;
    HookUtils::ExecuteHook(LocationProcessStage::NETWORK_SA_REMOVE_REQUEST_PROCESS, ((void *)(&info)), nullptr);
    return info.result;
}

bool HookUtils::ExecuteHookWhenSetAgnssServer(std::string& addrName, int& port)
{
    AgnssStruct agnssStruct;
    agnssStruct.addrName = &addrName;
    agnssStruct.port = &port;
    agnssStruct.result = false;
    ExecuteHook(LocationProcessStage::SET_AGNSS_SERVER_PROCESS, (void *)&agnssStruct, nullptr);
    return agnssStruct.result;
}

bool HookUtils::ExecuteHookWhenSimStateChange(const std::string& data)
{
    LocatorRequestStruct locatorRequestStruct;
    locatorRequestStruct.bundleName = data;
    locatorRequestStruct.result = true;
    ExecuteHook(
        LocationProcessStage::SIM_STATE_CHANGED_PROCESS, (void *)&locatorRequestStruct, nullptr);
    return locatorRequestStruct.result;
}

bool HookUtils::ExecuteHookWhenStartScanBluetoothDevice(const std::string& packageName, const std::string& type)
{
    ScanStruct scanStruct;
    scanStruct.packageName = packageName;
    scanStruct.type = type;
    scanStruct.result = false;
    ExecuteHook(
        LocationProcessStage::START_SCAN_BLUETOOTH_DEVICE_PROCESS, (void *)&scanStruct, nullptr);
    return scanStruct.result;
}

bool HookUtils::ExecuteHookWhenReportBluetoothScanResult(const std::string& packageName, const std::string& type)
{
    ScanStruct scanStruct;
    scanStruct.packageName = packageName;
    scanStruct.type = type;
    scanStruct.result = false;
    ExecuteHook(
        LocationProcessStage::REPORT_BLUETOOTH_SCAN_RESULT_PROCESS, (void *)&scanStruct, nullptr);
    return scanStruct.result;
}
} // namespace Location
} // namespace OHOS
