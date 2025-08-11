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

#ifndef BEACON_FENCE_MANAGER_H
#define BEACON_FENCE_MANAGER_H

#include <map>
#include <mutex>
#include <singleton.h>
#include <string>

#ifdef BLUETOOTH_ENABLE
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_host.h"
#include "ohos_bt_gatt.h"
#endif
#include "ffrt.h"
#include "event_handler.h"
#include "event_runner.h"
#include "app_identity.h"
#include "iremote_stub.h"
#include "beacon_fence_request.h"
#include "constant_definition.h"
#include "i_gnss_geofence_callback.h"
#include "beacon_fence.h"

namespace OHOS {
namespace Location {
#ifdef BLUETOOTH_ENABLE
class BeaconBleCallbackWapper : public Bluetooth::BleCentralManagerCallback {
public:
    void OnScanCallback(const Bluetooth::BleScanResult &result) override;
    void OnFoundOrLostCallback(const Bluetooth::BleScanResult &result, uint8_t callbackType) override;
    void OnBleBatchScanResultsEvent(const std::vector<Bluetooth::BleScanResult> &results) override;
    void OnStartOrStopScanEvent(int32_t resultCode, bool isStartScan) override;
    void OnNotifyMsgReportFromLpDevice(const Bluetooth::UUID &btUuid,
        int msgType, const std::vector<uint8_t> &value) override;
};
#endif

class BeaconFenceManager {
public:
    BeaconFenceManager();
    ~BeaconFenceManager();
    static BeaconFenceManager* GetInstance();
    ErrCode AddBeaconFence(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest, const AppIdentity& identity);
    ErrCode RemoveBeaconFence(const std::shared_ptr<BeaconFence>& beaconFence);
    void StartAddBeaconFence(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest, const AppIdentity& identity);
#ifdef BLUETOOTH_ENABLE
    void ReportFoundOrLost(const Bluetooth::BleScanResult &result, uint8_t type);
#endif
    void RemoveBeaconFenceRequestByCallback(sptr<IRemoteObject> callbackObj);
    void RemoveBeaconFenceByPackageName(std::string& packageName);

private:
#ifdef BLUETOOTH_ENABLE
    void StartBluetoothScan();
    void StopBluetoothScan();
    void ConstructFilter(std::vector<Bluetooth::BleScanFilter>& filters);
    std::shared_ptr<BeaconFenceRequest> GetBeaconFenceRequestByScanResult(const Bluetooth::BleScanResult &result);
#endif
    int32_t GenerateBeaconFenceId();
    bool IsStrValidForStoi(const std::string &str);
    void TransitionStatusChange(std::shared_ptr<BeaconFenceRequest> beaconFenceRequest, GeofenceTransitionEvent event,
        const AppIdentity &identity);
    std::shared_ptr<BeaconFenceRequest> GetBeaconFenceRequestByServiceUuid(std::string serviceUuid);
    std::shared_ptr<BeaconFenceRequest> GetBeaconFenceRequestByCallback(sptr<IRemoteObject> callbackObj);
    std::shared_ptr<BeaconFenceRequest> GetBeaconFenceRequestByPackageName(std::string& packageName);
    bool MatchesData(std::vector<uint8_t> fData, std::string scanData);
    std::string ExtractiBeaconUUID(const std::vector<uint8_t>& data);
    void RemoveBeaconFenceRequestByBeacon(std::shared_ptr<BeaconFence> beaconFence);
    std::shared_ptr<BeaconFenceRequest> GetBeaconFenceRequestByBeacon(std::shared_ptr<BeaconFence> beaconFence);
    bool CompareUUID(const std::string& uuid1, const std::string& uuid2);
    bool CompareBeaconFence(std::shared_ptr<BeaconFence> beaconFence1, std::shared_ptr<BeaconFence> beaconFence2);
    std::vector<BeaconManufactureData> GetBeaconManufactureDataForFilter();
    void OnReportOperationResultByCallback(const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest,
        GnssGeofenceOperateType type, GnssGeofenceOperateResult result);
    bool isBeaconFenceRequestExists(const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest);
    void RegisterBeaconFenceCallback(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest,
        const AppIdentity& appIdentity);
    AppIdentity GetAppIdentityByBeaconFenceRequest(const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest);
    
    std::mutex beaconFenceRequestMapMutex_;
    std::mutex filterUuidMutex_;
    std::map<sptr<IRemoteObject>,
        std::pair<AppIdentity, sptr<IRemoteObject::DeathRecipient>>> handlerEventMap_;
#ifdef BLUETOOTH_ENABLE
    std::shared_ptr<Bluetooth::BleCentralManager> bleCentralManager_;
#endif
    std::map<std::shared_ptr<BeaconFenceRequest>, std::pair<sptr<IRemoteObject>, AppIdentity>> beaconFenceRequestMap_;
    int32_t beaconFenceId_;
    std::vector<std::string> filterUuid_;
};

class BeaconFenceRequestMessage {
public:
    void SetBeaconFenceRequest(std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest);
    std::shared_ptr<BeaconFenceRequest> GetBeaconFenceRequest();
    void SetAppIdentity(AppIdentity& appIdentity);
    AppIdentity GetAppIdentity();
private:
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest_;
    AppIdentity appIdentity_;
};

class BeaconFenceCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    BeaconFenceCallbackDeathRecipient();
    ~BeaconFenceCallbackDeathRecipient() override;
 
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
};
} // namespace Location
} // namespace OHOS
#endif // BEACON_FENCE_MANAGER_H
