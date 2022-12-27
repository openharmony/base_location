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
#include "system_ability_definition.h"
#include "switch_callback_proxy.h"

#include "common_event_manager.h"
#include "common_hisysevent.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "country_code.h"
#include "geo_convert_proxy.h"
#include "gnss_ability_proxy.h"
#include "locator_background_proxy.h"
#include "location_config_manager.h"
#include "location_log.h"
#include "network_ability_proxy.h"
#include "passive_ability_proxy.h"
#include "permission_status_change_cb.h"

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<LocatorAbility>::GetInstance().get());

const uint32_t EVENT_UPDATE_SA = 0x0001;
const uint32_t EVENT_INIT_REQUEST_MANAGER = 0x0002;
const uint32_t EVENT_APPLY_REQUIREMENTS = 0x0003;
const uint32_t EVENT_RETRY_REGISTER_ACTION = 0x0004;
const uint32_t RETRY_INTERVAL_UNITE = 1000;
const uint32_t RETRY_INTERVAL_OF_INIT_REQUEST_MANAGER = 5 * RETRY_INTERVAL_UNITE;
const uint32_t SET_ENABLE = 3;
const uint32_t REG_GNSS_STATUS = 7;
const uint32_t UNREG_GNSS_STATUS = 8;
const uint32_t REG_NMEA = 9;
const uint32_t UNREG_NMEA = 10;
const uint32_t REG_CACHED = 11;
const uint32_t UNREG_CACHED = 12;
const uint32_t GET_CACHED_SIZE = 13;
const uint32_t FLUSH_CACHED = 14;
const uint32_t SEND_COMMANDS = 15;
const uint32_t ADD_FENCE_INFO = 16;
const uint32_t REMOVE_FENCE_INFO = 17;
const float_t PRECISION = 0.000001;

LocatorAbility::LocatorAbility() : SystemAbility(LOCATION_LOCATOR_SA_ID, true)
{
    switchCallbacks_ = std::make_unique<std::map<pid_t, sptr<ISwitchCallback>>>();
    requests_ = std::make_shared<std::map<std::string, std::list<std::shared_ptr<Request>>>>();
    receivers_ = std::make_shared<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>>();
    proxyMap_ = std::make_shared<std::map<std::string, sptr<IRemoteObject>>>();
    permissionMap_ = std::make_shared<std::map<uint32_t, std::shared_ptr<PermissionStatusChangeCb>>>();
    InitRequestManagerMap();
    reportManager_ = DelayedSingleton<ReportManager>::GetInstance();
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
    LBSLOGI(LOCATOR, "LocatorAbility::OnStart start ability success.");
}

void LocatorAbility::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToAbility_ = false;
    LBSLOGI(LOCATOR, "LocatorAbility::OnStop ability stopped.");
}

void LocatorAbility::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(LOCATOR, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (locatorEventSubscriber_ == nullptr) {
        LBSLOGE(LOCATOR, "OnAddSystemAbility subscribeer is nullptr");
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(locatorEventSubscriber_);
    LBSLOGI(LOCATOR, "SubscribeCommonEvent locatorEventSubscriber_ result = %{public}d", result);
    if (countryCodeManager_ == nullptr) {
        countryCodeManager_ = DelayedSingleton<CountryCodeManager>::GetInstance();
    }
    if (countryCodeManager_ != nullptr) {
        countryCodeManager_->ReSubscribeEvent();
    }
}

void LocatorAbility::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(LOCATOR, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (locatorEventSubscriber_ == nullptr) {
        LBSLOGE(LOCATOR, "OnRemoveSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(locatorEventSubscriber_);
    LBSLOGI(LOCATOR, "UnSubscribeCommonEvent locatorEventSubscriber_ result = %{public}d", result);
    if (countryCodeManager_ == nullptr) {
        countryCodeManager_ = DelayedSingleton<CountryCodeManager>::GetInstance();
    }
    if (countryCodeManager_ != nullptr) {
        countryCodeManager_->ReUnsubscribeEvent();
    }
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

    deviceId_ = CommonUtils::InitDeviceId();
    requestManager_ = DelayedSingleton<RequestManager>::GetInstance();
    locatorHandler_ = std::make_shared<LocatorHandler>(AppExecFwk::EventRunner::Create(true));
    if (countryCodeManager_ == nullptr) {
        countryCodeManager_ = DelayedSingleton<CountryCodeManager>::GetInstance();
    }
    InitSaAbility();
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_INIT_REQUEST_MANAGER, 0, RETRY_INTERVAL_OF_INIT_REQUEST_MANAGER);
    }
    RegisterAction();
    registerToAbility_ = true;
    return registerToAbility_;
}

LocatorHandler::LocatorHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

LocatorHandler::~LocatorHandler() {}

void LocatorHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    auto requestManager = DelayedSingleton<RequestManager>::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(LOCATOR, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_UPDATE_SA: {
            if (locatorAbility != nullptr) {
                locatorAbility->UpdateSaAbilityHandler();
            }
            break;
        }
        case EVENT_RETRY_REGISTER_ACTION: {
            if (locatorAbility != nullptr) {
                locatorAbility->RegisterAction();
            }
            break;
        }
        case EVENT_INIT_REQUEST_MANAGER: {
            if (requestManager == nullptr || !requestManager->InitSystemListeners()) {
                LBSLOGE(LOCATOR, "InitSystemListeners failed");
            }
            break;
        }
        case EVENT_APPLY_REQUIREMENTS: {
            if (requestManager != nullptr) {
                requestManager->HandleRequest();
            }
            break;
        }
        default:
            break;
    }
}

