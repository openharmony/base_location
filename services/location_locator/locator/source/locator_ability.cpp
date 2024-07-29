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

#include "locator_ability.h"

#include "accesstoken_kit.h"
#include "event_runner.h"
#include "privacy_kit.h"
#include "system_ability_definition.h"
#include "uri.h"

#include "common_event_manager.h"
#include "common_hisysevent.h"
#include "location_log_event_ids.h"
#include "common_utils.h"
#include "constant_definition.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_convert_proxy.h"
#endif
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability_proxy.h"
#endif
#include "hook_utils.h"
#include "locator_background_proxy.h"
#include "location_config_manager.h"
#include "location_data_rdb_helper.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locationhub_ipc_interface_code.h"
#include "locator_required_data_manager.h"
#include "location_data_rdb_manager.h"
#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability_proxy.h"
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
#include "passive_ability_proxy.h"
#endif
#include "permission_status_change_cb.h"
#include "work_record_statistic.h"
#include "permission_manager.h"
#ifdef RES_SCHED_SUPPROT
#include "res_type.h"
#include "res_sched_client.h"
#endif
#include "app_mgr_interface.h"
#include "app_state_data.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "geo_convert_request.h"

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    LocatorAbility::GetInstance());

const uint32_t EVENT_UPDATE_SA = 0x0001;
const uint32_t EVENT_INIT_REQUEST_MANAGER = 0x0002;
const uint32_t EVENT_APPLY_REQUIREMENTS = 0x0003;
const uint32_t EVENT_RETRY_REGISTER_ACTION = 0x0004;
const uint32_t EVENT_REPORT_LOCATION_MESSAGE = 0x0005;
const uint32_t EVENT_SEND_SWITCHSTATE_TO_HIFENCE = 0x0006;
const uint32_t EVENT_START_LOCATING = 0x0007;
const uint32_t EVENT_STOP_LOCATING = 0x0008;
const uint32_t EVENT_UPDATE_LASTLOCATION_REQUESTNUM = 0x0009;
const uint32_t EVENT_UNLOAD_SA = 0x0010;
const uint32_t EVENT_GET_CACHED_LOCATION_SUCCESS = 0x0014;
const uint32_t EVENT_GET_CACHED_LOCATION_FAILED = 0x0015;
const uint32_t EVENT_REG_LOCATION_ERROR = 0x0011;
const uint32_t EVENT_UNREG_LOCATION_ERROR = 0x0012;
const uint32_t EVENT_REPORT_LOCATION_ERROR = 0x0013;
const uint32_t EVENT_PERIODIC_CHECK = 0x0016;
const uint32_t EVENT_SYNC_LOCATION_STATUS = 0x0017;
const uint32_t EVENT_SYNC_STILL_MOVEMENT_STATE = 0x0018;
const uint32_t EVENT_SYNC_IDLE_STATE = 0x0019;
const uint32_t EVENT_INIT_MSDP_MONITOR_MANAGER = 0x0020;
const uint32_t EVENT_IS_STAND_BY = 0x0021;
const uint32_t EVENT_SET_LOCATION_WORKING_STATE = 0x0022;
const uint32_t EVENT_SEND_GEOREQUEST = 0x0023;

const uint32_t RETRY_INTERVAL_UNITE = 1000;
const uint32_t RETRY_INTERVAL_OF_INIT_REQUEST_MANAGER = 5 * RETRY_INTERVAL_UNITE;
const uint32_t RETRY_INTERVAL_OF_UNLOAD_SA = 30 * RETRY_INTERVAL_UNITE;
const float_t PRECISION = 0.000001;
const int COMMON_SA_ID = 4353;
const int COMMON_SWITCH_STATE_ID = 30;
const std::u16string COMMON_DESCRIPTION = u"location.IHifenceAbility";
const std::string UNLOAD_TASK = "locatior_sa_unload";
const std::string WIFI_SCAN_STATE_CHANGE = "wifiScanStateChange";
const uint32_t SET_ENABLE = 3;
const uint32_t EVENT_PERIODIC_INTERVAL = 3 * 60 * 1000;
const uint32_t REQUEST_DEFAULT_TIMEOUT_SECOUND = 5 * 60;

LocatorAbility* LocatorAbility::GetInstance()
{
    static LocatorAbility data;
    return &data;
}

LocatorAbility::LocatorAbility() : SystemAbility(LOCATION_LOCATOR_SA_ID, true)
{
    locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true,
        AppExecFwk::ThreadMode::FFRT));
    switchCallbacks_ = std::make_unique<std::map<pid_t, sptr<ISwitchCallback>>>();
    requests_ = std::make_shared<std::map<std::string, std::list<std::shared_ptr<Request>>>>();
    receivers_ = std::make_shared<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>>();
    proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    loadedSaMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    permissionMap_ = std::make_shared<std::map<uint32_t, std::shared_ptr<PermissionStatusChangeCb>>>();
    InitRequestManagerMap();
    reportManager_ = ReportManager::GetInstance();
    deviceId_ = CommonUtils::InitDeviceId();
#ifdef MOVEMENT_CLIENT_ENABLE
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_INIT_MSDP_MONITOR_MANAGER, 0, 0);
    }
#endif
    requestManager_ = RequestManager::GetInstance();
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_IS_STAND_BY, 0, 0);
    }
    LBSLOGI(LOCATOR, "LocatorAbility constructed.");
}

LocatorAbility::~LocatorAbility() {}

void LocatorAbility::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(LOCATOR, "LocatorAbility has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(LOCATOR, "failed to init LocatorAbility");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_SET_LOCATION_WORKING_STATE, 0, 0);
        locatorHandler_->SendHighPriorityEvent(EVENT_SYNC_LOCATION_STATUS, 0, 0);
    }
    LBSLOGI(LOCATOR, "LocatorAbility::OnStart start ability success.");
}

void LocatorAbility::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToAbility_ = false;
    if (!LocationDataRdbManager::SetLocationWorkingState(0)) {
        LBSLOGD(LOCATOR, "LocatorAbility::reset LocationWorkingState failed.");
    }
    LBSLOGI(LOCATOR, "LocatorAbility::OnStop ability stopped.");
}

void LocatorAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        RegisterAction();
        RegisterLocationPrivacyAction();
    }
}

void LocatorAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(LOCATOR, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }

    if (locationPrivacyEventSubscriber_ != nullptr) {
        bool ret = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(locationPrivacyEventSubscriber_);
        locationPrivacyEventSubscriber_ = nullptr;
        isLocationPrivacyActionRegistered_ = false;
        LBSLOGI(LOCATOR, "UnSubscribeCommonEvent locationPrivacyEventSubscriber_ result = %{public}d", ret);
        return;
    }

    if (locatorEventSubscriber_ == nullptr) {
        LBSLOGE(LOCATOR, "OnRemoveSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(locatorEventSubscriber_);
    isActionRegistered = false;
    LBSLOGI(LOCATOR, "UnSubscribeCommonEvent locatorEventSubscriber_ result = %{public}d", result);
}

bool LocatorAbility::Init()
{
    if (registerToAbility_) {
        return true;
    }
    LBSLOGI(LOCATOR, "LocatorAbility Init.");
    bool ret = Publish(AsObject());
    if (!ret) {
        LBSLOGE(LOCATOR, "Init add system ability failed!");
        return false;
    }
    UpdateSaAbility();
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_INIT_REQUEST_MANAGER, 0, RETRY_INTERVAL_OF_INIT_REQUEST_MANAGER);
        locatorHandler_->SendHighPriorityEvent(EVENT_PERIODIC_CHECK, 0, EVENT_PERIODIC_INTERVAL);
    }
    registerToAbility_ = true;
    return registerToAbility_;
}

void LocatorAbility::InitRequestManagerMap()
{
    std::unique_lock<ffrt::mutex> lock(requestsMutex_);
    if (requests_ != nullptr) {
#ifdef FEATURE_GNSS_SUPPORT
        std::list<std::shared_ptr<Request>> gnssList;
        requests_->insert(make_pair(GNSS_ABILITY, gnssList));
#endif
#ifdef FEATURE_NETWORK_SUPPORT
        std::list<std::shared_ptr<Request>> networkList;
        requests_->insert(make_pair(NETWORK_ABILITY, networkList));
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
        std::list<std::shared_ptr<Request>> passiveList;
        requests_->insert(make_pair(PASSIVE_ABILITY, passiveList));
#endif
    }
}

std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> LocatorAbility::GetRequests()
{
    std::unique_lock<ffrt::mutex> lock(requestsMutex_);
    return requests_;
}

int LocatorAbility::GetActiveRequestNum()
{
    std::unique_lock<ffrt::mutex> lock(requestsMutex_);
    int num = 0;
#ifdef FEATURE_GNSS_SUPPORT
    auto gpsListIter = requests_->find(GNSS_ABILITY);
    if (gpsListIter != requests_->end()) {
        auto list = &(gpsListIter->second);
        num += static_cast<int>(list->size());
    }
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    auto networkListIter = requests_->find(NETWORK_ABILITY);
    if (networkListIter != requests_->end()) {
        auto list = &(networkListIter->second);
        num += static_cast<int>(list->size());
    }
#endif
    return num;
}

