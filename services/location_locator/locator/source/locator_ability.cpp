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
#include <cmath>
#include <cstdlib>
#include <thread>
#include "constant_definition.h"
#include "event_runner.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "location_log.h"
#include "location_config_manager.h"
#include "locator_background_proxy.h"
#include "locator_event_manager.h"
#include "request_manager.h"
#include "system_ability_definition.h"
#include "country_code.h"
#include "privacy_kit.h"

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
    countryCodeManager_->ReSubscribeEvent();
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
    countryCodeManager_->ReUnsubscribeEvent();
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
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(LOCATOR, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_UPDATE_SA: {
            DelayedSingleton<LocatorAbility>::GetInstance()->UpdateSaAbilityHandler();
            break;
        }
        case EVENT_RETRY_REGISTER_ACTION: {
            DelayedSingleton<LocatorAbility>::GetInstance()->RegisterAction();
            break;
        }
        case EVENT_INIT_REQUEST_MANAGER: {
            if (!DelayedSingleton<RequestManager>::GetInstance()->InitSystemListeners()) {
                LBSLOGE(LOCATOR, "InitSystemListeners failed");
            }
            break;
        }
        case EVENT_APPLY_REQUIREMENTS: {
            DelayedSingleton<RequestManager>::GetInstance()->HandleRequest();
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
    locatorHandler_->SendHighPriorityEvent(EVENT_APPLY_REQUIREMENTS, 0, RETRY_INTERVAL_UNITE);
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
    DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get()->OnSaStateChange(isEnabled_);
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
        if (callback != nullptr) {
            callback->OnSwitchChange(state);
        }
    }
}

void LocatorAbility::EnableAbility(bool isEnabled)
{
    if (isEnabled_ == isEnabled) {
        LBSLOGD(LOCATOR, "no need to set location ability, enable:%{public}d", isEnabled_);
        return;
    }
    LBSLOGI(LOCATOR, "EnableAbility %{public}d", isEnabled);
    int modeValue = isEnabled ? 1 : 0;
    LocationConfigManager::GetInstance().SetLocationSwitchState(modeValue);
    UpdateSaAbility();
    std::string state = isEnabled ? "enable" : "disable";
    WriteLocationSwitchStateEvent(state);
}

int LocatorAbility::GetSwitchState()
{
    int state = QuerySwitchState();
    isEnabled_ = (state == ENABLED);
    return isEnabled_ ? ENABLED : DISABLED;
}

int LocatorAbility::QuerySwitchState()
{
    return LocationConfigManager::GetInstance().GetLocationSwitchState();
}

bool LocatorAbility::IsLocationPrivacyConfirmed(const int type)
{
    return LocationConfigManager::GetInstance().GetPrivacyTypeState(type);
}

int LocatorAbility::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    return LocationConfigManager::GetInstance().SetPrivacyTypeState(type, isConfirmed);
}

void LocatorAbility::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "register an invalid switch callback");
        return;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) SwitchCallbackDeathRecipient());
    callback->AddDeathRecipient(death.GetRefPtr());
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return;
    }
    switchCallbacks_->erase(uid);
    switchCallbacks_->insert(std::make_pair(uid, switchCallback));
    LBSLOGD(LOCATOR, "after uid:%{public}d register, switch callback size:%{public}s",
        uid, std::to_string(switchCallbacks_->size()).c_str());
}

void LocatorAbility::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR, "unregister an invalid switch callback");
        return;
    }
    sptr<ISwitchCallback> switchCallback = iface_cast<ISwitchCallback>(callback);
    if (switchCallback == nullptr) {
        LBSLOGE(LOCATOR, "cast switch callback fail!");
        return;
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
}

void LocatorAbility::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    LBSLOGD(LOCATOR, "uid is: %{public}d", uid);
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return;
        }
        dataToStub.WriteRemoteObject(callback);
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return;
        }
        obj->SendRequest(REG_GNSS_STATUS, dataToStub, replyToStub, option);
    }
}

void LocatorAbility::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return;
        }

        dataToStub.WriteRemoteObject(callback);
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return;
        }
        obj->SendRequest(UNREG_GNSS_STATUS, dataToStub, replyToStub, option);
    }
}