void LocatorAbility::InitRequestManagerMap()
{
    if (requests_ != nullptr) {
        std::list<std::shared_ptr<Request>> gnssList;
        requests_->insert(make_pair(GNSS_ABILITY, gnssList));
        std::list<std::shared_ptr<Request>> networkList;
        requests_->insert(make_pair(NETWORK_ABILITY, networkList));
        std::list<std::shared_ptr<Request>> passiveList;
        requests_->insert(make_pair(PASSIVE_ABILITY, passiveList));
    }
}

std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> LocatorAbility::GetRequests()
{
    return requests_;
}

int LocatorAbility::GetActiveRequestNum()
{
    int num = 0;
    auto gpsListIter = requests_->find(GNSS_ABILITY);
    auto networkListIter = requests_->find(NETWORK_ABILITY);
    if (gpsListIter != requests_->end()) {
        auto list = &(gpsListIter->second);
        num += list->size();
    }
    if (networkListIter != requests_->end()) {
        auto list = &(networkListIter->second);
        num += list->size();
    }
    return num;
}

std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> LocatorAbility::GetReceivers()
{
    return receivers_;
}

std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> LocatorAbility::GetProxyMap()
{
    return proxyMap_;
}

void LocatorAbility::ApplyRequests()
{
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(EVENT_APPLY_REQUIREMENTS, 0, RETRY_INTERVAL_UNITE);
    }
}

void LocatorAbility::InitSaAbility()
{
    LBSLOGI(LOCATOR, "initSaAbility start");
    if (proxyMap_ == nullptr) {
        return;
    }
    // init gnss ability sa
    sptr<IRemoteObject> objectGnss = CommonUtils::GetRemoteObject(LOCATION_GNSS_SA_ID, CommonUtils::InitDeviceId());
    if (objectGnss != nullptr) {
        proxyMap_->insert(make_pair(GNSS_ABILITY, objectGnss));
    } else {
        LBSLOGE(LOCATOR, "GetRemoteObject gnss sa is null");
    }

    // init network ability sa
    sptr<IRemoteObject> objectNetwork = CommonUtils::GetRemoteObject(LOCATION_NETWORK_LOCATING_SA_ID,
        CommonUtils::InitDeviceId());
    if (objectNetwork != nullptr) {
        proxyMap_->insert(make_pair(NETWORK_ABILITY, objectNetwork));
    } else {
        LBSLOGE(LOCATOR, "GetRemoteObject network sa is null");
    }

    // init passive ability sa
    sptr<IRemoteObject> objectPassive = CommonUtils::GetRemoteObject(LOCATION_NOPOWER_LOCATING_SA_ID,
        CommonUtils::InitDeviceId());
    if (objectPassive != nullptr) {
        proxyMap_->insert(make_pair(PASSIVE_ABILITY, objectPassive));
    } else {
        LBSLOGE(LOCATOR, "GetRemoteObject passive sa is null");
    }

    UpdateSaAbilityHandler();
}

