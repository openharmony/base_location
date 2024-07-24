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

#include "locator_impl.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "common_utils.h"
#include "country_code.h"

#include "location_data_rdb_observer.h"
#include "location_data_rdb_helper.h"
#include "location_data_rdb_manager.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locator.h"

namespace OHOS {
namespace Location {
constexpr uint32_t WAIT_MS = 1000;
std::shared_ptr<LocatorImpl> LocatorImpl::instance_ = nullptr;
std::mutex LocatorImpl::locatorMutex_;
auto g_locatorImpl = Locator::GetInstance();
std::mutex g_resumeFuncMapMutex;
std::mutex g_locationCallbackMapMutex;
std::mutex g_gnssStatusInfoCallbacksMutex;
std::mutex g_nmeaCallbacksMutex;
std::shared_ptr<CallbackResumeManager> g_callbackResumer = std::make_shared<CallbackResumeManager>();
using CallbackResumeHandle = std::function<void()>;
std::map<std::string, CallbackResumeHandle> g_resumeFuncMap;
std::map<sptr<ILocatorCallback>, RequestConfig> g_locationCallbackMap;
std::set<sptr<IRemoteObject>> g_gnssStatusInfoCallbacks;
std::set<sptr<IRemoteObject>> g_nmeaCallbacks;

std::shared_ptr<LocatorImpl> LocatorImpl::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(locatorMutex_);
        if (instance_ == nullptr) {
            std::shared_ptr<LocatorImpl> locator = std::make_shared<LocatorImpl>();
            instance_ = locator;
        }
    }
    return instance_;
}

LocatorImpl::LocatorImpl()
{
    locationDataManager_ = LocationDataManager::GetInstance();
}

LocatorImpl::~LocatorImpl()
{
}

bool LocatorImpl::IsLocationEnabled()
{
    int32_t state = DISABLED;
    int res = LocationDataRdbManager::GetSwitchMode();
    if (res == DISABLED || res == ENABLED) {
        return (res == ENABLED);
    }
    auto locationDataRdbHelper =
        LocationDataRdbHelper::GetInstance();
    if (locationDataRdbHelper == nullptr) {
        return false;
    }
    state = LocationDataRdbManager::QuerySwitchState();
    return (state == ENABLED);
}

void LocatorImpl::ShowNotification()
{
    LBSLOGI(LOCATION_NAPI, "ShowNotification");
}

void LocatorImpl::RequestPermission()
{
    LBSLOGI(LOCATION_NAPI, "permission need to be granted");
}

void LocatorImpl::RequestEnableLocation()
{
    LBSLOGI(LOCATION_NAPI, "RequestEnableLocation");
}

void LocatorImpl::EnableAbility(bool enable)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    LocationErrCode errorCode = CheckEdmPolicy(enable);
    if (errorCode != ERRCODE_SUCCESS) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    LocationErrCode errCode = proxy->EnableAbilityV9(enable);
    // cache the value
    if (errCode == ERRCODE_SUCCESS) {
        if (locationDataManager_ != nullptr) {
            locationDataManager_->SetCachedSwitchState(enable ? ENABLED : DISABLED);
        }
    }
}

void LocatorImpl::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    if (IsLocationCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s locatorCallback has registered", __func__);
        return;
    }
    AddLocationCallBack(requestConfig, callback);
    int errCode = proxy->StartLocating(requestConfig, callback, "location.ILocator", 0, 0);
    if (errCode != ERRCODE_SUCCESS) {
        RemoveLocationCallBack(callback);
    }
}

void LocatorImpl::StopLocating(sptr<ILocatorCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    proxy->StopLocating(callback);
    std::unique_lock<std::mutex> lock(g_locationCallbackMapMutex);
    auto iter = g_locationCallbackMap.find(callback);
    if (iter != g_locationCallbackMap.end()) {
        g_locationCallbackMap.erase(iter);
    }
}