void LocatorAbility::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return;
        }

        dataToStub.WriteRemoteObject(callback);
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return;
        }
        obj->SendRequest(REG_NMEA, dataToStub, replyToStub, option);
    }
}

void LocatorAbility::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return;
        }

        dataToStub.WriteRemoteObject(callback);
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return;
        }
        obj->SendRequest(UNREG_NMEA, dataToStub, replyToStub, option);
    }
}

void LocatorAbility::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "RegisterCountryCodeCallback countryCodeManager_ is nullptr");
        return;
    }
    return countryCodeManager_->RegisterCountryCodeCallback(callback, uid);
}

void LocatorAbility::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "UnregisterCountryCodeCallback countryCodeManager_ is nullptr");
        return;
    }
    return countryCodeManager_->UnregisterCountryCodeCallback(callback);
}

int LocatorAbility::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback, std::string bundleName)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return REPLY_CODE_EXCEPTION;
        }

        dataToStub.WriteInt32(request->reportingPeriodSec);
        dataToStub.WriteBool(request->wakeUpCacheQueueFull);
        dataToStub.WriteRemoteObject(callback->AsObject());
        dataToStub.WriteString16(Str8ToStr16(bundleName));
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return REPLY_CODE_EXCEPTION;
        }
        obj->SendRequest(REG_CACHED, dataToStub, replyToStub, option);
    }
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbility::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return REPLY_CODE_EXCEPTION;
        }

        dataToStub.WriteRemoteObject(callback->AsObject());
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return REPLY_CODE_EXCEPTION;
        }
        obj->SendRequest(UNREG_CACHED, dataToStub, replyToStub, option);
    }
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbility::GetCachedGnssLocationsSize()
{
    int size = 0;
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return REPLY_CODE_EXCEPTION;
        }
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return REPLY_CODE_EXCEPTION;
        }

        int error = obj->SendRequest(GET_CACHED_SIZE, dataToStub, replyToStub, option);
        if (error == NO_ERROR) {
            size = replyToStub.ReadInt32();
        }
    }
    return size;
}

int LocatorAbility::FlushCachedGnssLocations()
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return REPLY_CODE_EXCEPTION;
        }
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return REPLY_CODE_EXCEPTION;
        }

        return obj->SendRequest(FLUSH_CACHED, dataToStub, replyToStub, option);
    }
    return REPLY_CODE_EXCEPTION;
}

void LocatorAbility::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return;
        }
        dataToStub.WriteInt32(commands->scenario);
        dataToStub.WriteString16(Str8ToStr16(commands->command));
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return;
        }
        obj->SendRequest(SEND_COMMANDS, dataToStub, replyToStub, option);
    }
}

void LocatorAbility::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return;
        }
        dataToStub.WriteInt32(request->priority);
        dataToStub.WriteInt32(request->scenario);
        dataToStub.WriteDouble(request->geofence.latitude);
        dataToStub.WriteDouble(request->geofence.longitude);
        dataToStub.WriteDouble(request->geofence.radius);
        dataToStub.WriteDouble(request->geofence.expiration);
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return;
        }
        obj->SendRequest(ADD_FENCE_INFO, dataToStub, replyToStub, option);
    }
}

void LocatorAbility::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    auto remoteObject = proxyMap_->find(GNSS_ABILITY);
    if (remoteObject != proxyMap_->end()) {
        auto obj = remoteObject->second;
        MessageParcel dataToStub;
        if (!dataToStub.WriteInterfaceToken(GnssAbilityProxy::GetDescriptor())) {
            return;
        }
        dataToStub.WriteInt32(request->priority);
        dataToStub.WriteInt32(request->scenario);
        dataToStub.WriteDouble(request->geofence.latitude);
        dataToStub.WriteDouble(request->geofence.longitude);
        dataToStub.WriteDouble(request->geofence.radius);
        dataToStub.WriteDouble(request->geofence.expiration);
        MessageParcel replyToStub;
        MessageOption option;
        if (obj == nullptr) {
            return;
        }
        obj->SendRequest(REMOVE_FENCE_INFO, dataToStub, replyToStub, option);
    }
}