std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> LocatorAbility::GetReceivers()
{
    std::unique_lock<ffrt::mutex> lock(receiversMutex_);
    return receivers_;
}

std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> LocatorAbility::GetProxyMap()
{
    std::unique_lock<std::mutex> lock(proxyMapMutex_);
    return proxyMap_;
}

void LocatorAbility::ApplyRequests(int delay)
{
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_APPLY_REQUIREMENTS, 0, delay * RETRY_INTERVAL_UNITE);
    }
}

void LocatorAbility::InitSaAbility()
{
    LBSLOGI(LOCATOR, "initSaAbility start");
    if (proxyMap_ == nullptr) {
        return;
    }
    UpdateSaAbilityHandler();
}

bool LocatorAbility::CheckSaValid()
{
    std::unique_lock<std::mutex> lock(proxyMapMutex_);
#ifdef FEATURE_GNSS_SUPPORT
    auto objectGnss = proxyMap_->find(GNSS_ABILITY);
    if (objectGnss == proxyMap_->end()) {
        LBSLOGI(LOCATOR, "gnss sa is null");
        return false;
    }
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    auto objectNetwork = proxyMap_->find(NETWORK_ABILITY);
    if (objectNetwork == proxyMap_->end()) {
        LBSLOGI(LOCATOR, "network sa is null");
        return false;
    }
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    auto objectPassive = proxyMap_->find(PASSIVE_ABILITY);
    if (objectPassive == proxyMap_->end()) {
        LBSLOGI(LOCATOR, "passive sa is null");
        return false;
    }
#endif
    return true;
}

LocationErrCode LocatorAbility::UpdateSaAbility()
{
    auto event = AppExecFwk::InnerEvent::Get(EVENT_UPDATE_SA, 0);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(event);
    }
    return ERRCODE_SUCCESS;
}

void LocatorAbility::UpdateSaAbilityHandler()
{
    int state = LocationDataRdbManager::QuerySwitchState();
    LBSLOGI(LOCATOR, "update location subability enable state, switch state=%{public}d, action registered=%{public}d",
        state, isActionRegistered);
    bool isEnabled = (state == ENABLED);
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    if (locatorBackgroundProxy == nullptr) {
        LBSLOGE(LOCATOR, "UpdateSaAbilityHandler: LocatorBackgroundProxy is nullptr");
        return;
    }
    locatorBackgroundProxy->OnSaStateChange(isEnabled);
    UpdateLoadedSaMap();
    std::unique_lock<ffrt::mutex> lock(loadedSaMapMutex_);
    for (auto iter = loadedSaMap_->begin(); iter != loadedSaMap_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = iter->second;
        MessageParcel data;
        if (iter->first == GNSS_ABILITY) {
#ifdef FEATURE_GNSS_SUPPORT
            data.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
#endif
        } else if (iter->first == NETWORK_ABILITY) {
#ifdef FEATURE_NETWORK_SUPPORT
            data.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
#endif
        } else if (iter->first == PASSIVE_ABILITY) {
#ifdef FEATURE_PASSIVE_SUPPORT
            data.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
#endif
        }
        data.WriteBool(isEnabled);

        MessageParcel reply;
        MessageOption option;
        int error = remoteObject->SendRequest(SET_ENABLE, data, reply, option);
        if (error != ERR_OK) {
            LBSLOGI(LOCATOR, "enable %{public}s ability, remote result %{public}d", (iter->first).c_str(), error);
        }
    }
    SendSwitchState(isEnabled ? 1 : 0);
}

bool LocatorAbility::CancelIdleState()
{
    bool ret = CancelIdle();
    if (!ret) {
        LBSLOGE(LOCATOR, "%{public}s cancel idle failed!", __func__);
        return false;
    }
    return true;
}

void LocatorAbility::RemoveUnloadTask(uint32_t code)
{
    if (locatorHandler_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s locatorHandler is nullptr", __func__);
        return;
    }
    if (code == static_cast<uint16_t>(LocatorInterfaceCode::PROXY_PID_FOR_FREEZE) ||
        code == static_cast<uint16_t>(LocatorInterfaceCode::RESET_ALL_PROXY)) {
        return;
    }
    locatorHandler_->RemoveTask(UNLOAD_TASK);
}

void LocatorAbility::PostUnloadTask(uint32_t code)
{
    if (code == static_cast<uint16_t>(LocatorInterfaceCode::PROXY_PID_FOR_FREEZE) ||
        code == static_cast<uint16_t>(LocatorInterfaceCode::RESET_ALL_PROXY)) {
        return;
    }
    if (CheckIfLocatorConnecting()) {
        return;
    }
    auto task = [this]() {
        LocationSaLoadManager::UnInitLocationSa(LOCATION_LOCATOR_SA_ID);
    };
    if (locatorHandler_ != nullptr) {
        locatorHandler_->PostTask(task, UNLOAD_TASK, RETRY_INTERVAL_OF_UNLOAD_SA);
    }
}

void LocatorAbility::SendSwitchState(const int state)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_SEND_SWITCHSTATE_TO_HIFENCE, state);
    if (locatorHandler_ != nullptr && locatorHandler_->SendEvent(event)) {
        LBSLOGD(LOCATOR, "%{public}s: EVENT_SEND_SWITCHSTATE_TO_HIFENCE Send Success", __func__);
    }
}

bool LocatorAbility::CheckIfLocatorConnecting()
{
    return LocatorRequiredDataManager::GetInstance()->IsConnecting() || GetActiveRequestNum() > 0;
}

LocationErrCode LocatorAbility::EnableAbility(bool isEnabled)
{
    LBSLOGI(LOCATOR, "EnableAbility %{public}d", isEnabled);
    int modeValue = isEnabled ? 1 : 0;
    int currentSwitchState = LocationDataRdbManager::QuerySwitchState();
    if (modeValue == currentSwitchState) {
        LBSLOGD(LOCATOR, "no need to set location ability, enable:%{public}d", modeValue);
        return ERRCODE_SUCCESS;
    }
    if (LocationDataRdbManager::SetSwitchState(modeValue) != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR, "%{public}s: can not set state to db", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (currentSwitchState != DEFAULT_STATE) {
        // update param
        LocationDataRdbManager::SetSwitchMode(isEnabled ? ENABLED : DISABLED);
    }
    UpdateSaAbility();
    ApplyRequests(0);
    std::string state = isEnabled ? "enable" : "disable";
    ReportDataToResSched(state);
    WriteLocationSwitchStateEvent(state);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::GetSwitchState(int& state)
{
    int res = LocationDataRdbManager::GetSwitchMode();
    if (res == DISABLED || res == ENABLED) {
        state = res;
        return ERRCODE_SUCCESS;
    }
    state = LocationDataRdbManager::QuerySwitchState();
    if (res == DEFAULT_STATE && state != DEFAULT_STATE) {
        // update param
        LocationDataRdbManager::SetSwitchMode(state);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::IsLocationPrivacyConfirmed(const int type, bool& isConfirmed)
{
    return LocationConfigManager::GetInstance()->GetPrivacyTypeState(type, isConfirmed);
}

LocationErrCode LocatorAbility::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    return LocationConfigManager::GetInstance()->SetPrivacyTypeState(type, isConfirmed);
}

LocationErrCode LocatorAbility::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "register an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) SwitchCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<std::mutex> lock(switchMutex_);
    switchCallbacks_->erase(uid);
    switchCallbacks_->insert(std::make_pair(uid, switchCallback));
    LBSLOGD(LOCATOR, "after uid:%{public}d register, switch callback size:%{public}s",
        uid, std::to_string(switchCallbacks_->size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "unregister an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(switchMutex_);
    pid_t uid = -1;
    for (auto iter = switchCallbacks_->begin(); iter != switchCallbacks_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        if (remoteObject == callback) {
            uid = iter->first;
            break;
        }
    }
    switchCallbacks_->erase(uid);
    LBSLOGD(LOCATOR, "after uid:%{public}d unregister, switch callback size:%{public}s",
        uid, std::to_string(switchCallbacks_->size()).c_str());
    return ERRCODE_SUCCESS;
}

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::SendGnssRequest(int type, MessageParcel &data, MessageParcel &reply)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_GNSS_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> objectGnss =
            CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    if (objectGnss == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    MessageOption option;
    objectGnss->SendRequest(type, data, reply, option);
    return LocationErrCode(reply.ReadInt32());
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR, "uid is: %{public}d", uid);
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::REG_GNSS_STATUS), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::UNREG_GNSS_STATUS), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::REG_NMEA), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::UNREG_NMEA), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback, std::string bundleName)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteInt32(request->reportingPeriodSec);
    dataToStub.WriteBool(request->wakeUpCacheQueueFull);
    dataToStub.WriteRemoteObject(callback->AsObject());
    dataToStub.WriteString16(Str8ToStr16(bundleName));
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::REG_CACHED), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteRemoteObject(callback->AsObject());
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::UNREG_CACHED), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::GetCachedGnssLocationsSize(int& size)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode =
        SendGnssRequest(static_cast<int>(GnssInterfaceCode::GET_CACHED_SIZE), dataToStub, replyToStub);
    if (errorCode == ERRCODE_SUCCESS) {
        size = replyToStub.ReadInt32();
    }
    return errorCode;
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::FlushCachedGnssLocations()
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::FLUSH_CACHED), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteInt32(commands->scenario);
    dataToStub.WriteString16(Str8ToStr16(commands->command));
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::SEND_COMMANDS), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::AddFence(std::shared_ptr<GeofenceRequest>& request)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    request->Marshalling(dataToStub);
    return SendGnssRequest(
        static_cast<int>(GnssInterfaceCode::ADD_FENCE_INFO), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::RemoveFence(std::shared_ptr<GeofenceRequest>& request)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    request->Marshalling(dataToStub);
    return SendGnssRequest(
        static_cast<int>(GnssInterfaceCode::REMOVE_FENCE_INFO), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    request->Marshalling(dataToStub);
    return SendGnssRequest(
        static_cast<int>(GnssInterfaceCode::ADD_GNSS_GEOFENCE), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataToStub.WriteInt32(request->GetFenceId());
    dataToStub.WriteString(request->GetBundleName());
    return SendGnssRequest(
        static_cast<int>(GnssInterfaceCode::REMOVE_GNSS_GEOFENCE), dataToStub, replyToStub);
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::SendLocationMockMsgToGnssSa(const sptr<IRemoteObject> obj,
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToGnssSa obj is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<GnssAbilityProxy> gnssProxy = std::make_unique<GnssAbilityProxy>(obj);
    LocationErrCode errorCode = ERRCODE_NOT_SUPPORTED;
    if (msgId == static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK)) {
        errorCode = gnssProxy->EnableMock();
    } else if (msgId == static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK)) {
        errorCode = gnssProxy->DisableMock();
    } else if (msgId == static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS)) {
        errorCode = gnssProxy->SetMocked(timeInterval, location);
    }
    return errorCode;
}
#endif