std::unique_ptr<Location> LocatorImpl::GetCachedLocation()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return nullptr;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return nullptr;
    }
    std::unique_ptr<Location> location = nullptr;
    MessageParcel reply;
    proxy->GetCacheLocation(reply);
    int exception = reply.ReadInt32();
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        location = Location::Unmarshalling(reply);
    }

    return location;
}

bool LocatorImpl::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (locationDataManager_ == nullptr) {
        return false;
    }
    return locationDataManager_->RegisterSwitchCallback(callback, IPCSkeleton::GetCallingUid()) == ERRCODE_SUCCESS;
}

bool LocatorImpl::UnregisterSwitchCallback(const sptr<IRemoteObject>& callback)
{
    if (locationDataManager_ == nullptr) {
        return false;
    }
    return locationDataManager_->UnregisterSwitchCallback(callback) == ERRCODE_SUCCESS;
}

bool LocatorImpl::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    if (IsSatelliteStatusChangeCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return false;
    }
    AddSatelliteStatusChangeCallBack(callback);
    proxy->RegisterGnssStatusCallback(callback, DEFAULT_UID);
    return true;
}

bool LocatorImpl::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->UnregisterGnssStatusCallback(callback);
    RemoveSatelliteStatusChangeCallBack(callback);
    return true;
}

bool LocatorImpl::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    if (IsNmeaCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return false;
    }
    AddNmeaCallBack(callback);
    proxy->RegisterNmeaMessageCallback(callback, DEFAULT_UID);
    return true;
}

bool LocatorImpl::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->UnregisterNmeaMessageCallback(callback);
    RemoveNmeaCallBack(callback);
    return true;
}

bool LocatorImpl::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return false;
    }
    countryCodeManager->RegisterCountryCodeCallback(callback, uid);
    return true;
}

bool LocatorImpl::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return false;
    }
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    return true;
}

void LocatorImpl::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    proxy->RegisterCachedLocationCallback(request, callback, "location.ILocator");
}

void LocatorImpl::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    proxy->UnregisterCachedLocationCallback(callback);
}

bool LocatorImpl::IsGeoServiceAvailable()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    bool result = false;
    MessageParcel reply;
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->IsGeoConvertAvailable(reply);
    int exception = reply.ReadInt32();
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        result = reply.ReadBool();
    }
    return result;
}

void LocatorImpl::GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    MessageParcel reply;
    proxy->GetAddressByCoordinate(data, reply);
    int exception = reply.ReadInt32();
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        int resultSize = reply.ReadInt32();
        if (resultSize > GeoAddress::MAX_RESULT) {
            resultSize = GeoAddress::MAX_RESULT;
        }
        for (int i = 0; i < resultSize; i++) {
            replyList.push_back(GeoAddress::Unmarshalling(reply));
        }
    }
}

void LocatorImpl::GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    MessageParcel reply;
    proxy->GetAddressByLocationName(data, reply);
    int exception = reply.ReadInt32();
    if (exception == ERRCODE_PERMISSION_DENIED) {
        LBSLOGE(LOCATOR_STANDARD, "can not get cached location without location permission.");
    } else if (exception != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
    } else {
        int resultSize = reply.ReadInt32();
        if (resultSize > GeoAddress::MAX_RESULT) {
            resultSize = GeoAddress::MAX_RESULT;
        }
        for (int i = 0; i < resultSize; i++) {
            replyList.push_back(GeoAddress::Unmarshalling(reply));
        }
    }
}

bool LocatorImpl::IsLocationPrivacyConfirmed(const int type)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationPrivacyConfirmed()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->IsLocationPrivacyConfirmed(type);
    return flag;
}

int LocatorImpl::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatus()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    int flag = proxy->SetLocationPrivacyConfirmStatus(type, isConfirmed);
    return flag;
}

int LocatorImpl::GetCachedGnssLocationsSize()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return -1;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedGnssLocationsSize()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    int size = proxy->GetCachedGnssLocationsSize();
    return size;
}