std::shared_ptr<CountryCode> LocatorAbility::GetIsoCountryCode()
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "GetIsoCountryCode countryCodeManager_ is nullptr");
        return nullptr;
    }
    return countryCodeManager_->GetIsoCountryCode();
}

bool LocatorAbility::SendLocationMockMsgToGnssSa(const sptr<IRemoteObject> obj,
    const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToGnssSa obj is nullptr");
        return false;
    }
    std::unique_ptr<GnssAbilityProxy> gnssProxy = std::make_unique<GnssAbilityProxy>(obj);
    if (msgId == ENABLE_LOCATION_MOCK) {
        return gnssProxy->EnableMock(config);
    } else if (msgId == DISABLE_LOCATION_MOCK) {
        return gnssProxy->DisableMock(config);
    } else if (msgId == SET_MOCKED_LOCATIONS) {
        return gnssProxy->SetMocked(config, location);
    }
    return false;
}

bool LocatorAbility::SendLocationMockMsgToNetworkSa(const sptr<IRemoteObject> obj,
    const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    if (obj == nullptr) {
        LBSLOGE(LOCATOR, "SendLocationMockMsgToNetworkSa obj is nullptr");
        return false;
    }
    std::unique_ptr<NetworkAbilityProxy> networkProxy =
        std::make_unique<NetworkAbilityProxy>(obj);
    if (msgId == ENABLE_LOCATION_MOCK) {
        return networkProxy->EnableMock(config);
    } else if (msgId == DISABLE_LOCATION_MOCK) {
        return networkProxy->DisableMock(config);
    } else if (msgId == SET_MOCKED_LOCATIONS) {
        return networkProxy->SetMocked(config, location);
    }
    return false;
}

bool LocatorAbility::ProcessLocationMockMsg(
    const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location, int msgId)
{
    std::shared_ptr<Request> request = std::make_shared<Request>();
    if (request != nullptr) {
        request->SetLocationMockConfig(config);
    }
    std::shared_ptr<std::list<std::string>> proxys = std::make_shared<std::list<std::string>>();
    request->GetProxyName(proxys);
    if (proxys->empty()) {
        LBSLOGE(LOCATOR, "ProcessLocationMockMsg GetProxyName failed");
        return false;
    }
    bool result = true;
    for (std::list<std::string>::iterator iter = proxys->begin(); iter != proxys->end(); ++iter) {
        std::string abilityName = *iter;
        if (abilityName.compare(GNSS_ABILITY) != 0 &&
            abilityName.compare(NETWORK_ABILITY) != 0) {
            LBSLOGD(LOCATOR, "abilityName not match. abilityName = %{public}s", abilityName.c_str());
            continue;
        }
        auto remoteObject = proxyMap_->find(abilityName);
        if (remoteObject == proxyMap_->end()) {
            continue;
        }
        auto obj = remoteObject->second;
        if (abilityName == GNSS_ABILITY) {
            SendLocationMockMsgToGnssSa(obj, config, location, msgId);
        } else if (abilityName == NETWORK_ABILITY) {
            SendLocationMockMsgToNetworkSa(obj, config, location, msgId);
        }
    }
    return result;
}

bool LocatorAbility::EnableLocationMock(const LocationMockConfig& config)
{
    std::vector<std::shared_ptr<Location>> location;
    return ProcessLocationMockMsg(config, location, ENABLE_LOCATION_MOCK);
}

bool LocatorAbility::DisableLocationMock(const LocationMockConfig& config)
{
    std::vector<std::shared_ptr<Location>> location;
    return ProcessLocationMockMsg(config, location, DISABLE_LOCATION_MOCK);
}

bool LocatorAbility::SetMockedLocations(
    const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location)
{
    return ProcessLocationMockMsg(config, location, SET_MOCKED_LOCATIONS);
}