#ifdef FEATURE_NETWORK_SUPPORT
LocationErrCode LocatorAbility::SendLocationMockMsgToNetworkSa(const sptr<IRemoteObject> obj,
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToNetworkSa obj is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<NetworkAbilityProxy> networkProxy =
        std::make_unique<NetworkAbilityProxy>(obj);
    LocationErrCode errorCode = ERRCODE_NOT_SUPPORTED;
    if (msgId == static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK)) {
        errorCode = networkProxy->EnableMock();
    } else if (msgId == static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK)) {
        errorCode = networkProxy->DisableMock();
    } else if (msgId == static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS)) {
        errorCode = networkProxy->SetMocked(timeInterval, location);
    }
    return errorCode;
}
#endif

#ifdef FEATURE_PASSIVE_SUPPORT
LocationErrCode LocatorAbility::SendLocationMockMsgToPassiveSa(const sptr<IRemoteObject> obj,
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToNetworkSa obj is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<PassiveAbilityProxy> passiveProxy =
        std::make_unique<PassiveAbilityProxy>(obj);
    LocationErrCode errorCode = ERRCODE_NOT_SUPPORTED;
    if (msgId == static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK)) {
        errorCode = passiveProxy->EnableMock();
    } else if (msgId == static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK)) {
        errorCode = passiveProxy->DisableMock();
    } else if (msgId == static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS)) {
        errorCode = passiveProxy->SetMocked(timeInterval, location);
    }
    return errorCode;
}
#endif

LocationErrCode LocatorAbility::ProcessLocationMockMsg(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
#if !defined(FEATURE_GNSS_SUPPORT) && !defined(FEATURE_NETWORK_SUPPORT) && !defined(FEATURE_PASSIVE_SUPPORT)
    LBSLOGE(LOCATOR, "%{public}s: mock service unavailable", __func__);
    return ERRCODE_NOT_SUPPORTED;
#endif
    if (!CheckSaValid()) {
        UpdateProxyMap();
    }

    std::unique_lock<std::mutex> lock(proxyMapMutex_);
    for (auto iter = proxyMap_->begin(); iter != proxyMap_->end(); iter++) {
        auto obj = iter->second;
        if (iter->first == GNSS_ABILITY) {
#ifdef FEATURE_GNSS_SUPPORT
            SendLocationMockMsgToGnssSa(obj, timeInterval, location, msgId);
#endif
        } else if (iter->first == NETWORK_ABILITY) {
#ifdef FEATURE_NETWORK_SUPPORT
            SendLocationMockMsgToNetworkSa(obj, timeInterval, location, msgId);
#endif
        } else if (iter->first == PASSIVE_ABILITY) {
#ifdef FEATURE_PASSIVE_SUPPORT
            SendLocationMockMsgToPassiveSa(obj, timeInterval, location, msgId);
#endif
        }
    }
    return ERRCODE_SUCCESS;
}

void LocatorAbility::UpdateLoadedSaMap()
{
    std::unique_lock<ffrt::mutex> lock(loadedSaMapMutex_);
    loadedSaMap_->clear();
    if (LocationSaLoadManager::CheckIfSystemAbilityAvailable(LOCATION_GNSS_SA_ID)) {
        sptr<IRemoteObject> objectGnss =
            CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
        loadedSaMap_->insert(make_pair(GNSS_ABILITY, objectGnss));
    }
    if (LocationSaLoadManager::CheckIfSystemAbilityAvailable(LOCATION_NETWORK_LOCATING_SA_ID)) {
        sptr<IRemoteObject> objectNetwork =
            CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID, CommonUtils::InitDeviceId());
        loadedSaMap_->insert(make_pair(NETWORK_ABILITY, objectNetwork));
    }
    if (LocationSaLoadManager::CheckIfSystemAbilityAvailable(LOCATION_NOPOWER_LOCATING_SA_ID)) {
        sptr<IRemoteObject> objectPassive =
            CommonUtils::GetRemoteObject(LOCATION_NOPOWER_LOCATING_SA_ID, CommonUtils::InitDeviceId());
        loadedSaMap_->insert(make_pair(PASSIVE_ABILITY, objectPassive));
    }
}

void LocatorAbility::UpdateProxyMap()
{
    std::unique_lock<std::mutex> lock(proxyMapMutex_);
#ifdef FEATURE_GNSS_SUPPORT
    // init gnss ability sa
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_GNSS_SA_ID)) {
        return;
    }
    sptr<IRemoteObject> objectGnss = CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    if (objectGnss != nullptr) {
        proxyMap_->insert(make_pair(GNSS_ABILITY, objectGnss));
    } else {
        LBSLOGE(LOCATOR, "GetRemoteObject gnss sa is null");
    }
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    // init network ability sa
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_NETWORK_LOCATING_SA_ID)) {
        return;
    }
    sptr<IRemoteObject> objectNetwork = CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID,
        CommonUtils::InitDeviceId());
    if (objectNetwork != nullptr) {
        proxyMap_->insert(make_pair(NETWORK_ABILITY, objectNetwork));
    } else {
        LBSLOGE(LOCATOR, "GetRemoteObject network sa is null");
    }
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    // init passive ability sa
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID)) {
        return;
    }
    sptr<IRemoteObject> objectPassive = CommonUtils::GetRemoteObject(LOCATION_NOPOWER_LOCATING_SA_ID,
        CommonUtils::InitDeviceId());
    if (objectPassive != nullptr) {
        proxyMap_->insert(make_pair(PASSIVE_ABILITY, objectPassive));
    } else {
        LBSLOGE(LOCATOR, "GetRemoteObject passive sa is null");
    }
#endif
}

LocationErrCode LocatorAbility::EnableLocationMock()
{
    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> location;
    return ProcessLocationMockMsg(timeInterval, location,
        static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK));
}

LocationErrCode LocatorAbility::DisableLocationMock()
{
    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> location;
    return ProcessLocationMockMsg(timeInterval, location,
        static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK));
}

LocationErrCode LocatorAbility::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    return ProcessLocationMockMsg(timeInterval, location,
        static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS));
}

LocationErrCode LocatorAbility::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback, AppIdentity &identity)
{
#if !defined(FEATURE_GNSS_SUPPORT) && !defined(FEATURE_NETWORK_SUPPORT) && !defined(FEATURE_PASSIVE_SUPPORT)
    LBSLOGE(LOCATOR, "%{public}s: service unavailable", __func__);
    return ERRCODE_NOT_SUPPORTED;
#endif
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED) {
        ReportErrorStatus(callback, ERROR_SWITCH_UNOPEN);
    }
    // update offset before add request
    if (reportManager_ == nullptr || requestManager_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reportManager_->UpdateRandom();
    std::shared_ptr<Request> request = std::make_shared<Request>(requestConfig, callback, identity);
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) LocatorCallbackDeathRecipient(identity.GetTokenId()));
    callback->AsObject()->AddDeathRecipient(death);
    request->SetLocatorCallbackRecipient(death);
    HookUtils::ExecuteHookWhenStartLocation(request);
    OHOS::Security::AccessToken::PermUsedTypeEnum type =
        Security::AccessToken::AccessTokenKit::GetUserGrantedPermissionUsedType(request->GetTokenId(),
        ACCESS_APPROXIMATELY_LOCATION);
    request->SetPermUsedType(static_cast<int>(type));