int LocatorImpl::FlushCachedGnssLocations()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return -1;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::FlushCachedGnssLocations()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    int res = proxy->FlushCachedGnssLocations();
    return res;
}

bool LocatorImpl::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SendCommand()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->SendCommand(commands);
    return true;
}

std::shared_ptr<CountryCode> LocatorImpl::GetIsoCountryCode()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCode()");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return nullptr;
    }
    return countryCodeManager->GetIsoCountryCode();
}

bool LocatorImpl::EnableLocationMock()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMock()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->EnableLocationMock();
    return flag;
}

bool LocatorImpl::DisableLocationMock()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMock()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->DisableLocationMock();
    return flag;
}

bool LocatorImpl::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocations()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->SetMockedLocations(timeInterval, location);
    return flag;
}

bool LocatorImpl::EnableReverseGeocodingMock()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableReverseGeocodingMock()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->EnableReverseGeocodingMock();
    return flag;
}

bool LocatorImpl::DisableReverseGeocodingMock()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableReverseGeocodingMock()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->DisableReverseGeocodingMock();
    return flag;
}

bool LocatorImpl::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetReverseGeocodingMockInfo()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->SetReverseGeocodingMockInfo(mockInfo);
    return flag;
}

LocationErrCode LocatorImpl::IsLocationEnabledV9(bool &isEnabled)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationEnabledV9()");
    int32_t state = DISABLED;
    int res = LocationDataRdbManager::GetSwitchMode();
    if (res == DISABLED || res == ENABLED) {
        isEnabled = (res == ENABLED);
        return ERRCODE_SUCCESS;
    }
    auto locationDataRdbHelper =
        LocationDataRdbHelper::GetInstance();
    if (locationDataRdbHelper == nullptr) {
        return ERRCODE_NOT_SUPPORTED;
    }
    state = LocationDataRdbManager::QuerySwitchState();
    isEnabled = (state == ENABLED);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::CheckEdmPolicy(bool enable)
{
    std::string policy = "";
    bool res = CommonUtils::GetEdmPolicy(policy);
    if (!res || policy.empty()) {
        LBSLOGE(LOCATOR_STANDARD, "get edm policy failed!");
        return ERRCODE_SUCCESS;
    }
    if (policy == "force_open" && enable == false) {
        LBSLOGE(LOCATOR_STANDARD, "disable location switch is not allowed");
        return ERRCODE_EDM_POLICY_ABANDON;
    } else if (policy == "disallow" && enable == true) {
        LBSLOGE(LOCATOR_STANDARD, "enable location switch is not allowed");
        return ERRCODE_EDM_POLICY_ABANDON;
    }
    return ERRCODE_SUCCESS;
}


LocationErrCode LocatorImpl::EnableAbilityV9(bool enable)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode = CheckEdmPolicy(enable);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableAbilityV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->EnableAbilityV9(enable);
    // cache the value
    if (errCode == ERRCODE_SUCCESS) {
        if (locationDataManager_ != nullptr) {
            locationDataManager_->SetCachedSwitchState(enable ? ENABLED : DISABLED);
        }
    }
    return errCode;
}

LocationErrCode LocatorImpl::StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StartLocatingV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (IsLocationCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s locatorCallback has registered", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AddLocationCallBack(requestConfig, callback);
    LocationErrCode errCode = proxy->StartLocatingV9(requestConfig, callback);
    if (errCode != ERRCODE_SUCCESS) {
        RemoveLocationCallBack(callback);
    }
    return errCode;
}

LocationErrCode LocatorImpl::StopLocatingV9(sptr<ILocatorCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StopLocatingV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->StopLocatingV9(callback);
    RemoveLocationCallBack(callback);
    return errCode;
}

LocationErrCode LocatorImpl::GetCachedLocationV9(std::unique_ptr<Location> &loc)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedLocationV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->GetCacheLocationV9(loc);
    return errCode;
}