int LocatorAbility::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback, AppIdentity &identity)
{
    if (isEnabled_ == DISABLED) {
        ReportErrorStatus(callback, ERROR_SWITCH_UNOPEN);
    }
    if (!CheckSaValid()) {
        InitSaAbility();
    }
    // update offset before add request
    reportManager_->UpdateRandom();
    // generate request object according to input params
    std::shared_ptr<Request> request = std::make_shared<Request>();
    if (request != nullptr) {
        request->SetUid(identity.GetUid());
        request->SetPid(identity.GetPid());
        request->SetTokenId(identity.GetTokenId());
        request->SetFirstTokenId(identity.GetFirstTokenId());
        request->SetPackageName(identity.GetBundleName());
        request->SetRequestConfig(*requestConfig);
        request->SetLocatorCallBack(callback);
    }
    UpdateUsingPermission(identity.GetTokenId(), identity.GetFirstTokenId(), true);
    RegisterPermissionCallback(identity.GetTokenId(),
        {ACCESS_APPROXIMATELY_LOCATION, ACCESS_LOCATION, ACCESS_BACKGROUND_LOCATION});
    LBSLOGI(LOCATOR, "start locating");
    requestManager_->HandleStartLocating(request);
    ReportLocationStatus(callback, SESSION_START);
    return REPLY_CODE_NO_EXCEPTION;
}

int LocatorAbility::StopLocating(sptr<ILocatorCallback>& callback)
{
    LBSLOGI(LOCATOR, "stop locating");
    requestManager_->HandleStopLocating(callback);
    ReportLocationStatus(callback, SESSION_STOP);
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t callingFirstTokenid = IPCSkeleton::GetFirstTokenID();
    UpdateUsingPermission(callingTokenId, callingFirstTokenid, false);
    UnregisterPermissionCallback(callingTokenId);
    return REPLY_CODE_NO_EXCEPTION;
}

void LocatorAbility::UpdateUsingPermission(uint32_t callingTokenId, uint32_t callingFirstTokenid, bool isStart)
{
    if (CommonUtils::CheckLocationPermission(callingTokenId, callingFirstTokenid)) {
        isStart ? PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_LOCATION) :
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_LOCATION);
    }
    if (CommonUtils::CheckApproximatelyPermission(callingTokenId, callingFirstTokenid)) {
        isStart ? PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION) :
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_APPROXIMATELY_LOCATION);
    }
    if (requestManager_->IsAppBackground() &&
        CommonUtils::CheckBackgroundPermission(callingTokenId, callingFirstTokenid)) {
        isStart ? PrivacyKit::StartUsingPermission(callingTokenId, ACCESS_BACKGROUND_LOCATION) :
            PrivacyKit::StopUsingPermission(callingTokenId, ACCESS_BACKGROUND_LOCATION);
    }
}