#ifdef EMULATOR_ENABLED
    // for emulator, report cache location is unnecessary
    HandleStartLocating(request, callback);
#else
    if (NeedReportCacheLocation(request, callback)) {
        LBSLOGI(LOCATOR, "report cache location to %{public}s", identity.GetBundleName().c_str());
        callback->AsObject()->RemoveDeathRecipient(death);
    } else {
        HandleStartLocating(request, callback);
    }
#endif
    return ERRCODE_SUCCESS;
}

bool LocatorAbility::IsCacheVaildScenario(const sptr<RequestConfig>& requestConfig)
{
    if (requestConfig->GetPriority() != LOCATION_PRIORITY_ACCURACY &&
        ((requestConfig->GetPriority() == LOCATION_PRIORITY_LOCATING_SPEED) ||
        (requestConfig->GetScenario() == SCENE_DAILY_LIFE_SERVICE) ||
        ((requestConfig->GetScenario() == SCENE_UNSET) && (requestConfig->GetPriority() == PRIORITY_FAST_FIRST_FIX)) ||
        ((requestConfig->GetScenario() == SCENE_UNSET) && (requestConfig->GetPriority() == PRIORITY_LOW_POWER)))) {
        return true;
    }
    return false;
}

bool LocatorAbility::IsSingleRequest(const sptr<RequestConfig>& requestConfig)
{
    if (requestConfig->GetFixNumber() == 1) {
        return true;
    }
    return false;
}

void LocatorAbility::UpdatePermissionUsedRecord(uint32_t tokenId, std::string permissionName,
    int permUsedType, int succCnt, int failCnt)
{
    Security::AccessToken::AddPermParamInfo info;
    info.tokenId = tokenId;
    info.permissionName = permissionName;
    info.successCount = succCnt;
    info.failCount = failCnt;
    info.type = static_cast<OHOS::Security::AccessToken::PermissionUsedType>(permUsedType);
    Security::AccessToken::PrivacyKit::AddPermissionUsedRecord(info);
}

bool LocatorAbility::NeedReportCacheLocation(const std::shared_ptr<Request>& request, sptr<ILocatorCallback>& callback)
{
    if (reportManager_ == nullptr || request == nullptr) {
        return false;
    }
    // report cache location
    if (IsSingleRequest(request->GetRequestConfig()) && IsCacheVaildScenario(request->GetRequestConfig())) {
        auto cacheLocation = reportManager_->GetCacheLocation(request);
        if (cacheLocation != nullptr && callback != nullptr) {
            auto workRecordStatistic = WorkRecordStatistic::GetInstance();
            if (!workRecordStatistic->Update("CacheLocation", 1)) {
                LBSLOGE(LOCATOR, "%{public}s line:%{public}d workRecordStatistic::Update failed", __func__, __LINE__);
            }
            PrivacyKit::StartUsingPermission(request->GetTokenId(), ACCESS_APPROXIMATELY_LOCATION);
            callback->OnLocationReport(cacheLocation);
            // add location permission using record
            UpdatePermissionUsedRecord(request->GetTokenId(), ACCESS_APPROXIMATELY_LOCATION,
                request->GetPermUsedType(), 1, 0);
            PrivacyKit::StopUsingPermission(request->GetTokenId(), ACCESS_APPROXIMATELY_LOCATION);
            if (locatorHandler_ != nullptr &&
                locatorHandler_->SendHighPriorityEvent(EVENT_UPDATE_LASTLOCATION_REQUESTNUM, 0, 1)) {
                LBSLOGD(LOCATOR, "%{public}s: EVENT_UPDATE_LASTLOCATION_REQUESTNUM Send Success", __func__);
            }
            return true;
        }
    } else if (!IsSingleRequest(request->GetRequestConfig()) && IsCacheVaildScenario(request->GetRequestConfig())) {
        auto cacheLocation = reportManager_->GetCacheLocation(request);
        if (cacheLocation != nullptr && callback != nullptr) {
            auto workRecordStatistic = WorkRecordStatistic::GetInstance();
            if (!workRecordStatistic->Update("CacheLocation", 1)) {
                LBSLOGE(LOCATOR, "%{public}s line:%{public}d workRecordStatistic::Update failed", __func__, __LINE__);
            }
            callback->OnLocationReport(cacheLocation);
            // add location permission using record
            UpdatePermissionUsedRecord(request->GetTokenId(), ACCESS_APPROXIMATELY_LOCATION,
                request->GetPermUsedType(), 1, 0);
        }
    }
    return false;
}

void LocatorAbility::HandleStartLocating(const std::shared_ptr<Request>& request, sptr<ILocatorCallback>& callback)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_START_LOCATING, request);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendEvent(event);
    }
    if (callback != nullptr) {
        ReportLocationStatus(callback, SESSION_START);
    }
}

LocationErrCode LocatorAbility::StopLocating(sptr<ILocatorCallback>& callback)
{
#if !defined(FEATURE_GNSS_SUPPORT) && !defined(FEATURE_NETWORK_SUPPORT) && !defined(FEATURE_PASSIVE_SUPPORT)
    LBSLOGE(LOCATOR, "%{public}s: service unavailable", __func__);
    return ERRCODE_NOT_SUPPORTED;
#endif
    if (requestManager_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<LocatorCallbackMessage> callbackMessage = std::make_unique<LocatorCallbackMessage>();
    callbackMessage->SetCallback(callback);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_STOP_LOCATING, callbackMessage);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendEvent(event);
    }
    if (callback != nullptr) {
        ReportLocationStatus(callback, SESSION_STOP);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::GetCacheLocation(std::unique_ptr<Location>& loc, AppIdentity &identity)
{
    if (locatorHandler_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto lastLocation = reportManager_->GetLastLocation();
    if (locatorHandler_ != nullptr &&
        locatorHandler_->SendHighPriorityEvent(EVENT_UPDATE_LASTLOCATION_REQUESTNUM, 0, 1)) {
        LBSLOGD(LOCATOR, "%{public}s: EVENT_UPDATE_LASTLOCATION_REQUESTNUM Send Success", __func__);
    }
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    sptr<ILocatorCallback> callback;
    std::shared_ptr<Request> request = std::make_shared<Request>(requestConfig, callback, identity);
    loc = reportManager_->GetPermittedLocation(request, lastLocation);
    reportManager_->UpdateLocationByRequest(identity.GetTokenId(), identity.GetTokenIdEx(), loc);
    if (loc == nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_GET_CACHED_LOCATION_FAILED, identity.GetTokenId(), 0);
        return ERRCODE_LOCATING_FAIL;
    }
    if (fabs(loc->GetLatitude() - 0.0) > PRECISION
        && fabs(loc->GetLongitude() - 0.0) > PRECISION) {
        // add location permission using record
        locatorHandler_->SendHighPriorityEvent(EVENT_GET_CACHED_LOCATION_SUCCESS, identity.GetTokenId(), 0);
        return ERRCODE_SUCCESS;
    }
    locatorHandler_->SendHighPriorityEvent(EVENT_GET_CACHED_LOCATION_FAILED, identity.GetTokenId(), 0);
    return ERRCODE_LOCATING_FAIL;
}

bool LocatorAbility::CheckIsReportPermitted(AppIdentity &identity)
{
    std::unique_lock<ffrt::mutex> lock(requestsMutex_);
    if (requests_ == nullptr || requests_->empty()) {
        LBSLOGE(LOCATOR, "requests map is empty");
        return false;
    }

    bool isPermitted = true;
    int switchState = DISABLED;
    GetSwitchState(switchState);
    for (auto mapIter = requests_->begin(); mapIter != requests_->end(); mapIter++) {
        auto list = mapIter->second;
        for (auto request : list) {
            if (request == nullptr || request->GetTokenId() != identity.GetTokenId()) {
                continue;
            }
            auto locationErrorCallback = request->GetLocationErrorCallBack();
            if (locationErrorCallback != nullptr) {
                if (switchState == DISABLED) {
                    LBSLOGE(LOCATOR, "%{public}s line:%{public}d location switch is off", __func__, __LINE__);
                    isPermitted = false;
                    locationErrorCallback->OnErrorReport(LOCATING_FAILED_LOCATION_SWITCH_OFF);
                    continue;
                }

                std::string bundleName = "";
                auto tokenId = request->GetTokenId();
                auto firstTokenId = request->GetFirstTokenId();
                auto tokenIdEx = request->GetTokenIdEx();
                auto uid =  request->GetUid();
                if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
                    LBSLOGE(LOCATOR, "Fail to Get bundle name: uid = %{public}d.", uid);
                }
                if (reportManager_->IsAppBackground(bundleName, tokenId, tokenIdEx, uid) &&
                    !PermissionManager::CheckBackgroundPermission(tokenId, firstTokenId)) {
                    isPermitted = false;
                    //app background, no background permission, not ContinuousTasks
                    locationErrorCallback->OnErrorReport(LOCATING_FAILED_BACKGROUND_PERMISSION_DENIED);
                    continue;
                }
                if (!PermissionManager::CheckLocationPermission(tokenId, firstTokenId) &&
                    !PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) {
                    LBSLOGE(LOCATOR, "%{public}d has no location permission failed", tokenId);
                    isPermitted = false;
                    locationErrorCallback->OnErrorReport(LOCATING_FAILED_LOCATION_PERMISSION_DENIED);
                    continue;
                }
            }
        }
    }
    return isPermitted;
}