LocationErrCode LocatorImpl::RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (locationDataManager_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return locationDataManager_->
        RegisterSwitchCallback(callback, IPCSkeleton::GetCallingUid());
}

LocationErrCode LocatorImpl::UnregisterSwitchCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (locationDataManager_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return locationDataManager_->UnregisterSwitchCallback(callback);
}

LocationErrCode LocatorImpl::RegisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterGnssStatusCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (IsSatelliteStatusChangeCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AddSatelliteStatusChangeCallBack(callback);
    LocationErrCode errCode = proxy->RegisterGnssStatusCallbackV9(callback);
    if (errCode != ERRCODE_SUCCESS) {
        RemoveSatelliteStatusChangeCallBack(callback);
    }
    return errCode;
}

LocationErrCode LocatorImpl::UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterGnssStatusCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->UnregisterGnssStatusCallbackV9(callback);
    RemoveSatelliteStatusChangeCallBack(callback);
    return errCode;
}

LocationErrCode LocatorImpl::RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterNmeaMessageCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (IsNmeaCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AddNmeaCallBack(callback);
    LocationErrCode errCode = proxy->RegisterNmeaMessageCallbackV9(callback);
    if (errCode != ERRCODE_SUCCESS) {
        RemoveNmeaCallBack(callback);
    }
    return errCode;
}

LocationErrCode LocatorImpl::UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterNmeaMessageCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->UnregisterNmeaMessageCallbackV9(callback);
    RemoveNmeaCallBack(callback);
    return errCode;
}

LocationErrCode LocatorImpl::RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCountryCodeCallbackV9()");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCodeManager->RegisterCountryCodeCallback(callback, 0);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::UnregisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCountryCodeCallbackV9()");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCachedLocationCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->RegisterCachedLocationCallbackV9(request, callback, "location.ILocator");
    return errCode;
}

LocationErrCode LocatorImpl::UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCachedLocationCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->UnregisterCachedLocationCallbackV9(callback);
    return errCode;
}

LocationErrCode LocatorImpl::IsGeoServiceAvailableV9(bool &isAvailable)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsGeoServiceAvailableV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->IsGeoConvertAvailableV9(isAvailable);
    return errCode;
}

LocationErrCode LocatorImpl::GetAddressByCoordinateV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByCoordinateV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->GetAddressByCoordinateV9(data, replyList);
    return errCode;
}

LocationErrCode LocatorImpl::GetAddressByLocationNameV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByLocationNameV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->GetAddressByLocationNameV9(data, replyList);
    return errCode;
}

LocationErrCode LocatorImpl::IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationPrivacyConfirmedV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->IsLocationPrivacyConfirmedV9(type, isConfirmed);
    return errCode;
}

LocationErrCode LocatorImpl::SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatusV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->SetLocationPrivacyConfirmStatusV9(type, isConfirmed);
    return errCode;
}

LocationErrCode LocatorImpl::GetCachedGnssLocationsSizeV9(int &size)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedGnssLocationsSizeV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->GetCachedGnssLocationsSizeV9(size);
    return errCode;
}

LocationErrCode LocatorImpl::FlushCachedGnssLocationsV9()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::FlushCachedGnssLocationsV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->FlushCachedGnssLocationsV9();
    return errCode;
}

LocationErrCode LocatorImpl::SendCommandV9(std::unique_ptr<LocationCommand>& commands)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SendCommandV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->SendCommandV9(commands);
    return errCode;
}

LocationErrCode LocatorImpl::GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCodeV9()");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCode = countryCodeManager->GetIsoCountryCode();
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::EnableLocationMockV9()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMockV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->EnableLocationMockV9();
    return errCode;
}

LocationErrCode LocatorImpl::DisableLocationMockV9()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMockV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->DisableLocationMockV9();
    return errCode;
}

LocationErrCode LocatorImpl::SetMockedLocationsV9(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocationsV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->SetMockedLocationsV9(timeInterval, location);
    return errCode;
}

