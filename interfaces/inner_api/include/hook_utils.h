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

#ifndef LOCATION_HOOK_UTILS_H
#define LOCATION_HOOK_UTILS_H

#include "hookmgr.h"
#include "constant_definition.h"
#include "request.h"
#include "location.h"
#include "locating_required_data.h"

namespace OHOS {
namespace Location {
enum class LocationProcessStage {
    LOCATOR_SA_START_LOCATING = 0,
    LOCATOR_SA_REQUEST_PROCESS,
    GNSS_SA_REQUEST_PROCESS,
    LOCATION_REPORT_PROCESS,
    GNSS_SA_LOCATION_REPORT_PROCESS,
    NETWORK_SA_LOCATION_REPORT_PROCESS,
    LOCATOR_SA_LOCATION_REPORT_PROCESS,
    START_GNSS_PROCESS,
    STOP_GNSS_PROCESS,
    CHECK_GNSS_LOCATION_VALIDITY,
    MOCK_LOCATION_PROCESS,
    FENCE_REQUEST_PROCESS,
    REQUEST_MANAGER_HANDLE_STOP,
    LOCATOR_SA_GET_ADDRESSES_FROM_LOCATION_PROCESS,
    LOCATOR_SA_GET_ADDRESSES_FROM_LOCATIONNAME_PROCESS,
    WRITE_DFX_INNER_EVENT_PROCESS,
    ADD_REQUEST_TO_WORK_RECORD,
    LOCATOR_SA_LOCATION_PERMISSION_CHECK,
    LOCATOR_SA_COMMAND_PROCESS,
    REPORT_MANAGER_GET_CACHE_LOCATION_PROCESS,
    ENABLE_ABILITY_PROCESS,
    PRE_START_LOCATING_PROCESS,
    NETWORK_SA_ADD_REQUEST_PROCESS,
    NETWORK_SA_REMOVE_REQUEST_PROCESS,
    GNSS_REQUEST_RECORD_PROCESS,
	FUSION_REPORT_PROCESS,
	GNSS_STATUS_REPORT_PROCESS,
    SET_AGNSS_SERVER_PROCESS,
    SIM_STATE_CHANGED_PROCESS,
    APPROXIMATELY_LOCATION_PROCESS,
    START_SCAN_BLUETOOTH_DEVICE_PROCESS,
    REPORT_BLUETOOTH_SCAN_RESULT_PROCESS,
    WIFI_SCAN_STATE_CHANGE,
    CUST_CONFIG_POLICY_CHANGE_PROCESS,
    IS_APP_BACKGROUND_PROCESS,
    LOCATOR_SA_SYNC_SWITCH_STATUS,
    REPORT_BEACON_FENCE_OPERATE_RESULT_PROCESS,
    REMOVE_BEACON_FENCE_BY_CALLBACK_PROCESS,
    BEACON_FENCE_TRANSITION_STATUS_CHANGE_PROCESS,
    ENABLE_GNSS_PROCESS,
    ON_LOCATION_REPORT_PROCESS,
    ON_USER_SWITCH_PROCESS,
    INIT_GEOFENCE_ID_PROCESS,
};

typedef struct {
    std::vector<std::shared_ptr<LocatingRequiredData>> result;
} WifiScanResult;

typedef struct {
    Location location;
    Request request;
    std::string abilityName;
    int retCode;
} LocationSupplicantInfo;

typedef struct {
    Location location;
    Location lastFuseLocation;
    Location resultLocation;
} LocationFusionInfo;

typedef struct {
    std::vector<float> cn0;
    int64_t availableTimeStamp;
} GnssStatusInfo;

typedef struct {
    Location location;
    bool result;
    bool isGnssSpoofed;
} GnssLocationValidStruct;

typedef struct {
    bool enableMock;
} MockLocationStruct;

typedef struct {
    int32_t eventId;
    std::vector<std::string> names;
    std::vector<std::string> values;
} DfxInnerInfo;

typedef struct {
    bool deviceStillState;
    bool deviceIdleState;
    bool result;
    int nlpRequestType;
    int cacheTime;
    std::string abilityName;
    std::string bundleName;
} LocatorRequestStruct;

typedef struct {
    std::string packageName;
    std::string command;
    bool result;
} CommandStruct;

typedef struct {
    bool isEnabled;
    int32_t userId;
    std::string bundleName;
    bool result;
} EnableAbilityStruct;

typedef struct {
    std::string uuid;
    bool result;
} NetworkRequestInfo;

typedef struct {
    std::string* addrName;
    int* port;
    bool result;
} AgnssStruct;

typedef struct {
    std::string packageName;
    std::string type;
    bool result;
} ScanStruct;

typedef struct {
    bool locationEnable;
} LocationStatusStruct;

typedef struct {
    std::string fenceId;
    int transitionEvent;
    std::string fenceExtensionAbilityName;
    std::string packageName;
    bool result;
} BeaconFenceStruct;

class HookUtils {
public:
    static HOOK_MGR* GetLocationExtHookMgr();
    static LocationErrCode RegisterHook(LocationProcessStage stage, int prio, OhosHook hook);
    static void UnregisterHook(LocationProcessStage stage, OhosHook hook);
    static LocationErrCode ExecuteHook(LocationProcessStage stage, void *executionContext,
        const HOOK_EXEC_OPTIONS *options);
    static void ExecuteHookWhenStartLocation(std::shared_ptr<Request> request);
    static void ExecuteHookWhenStopLocation(std::shared_ptr<Request> request);
    static void ExecuteHookWhenGetAddressFromLocation(std::string packageName);
    static void ExecuteHookWhenGetAddressFromLocationName(std::string packageName);
    static void ExecuteHookWhenReportInnerInfo(
        int32_t event, std::vector<std::string>& names, std::vector<std::string>& values);
    static bool ExecuteHookWhenAddWorkRecord(bool stillState, bool idleState, std::string abilityName,
        std::string bundleName);
    static bool CheckGnssLocationValidity(const std::unique_ptr<Location>& location);
    static bool ExecuteHookWhenCheckAppForUser(std::string packageName);
    static int ExecuteHookReportManagerGetCacheLocation(std::string packageName, int nlpRequestType);
    static bool ExecuteHookEnableAbility(std::string packageName, bool isEnabled, int32_t userId);
    static bool ExecuteHookWhenPreStartLocating(std::string packageName);
    static bool ExecuteHookWhenAddNetworkRequest(std::string uuid);
    static bool ExecuteHookWhenRemoveNetworkRequest(std::string uuid);
    static bool ExecuteHookWhenSetAgnssServer(std::string& addrName, int& port);
    static bool ExecuteHookWhenSimStateChange(const std::string& data);
    static bool ExecuteHookWhenReportBluetoothScanResult(const std::string& packageName, const std::string& type);
    static bool ExecuteHookWhenStartScanBluetoothDevice(const std::string& packageName, const std::string& type);
    static void ExecuteHookWhenWifiScanStateChanged(
            const std::vector<std::shared_ptr<LocatingRequiredData>>& result);
    static bool ExecuteHookWhenCustConfigPolicyChange();
    static bool ExecuteHookWhenCheckIsAppBackground(const std::string& packageName);
    static bool ExecuteHookWhenSyncSwitchStates(int status);
    static bool ExecuteHookWhenReportBeaconFenceOperateResult(const std::string& fenceId, int transitionEvent,
        const std::string& fenceExtensionAbilityName, const std::string& packageName);
    static bool ExecuteHookWhenRemoveBeaconFenceByCallback(const std::string& packageName);
    static bool ExecuteHookWhenBeaconFenceTransitionStatusChange(const std::string& packageName);
    static void ExecuteHookWhenGnssEnable();
    static void ExecuteHookWhenOnUserSwitch(int32_t userId);
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_HOOK_UTILS_H