LocationErrCode LocatorAbility::ReportLocation(
    const std::unique_ptr<Location>& location, std::string abilityName, AppIdentity &identity)
{
    if (requests_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
    sptr<ILocatorCallback> callback;
    std::shared_ptr<Request> request = std::make_shared<Request>(requestConfig, callback, identity);
    if (!CheckIsReportPermitted(identity)) {
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d report is not allowed", __func__, __LINE__);
        return ERRCODE_NOT_SUPPORTED;
    }
    std::unique_ptr<LocationMessage> locationMessage = std::make_unique<LocationMessage>();
    locationMessage->SetAbilityName(abilityName);
    locationMessage->SetLocation(location);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_REPORT_LOCATION_MESSAGE, locationMessage);
    if (locatorHandler_ != nullptr && locatorHandler_->SendEvent(event)) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_SERVICE_UNAVAILABLE;
}

LocationErrCode LocatorAbility::ReportLocationStatus(sptr<ILocatorCallback>& callback, int result)
{
    int state = DISABLED;
    LocationErrCode errorCode = GetSwitchState(state);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (state == DISABLED) {
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d location switch is off",
            __func__, __LINE__);
        return ERRCODE_SWITCH_OFF;
    }
    if (reportManager_->ReportRemoteCallback(callback, ILocatorCallback::RECEIVE_LOCATION_STATUS_EVENT, result)) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_SERVICE_UNAVAILABLE;
}

LocationErrCode LocatorAbility::ReportErrorStatus(sptr<ILocatorCallback>& callback, int result)
{
    int state = DISABLED;
    LocationErrCode errorCode = GetSwitchState(state);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (state == DISABLED) {
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d location switch is off",
            __func__, __LINE__);
        return ERRCODE_SWITCH_OFF;
    }
    if (reportManager_->ReportRemoteCallback(callback, ILocatorCallback::RECEIVE_ERROR_INFO_EVENT, result)) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_SERVICE_UNAVAILABLE;
}

void LocatorAbility::RegisterAction()
{
    if (isActionRegistered) {
        LBSLOGI(LOCATOR, "action has already registered");
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_DEVICE_IDLE_MODE_CHANGED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    locatorEventSubscriber_ = std::make_shared<LocatorEventSubscriber>(subscriberInfo);

    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(locatorEventSubscriber_);
    if (!result) {
        LBSLOGE(LOCATOR, "Failed to subscriber locator event, result = %{public}d", result);
        isActionRegistered = false;
    } else {
        LBSLOGI(LOCATOR, "success to subscriber locator event, result = %{public}d", result);
        isActionRegistered = true;
    }
}

void LocatorAbility::RegisterLocationPrivacyAction()
{
    if (isLocationPrivacyActionRegistered_) {
        LBSLOGI(LOCATOR, "location privacy action has already registered");
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(LOCATION_PRIVACY_ACCEPT_EVENT);
    matchingSkills.AddEvent(LOCATION_PRIVACY_REJECT_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    locationPrivacyEventSubscriber_ = std::make_shared<LocatorEventSubscriber>(subscriberInfo);
    subscriberInfo.SetPermission("ohos.permission.PUBLISH_LOCATION_EVENT");

    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(locationPrivacyEventSubscriber_);
    if (!result) {
        LBSLOGE(LOCATOR, "Failed to subscriber location privacy event, result = %{public}d", result);
        isLocationPrivacyActionRegistered_ = false;
    } else {
        LBSLOGI(LOCATOR, "success to subscriber location privacy event, result = %{public}d", result);
        isLocationPrivacyActionRegistered_ = true;
    }
}

#ifdef FEATURE_GEOCODE_SUPPORT
LocationErrCode LocatorAbility::IsGeoConvertAvailable(bool &isAvailable)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        isAvailable = false;
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    SendGeoRequest(static_cast<int>(LocatorInterfaceCode::GEO_IS_AVAILABLE), dataParcel, replyParcel);
    LocationErrCode errorCode = LocationErrCode(replyParcel.ReadInt32());
    if (errorCode == ERRCODE_SUCCESS) {
        isAvailable = replyParcel.ReadBool();
    } else {
        isAvailable = false;
    }
    return errorCode;
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
void LocatorAbility::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply, std::string bundleName)
{
    MessageParcel dataParcel;
    auto requestTime = CommonUtils::GetCurrentTimeStamp();
    GeoCodeType requestType = GeoCodeType::REQUEST_REVERSE_GEOCODE;
    GeoConvertRequest::OrderParcel(data, dataParcel, requestType, bundleName);
    auto geoConvertRequest = GeoConvertRequest::Unmarshalling(dataParcel, requestType);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_SEND_GEOREQUEST, geoConvertRequest);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendEvent(event);
    }
    int errorCode = reply.ReadInt32();
    WriteLocationInnerEvent(GEOCODE_REQUEST, {
        "type", "ReverseGeocode",
        "appName", bundleName,
        "subCode", std::to_string(errorCode),
        "requestTime", std::to_string(requestTime),
        "receiveTime", std::to_string(CommonUtils::GetCurrentTimeStamp()),
    });
    HookUtils::ExecuteHookWhenGetAddressFromLocation(bundleName);
    reply.RewindRead(0);
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
void LocatorAbility::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply, std::string bundleName)
{
    MessageParcel dataParcel;
    auto requestTime = CommonUtils::GetCurrentTimeStamp();
    GeoCodeType requestType = GeoCodeType::REQUEST_GEOCODE;
    GeoConvertRequest::OrderParcel(data, dataParcel, requestType, bundleName);
    auto geoConvertRequest = GeoConvertRequest::Unmarshalling(dataParcel, requestType);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_SEND_GEOREQUEST, geoConvertRequest);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendEvent(event);
    }
    int errorCode = reply.ReadInt32();
    WriteLocationInnerEvent(GEOCODE_REQUEST, {
        "type", "Geocode",
        "appName", bundleName,
        "subCode", std::to_string(errorCode),
        "requestTime", std::to_string(requestTime),
        "receiveTime", std::to_string(CommonUtils::GetCurrentTimeStamp()),
    });
    HookUtils::ExecuteHookWhenGetAddressFromLocationName(bundleName);
    reply.RewindRead(0);
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
LocationErrCode LocatorAbility::SendGeoRequest(int type, MessageParcel &data, MessageParcel &reply)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_GEO_CONVERT_SA_ID)) {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    MessageOption option;
    remoteObject->SendRequest(type, data, reply, option);
    return ERRCODE_SUCCESS;
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
LocationErrCode LocatorAbility::EnableReverseGeocodingMock()
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    SendGeoRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK), dataParcel, replyParcel);
    return LocationErrCode(replyParcel.ReadInt32());
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
LocationErrCode LocatorAbility::DisableReverseGeocodingMock()
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    SendGeoRequest(static_cast<int>(LocatorInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK), dataParcel, replyParcel);
    return LocationErrCode(replyParcel.ReadInt32());
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
LocationErrCode LocatorAbility::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    dataParcel.WriteInt32(mockInfo.size());
    for (size_t i = 0; i < mockInfo.size(); i++) {
        mockInfo[i]->Marshalling(dataParcel);
    }
    SendGeoRequest(static_cast<int>(LocatorInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO), dataParcel, replyParcel);
    return LocationErrCode(replyParcel.ReadInt32());
}
#endif

