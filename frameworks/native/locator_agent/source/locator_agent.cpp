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

#include "locator_agent.h"

#include "location_sa_load_manager.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "location_log.h"
#include "locator_service_proxy.h"

namespace OHOS {
namespace Location {
LocatorAgentManager* LocatorAgentManager::GetInstance()
{
    static LocatorAgentManager data;
    return &data;
}

LocatorAgentManager::LocatorAgentManager()
{
    nmeaCallbackHost_ =
        sptr<NativeNmeaCallbackHost>(new (std::nothrow) NativeNmeaCallbackHost());
    gnssCallbackHost_ =
        sptr<NativeSvCallbackHost>(new (std::nothrow) NativeSvCallbackHost());
    locationCallbackHost_ =
        sptr<NativeLocationCallbackHost>(new (std::nothrow) NativeLocationCallbackHost());
}

LocatorAgentManager::~LocatorAgentManager()
{
    auto remote = CheckLocatorSystemAbilityLoaded();
    ResetLocatorAgent(remote);
}

LocationErrCode LocatorAgentManager::StartGnssLocating(const LocationCallbackIfaces& callback)
{
    if (locationCallbackHost_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetScenario(SCENE_UNSET);
    requestConfig->SetFixNumber(0);
    locationCallbackHost_->SetCallback(callback);
    auto locatorCallback = sptr<ILocatorCallback>(locationCallbackHost_);
    if (locatorCallback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->StartLocating(*requestConfig, locatorCallback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    LBSLOGI(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, locationErrCode);
    return locationErrCode;
}

LocationErrCode LocatorAgentManager::StopGnssLocating()
{
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto locatorCallback = sptr<ILocatorCallback>(locationCallbackHost_);
    if (locatorCallback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->StopLocating(locatorCallback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    LBSLOGI(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, locationErrCode);
    return locationErrCode;
}

LocationErrCode LocatorAgentManager::RegisterGnssStatusCallback(const SvStatusCallbackIfaces& callback)
{
    if (gnssCallbackHost_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    gnssCallbackHost_->SetCallback(callback);
    auto gnssCallback = sptr<IGnssStatusCallback>(gnssCallbackHost_);
    if (gnssCallback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->RegisterGnssStatusCallback(gnssCallback->AsObject());
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    LBSLOGI(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, locationErrCode);
    return locationErrCode;
}

LocationErrCode LocatorAgentManager::UnregisterGnssStatusCallback()
{
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto gnssCallback = sptr<IGnssStatusCallback>(gnssCallbackHost_);
    if (gnssCallback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->UnregisterGnssStatusCallback(gnssCallback->AsObject());
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    LBSLOGI(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, locationErrCode);
    return locationErrCode;
}

LocationErrCode LocatorAgentManager::RegisterNmeaMessageCallback(const GnssNmeaCallbackIfaces& callback)
{
    if (nmeaCallbackHost_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    nmeaCallbackHost_->SetCallback(callback);
    auto nmeaCallback = sptr<INmeaMessageCallback>(nmeaCallbackHost_);
    if (nmeaCallback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->RegisterNmeaMessageCallback(nmeaCallback->AsObject());
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    LBSLOGI(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, locationErrCode);
    return locationErrCode;
}

LocationErrCode LocatorAgentManager::UnregisterNmeaMessageCallback()
{
    auto proxy = GetLocatorAgent();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    auto nmeaCallback = sptr<INmeaMessageCallback>(nmeaCallbackHost_);
    if (nmeaCallback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->UnregisterNmeaMessageCallback(nmeaCallback->AsObject());
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    LBSLOGI(LOCATOR_STANDARD, "%{public}s ret = %{public}d", __func__, locationErrCode);
    return locationErrCode;
}

sptr<ILocatorService> LocatorAgentManager::GetLocatorAgent()
{
    std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
    lock.lock();
    if (client_ != nullptr) {
        LBSLOGI(LOCATOR_STANDARD, "%{public}s get proxy success.", __func__);
        lock.unlock();
        return client_;
    }
    lock.unlock();
    sptr<IRemoteObject> saObject = CheckLocatorSystemAbilityLoaded();
    return InitLocatorAgent(saObject);
}

bool LocatorAgentManager::TryLoadLocatorSystemAbility()
{
    auto instance = LocationSaLoadManager::GetInstance();
    if (instance == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get instance failed.", __func__);
        return false;
    }
    if (instance->LoadLocationSa(LOCATION_LOCATOR_SA_ID) != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s load sa failed.", __func__);
        return false;
    }
    return true;
}

sptr<ILocatorService> LocatorAgentManager::InitLocatorAgent(sptr<IRemoteObject>& saObject)
{
    if (saObject == nullptr) {
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    client_ = iface_cast<ILocatorService>(saObject);
    if (!client_ || !client_->AsObject()) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get locator service failed.", __func__);
        return nullptr;
    }
    recipient_ = sptr<LocatorAgentDeathRecipient>(new (std::nothrow) LocatorAgentDeathRecipient(*this));
    if ((saObject->IsProxyObject()) && (!saObject->AddDeathRecipient(recipient_))) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: deathRecipient add failed.", __func__);
        return nullptr;
    }
    LBSLOGI(LOCATOR_STANDARD, "%{public}s: client reset success.", __func__);
    return client_;
}

sptr<IRemoteObject> LocatorAgentManager::CheckLocatorSystemAbilityLoaded()
{
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
        return nullptr;
    }

    sptr<IRemoteObject> saObject = samgr->CheckSystemAbility(LOCATION_LOCATOR_SA_ID);
    if (saObject == nullptr) {
        if (!TryLoadLocatorSystemAbility()) {
            return nullptr;
        }
        saObject = samgr->CheckSystemAbility(LOCATION_LOCATOR_SA_ID);
        if (saObject == nullptr) {
            return nullptr;
        }
    }
    return saObject;
}

void LocatorAgentManager::ResetLocatorAgent(const wptr<IRemoteObject> &remote)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: remote is nullptr.", __func__);
        return;
    }
    if (client_ == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: proxy is nullptr.", __func__);
        return;
    }
    if (remote.promote() != nullptr) {
        remote.promote()->RemoveDeathRecipient(recipient_);
    }
    client_ = nullptr;
    recipient_ = nullptr;
}
}
}
