/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef LOCATOR_ABILITY_H
#define LOCATOR_ABILITY_H

#include <map>
#include <mutex>
#include <singleton.h>

#include "event_handler.h"
#include "ffrt.h"
#include "system_ability.h"

#include "app_identity.h"
#include "common_utils.h"
#include "geocoding_mock_info.h"
#include "i_switch_callback.h"
#include "i_cached_locations_callback.h"
#include "ibluetooth_scan_result_callback.h"
#include "locator_event_subscriber.h"
#include "locator_service_stub.h"
#include "permission_status_change_cb.h"
#include "request.h"
#include "request_manager.h"
#include "report_manager.h"
#include "want_agent_helper.h"
#include "geofence_request.h"
#include "common_event_support.h"
#ifdef MOVEMENT_CLIENT_ENABLE
#include "locator_msdp_monitor_manager.h"
#endif
#include "idata_types.h"
#include "proxy_freeze_manager.h"
#include "beacon_fence_request.h"
#include "beacon_fence.h"
#include "beacon_fence_manager.h"
#include "locationhub_ipc_interface_code.h"

namespace OHOS {
namespace Location {
class ScanCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    ScanCallbackDeathRecipient();
    ~ScanCallbackDeathRecipient() override;
};

class LocatorHandler : public AppExecFwk::EventHandler {
public:
    using LocatorEventHandle = std::function<void(const AppExecFwk::InnerEvent::Pointer &)>;
    using LocatorEventHandleMap = std::map<int, LocatorEventHandle>;
    explicit LocatorHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~LocatorHandler() override;
    void InitLocatorHandlerEventMap();
    void ConstructDbHandleMap();
    void ConstructGeocodeHandleMap();
    void ConstructBluetoothScanHandleMap();
private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    void UpdateSaEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void InitRequestManagerEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void ApplyRequirementsEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void RetryRegisterActionEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void ReportLocationMessageEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void SendSwitchStateToHifenceEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void UnloadSaEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StartLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StopLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void GetCachedLocationSuccess(const AppExecFwk::InnerEvent::Pointer& event);
    void GetCachedLocationFailed(const AppExecFwk::InnerEvent::Pointer& event);
    void StartScanBluetoothDeviceEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StopScanBluetoothDeviceEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void RegLocationErrorEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void UnRegLocationErrorEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void ReportNetworkLocatingErrorEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void RequestCheckEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void SyncStillMovementState(const AppExecFwk::InnerEvent::Pointer& event);
    void SyncIdleState(const AppExecFwk::InnerEvent::Pointer& event);
    void SendGeoRequestEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void SyncSwitchStatus(const AppExecFwk::InnerEvent::Pointer& event);
    void InitSaAbilityEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void InitMonitorManagerEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void IsStandByEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void SetLocationWorkingStateEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void SetSwitchStateToDbEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void SetSwitchStateToDbForUserEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void WatchSwitchParameter(const AppExecFwk::InnerEvent::Pointer& event);
    LocatorEventHandleMap locatorHandlerEventMap_;