LocationErrCode LocatorAbility::ProxyForFreeze(std::set<int> pidList, bool isProxy)
{
    std::unique_lock<std::mutex> lock(proxyPidsMutex_, std::defer_lock);
    lock.lock();
    if (isProxy) {
        for (auto it = pidList.begin(); it != pidList.end(); it++) {
            proxyPids_.insert(*it);
            LBSLOGI(LOCATOR, "Start locator proxy, pid: %{public}d, isProxy: %{public}d, timestamp = %{public}s",
                *it, isProxy, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
        }
    } else {
        for (auto it = pidList.begin(); it != pidList.end(); it++) {
            proxyPids_.erase(*it);
            LBSLOGI(LOCATOR, "Start locator proxy, pid: %{public}d, isProxy: %{public}d, timestamp = %{public}s",
                *it, isProxy, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
        }
    }
    lock.unlock();
    if (GetActiveRequestNum() <= 0) {
        LBSLOGD(LOCATOR, "no active request, do not refresh.");
        return ERRCODE_SUCCESS;
    }
    // for proxy uid update, should send message to refresh requests
    ApplyRequests(0);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::ResetAllProxy()
{
    LBSLOGI(LOCATOR, "Start locator ResetAllProxy");
    std::unique_lock<std::mutex> lock(proxyPidsMutex_, std::defer_lock);
    lock.lock();
    proxyPids_.clear();
    lock.unlock();
    if (GetActiveRequestNum() <= 0) {
        LBSLOGD(LOCATOR, "no active request, do not refresh.");
        return ERRCODE_SUCCESS;
    }
    // for proxy uid update, should send message to refresh requests
    ApplyRequests(0);
    return ERRCODE_SUCCESS;
}

bool LocatorAbility::IsProxyPid(int32_t pid)
{
    std::unique_lock<std::mutex> lock(proxyPidsMutex_);
    return proxyPids_.find(pid) != proxyPids_.end();
}

void LocatorAbility::RegisterPermissionCallback(const uint32_t callingTokenId,
    const std::vector<std::string>& permissionNameList)
{
    std::unique_lock<ffrt::mutex> lock(permissionMapMutex_);
    if (permissionMap_ == nullptr) {
        LBSLOGE(LOCATOR, "permissionMap is null.");
        return;
    }
    PermStateChangeScope scopeInfo;
    scopeInfo.permList = permissionNameList;
    scopeInfo.tokenIDs = {callingTokenId};
    auto callbackPtr = std::make_shared<PermissionStatusChangeCb>(scopeInfo);
    permissionMap_->erase(callingTokenId);
    permissionMap_->insert(std::make_pair(callingTokenId, callbackPtr));
    LBSLOGD(LOCATOR, "after tokenId:%{public}d register, permission callback size:%{public}s",
        callingTokenId, std::to_string(permissionMap_->size()).c_str());
    int32_t res = AccessTokenKit::RegisterPermStateChangeCallback(callbackPtr);
    if (res != SUCCESS) {
        LBSLOGE(LOCATOR, "RegisterPermStateChangeCallback failed.");
    }
}

void LocatorAbility::UnregisterPermissionCallback(const uint32_t callingTokenId)
{
    std::unique_lock<ffrt::mutex> lock(permissionMapMutex_);
    if (permissionMap_ == nullptr) {
        LBSLOGE(LOCATOR, "permissionMap is null.");
        return;
    }
    auto iter = permissionMap_->find(callingTokenId);
    if (iter != permissionMap_->end()) {
        auto callbackPtr = iter->second;
        int32_t res = AccessTokenKit::UnRegisterPermStateChangeCallback(callbackPtr);
        if (res != SUCCESS) {
            LBSLOGE(LOCATOR, "UnRegisterPermStateChangeCallback failed.");
        }
    }
    permissionMap_->erase(callingTokenId);
    LBSLOGD(LOCATOR, "after tokenId:%{public}d unregister, permission callback size:%{public}s",
        callingTokenId, std::to_string(permissionMap_->size()).c_str());
}

void LocatorAbility::ReportDataToResSched(std::string state)
{
#ifdef RES_SCHED_SUPPROT
    std::unordered_map<std::string, std::string> payload;
    payload["state"] = state;
    uint32_t type = ResourceSchedule::ResType::RES_TYPE_LOCATION_STATUS_CHANGE;
    int64_t value =  ResourceSchedule::ResType::LocationStatus::LOCATION_SWTICH_CHANGE;
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, value, payload);
#endif
}

void LocatorAbility::UpdateLastLocationRequestNum()
{
    if (locatorHandler_ != nullptr &&
        locatorHandler_->SendHighPriorityEvent(EVENT_UPDATE_LASTLOCATION_REQUESTNUM, 0, RETRY_INTERVAL_UNITE)) {
        LBSLOGD(LOCATOR, "%{public}s: EVENT_UPDATE_LASTLOCATION_REQUESTNUM Send Success", __func__);
    }
}

#ifdef FEATURE_GNSS_SUPPORT
LocationErrCode LocatorAbility::QuerySupportCoordinateSystemType(
    std::vector<CoordinateSystemType>& coordinateSystemTypes)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto errCode = SendGnssRequest(
        static_cast<int>(GnssInterfaceCode::GET_GEOFENCE_SUPPORT_COORDINATE_SYSTEM_TYPE),
        dataToStub, replyToStub);
    if (errCode == ERRCODE_SUCCESS) {
        int size = replyToStub.ReadInt32();
        size = size > COORDINATE_SYSTEM_TYPE_SIZE ? COORDINATE_SYSTEM_TYPE_SIZE : size;
        for (int i = 0; i < size; i++) {
            int coordinateSystemType = replyToStub.ReadInt32();
            coordinateSystemTypes.push_back(static_cast<CoordinateSystemType>(coordinateSystemType));
        }
    }
    return errCode;
}

LocationErrCode LocatorAbility::SendNetworkLocation(const std::unique_ptr<Location>& location)
{
    LBSLOGI(LOCATOR, "%{public}s: send network location", __func__);
    int64_t time = location->GetTimeStamp();
    int64_t timeSinceBoot = location->GetTimeSinceBoot();
    double acc = location->GetAccuracy();
    LBSLOGI(LOCATOR,
        "receive network location: [ time=%{public}s timeSinceBoot=%{public}s acc=%{public}f]",
        std::to_string(time).c_str(), std::to_string(timeSinceBoot).c_str(), acc);
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    location->Marshalling(dataToStub);
    return SendGnssRequest(static_cast<int>(GnssInterfaceCode::SEND_NETWORK_LOCATION), dataToStub, replyToStub);
}
#endif

LocationErrCode LocatorAbility::RegisterLocationError(sptr<ILocatorCallback>& callback, AppIdentity &identity)
{
    std::unique_ptr<LocatorCallbackMessage> callbackMessage = std::make_unique<LocatorCallbackMessage>();
    callbackMessage->SetCallback(callback);
    callbackMessage->SetAppIdentity(identity);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_REG_LOCATION_ERROR, callbackMessage);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendEvent(event);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::UnregisterLocationError(sptr<ILocatorCallback>& callback, AppIdentity &identity)
{
    std::unique_ptr<LocatorCallbackMessage> callbackMessage = std::make_unique<LocatorCallbackMessage>();
    callbackMessage->SetCallback(callback);
    callbackMessage->SetAppIdentity(identity);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_UNREG_LOCATION_ERROR, callbackMessage);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendEvent(event);
    }
    return ERRCODE_SUCCESS;
}

void LocatorAbility::ReportLocationError(std::string uuid, int32_t errCode)
{
    std::unique_ptr<LocatorErrorMessage> locatorErrorMessage = std::make_unique<LocatorErrorMessage>();
    locatorErrorMessage->SetUuid(uuid);
    locatorErrorMessage->SetErrCode(errCode);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_REPORT_LOCATION_ERROR, locatorErrorMessage);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendEvent(event);
    }
}

LocationErrCode LocatorAbility::RemoveInvalidRequests()
{
    std::list<std::shared_ptr<Request>> invalidRequestList;
    int32_t requestNum = 0;
    int32_t invalidRequestNum = 0;
    {
        std::unique_lock<ffrt::mutex> lock(requestsMutex_);
#ifdef FEATURE_GNSS_SUPPORT
        auto gpsListIter = requests_->find(GNSS_ABILITY);
        if (gpsListIter != requests_->end()) {
            auto list = &(gpsListIter->second);
            requestNum += static_cast<int>(list->size());
            for (auto& item : *list) {
                if (IsInvalidRequest(item)) {
                    invalidRequestList.push_back(item);
                    invalidRequestNum++;
                }
            }
        }
#endif
#ifdef FEATURE_NETWORK_SUPPORT
        auto networkListIter = requests_->find(NETWORK_ABILITY);
        if (networkListIter != requests_->end()) {
            auto list = &(networkListIter->second);
            requestNum += static_cast<int>(list->size());
            for (auto& item : *list) {
                if (IsInvalidRequest(item)) {
                    invalidRequestList.push_back(item);
                    invalidRequestNum++;
                }
            }
        }
#endif
    }
    LBSLOGI(LOCATOR, "request num : %{public}d, invalid request num: %{public}d", requestNum, invalidRequestNum);
    for (auto& item : invalidRequestList) {
        sptr<ILocatorCallback> callback = item->GetLocatorCallBack();
        StopLocating(callback);
    }
    return ERRCODE_SUCCESS;
}

bool LocatorAbility::IsInvalidRequest(std::shared_ptr<Request>& request)
{
    LBSLOGI(LOCATOR, "request : %{public}s %{public}s", request->GetPackageName().c_str(),
        request->GetRequestConfig()->ToString().c_str());
    int64_t timeDiff = fabs(CommonUtils::GetCurrentTime() - request->GetRequestConfig()->GetTimeStamp());
    if (request->GetRequestConfig()->GetFixNumber() == 1 &&
        timeDiff > (request->GetRequestConfig()->GetTimeOut() / MILLI_PER_SEC)) {
        LBSLOGI(LOCATOR, "once request is timeout");
        return true;
    }

    if (timeDiff > REQUEST_DEFAULT_TIMEOUT_SECOUND && !IsProcessRunning(request->GetPid(), request->GetTokenId())) {
        LBSLOGI(LOCATOR, "request process is not running");
        return true;
    }
    return false;
}