LocationErrCode LocatorImpl::EnableReverseGeocodingMockV9()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableReverseGeocodingMockV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->EnableReverseGeocodingMockV9();
    return errCode;
}

LocationErrCode LocatorImpl::DisableReverseGeocodingMockV9()
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableReverseGeocodingMockV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->DisableReverseGeocodingMockV9();
    return errCode;
}

LocationErrCode LocatorImpl::SetReverseGeocodingMockInfoV9(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetReverseGeocodingMockInfoV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->SetReverseGeocodingMockInfoV9(mockInfo);
    return errCode;
}

LocationErrCode LocatorImpl::ProxyForFreeze(std::set<int> pidList, bool isProxy)
{
    if (!LocationSaLoadManager::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGI(LOCATOR_STANDARD, "%{public}s locator sa is not available.", __func__);
        isServerExist_ = false;
        return ERRCODE_SUCCESS;
    }
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s init locator sa failed", __func__);
        isServerExist_ = false;
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ProxyForFreeze()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->ProxyForFreeze(pidList, isProxy);
    return errCode;
}

LocationErrCode LocatorImpl::ResetAllProxy()
{
    if (!LocationSaLoadManager::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGI(LOCATOR_STANDARD, "%{public}s, no need reset proxy", __func__);
        isServerExist_ = false;
        return ERRCODE_SUCCESS;
    }
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        isServerExist_ = false;
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ResetAllProxy()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->ResetAllProxy();
    return errCode;
}

LocationErrCode LocatorImpl::RegisterLocatingRequiredDataCallback(
    std::unique_ptr<LocatingRequiredDataConfig>& dataConfig, sptr<ILocatingRequiredDataCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::%{public}s", __func__);
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return proxy->RegisterLocatingRequiredDataCallback(dataConfig, callback);
}

LocationErrCode LocatorImpl::UnRegisterLocatingRequiredDataCallback(sptr<ILocatingRequiredDataCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::%{public}s", __func__);
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return proxy->UnRegisterLocatingRequiredDataCallback(callback);
}

LocationErrCode LocatorImpl::SubscribeLocationError(sptr<ILocatorCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StartLocatingV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->SubscribeLocationError(callback);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "SubscribeLocationError failed.");
    }
    return errCode;
}

LocationErrCode LocatorImpl::UnSubscribeLocationError(sptr<ILocatorCallback>& callback)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StopLocatingV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->UnSubscribeLocationError(callback);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "UnSubscribeLocationError failed.");
    }
    return errCode;
}

void LocatorImpl::ResetLocatorProxy(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: remote is nullptr.", __func__);
        return;
    }
    if (client_ == nullptr || !isServerExist_) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: proxy is nullptr.", __func__);
        return;
    }
    if (remote.promote() != nullptr) {
        remote.promote()->RemoveDeathRecipient(recipient_);
    }
    isServerExist_ = false;
    if (g_callbackResumer != nullptr && !IsCallbackResuming()) {
        // only the first request will be handled
        UpdateCallbackResumingState(true);
        // wait for remote died finished
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
        if (HasGnssNetworkRequest() && LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
            g_callbackResumer->ResumeCallback();
        }
        UpdateCallbackResumingState(false);
    }
}

sptr<LocatorProxy> LocatorImpl::GetProxy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (client_ != nullptr && isServerExist_) {
        return client_;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
        return nullptr;
    }
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(LOCATION_LOCATOR_SA_ID);
    if (obj == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get remote service failed.", __func__);
        return nullptr;
    }
    recipient_ = sptr<LocatorDeathRecipient>(new (std::nothrow) LocatorDeathRecipient(*this));
    if ((obj->IsProxyObject()) && (!obj->AddDeathRecipient(recipient_))) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: deathRecipient add failed.", __func__);
        return nullptr;
    }
    isServerExist_ = true;
    client_ = sptr<LocatorProxy>(new (std::nothrow) LocatorProxy(obj));
    LBSLOGI(LOCATOR_STANDARD, "%{public}s: create locator proxy", __func__);
    if (saStatusListener_ == nullptr) {
        saStatusListener_ = sptr<LocatorSystemAbilityListener>(new LocatorSystemAbilityListener());
    }
    int32_t result = sam->SubscribeSystemAbility(static_cast<int32_t>(LOCATION_LOCATOR_SA_ID), saStatusListener_);
    if (result != ERR_OK) {
        LBSLOGE(LOCATOR, "%{public}s SubcribeSystemAbility result is %{public}d!", __func__, result);
    }
    return client_;
}