bool LocatorAbility::CheckSaValid()
{
    auto objectGnss = proxyMap_->find(GNSS_ABILITY);
    if (objectGnss == proxyMap_->end()) {
        LBSLOGI(LOCATOR, "gnss sa is null");
        return false;
    }

    auto objectNetwork = proxyMap_->find(NETWORK_ABILITY);
    if (objectNetwork == proxyMap_->end()) {
        LBSLOGI(LOCATOR, "network sa is null");
        return false;
    }

    auto objectPassive = proxyMap_->find(PASSIVE_ABILITY);
    if (objectPassive == proxyMap_->end()) {
        LBSLOGI(LOCATOR, "passive sa is null");
        return false;
    }

    return true;
}

void LocatorAbility::UpdateSaAbility()
{
    auto event = AppExecFwk::InnerEvent::Get(EVENT_UPDATE_SA, 0);
    if (locatorHandler_ != nullptr) {
        locatorHandler_->SendHighPriorityEvent(event);
    }
}

void LocatorAbility::UpdateSaAbilityHandler()
{
    int state = QuerySwitchState();
    LBSLOGI(LOCATOR, "update location subability enable state, switch state=%{public}d, action registered=%{public}d",
        state, isActionRegistered);
    bool currentEnable = isEnabled_;
    isEnabled_ = (state == ENABLED);
    if (isEnabled_ == currentEnable) {
        return;
    }
    if (proxyMap_ == nullptr) {
        return;
    }
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance();
    if (locatorBackgroundProxy == nullptr) {
        LBSLOGE(LOCATOR, "UpdateSaAbilityHandler: LocatorBackgroundProxy is nullptr");
        return;
    }
    locatorBackgroundProxy.get()->OnSaStateChange(isEnabled_);
    for (auto iter = proxyMap_->begin(); iter != proxyMap_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = iter->second;
        MessageParcel data;
        if (iter->first == GNSS_ABILITY) {
            data.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor());
        } else if (iter->first == NETWORK_ABILITY) {
            data.WriteInterfaceToken(NetworkAbilityProxy::GetDescriptor());
        } else if (iter->first == PASSIVE_ABILITY) {
            data.WriteInterfaceToken(PassiveAbilityProxy::GetDescriptor());
        }
        data.WriteBool(isEnabled_);

        MessageParcel reply;
        MessageOption option;
        int error = remoteObject->SendRequest(SET_ENABLE, data, reply, option);
        LBSLOGD(LOCATOR, "enable %{public}s ability, remote result %{public}d", (iter->first).c_str(), error);
    }
    for (auto iter = switchCallbacks_->begin(); iter != switchCallbacks_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        auto callback = std::make_unique<SwitchCallbackProxy>(remoteObject);
        callback->OnSwitchChange(state);
    }
}

LocationErrCode LocatorAbility::EnableAbility(bool isEnabled)
{
    if (isEnabled_ == isEnabled) {
        LBSLOGD(LOCATOR, "no need to set location ability, enable:%{public}d", isEnabled_);
        return ERRCODE_SUCCESS;
    }
    LBSLOGI(LOCATOR, "EnableAbility %{public}d", isEnabled);
    int modeValue = isEnabled ? 1 : 0;
    LocationConfigManager::GetInstance().SetLocationSwitchState(modeValue);
    UpdateSaAbility();
    std::string state = isEnabled ? "enable" : "disable";
    WriteLocationSwitchStateEvent(state);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::GetSwitchState(int& state)
{
    isEnabled_ = (QuerySwitchState() == ENABLED);
    state = isEnabled_ ? ENABLED : DISABLED;
    return ERRCODE_SUCCESS;
}

int LocatorAbility::QuerySwitchState()
{
    return LocationConfigManager::GetInstance().GetLocationSwitchState();
}

LocationErrCode LocatorAbility::IsLocationPrivacyConfirmed(const int type)
{
    return LocationConfigManager::GetInstance().GetPrivacyTypeState(type);
}

LocationErrCode LocatorAbility::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    return LocationConfigManager::GetInstance().SetPrivacyTypeState(type, isConfirmed);
}

