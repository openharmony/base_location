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

#include "request_manager.h"
#include "constant_definition.h"
#include "location_log.h"
#include "core_service_client.h"
#include "locale_config.h"
#include "country_code.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "common_utils.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
CountryCodeManager::CountryCodeManager()
{
    callback_ = nullptr;
    lastCountryByLocation_ = std::make_shared<CountryCode>();
    lastCountry_ = std::make_shared<CountryCode>();
    StartPassiveLocationListen();
}

CountryCodeManager::~CountryCodeManager()
{
}

void CountryCodeManager::NotifyAllListener()
{
    if (lastCountry_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "NotifyAllListener cancel,lastCountry_ is invalid");
        return;
    }
    auto country = std::make_shared<CountryCode>(*lastCountry_);
    for (auto iter = countryCodeCallback_->begin(); iter != countryCodeCallback_->end(); iter++) {
        sptr<ICountryCodeCallback> callback = (iter->second);
        if (callback) {
            callback->OnCountryCodeChange(country);
        }
    }
}

void CountryCodeManager::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(COUNTRY_CODE, "callback is invalid");
        return;
    }

    sptr<ICountryCodeCallback> countryCodeCallback = iface_cast<ICountryCodeCallback>(callback);
    if (countryCodeCallback == nullptr) {
        LBSLOGE(COUNTRY_CODE, "iface_cast ICountryCodeCallback failed!");
        return;
    }
    countryCodeCallback_->erase(uid);
    countryCodeCallback_->insert(std::make_pair(uid, countryCodeCallback));
    LBSLOGD(COUNTRY_CODE, "after uid:%{public}d register, countryCodeCallback_ size:%{public}s",
        uid, std::to_string(countryCodeCallback_->size()).c_str());
    if (countryCodeCallback_->size() == 1) {
        SubscribeSimEvent();
    }
}

void CountryCodeManager::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(COUNTRY_CODE, "unregister an invalid callback");
        return;
    }
    sptr<ICountryCodeCallback> countryCodeCallback = iface_cast<ICountryCodeCallback>(callback);
    if (countryCodeCallback == nullptr) {
        LBSLOGE(COUNTRY_CODE, "iface_cast ICountryCodeCallback failed!");
        return;
    }

    pid_t uid = -1;
    for (auto iter = countryCodeCallback_->begin(); iter != countryCodeCallback_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        if (remoteObject == callback) {
            uid = iter->first;
            break;
        }
    }
    countryCodeCallback_->erase(uid);
    LBSLOGD(COUNTRY_CODE, "after uid:%{public}d unregister, countryCodeCallback_ size:%{public}s",
        uid, std::to_string(countryCodeCallback_->size()).c_str());
    if (countryCodeCallback_->size() == 0) {
        UnsubscribeSimEvent();
    }
}

std::string CountryCodeManager::GetCountryCodeByLastLocation()
{
    std::u16string code = "";
    if (lastCountryByLocation_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "lastCountryByLocation_ is nullptr");
        return code;
    }
    if (lastCountryByLocation_->GetCountryCodeStr().empty()) {
        std::shared_ptr<ReportManager> reportManager = DelayedSingleton<ReportManager>::GetInstance();
        if (reportManager) {
            auto lastLocation = reportManager->GetLastLocation();
            code = GetCountryCodeByLocation(lastLocation);
            lastCountryByLocation_->SetCountryCodeStr(code);
        }
    }
    return lastCountryByLocation_->GetCountryCodeStr();
}

void CountryCodeManager::UpdateCountryCode(std::string countryCode, int type)
{
    if (lastCountry_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "lastCountry_ is nullptr");
        return;
    }
    if (lastCountry_->IsMoreReliable(type)) {
        LBSLOGI(COUNTRY_CODE, "lastCountry_ is more reliable,there is no need to update the data");
        return;
    }
    lastCountry_->SetCountryCodeStr(countryCode);
    lastCountry_->SetCountryCodeType(type);
}

bool CountryCodeManager::UpdateCountryCodeByLocation(std::string countryCode, int type)
{
    if (lastCountryByLocation_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "lastCountryByLocation_ is nullptr");
        return false;
    }
    if (lastCountryByLocation_->GetCountryCodeStr() == countryCode) {
        LBSLOGE(COUNTRY_CODE, "countryCode is same");
        return false;
    }

    lastCountryByLocation_->SetCountryCodeStr(countryCode);
    lastCountryByLocation_->SetCountryCodeType(type);
    return true;
}

std::string CountryCodeManager::GetCountryCodeByLocation(std::shared_ptr<Location>& location)
{
    if (location == nullptr) {
        LBSLOGE(COUNTRY_CODE, "GetCountryCodeByLocation location is nullptr");
        return;
    }
    return "";
}

void CountryCodeManager::StartPassiveLocationListen()
{
    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_LOW_POWER);
    requestConfig->SetTimeInterval(DEFAULT_TIME_INTERVAL);

    callback_ = sptr<LocatorCallback>(new (std::nothrow)LocatorCallback());
    if (callback_ == nullptr) {
        LBSLOGE(COUNTRY_CODE, "callback_ is nullptr");
        return;
    }

    auto request = std::make_shared<Request>();
    request->SetUid(SYSTEM_UID);
    request->SetPid(getpid());
    request->SetPackageName(PROC_NAME);
    request->SetRequestConfig(*requestConfig);
    request->SetLocatorCallBack(callback_);
    LBSLOGE(COUNTRY_CODE, "GetCountryCodeByLocation StartPassiveLocationListen");
    DelayedSingleton<RequestManager>::GetInstance().get()->HandleStartLocating(request_);
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocationStatus(callback_, SESSION_START);
}