    bool IsSwitchObserverReg();
    void SetIsSwitchObserverReg(bool isSwitchObserverReg);
    ffrt::mutex isSwitchObserverRegMutex_;
    bool isSwitchObserverReg_ = false;
};

typedef struct {
    bool state;
    int64_t timeSinceBoot;
} AppSwitchIgnoredState;

class LocatorAbility : public SystemAbility, public LocatorServiceStub {
DECLEAR_SYSTEM_ABILITY(LocatorAbility);

public:
    DISALLOW_COPY_AND_MOVE(LocatorAbility);
    static LocatorAbility* GetInstance();
    LocatorAbility();
    ~LocatorAbility() override;
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    void InitSaAbility();
    void InitRequestManagerMap();
    int32_t CallbackEnter(uint32_t code) override;
    int32_t CallbackExit(uint32_t code, int32_t result) override;
    LocationErrCode UpdateSaAbility();
    ErrCode GetSwitchState(int32_t& state) override;
    ErrCode EnableAbility(bool isEnabled) override;
    ErrCode EnableAbilityForUser(bool isEnabled, int32_t userId) override;
    ErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject>& cb) override;
    ErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject>& cb) override;
    ErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject>& cb) override;
    ErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& cb) override;
    ErrCode RegisterCachedLocationCallback(int32_t reportingPeriodSec, bool wakeUpCacheQueueFull,
        const sptr<ICachedLocationsCallback>& cb, const std::string& bundleName) override;
    ErrCode UnregisterCachedLocationCallback(const sptr<ICachedLocationsCallback>& cb) override;
    ErrCode GetCachedGnssLocationsSize(int32_t& size) override;
    ErrCode FlushCachedGnssLocations() override;
    ErrCode SendCommand(int32_t scenario, const std::string& command) override;
    ErrCode AddFence(const GeofenceRequest& request) override;
    ErrCode RemoveFence(const GeofenceRequest& request) override;
    ErrCode AddGnssGeofence(const GeofenceRequest& request) override;
    ErrCode RemoveGnssGeofence(int32_t fenceId) override;
    ErrCode StartLocating(const RequestConfig& requestConfig, const sptr<ILocatorCallback>& cb) override;
    ErrCode StopLocating(const sptr<ILocatorCallback>& cb) override;
    ErrCode GetCacheLocation(Location& location) override;
    ErrCode IsGeoConvertAvailable(bool& isAvailable) override;
    ErrCode GetAddressByCoordinate(const sptr<IRemoteObject>& cb,
        const GeocodeConvertLocationRequest& request) override;
    ErrCode GetAddressByLocationName(const sptr<IRemoteObject>& cb,
        const GeocodeConvertAddressRequest& request) override;
    ErrCode EnableReverseGeocodingMock() override;
    ErrCode DisableReverseGeocodingMock() override;
    ErrCode SetReverseGeocodingMockInfo(const std::vector<GeocodingMockInfo>& geocodingMockInfo) override;
    ErrCode IsLocationPrivacyConfirmed(int32_t type, bool& state) override;
    ErrCode SetLocationPrivacyConfirmStatus(int32_t type, bool isConfirmed) override;
    ErrCode EnableLocationMock() override;
    ErrCode DisableLocationMock() override;
    ErrCode SetMockedLocations(int32_t timeInterval, const std::vector<Location>& locations) override;
    ErrCode ReportLocation(const std::string& abilityName, const Location& location) override;
    ErrCode ReportLocationStatus(const sptr<ILocatorCallback>& callback, int result);
    ErrCode ReportErrorStatus(const sptr<ILocatorCallback>& callback, int result);
    LocationErrCode ProcessLocationMockMsg(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
    LocationErrCode SendLocationMockMsgToGnssSa(const sptr<IRemoteObject> obj,
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
    LocationErrCode SendLocationMockMsgToNetworkSa(const sptr<IRemoteObject> obj,
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
    LocationErrCode SendLocationMockMsgToPassiveSa(const sptr<IRemoteObject> obj,
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId);
    LocationErrCode RegisterWifiScanInfoCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterWifiScanInfoCallback(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterBluetoothScanInfoCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterBluetoothScanInfoCallback(const sptr<IRemoteObject>& callback);
    LocationErrCode RegisterBleScanInfoCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterBleScanInfoCallback(const sptr<IRemoteObject>& callback);
    ErrCode SubscribeBluetoothScanResultChange(const sptr<IBluetoothScanResultCallback>& cb) override;
    ErrCode UnSubscribeBluetoothScanResultChange(const sptr<IBluetoothScanResultCallback>& cb) override;
    LocationErrCode RegisterLocationError(const sptr<ILocatorCallback>& callback, AppIdentity &identity);
    LocationErrCode UnregisterLocationError(const sptr<ILocatorCallback>& callback, AppIdentity &identity);
    ErrCode ReportLocationError(int32_t errCodeNum, const std::string& errMsg, const std::string& uuid) override;
    ErrCode SetLocationSwitchIgnored(bool isEnabled) override;
    ErrCode AddBeaconFence(const BeaconFenceRequest& beaconFenceRequest) override;
    ErrCode RemoveBeaconFence(const BeaconFence& beaconFence) override;
    ErrCode IsBeaconFenceSupported(bool& beaconFenceSupportedState) override;

    std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> GetRequests();
    std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> GetReceivers();
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> GetProxyMap();
    void UpdateSaAbilityHandler();
    void ApplyRequests(int delay);
    void RegisterAction();
    void RegisterLocationPrivacyAction();
    ErrCode ProxyForFreeze(const std::vector<int32_t>& pidList, bool isProxy) override;
    ErrCode ResetAllProxy() override;
    int GetActiveRequestNum();
    void RegisterPermissionCallback(const uint32_t callingTokenId, const std::vector<std::string>& permissionNameList);
    void UnregisterPermissionCallback(const uint32_t callingTokenId);
    int UpdatePermissionUsedRecord(uint32_t tokenId, std::string permissionName,
        int permUsedType, int succCnt, int failCnt);
    LocationErrCode RemoveInvalidRequests();
    bool IsInvalidRequest(std::shared_ptr<Request>& request);
    bool IsProcessRunning(pid_t pid, const uint32_t tokenId);
    ErrCode QuerySupportCoordinateSystemType(std::vector<CoordinateType>& coordinateTypes) override;
    LocationErrCode SendNetworkLocation(const std::unique_ptr<Location>& location);
    void SyncStillMovementState(bool stillState);
    void SyncIdleState(bool stillState);
    LocationErrCode SendGeoRequest(int type, MessageParcel &data, MessageParcel &reply);
    void ReportDataToResSched(std::string state);
    bool IsHapCaller(const uint32_t tokenId);
    void HandleStartLocating(const std::shared_ptr<Request>& request, const sptr<ILocatorCallback>& callback);
    bool GetLocationSwitchIgnoredFlag(uint32_t tokenId);
    bool CancelIdleState(uint32_t code);
    void RemoveUnloadTask(uint32_t code);
    void PostUnloadTask(uint32_t code);
    ErrCode RegisterLocatingRequiredDataCallback(const LocatingRequiredDataConfig& dataConfig,
        const sptr<ILocatingRequiredDataCallback>& cb) override;
    ErrCode UnRegisterLocatingRequiredDataCallback(const sptr<ILocatingRequiredDataCallback>& cb) override;
    ErrCode SubscribeLocationError(const sptr<ILocatorCallback>& cb) override;
    ErrCode UnSubscribeLocationError(const sptr<ILocatorCallback>& cb) override;
    ErrCode GetCurrentWifiBssidForLocating(std::string& bssid) override;
    ErrCode IsPoiServiceSupported(bool& poiServiceSupportState) override;
    LocationErrCode SetSwitchState(bool isEnabled);

private:
    bool Init();
    bool CheckSaValid();
#ifdef FEATURE_GNSS_SUPPORT
    LocationErrCode SendGnssRequest(int type, MessageParcel &data, MessageParcel &reply);
#endif
    void UpdateProxyMap();
    bool CheckIfLocatorConnecting();
    void UpdateLoadedSaMap();
    bool NeedReportCacheLocation(const std::shared_ptr<Request>& request, const sptr<ILocatorCallback>& callback);
    bool ReportSingleCacheLocation(const std::shared_ptr<Request>& request, const sptr<ILocatorCallback>& callback,
        std::unique_ptr<Location>& cacheLocation);
    bool ReportCacheLocation(const std::shared_ptr<Request>& request, const sptr<ILocatorCallback>& callback,
        std::unique_ptr<Location>& cacheLocation);
    bool IsCacheVaildScenario(const sptr<RequestConfig>& requestConfig);
    bool IsSingleRequest(const sptr<RequestConfig>& requestConfig);
    void SendSwitchState(const int state);
    bool SetLocationhubStateToSyspara(int value);
    void SetLocationSwitchIgnoredFlag(uint32_t tokenId, bool enable);
    void GetAppIdentityInfo(AppIdentity& identity);
    LocationErrCode SetSwitchStateForUser(bool isEnabled, int32_t userId);
    bool CheckLocationSwitchState();
    bool CheckBluetoothSwitchState();
    bool CheckLocationPermission(uint32_t callingTokenId, uint32_t callingFirstTokenid);
    bool CheckPreciseLocationPermissions(uint32_t callingTokenId, uint32_t callingFirstTokenid);
    ErrCode StartLocatingProcess(const RequestConfig& requestConfig, const sptr<ILocatorCallback>& cb,
        AppIdentity& identity);
    bool CheckRequestAvailable(LocatorInterfaceCode code, AppIdentity &identity);

    bool registerToAbility_ = false;
    bool isActionRegistered = false;
    bool isLocationPrivacyActionRegistered_ = false;
    std::string deviceId_;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::shared_ptr<LocatorEventSubscriber> locatorEventSubscriber_;
    std::shared_ptr<LocatorEventSubscriber> locationPrivacyEventSubscriber_;
    std::mutex switchMutex_;
    ffrt::mutex requestsMutex_;
    ffrt::mutex receiversMutex_;
    std::mutex proxyMapMutex_;
    ffrt::mutex permissionMapMutex_;
    ffrt::mutex loadedSaMapMutex_;
    std::unique_ptr<std::map<pid_t, sptr<ISwitchCallback>>> switchCallbacks_;
    std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> requests_;
    std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> receivers_;
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> proxyMap_;
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> loadedSaMap_;
    std::shared_ptr<std::map<uint32_t, std::shared_ptr<PermissionStatusChangeCb>>> permissionMap_;
    std::shared_ptr<LocatorHandler> locatorHandler_;
    RequestManager* requestManager_;
    ReportManager* reportManager_;
    std::mutex proxyPidsMutex_;
    std::set<int32_t> proxyPids_;
    std::map<uint32_t, AppSwitchIgnoredState> locationSettingsIgnoredFlagMap_;
    std::mutex LocationSwitchIgnoredFlagMutex_;
    std::mutex testMutex_;
    sptr<IRemoteObject::DeathRecipient> scanRecipient_ = new (std::nothrow) ScanCallbackDeathRecipient();
};

class LocationMessage {
public:
    void SetAbilityName(std::string abilityName);
    std::string GetAbilityName();
    void SetLocation(const std::unique_ptr<Location>& location);
    std::unique_ptr<Location> GetLocation();
private:
    std::string abilityName_;
    std::unique_ptr<Location> location_;
};

class LocatorCallbackMessage {
public:
    void SetCallback(const sptr<ILocatorCallback>& callback);
    sptr<ILocatorCallback> GetCallback();
    void SetAppIdentity(AppIdentity& appIdentity);
    AppIdentity GetAppIdentity();
private:
    std::string abilityName_;
    AppIdentity appIdentity_;
    sptr<ILocatorCallback> callback_;
};

class BluetoothScanResultCallbackMessage {
public:
    void SetCallback(const sptr<IBluetoothScanResultCallback>& callback);
    sptr<IBluetoothScanResultCallback> GetCallback();
    void SetAppIdentity(AppIdentity& appIdentity);
    AppIdentity GetAppIdentity();
private:
    sptr<IBluetoothScanResultCallback> callback_;
    AppIdentity appIdentity_;
};

class LocatorErrorMessage {
public:
    void SetUuid(std::string uuid);
    std::string GetUuid();
    void SetErrCode(int32_t errCode);
    int32_t GetErrCode();
    void SetNetErrCode(int32_t netErrCode);
    int32_t GetNetErrCode();
    void SetErrMsg(std::string errMsg);
    std::string GetErrMsg();
private:
    std::string uuid_;
    int32_t errCode_;
    int32_t netErrCode_;
	std::string errMsg_;
};

class LocatorSwitchMessage {
public:
    void SetUserId(int32_t userId);
    int32_t GetUserId();
    void SetModeValue(int32_t modeValue);
    int32_t GetModeValue();
private:
    int32_t modeValue_;
    int32_t userId_;
};

class LocatorCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    LocatorCallbackDeathRecipient(int32_t tokenId);
    ~LocatorCallbackDeathRecipient() override;
private:
    int32_t tokenId_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_ABILITY_H