LocationErrCode LocatorAbility::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "register an invalid switch callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) SwitchCallbackDeathRecipient());
    callback->AddDeathRecipient(death.GetRefPtr());
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
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

LocationErrCode LocatorAbility::SendGnssRequest(int type, MessageParcel &data, MessageParcel &reply)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject == proxyMap_->end()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    auto obj = remoteObject->second;
    if (obj == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    MessageOption option;
    int error = obj->SendRequest(REG_GNSS_STATUS, dataToStub, replyToStub, option);
    if (error != NO_ERROR) {
        LBSLOGE(LOCATOR, "msg id = %{public}d, send request failed, error : %{public}d", REG_GNSS_STATUS, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    GnssErrCode errorCode = replyToStub.ReadInt32();
    if (errorCode != GNSS_OPT_SUCCESS) {
        LBSLOGE(LOCATOR, "gnss failed! error : %{public}d", errorCode);
        return ERRCODE_GNSS_FAIL;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR, "uid is: %{public}d", uid);
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(REG_GNSS_STATUS, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(UNREG_GNSS_STATUS, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(REG_NMEA, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteRemoteObject(callback);
    return SendGnssRequest(UNREG_NMEA, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "RegisterCountryCodeCallback countryCodeManager_ is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCodeManager_->RegisterCountryCodeCallback(callback, uid);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "UnregisterCountryCodeCallback countryCodeManager_ is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCodeManager_->UnregisterCountryCodeCallback(callback);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback, std::string bundleName)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteInt32(request->reportingPeriodSec);
    dataToStub.WriteBool(request->wakeUpCacheQueueFull);
    dataToStub.WriteRemoteObject(callback->AsObject());
    dataToStub.WriteString16(Str8ToStr16(bundleName));
    return SendGnssRequest(REG_CACHED, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteRemoteObject(callback->AsObject());
    return SendGnssRequest(UNREG_CACHED, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::GetCachedGnssLocationsSize(int& size)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    LocationErrCode errorCode = SendGnssRequest(GET_CACHED_SIZE, dataToStub, replyToStub);
    if (errorCode == ERRCODE_SUCCESS) {
        size = replyToStub.ReadInt32();
    }
    return errorCode;
}

LocationErrCode LocatorAbility::FlushCachedGnssLocations(MessageParcel &reply)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    return SendGnssRequest(FLUSH_CACHED, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteInt32(commands->scenario);
    dataToStub.WriteString16(Str8ToStr16(commands->command));
    return SendGnssRequest(SEND_COMMANDS, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteInt32(request->scenario);
    dataToStub.WriteDouble(request->geofence.latitude);
    dataToStub.WriteDouble(request->geofence.longitude);
    dataToStub.WriteDouble(request->geofence.radius);
    dataToStub.WriteDouble(request->geofence.expiration);
    return SendGnssRequest(ADD_FENCE_INFO, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{

    MessageParcel dataToStub;
    MessageParcel replyToStub;
    if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
        return ERRCODE_INVALID_TOKEN;
    }
    dataToStub.WriteInt32(request->scenario);
    dataToStub.WriteDouble(request->geofence.latitude);
    dataToStub.WriteDouble(request->geofence.longitude);
    dataToStub.WriteDouble(request->geofence.radius);
    dataToStub.WriteDouble(request->geofence.expiration);
    return SendGnssRequest(REMOVE_FENCE_INFO, dataToStub, replyToStub);
}

LocationErrCode LocatorAbility::GetIsoCountryCode(std::shared_ptr<CountryCode>& countryCode)
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "GetIsoCountryCode countryCodeManager_ is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCode = countryCodeManager_->GetIsoCountryCode();
    return ERRCODE_SUCCESS;
}

bool LocatorAbility::SendLocationMockMsgToGnssSa(const sptr<IRemoteObject> obj,
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToGnssSa obj is nullptr");
        return false;
    }
    std::unique_ptr<GnssAbilityProxy> gnssProxy = std::make_unique<GnssAbilityProxy>(obj);
    if (msgId == ENABLE_LOCATION_MOCK) {
        return gnssProxy->EnableMock();
    } else if (msgId == DISABLE_LOCATION_MOCK) {
        return gnssProxy->DisableMock();
    } else if (msgId == SET_MOCKED_LOCATIONS) {
        return gnssProxy->SetMocked(timeInterval, location);
    }
    return false;
}

bool LocatorAbility::SendLocationMockMsgToNetworkSa(const sptr<IRemoteObject> obj,
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToNetworkSa obj is nullptr");
        return false;
    }
    std::unique_ptr<NetworkAbilityProxy> networkProxy =
        std::make_unique<NetworkAbilityProxy>(obj);
    if (msgId == ENABLE_LOCATION_MOCK) {
        return networkProxy->EnableMock();
    } else if (msgId == DISABLE_LOCATION_MOCK) {
        return networkProxy->DisableMock();
    } else if (msgId == SET_MOCKED_LOCATIONS) {
        return networkProxy->SetMocked(timeInterval, location);
    }
    return false;
}

bool LocatorAbility::SendLocationMockMsgToPassiveSa(const sptr<IRemoteObject> obj,
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToNetworkSa obj is nullptr");
        return false;
    }
    std::unique_ptr<PassiveAbilityProxy> passiveProxy =
        std::make_unique<PassiveAbilityProxy>(obj);
    if (msgId == ENABLE_LOCATION_MOCK) {
        return passiveProxy->EnableMock();
    } else if (msgId == DISABLE_LOCATION_MOCK) {
        return passiveProxy->DisableMock();
    } else if (msgId == SET_MOCKED_LOCATIONS) {
        return passiveProxy->SetMocked(timeInterval, location);
    }
    return false;
}

LocationErrCode LocatorAbility::ProcessLocationMockMsg(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    for (auto iter = proxyMap_->begin(); iter != proxyMap_->end(); iter++) {
        auto obj = iter->second;
        if (iter->first == GNSS_ABILITY) {
            SendLocationMockMsgToGnssSa(obj, timeInterval, location, msgId);
        } else if (iter->first == NETWORK_ABILITY) {
            SendLocationMockMsgToNetworkSa(obj, timeInterval, location, msgId);
        } else if (iter->first == PASSIVE_ABILITY) {
            SendLocationMockMsgToPassiveSa(obj, timeInterval, location, msgId);
        }
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::EnableLocationMock()
{
    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> location;
    return ProcessLocationMockMsg(timeInterval, location, ENABLE_LOCATION_MOCK);
}

LocationErrCode LocatorAbility::DisableLocationMock()
{
    int timeInterval = 0;
    std::vector<std::shared_ptr<Location>> location;
    return ProcessLocationMockMsg(timeInterval, location, DISABLE_LOCATION_MOCK);
}

LocationErrCode LocatorAbility::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    return ProcessLocationMockMsg(timeInterval, location, SET_MOCKED_LOCATIONS);
}

LocationErrCode LocatorAbility::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback, AppIdentity &identity)
{
    if (isEnabled_ == DISABLED) {
        ReportErrorStatus(callback, ERROR_SWITCH_UNOPEN);
    }
    if (!CheckSaValid()) {
        InitSaAbility();
    }
    // update offset before add request
    if (reportManager_ == nullptr || requestManager_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    reportManager_->UpdateRandom();
    // generate request object according to input params
    std::shared_ptr<Request> request = std::make_shared<Request>();
    request->SetUid(identity.GetUid());
    request->SetPid(identity.GetPid());
    request->SetTokenId(identity.GetTokenId());
    request->SetFirstTokenId(identity.GetFirstTokenId());
    request->SetPackageName(identity.GetBundleName());
    request->SetRequestConfig(*requestConfig);
    request->SetLocatorCallBack(callback);
    request->SetUuid(std::to_string(CommonUtils::IntRandom(MIN_INT_RANDOM, MAX_INT_RANDOM)));
    LBSLOGI(LOCATOR, "start locating");
    requestManager_->HandleStartLocating(request);
    ReportLocationStatus(callback, SESSION_START);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::StopLocating(sptr<ILocatorCallback>& callback)
{
    LBSLOGI(LOCATOR, "stop locating");
    if (requestManager_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    requestManager_->HandleStopLocating(callback);
    ReportLocationStatus(callback, SESSION_STOP);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::GetCacheLocation(std::unique_ptr<Location>& loc, AppIdentity &identity)
{
    auto lastLocation = reportManager_->GetLastLocation();
    loc = reportManager_->GetPermittedLocation(identity.GetTokenId(),
        identity.GetFirstTokenId(), lastLocation);
    if (loc == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (fabs(loc->GetLatitude() - 0.0) > PRECISION
        && fabs(loc->GetLongitude() - 0.0) > PRECISION) {
        return ERRCODE_SUCCESS;
    }
    return ERRCODE_SERVICE_UNAVAILABLE;
}

int LocatorAbility::ReportLocation(const std::unique_ptr<Location>& location, std::string abilityName)
{
    if (requests_ == nullptr) {
        return REPLY_CODE_EXCEPTION;
    }
    if (GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "location switch is off");
        return REPLY_CODE_EXCEPTION;
    }
    LBSLOGI(LOCATOR, "start report location");
    if (reportManager_->OnReportLocation(location, abilityName)) {
        return REPLY_CODE_NO_EXCEPTION;
    }
    return REPLY_CODE_EXCEPTION;
}

int LocatorAbility::ReportLocationStatus(sptr<ILocatorCallback>& callback, int result)
{
    if (GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "location switch is off");
        return REPLY_CODE_EXCEPTION;
    }
    if (reportManager_->ReportRemoteCallback(callback, ILocatorCallback::RECEIVE_LOCATION_STATUS_EVENT, result)) {
        return REPLY_CODE_NO_EXCEPTION;
    }
    return REPLY_CODE_EXCEPTION;
}

int LocatorAbility::ReportErrorStatus(sptr<ILocatorCallback>& callback, int result)
{
    if (GetSwitchState() == DISABLED) {
        LBSLOGE(LOCATOR, "location switch is off");
        return REPLY_CODE_EXCEPTION;
    }
    if (reportManager_->ReportRemoteCallback(callback, ILocatorCallback::RECEIVE_ERROR_INFO_EVENT, result)) {
        return REPLY_CODE_NO_EXCEPTION;
    }
    return REPLY_CODE_EXCEPTION;
}

void LocatorAbility::RegisterAction()
{
    if (isActionRegistered) {
        LBSLOGI(LOCATOR, "action has already registered");
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(MODE_CHANGED_EVENT);
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

LocationErrCode LocatorAbility::IsGeoConvertAvailable(bool &isAvailable)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        isAvailable = false;
        return ERRCODE_INVALID_TOKEN;
    }
    LocationErrCode errorCode = SendGeoRequest(GEO_IS_AVAILABLE, dataParcel, replyParcel);
    if (errorCode == ERRCODE_SUCCESS) {
        isAvailable = replyParcel.ReadBool();
    } else {
        isAvailable = false;
    }
    return errorCode;
}

void LocatorAbility::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    LBSLOGI(LOCATOR, "locator_ability GetAddressByCoordinate");
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        reply.WriteInt32(ERRCODE_INVALID_TOKEN);
        return;
    }
    dataParcel.WriteDouble(data.ReadDouble()); // latitude
    dataParcel.WriteDouble(data.ReadDouble()); // longitude
    dataParcel.WriteInt32(data.ReadInt32()); // maxItems
    dataParcel.WriteInt32(data.ReadInt32()); // locale object size = 1
    dataParcel.WriteString16(data.ReadString16()); // locale.getLanguage()
    dataParcel.WriteString16(data.ReadString16()); // locale.getCountry()
    dataParcel.WriteString16(data.ReadString16()); // locale.getVariant()
    dataParcel.WriteString16(data.ReadString16()); // ""
    LocationErrCode errorCode = SendGeoRequest(GET_FROM_COORDINATE, dataParcel, replyParcel);
    reply.WriteInt32(errorCode);
    if (errorCode == ERRCODE_SUCCESS) {
        int resultSize = replyParcel.ReadInt32();
        if (resultSize > GeoAddress::MAX_RESULT) {
            resultSize = GeoAddress::MAX_RESULT;
        }
        reply.WriteInt32(resultSize);
        for (int i = 0; i < resultSize; i++) {
            auto geoAddress = GeoAddress::Unmarshalling(replyParcel);
            geoAddress->Marshalling(reply);
        }
    }
}

void LocatorAbility::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    LBSLOGI(LOCATOR, "locator_ability GetAddressByLocationName");
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        reply.WriteInt32(ERRCODE_INVALID_TOKEN);
        return;
    }
    dataParcel.WriteString16(data.ReadString16()); // description
    dataParcel.WriteDouble(data.ReadDouble()); // minLatitude
    dataParcel.WriteDouble(data.ReadDouble()); // minLongitude
    dataParcel.WriteDouble(data.ReadDouble()); // maxLatitude
    dataParcel.WriteDouble(data.ReadDouble()); // maxLongitude
    dataParcel.WriteInt32(data.ReadInt32()); // maxreplyItems
    dataParcel.WriteInt32(data.ReadInt32()); // locale object size = 1
    dataParcel.WriteString16(data.ReadString16()); // locale.getLanguage()
    dataParcel.WriteString16(data.ReadString16()); // locale.getCountry()
    dataParcel.WriteString16(data.ReadString16()); // locale.getVariant()
    dataParcel.WriteString16(data.ReadString16()); // ""
    SendGeoRequest(GET_FROM_LOCATION_NAME, dataParcel, replyParcel);
    reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
    reply.WriteInt32(replyParcel.ReadInt32());
}

LocationErrCode LocatorAbility::SendGeoRequest(int type, MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    MessageOption option;
    int error = remoteObject->SendRequest(type, data, reply, option);
    if (error != NO_ERROR) {
        LBSLOGE(LOCATOR, "LocatorAbility::SendGeoRequest type = %{public}d, send request failed, error : %{public}d", type, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    GeoCodeErrCode errorCode = reply.ReadInt32();
    if (errorCode != GEOCODE_ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "LocatorAbility::SendGeoRequest type = %{public}d, ErrCode = %{public}d", type, errorCode);
        return ERRCODE_GEOCODING_FAIL;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::EnableReverseGeocodingMock()
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<GeoConvertProxy> geoProxy = std::make_unique<GeoConvertProxy>(remoteObject);
    if (!geoProxy->EnableReverseGeocodingMock()) {
        return ERRCODE_GEOCODING_FAIL;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::DisableReverseGeocodingMock()
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::unique_ptr<GeoConvertProxy> geoProxy = std::make_unique<GeoConvertProxy>(remoteObject);
    if (!geoProxy->DisableReverseGeocodingMock()) {
        return ERRCODE_GEOCODING_FAIL;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return REPLY_CODE_EXCEPTION;
    }
    std::unique_ptr<GeoConvertProxy> geoProxy = std::make_unique<GeoConvertProxy>(remoteObject);
    if (!geoProxy->SetReverseGeocodingMockInfo(mockInfo)) {
        return ERRCODE_GEOCODING_FAIL;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::ProxyUidForFreeze(int32_t uid, bool isProxy)
{
    LBSLOGI(LOCATOR, "Start locator proxy, uid: %{public}d, isProxy: %{public}d", uid, isProxy);
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (isProxy) {
        proxyUids_.insert(uid);
    } else {
        proxyUids_.erase(uid);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorAbility::ResetAllProxy()
{
    LBSLOGI(LOCATOR, "Start locator ResetAllProxy");
    std::lock_guard<std::mutex> lock(proxyMutex_);
    proxyUids_.clear();
    return ERRCODE_SUCCESS;
}

bool LocatorAbility::IsProxyUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    return proxyUids_.find(uid) != proxyUids_.end();
}

void LocatorAbility::RegisterPermissionCallback(const uint32_t callingTokenId,
    const std::vector<std::string>& permissionNameList)
{
    if (permissionMap_ == nullptr) {
        LBSLOGE(LOCATOR, "permissionMap is null.");
        return;
    }
    PermStateChangeScope scopeInfo;
    scopeInfo.permList = permissionNameList;
    scopeInfo.tokenIDs = {callingTokenId};
    auto callbackPtr = std::make_shared<PermissionStatusChangeCb>(scopeInfo);
    std::lock_guard<std::mutex> lock(permissionMutex_);
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
    if (permissionMap_ == nullptr) {
        LBSLOGE(LOCATOR, "permissionMap is null.");
        return;
    }
    std::lock_guard<std::mutex> lock(permissionMutex_);
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
} // namespace Location
} // namespace OHOS