std::shared_ptr<CountryCode> CountryCodeManager::GetIsoCountryCode()
{
    LBSLOGI(COUNTRY_CODE, "CountryCodeManager::GetIsoCountryCode");
    int type = COUNTRY_CODE_FROM_LOCALE;
    int slotId = Telephony::CellularDataClient::GetInstance().GetDefaultCellularDataSlotId();
    std::string countryCodeStr8 = Str16ToStr8(
        DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetIsoCountryCodeForNetwork(slotId));
    type = COUNTRY_CODE_FROM_NETWORK;

    if (countryCodeStr8.empty()) {
        countryCodeStr8 = GetCountryCodeByLastLocation();
        type = COUNTRY_CODE_FROM_LOCATION;
    }

    if (countryCodeStr8.empty()) {
        countryCodeStr8 = Str16ToStr8(
            DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetISOCountryCodeForSim(slotId));
        type = COUNTRY_CODE_FROM_SIM;
    }

    if (countryCodeStr8.empty()) {
        countryCodeStr8 = Global::I18n::LocaleConfig::GetSystemRegion();
        type = COUNTRY_CODE_FROM_LOCALE;
    }

    CountryCode country;
    country.SetCountryCodeStr(countryCodeStr8);
    country.SetCountryCodeType(type);
    if (lastCountry_ && !country.IsSame(*lastCountry_) && !lastCountry_->IsMoreReliable(type)) {
        UpdateCountryCode(countryCodeStr8, type);
        NotifyAllListener();
    }
    return lastCountry_;
}

void CountryCodeManager::SubscribeSimEvent()
{
    LBSLOGD(COUNTRY_CODE, "SubscribeSimEvent");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(SIM_STATE_CHANGE_ACTION);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    if (simSubscriber_ == nullptr) {
        simSubscriber_ = std::make_shared<SimSubscriber>(subscriberInfo);
    }
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(simSubscriber_);
    if (!result) {
        LBSLOGE(COUNTRY_CODE, "SubscribeSimEvent failed.");
    }
    return result;
}

bool CountryCodeManager::SubscribeNetworkStatusEvent()
{
    LBSLOGD(COUNTRY_CODE, "SubscribeNetworkStatusEvent");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(SEARCH_NET_WORK_STATE_CHANGE_ACTION);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    if (networkSubscriber_ == nullptr) {
        networkSubscriber_ = std::make_shared<NetworkSubscriber>(subscriberInfo);
    }
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(networkSubscriber_);
    if (!result) {
        LBSLOGE(COUNTRY_CODE, "SubscribeNetworkStatusEvent failed.");
    }
    return result;
}

void CountryCodeManager::UnsubscribeSimEvent()
{
    LBSLOGD(COUNTRY_CODE, "UnsubscribeSimEvent");
    if (simSubscriber_) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(simSubscriber_);
    }
}

bool CountryCodeManager::UnsubscribeNetworkStatusEvent()
{
    LBSLOGD(COUNTRY_CODE, "UnsubscribeNetworkStatusEvent");
    if (networkSubscriber_) {
        OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(networkSubscriber_);
    }
}

void CountryCodeManager::LocatorCallback::OnLocationReport(const std::unique_ptr<Location>& location)
{
    auto manager = DelayedSingleton<CountryCodeManager>::GetInstance().get();
    if (manager == nullptr) {
        LBSLOGE(COUNTRY_CODE, "OnLocationReport CountryCodeManager is nullptr");
        return;
    }
    if (location == nullptr) {
        LBSLOGE(COUNTRY_CODE, "OnLocationReport location is nullptr");
        return;
    }
    std::string code = manager->GetCountryCodeByLocation(location);
    CountryCode country;
    country.SetCountryCodeStr(code);
    country.SetCountryCodeType(COUNTRY_CODE_FROM_LOCATION);
    LBSLOGI(COUNTRY_CODE, "OnLocationReport");
    if (manager->UpdateCountryCodeByLocation(code, COUNTRY_CODE_FROM_LOCATION)) {
        LBSLOGI(COUNTRY_CODE, "OnLocationReport,countryCode is change");
        manager->GetIsoCountryCode();
    }
}

void CountryCodeManager::LocatorCallback::OnLocatingStatusChange(const int status)
{
}

void CountryCodeManager::LocatorCallback::OnErrorReport(const int errorCode)
{
}

CountryCodeManager::NetworkSubscriber::NetworkSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info)
{
    LBSLOGD(COUNTRY_CODE, "create NetworkSubscriber");
}

void CountryCodeManager::NetworkSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    auto manager = DelayedSingleton<CountryCodeManager>::GetInstance().get();
    if (manager == nullptr) {
        LBSLOGE(COUNTRY_CODE, "CountryCodeManager is nullptr");
        return;
    }
    LBSLOGI(COUNTRY_CODE, "NetworkSubscriber::OnReceiveEvent");
    manager->GetIsoCountryCode();
}

CountryCodeManager::SimSubscriber::SimSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info)
{
    LBSLOGD(COUNTRY_CODE, "create SimSubscriber");
}

void CountryCodeManager::SimSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    auto manager = DelayedSingleton<CountryCodeManager>::GetInstance().get();
    if (manager == nullptr) {
        LBSLOGE(COUNTRY_CODE, "CountryCodeManager is nullptr");
        return;
    }
    LBSLOGI(COUNTRY_CODE, "SimSubscriber::OnReceiveEvent");
    manager->GetIsoCountryCode();
}
} // namespace Location
} // namespace OHOS