void LocatorImpl::UpdateCallbackResumingState(bool state)
{
    std::unique_lock<std::mutex> lock(resumeMutex_);
    isCallbackResuming_ = state;
}

bool LocatorImpl::IsCallbackResuming()
{
    std::unique_lock<std::mutex> lock(resumeMutex_);
    return isCallbackResuming_;
}

bool LocatorImpl::IsLocationCallbackRegistered(const sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        return true;
    }
    std::unique_lock<std::mutex> lock(g_locationCallbackMapMutex);
    for (auto iter = g_locationCallbackMap.begin(); iter != g_locationCallbackMap.end(); iter++) {
        auto locatorCallback = iter->first;
        if (locatorCallback == nullptr) {
            continue;
        }
        if (locatorCallback == callback) {
            return true;
        }
    }
    return false;
}

bool LocatorImpl::IsSatelliteStatusChangeCallbackRegistered(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        return true;
    }
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    for (auto gnssStatusCallback : g_gnssStatusInfoCallbacks) {
        if (gnssStatusCallback == callback) {
            return true;
        }
    }
    return false;
}

bool LocatorImpl::IsNmeaCallbackRegistered(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        return true;
    }
    std::unique_lock<std::mutex> lock(g_nmeaCallbacksMutex);
    for (auto nmeaCallback : g_nmeaCallbacks) {
        if (nmeaCallback == callback) {
            return true;
        }
    }
    return false;
}

void LocatorImpl::AddLocationCallBack(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    std::unique_lock<std::mutex> lock(g_locationCallbackMapMutex);
    g_locationCallbackMap.insert(std::make_pair(callback, *requestConfig));
}

void LocatorImpl::RemoveLocationCallBack(sptr<ILocatorCallback>& callback)
{
    std::unique_lock<std::mutex> lock(g_locationCallbackMapMutex);
    auto iter = g_locationCallbackMap.find(callback);
    if (iter != g_locationCallbackMap.end()) {
        g_locationCallbackMap.erase(iter);
    }
}

void LocatorImpl::AddSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    g_gnssStatusInfoCallbacks.insert(callback);
}

void LocatorImpl::RemoveSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    for (auto iter = g_gnssStatusInfoCallbacks.begin(); iter != g_gnssStatusInfoCallbacks.end(); iter++) {
        if (callback == *iter) {
            g_gnssStatusInfoCallbacks.erase(callback);
            break;
        }
    }
}

void LocatorImpl::AddNmeaCallBack(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(g_nmeaCallbacksMutex);
    g_nmeaCallbacks.insert(callback);
}

void LocatorImpl::RemoveNmeaCallBack(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(g_nmeaCallbacksMutex);
    for (auto iter = g_nmeaCallbacks.begin(); iter != g_nmeaCallbacks.end(); iter++) {
        if (callback == *iter) {
            g_nmeaCallbacks.erase(callback);
            break;
        }
    }
}

bool LocatorImpl::HasGnssNetworkRequest()
{
    bool ret = false;
    std::unique_lock<std::mutex> lock(g_locationCallbackMapMutex);
    for (auto iter = g_locationCallbackMap.begin(); iter != g_locationCallbackMap.end(); iter++) {
        auto locatorCallback = iter->first;
        if (locatorCallback == nullptr || iter->first == nullptr) {
            continue;
        }
        auto requestConfig = std::make_unique<RequestConfig>();
        requestConfig->Set(iter->second);
        if (requestConfig->GetScenario() != SCENE_NO_POWER &&
            (requestConfig->GetScenario() != SCENE_UNSET ||
            requestConfig->GetPriority() != PRIORITY_UNSET)) {
            ret = true;
            break;
        }
    }
    return ret;
}