bool LocatorAbility::IsProcessRunning(pid_t pid, const uint32_t tokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    sptr<ISystemAbilityManager> samgrClient = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrClient == nullptr) {
        LBSLOGE(LOCATOR, "Get system ability manager failed.");
        return true;
    }
    sptr<AppExecFwk::IAppMgr> iAppManager =
        iface_cast<AppExecFwk::IAppMgr>(samgrClient->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (iAppManager == nullptr) {
        LBSLOGE(LOCATOR, "Failed to get ability manager service.");
        return true;
    }
    std::vector<AppExecFwk::RunningProcessInfo> runningProcessList;
    int32_t res = iAppManager->GetAllRunningProcesses(runningProcessList);
    if (res != ERR_OK) {
        LBSLOGE(LOCATOR, "Failed to get all running process.");
        return true;
    }
    auto it = std::find_if(runningProcessList.begin(), runningProcessList.end(), [pid] (auto runningProcessInfo) {
        return pid == runningProcessInfo.pid_;
    });
    if (it != runningProcessList.end()) {
        LBSLOGD(LOCATOR, "process : %{public}d is found.", pid);
        return true;
    }
    return false;
}

void LocatorAbility::SyncStillMovementState(bool state)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_SYNC_STILL_MOVEMENT_STATE, state);
    if (locatorHandler_ != nullptr && locatorHandler_->SendEvent(event)) {
        LBSLOGD(LOCATOR, "%{public}s: EVENT_SYNC_MOVEMENT_STATE Send Success", __func__);
    }
}

void LocatorAbility::SyncIdleState(bool state)
{
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_SYNC_IDLE_STATE, state);
    if (locatorHandler_ != nullptr && locatorHandler_->SendEvent(event)) {
        LBSLOGD(LOCATOR, "%{public}s: EVENT_SYNC_IDLE_STATE Send Success", __func__);
    }
}

void LocationMessage::SetAbilityName(std::string abilityName)
{
    abilityName_ = abilityName;
}

std::string LocationMessage::GetAbilityName()
{
    return abilityName_;
}

void LocationMessage::SetLocation(const std::unique_ptr<Location>& location)
{
    if (location != nullptr) {
        location_ = std::make_unique<Location>(*location);
    }
}

std::unique_ptr<Location> LocationMessage::GetLocation()
{
    if (location_ != nullptr) {
        return std::make_unique<Location>(*location_);
    } else {
        return nullptr;
    }
}

void LocatorCallbackMessage::SetCallback(const sptr<ILocatorCallback>& callback)
{
    callback_ = callback;
}

sptr<ILocatorCallback> LocatorCallbackMessage::GetCallback()
{
    return callback_;
}

void LocatorCallbackMessage::SetAppIdentity(AppIdentity& appIdentity)
{
    appIdentity_ = appIdentity;
}

AppIdentity LocatorCallbackMessage::GetAppIdentity()
{
    return appIdentity_;
}

void LocatorErrorMessage::SetUuid(std::string uuid)
{
    uuid_ = uuid;
}

std::string LocatorErrorMessage::GetUuid()
{
    return uuid_;
}

void LocatorErrorMessage::SetErrCode(int32_t errCode)
{
    errCode_ = errCode;
}

int32_t LocatorErrorMessage::GetErrCode()
{
    return errCode_;
}

LocatorHandler::LocatorHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitLocatorHandlerEventMap();
}

LocatorHandler::~LocatorHandler() {}

void LocatorHandler::InitLocatorHandlerEventMap()
{
    if (locatorHandlerEventMap_.size() != 0) {
        return;
    }
    locatorHandlerEventMap_[EVENT_UPDATE_SA] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { UpdateSaEvent(event); };
    locatorHandlerEventMap_[EVENT_INIT_REQUEST_MANAGER] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { InitRequestManagerEvent(event); };
    locatorHandlerEventMap_[EVENT_APPLY_REQUIREMENTS] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { ApplyRequirementsEvent(event); };
    locatorHandlerEventMap_[EVENT_RETRY_REGISTER_ACTION] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { RetryRegisterActionEvent(event); };
    locatorHandlerEventMap_[EVENT_REPORT_LOCATION_MESSAGE] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { ReportLocationMessageEvent(event); };
    locatorHandlerEventMap_[EVENT_SEND_SWITCHSTATE_TO_HIFENCE] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { SendSwitchStateToHifenceEvent(event); };
    locatorHandlerEventMap_[EVENT_START_LOCATING] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { StartLocatingEvent(event); };
    locatorHandlerEventMap_[EVENT_STOP_LOCATING] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { StopLocatingEvent(event); };
    locatorHandlerEventMap_[EVENT_UPDATE_LASTLOCATION_REQUESTNUM] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { UpdateLastLocationRequestNum(event); };
    locatorHandlerEventMap_[EVENT_UNLOAD_SA] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { UnloadSaEvent(event); };
    locatorHandlerEventMap_[EVENT_GET_CACHED_LOCATION_SUCCESS] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { GetCachedLocationSuccess(event); };
    locatorHandlerEventMap_[EVENT_GET_CACHED_LOCATION_FAILED] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { GetCachedLocationFailed(event); };
    locatorHandlerEventMap_[EVENT_REG_LOCATION_ERROR] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { RegLocationErrorEvent(event); };
    locatorHandlerEventMap_[EVENT_UNREG_LOCATION_ERROR] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { UnRegLocationErrorEvent(event); };
    locatorHandlerEventMap_[EVENT_REPORT_LOCATION_ERROR] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { ReportLocationErrorEvent(event); };
    locatorHandlerEventMap_[EVENT_PERIODIC_CHECK] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { RequestCheckEvent(event); };
    locatorHandlerEventMap_[EVENT_SYNC_LOCATION_STATUS] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { SyncSwitchStatus(event); };
    locatorHandlerEventMap_[EVENT_SYNC_STILL_MOVEMENT_STATE] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { SyncStillMovementState(event); };
    locatorHandlerEventMap_[EVENT_SYNC_IDLE_STATE] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { SyncIdleState(event); };
    locatorHandlerEventMap_[EVENT_INIT_MSDP_MONITOR_MANAGER] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { InitMonitorManagerEvent(event); };
    locatorHandlerEventMap_[EVENT_IS_STAND_BY] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { IsStandByEvent(event); };
    locatorHandlerEventMap_[EVENT_SET_LOCATION_WORKING_STATE] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { SetLocationWorkingStateEvent(event); };
    ConstructGeocodeHandleMap();
}

void LocatorHandler::ConstructGeocodeHandleMap()
{
    locatorHandlerEventMap_[EVENT_SEND_GEOREQUEST] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { SendGeoRequestEvent(event); };
}

void LocatorHandler::GetCachedLocationSuccess(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto workRecordStatistic = WorkRecordStatistic::GetInstance();
    if (!workRecordStatistic->Update("CacheLocation", 1)) {
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d workRecordStatistic::Update failed", __func__, __LINE__);
    }
    int64_t tokenId = event->GetParam();
    OHOS::Security::AccessToken::PermUsedTypeEnum type =
        Security::AccessToken::AccessTokenKit::GetUserGrantedPermissionUsedType(tokenId, ACCESS_APPROXIMATELY_LOCATION);
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->UpdateLastLocationRequestNum();
        PrivacyKit::StartUsingPermission(tokenId, ACCESS_APPROXIMATELY_LOCATION);
        locatorAbility->UpdatePermissionUsedRecord(tokenId, ACCESS_APPROXIMATELY_LOCATION,
            static_cast<int>(type), 1, 0);
    }
    PrivacyKit::StopUsingPermission(tokenId, ACCESS_APPROXIMATELY_LOCATION);
}

void LocatorHandler::GetCachedLocationFailed(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto workRecordStatistic = WorkRecordStatistic::GetInstance();
    if (!workRecordStatistic->Update("CacheLocation", 1)) {
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d workRecordStatistic::Update failed", __func__, __LINE__);
    }
    int64_t tokenId = event->GetParam();
    OHOS::Security::AccessToken::PermUsedTypeEnum type =
        Security::AccessToken::AccessTokenKit::GetUserGrantedPermissionUsedType(tokenId, ACCESS_APPROXIMATELY_LOCATION);
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->UpdateLastLocationRequestNum();
        PrivacyKit::StartUsingPermission(tokenId, ACCESS_APPROXIMATELY_LOCATION);
        locatorAbility->UpdatePermissionUsedRecord(tokenId, ACCESS_APPROXIMATELY_LOCATION,
            static_cast<int>(type), 0, 1);
    }
    PrivacyKit::StopUsingPermission(tokenId, ACCESS_APPROXIMATELY_LOCATION);
}

void LocatorHandler::UpdateSaEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->UpdateSaAbilityHandler();
    }
}