int LocatorAbility::GetCacheLocation(MessageParcel& reply, AppIdentity &identity)
{
    auto lastLocation = reportManager_->GetLastLocation();
    auto finalLocation = reportManager_->GetPermittedLocation(identity.GetTokenId(),
        identity.GetFirstTokenId(), lastLocation);
    if (finalLocation == nullptr) {
        reply.WriteInt32(REPLY_CODE_EXCEPTION);
        reply.WriteString("get no cached result");
        LBSLOGI(LOCATOR, "GetCacheLocation location is null");
        return REPLY_CODE_EXCEPTION;
    }
    if (fabs(finalLocation->GetLatitude() - 0.0) > PRECISION
        && fabs(finalLocation->GetLongitude() - 0.0) > PRECISION) {
        reply.WriteInt32(REPLY_CODE_NO_EXCEPTION);
        finalLocation->Marshalling(reply);
        return REPLY_CODE_NO_EXCEPTION;
    }
    reply.WriteInt32(REPLY_CODE_EXCEPTION);
    reply.WriteString("get no cached result");
    LBSLOGI(LOCATOR, "GetCacheLocation location is null");
    return REPLY_CODE_EXCEPTION;
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

int LocatorAbility::IsGeoConvertAvailable(MessageParcel &replay)
{
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    return SendGeoRequest(GEO_IS_AVAILABLE, dataParcel, replay);
}

int LocatorAbility::GetAddressByCoordinate(MessageParcel &data, MessageParcel &replay)
{
    LBSLOGI(LOCATOR, "locator_ability GetAddressByCoordinate");
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    dataParcel.WriteDouble(data.ReadDouble()); // latitude
    dataParcel.WriteDouble(data.ReadDouble()); // longitude
    dataParcel.WriteInt32(data.ReadInt32()); // maxItems
    dataParcel.WriteInt32(data.ReadInt32()); // locale object size = 1
    dataParcel.WriteString16(data.ReadString16()); // locale.getLanguage()
    dataParcel.WriteString16(data.ReadString16()); // locale.getCountry()
    dataParcel.WriteString16(data.ReadString16()); // locale.getVariant()
    dataParcel.WriteString16(data.ReadString16()); // ""
    return SendGeoRequest(GET_FROM_COORDINATE, dataParcel, replay);
}

int LocatorAbility::GetAddressByLocationName(MessageParcel &data, MessageParcel &replay)
{
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(GeoConvertProxy::GetDescriptor())) {
        return REPLY_CODE_EXCEPTION;
    }
    dataParcel.WriteString16(data.ReadString16()); // description
    dataParcel.WriteDouble(data.ReadDouble()); // minLatitude
    dataParcel.WriteDouble(data.ReadDouble()); // minLongitude
    dataParcel.WriteDouble(data.ReadDouble()); // maxLatitude
    dataParcel.WriteDouble(data.ReadDouble()); // maxLongitude
    dataParcel.WriteInt32(data.ReadInt32()); // maxItems
    dataParcel.WriteInt32(data.ReadInt32()); // locale object size = 1
    dataParcel.WriteString16(data.ReadString16()); // locale.getLanguage()
    dataParcel.WriteString16(data.ReadString16()); // locale.getCountry()
    dataParcel.WriteString16(data.ReadString16()); // locale.getVariant()
    dataParcel.WriteString16(data.ReadString16()); // ""
    return SendGeoRequest(GET_FROM_LOCATION_NAME, dataParcel, replay);
}

int LocatorAbility::SendGeoRequest(int type, MessageParcel &data, MessageParcel &replay)
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return REPLY_CODE_EXCEPTION;
    }
    MessageOption option;
    return remoteObject->SendRequest(type, data, replay, option);
}

bool LocatorAbility::EnableReverseGeocodingMock()
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return REPLY_CODE_EXCEPTION;
    }
    std::unique_ptr<GeoConvertProxy> geoProxy = std::make_unique<GeoConvertProxy>(remoteObject);
    return geoProxy->EnableReverseGeocodingMock();
}

bool LocatorAbility::DisableReverseGeocodingMock()
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return REPLY_CODE_EXCEPTION;
    }
    std::unique_ptr<GeoConvertProxy> geoProxy = std::make_unique<GeoConvertProxy>(remoteObject);
    return geoProxy->DisableReverseGeocodingMock();
}

bool LocatorAbility::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    sptr<IRemoteObject> remoteObject = CommonUtils::GetRemoteObject(LOCATION_GEO_CONVERT_SA_ID,
        CommonUtils::InitDeviceId());
    if (remoteObject == nullptr) {
        return REPLY_CODE_EXCEPTION;
    }
    std::unique_ptr<GeoConvertProxy> geoProxy = std::make_unique<GeoConvertProxy>(remoteObject);
    return geoProxy->SetReverseGeocodingMockInfo(mockInfo);
}

bool LocatorAbility::ProxyUidForFreeze(int32_t uid, bool isProxy)
{
    LBSLOGI(LOCATOR, "Start locator proxy, uid: %{public}d, isProxy: %{public}d", uid, isProxy);
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (isProxy) {
        proxyUids_.insert(uid);
    } else {
        proxyUids_.erase(uid);
    }
    return true;
}

bool LocatorAbility::ResetAllProxy()
{
    LBSLOGI(LOCATOR, "Start locator ResetAllProxy");
    std::lock_guard<std::mutex> lock(proxyMutex_);
    proxyUids_.clear();
    return true;
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