void LocatorImpl::SetIsServerExist(bool isServerExist)
{
    isServerExist_ = isServerExist;
}

void CallbackResumeManager::InitResumeCallbackFuncMap()
{
    std::unique_lock<std::mutex> lock(g_resumeFuncMapMutex);
    if (g_resumeFuncMap.size() != 0) {
        return;
    }
    g_resumeFuncMap.insert(std::make_pair("satelliteStatusChange", [this]() { return ResumeGnssStatusCallback(); }));
    g_resumeFuncMap.insert(std::make_pair("nmeaMessage", [this]() { return ResumeNmeaMessageCallback(); }));
    g_resumeFuncMap.insert(std::make_pair("locationChange", [this]() { return ResumeLocating(); }));
}

void CallbackResumeManager::ResumeCallback()
{
    InitResumeCallbackFuncMap();
    std::unique_lock<std::mutex> lock(g_resumeFuncMapMutex);
    for (auto iter = g_resumeFuncMap.begin(); iter != g_resumeFuncMap.end(); iter++) {
        auto resumeFunc = iter->second;
        resumeFunc();
    }
}

void CallbackResumeManager::ResumeGnssStatusCallback()
{
    sptr<LocatorProxy> proxy = g_locatorImpl->GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ;
    }
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    for (auto gnssStatusCallback : g_gnssStatusInfoCallbacks) {
        if (gnssStatusCallback == nullptr) {
            continue;
        }
        proxy->RegisterGnssStatusCallbackV9(gnssStatusCallback);
    }
}

void CallbackResumeManager::ResumeNmeaMessageCallback()
{
    sptr<LocatorProxy> proxy = g_locatorImpl->GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    std::unique_lock<std::mutex> lock(g_nmeaCallbacksMutex);
    for (auto nmeaCallback : g_nmeaCallbacks) {
        if (nmeaCallback == nullptr) {
            continue;
        }
        proxy->RegisterNmeaMessageCallbackV9(nmeaCallback);
    }
}

void CallbackResumeManager::ResumeLocating()
{
    sptr<LocatorProxy> proxy = g_locatorImpl->GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    std::unique_lock<std::mutex> lock(g_locationCallbackMapMutex);
    for (auto iter = g_locationCallbackMap.begin(); iter != g_locationCallbackMap.end(); iter++) {
        auto locatorCallback = iter->first;
        if (locatorCallback == nullptr || iter->first == nullptr) {
            continue;
        }
        auto requestConfig = std::make_unique<RequestConfig>();
        requestConfig->Set(iter->second);
        LBSLOGI(LOCATOR_STANDARD, "ResumeLocating requestConfig = %{public}s", requestConfig->ToString().c_str());
        proxy->StartLocatingV9(requestConfig, locatorCallback);
    }
}

void LocatorSystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGD(LOCATOR_STANDARD, "%{public}s enter", __func__);
    std::unique_lock<std::mutex> lock(mutex_);
    if (needResume_) {
        if (g_callbackResumer != nullptr && !g_locatorImpl->HasGnssNetworkRequest()) {
            g_callbackResumer->ResumeCallback();
        }
        needResume_ = false;
    }
}

void LocatorSystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    LBSLOGD(LOCATOR_STANDARD, "%{public}s enter", __func__);
    auto locatorImpl = LocatorImpl::GetInstance();
    if (locatorImpl != nullptr) {
        locatorImpl->SetIsServerExist(false);
    }
    std::unique_lock<std::mutex> lock(mutex_);
    needResume_ = true;
}
}  // namespace Location
}  // namespace OHOS