void LocatorHandler::InitRequestManagerEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    if (requestManager == nullptr || !requestManager->InitSystemListeners()) {
        LBSLOGE(LOCATOR, "InitSystemListeners failed");
    }
}

void LocatorHandler::ApplyRequirementsEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    if (requestManager != nullptr) {
        requestManager->HandleRequest();
    }
}

void LocatorHandler::RetryRegisterActionEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->RegisterAction();
        locatorAbility->RegisterLocationPrivacyAction();
    }
}

void LocatorHandler::ReportLocationMessageEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto reportManager = ReportManager::GetInstance();
    if (reportManager != nullptr) {
        std::unique_ptr<LocationMessage> locationMessage = event->GetUniqueObject<LocationMessage>();
        if (locationMessage == nullptr) {
            return;
        }
        std::unique_ptr<Location> location = locationMessage->GetLocation();
        std::string abilityName = locationMessage->GetAbilityName();
        int64_t time = location->GetTimeStamp();
        int64_t timeSinceBoot = location->GetTimeSinceBoot();
        double acc = location->GetAccuracy();
        LBSLOGI(LOCATOR,
            "receive location: [%{public}s time=%{public}s timeSinceBoot=%{public}s acc=%{public}f]",
            abilityName.c_str(), std::to_string(time).c_str(), std::to_string(timeSinceBoot).c_str(), acc);
        reportManager->OnReportLocation(location, abilityName);
    }
}

void LocatorHandler::SendSwitchStateToHifenceEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        int state = event->GetParam();
        if (!LocationSaLoadManager::InitLocationSa(COMMON_SA_ID)) {
            return;
        }
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        if (!data.WriteInterfaceToken(COMMON_DESCRIPTION)) {
            return;
        }
        data.WriteInt32(state);
        sptr<IRemoteObject> object =
                CommonUtils::GetRemoteObject(COMMON_SA_ID, CommonUtils::InitDeviceId());
        if (object == nullptr) {
            return;
        }
        object->SendRequest(COMMON_SWITCH_STATE_ID, data, reply, option);
    }
}

void LocatorHandler::StartLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    std::shared_ptr<Request> request = event->GetSharedObject<Request>();
    if (request == nullptr) {
        return;
    }
    if (requestManager != nullptr) {
        requestManager->HandleStartLocating(request);
    }
}

void LocatorHandler::StopLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    std::unique_ptr<LocatorCallbackMessage> callbackMessage = event->GetUniqueObject<LocatorCallbackMessage>();
    if (callbackMessage == nullptr) {
        return;
    }
    if (requestManager != nullptr) {
        requestManager->HandleStopLocating(callbackMessage->GetCallback());
    }
}

void LocatorHandler::UpdateLastLocationRequestNum(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto workRecordStatistic = WorkRecordStatistic::GetInstance();
    if (!workRecordStatistic->Update("CacheLocation", -1)) {
        LBSLOGE(LOCATOR, "%{public}s line:%{public}d workRecordStatistic::Update failed", __func__, __LINE__);
    }
}

void LocatorHandler::UnloadSaEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locationSaLoadManager = LocationSaLoadManager::GetInstance();
    if (locationSaLoadManager != nullptr) {
        locationSaLoadManager->UnloadLocationSa(LOCATION_LOCATOR_SA_ID);
    }
}

void LocatorHandler::RegLocationErrorEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    std::unique_ptr<LocatorCallbackMessage> callbackMessage = event->GetUniqueObject<LocatorCallbackMessage>();
    if (callbackMessage == nullptr) {
        return;
    }
    if (requestManager != nullptr) {
        requestManager->UpdateLocationErrorCallbackToRequest(callbackMessage->GetCallback(),
            callbackMessage->GetAppIdentity().GetTokenId(), true);
    }
}

void LocatorHandler::UnRegLocationErrorEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    std::unique_ptr<LocatorCallbackMessage> callbackMessage = event->GetUniqueObject<LocatorCallbackMessage>();
    if (callbackMessage == nullptr) {
        return;
    }
    if (requestManager != nullptr) {
        requestManager->UpdateLocationErrorCallbackToRequest(callbackMessage->GetCallback(),
            callbackMessage->GetAppIdentity().GetTokenId(), false);
    }
}

void LocatorHandler::ReportLocationErrorEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    std::unique_ptr<LocatorErrorMessage> locatorErrorMessage = event->GetUniqueObject<LocatorErrorMessage>();
    if (locatorErrorMessage == nullptr) {
        return;
    }
    auto uuid = locatorErrorMessage->GetUuid();
    auto errCode = locatorErrorMessage->GetErrCode();
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility == nullptr) {
        LBSLOGE(REQUEST_MANAGER, "locatorAbility is null");
        return;
    }
    auto requests = locatorAbility->GetRequests();
    if (requests == nullptr || requests->empty()) {
        LBSLOGE(REQUEST_MANAGER, "requests map is empty");
        return;
    }
    for (auto mapIter = requests->begin(); mapIter != requests->end(); mapIter++) {
        auto list = mapIter->second;
        for (auto request : list) {
            if (uuid != "" && uuid != request->GetUuid()) {
                continue;
            } else if (uuid != "") {
                auto locationCallbackHost = request->GetLocatorCallBack();
                locationCallbackHost->OnErrorReport(errCode);
            }
            auto locationErrorCallbackHost = request->GetLocationErrorCallBack();
            if (locationErrorCallbackHost != nullptr) {
                LBSLOGE(LOCATOR, "errCode : %{public}d ,uuid : %{public}s", errCode, uuid.c_str());
                locationErrorCallbackHost->OnErrorReport(errCode);
            }
        }
    }
}

void LocatorHandler::SyncSwitchStatus(const AppExecFwk::InnerEvent::Pointer& event)
{
    int state = LocationDataRdbManager::QuerySwitchState();
    int cacheState = LocationDataRdbManager::GetSwitchMode();
    if (state != DEFAULT_STATE && state != cacheState) {
        LocationDataRdbManager::SetSwitchMode(state);
    }
}

void LocatorHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(LOCATOR, "ProcessEvent event:%{public}d, timestamp = %{public}s",
        eventId, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
    auto handleFunc = locatorHandlerEventMap_.find(eventId);
    if (handleFunc != locatorHandlerEventMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        memberFunc(event);
    } else {
        LBSLOGE(LOCATOR, "ProcessEvent event:%{public}d, unsupport service.", eventId);
    }
}

void LocatorHandler::RequestCheckEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->RemoveInvalidRequests();
    }
    SendHighPriorityEvent(EVENT_PERIODIC_CHECK, 0, EVENT_PERIODIC_INTERVAL);
}

void LocatorHandler::SyncStillMovementState(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    if (requestManager != nullptr) {
        bool state = event->GetParam();
        requestManager->SyncStillMovementState(state);
    }
}

void LocatorHandler::SyncIdleState(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    if (requestManager != nullptr) {
        bool state = event->GetParam();
        requestManager->SyncIdleState(state);
    }
}

void LocatorHandler::SendGeoRequestEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        std::unique_ptr<GeoConvertRequest> geoConvertRequest = event->GetUniqueObject<GeoConvertRequest>();
        if (geoConvertRequest == nullptr) {
            return;
        }
        MessageParcel dataParcel;
        MessageParcel replyParcel;
        if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
            return;
        }
        geoConvertRequest->Marshalling(dataParcel);
        locatorAbility->SendGeoRequest(
            geoConvertRequest->GetRequestType() == GeoCodeType::REQUEST_GEOCODE ?
            static_cast<int>(LocatorInterfaceCode::GET_FROM_LOCATION_NAME) :
            static_cast<int>(LocatorInterfaceCode::GET_FROM_COORDINATE),
            dataParcel, replyParcel);
    }
}

void LocatorHandler::InitMonitorManagerEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
#ifdef MOVEMENT_CLIENT_ENABLE
    LocatorMsdpMonitorManager::GetInstance();
#endif
}

void LocatorHandler::IsStandByEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto requestManager = RequestManager::GetInstance();
    if (requestManager != nullptr) {
        requestManager->IsStandby();
    }
}

void LocatorHandler::SetLocationWorkingStateEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (!LocationDataRdbManager::SetLocationWorkingState(0)) {
        LBSLOGD(LOCATOR, "LocatorAbility::reset LocationWorkingState failed.");
    }
}

LocatorCallbackDeathRecipient::LocatorCallbackDeathRecipient(int32_t tokenId)
{
    tokenId_ = tokenId;
}

LocatorCallbackDeathRecipient::~LocatorCallbackDeathRecipient()
{
}

void LocatorCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    sptr<ILocatorCallback> callback = iface_cast<ILocatorCallback>(remote.promote());
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->RemoveUnloadTask(DEFAULT_CODE);
        locatorAbility->StopLocating(callback);
        locatorAbility->PostUnloadTask(DEFAULT_CODE);
        LBSLOGI(LOCATOR, "locator callback OnRemoteDied tokenId = %{public}d", tokenId_);
    }
}
} // namespace Location
} // namespace OHOS
