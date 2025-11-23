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

#include "geo_convert_callback_host.h"
#include "location_data_rdb_observer.h"
#include "location_data_rdb_helper.h"
#include "location_data_rdb_manager.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locator.h"
#include "permission_manager.h"
#include "geocode_convert_address_request.h"
#include "geocode_convert_location_request.h"

namespace OHOS {
namespace Location {
constexpr uint32_t WAIT_MS = 1000;
std::shared_ptr<LocatorImpl> LocatorImpl::instance_ = nullptr;
std::mutex LocatorImpl::locatorMutex_;
auto g_locatorImpl = Locator::GetInstance();
std::mutex g_locationCallbackMapMutex;
std::mutex g_gnssStatusInfoCallbacksMutex;
std::mutex g_nmeaCallbacksMutex;
std::shared_ptr<CallbackResumeManager> g_callbackResumer = std::make_shared<CallbackResumeManager>();
using CallbackResumeHandle = std::function<void()>;
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
    UnLoad();
}

bool LocatorImpl::IsLocationEnabled()
{
    LBSLOGD(LOCATION_NAPI, "IsLocationEnabled");
    int32_t state = DEFAULT_SWITCH_STATE;
    state = LocationDataRdbManager::GetSwitchStateFromSysparaForCurrentUser();
    if (state == DISABLED || state == ENABLED) {
        return (state == ENABLED);
    }
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->GetSwitchState(state);
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    LocationErrCode errorCode = CheckEdmPolicy(enable);
    if (errorCode != ERRCODE_SUCCESS) {
        return;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    ErrCode errCode = proxy->EnableAbility(enable);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s failed. %{public}d", __func__, errCode);
    }
}

void LocatorImpl::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    if (IsLocationCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s locatorCallback has registered", __func__);
        return;
    }
    AddLocationCallBack(requestConfig, callback);
    int errCode = proxy->StartLocating(*requestConfig, callback);
    if (errCode != ERRCODE_SUCCESS) {
        RemoveLocationCallBack(callback);
    }
}

void LocatorImpl::StopLocating(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return;
    }
    RemoveLocationCallBack(callback);
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    proxy->StopLocating(callback);
}

std::unique_ptr<Location> LocatorImpl::GetCachedLocation()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return nullptr;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return nullptr;
    }
    std::unique_ptr<Location> location = std::make_unique<Location>();
    ErrCode errorCodeValue = proxy->GetCacheLocation(*location);
    if (errorCodeValue != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "cause some exception happened in lower service.");
        location = nullptr;
    }
    return location;
}

bool LocatorImpl::RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (locationDataManager_ == nullptr) {
        return false;
    }
    AppIdentity appIdentity;
    appIdentity.SetTokenId(IPCSkeleton::GetCallingTokenID());
    appIdentity.SetUid(IPCSkeleton::GetCallingUid());
    return locationDataManager_->RegisterSwitchCallback(callback, appIdentity) == ERRCODE_SUCCESS;
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    if (IsSatelliteStatusChangeCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return false;
    }
    AddSatelliteStatusChangeCallBack(callback);
    proxy->RegisterGnssStatusCallback(callback);
    return true;
}

bool LocatorImpl::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return false;
    }
    RemoveSatelliteStatusChangeCallBack(callback);
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->UnregisterGnssStatusCallback(callback);
    return true;
}

bool LocatorImpl::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    if (IsNmeaCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return false;
    }
    AddNmeaCallBack(callback);
    proxy->RegisterNmeaMessageCallback(callback);
    return true;
}

bool LocatorImpl::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return false;
    }
    RemoveNmeaCallBack(callback);
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->UnregisterNmeaMessageCallback(callback);
    return true;
}

bool LocatorImpl::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return false;
    }
    AppIdentity identity;
    identity.SetPid(IPCSkeleton::GetCallingPid());
    identity.SetUid(IPCSkeleton::GetCallingUid());
    identity.SetTokenId(IPCSkeleton::GetCallingTokenID());
    identity.SetTokenIdEx(IPCSkeleton::GetCallingFullTokenID());
    identity.SetFirstTokenId(IPCSkeleton::GetFirstTokenID());
    std::string bundleName = "";
    if (!CommonUtils::GetBundleNameByUid(identity.GetUid(), bundleName)) {
        LBSLOGD(LOCATOR, "Fail to Get bundle name: uid = %{public}d.", identity.GetUid());
    }
    identity.SetBundleName(bundleName);
    countryCodeManager->RegisterCountryCodeCallback(callback, identity);
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    proxy->RegisterCachedLocationCallback(request->reportingPeriodSec,
        request->wakeUpCacheQueueFull, callback, "location.ILocatorService");
}

void LocatorImpl::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    if (callback == nullptr) {
        return;
    }
    proxy->UnregisterCachedLocationCallback(callback);
}

bool LocatorImpl::IsGeoServiceAvailable()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    bool res = false;
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->IsGeoConvertAvailable(res);
    return res;
}

void LocatorImpl::GetAddressByCoordinate(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    sptr<GeoConvertCallbackHost> callback = new (std::nothrow) GeoConvertCallbackHost();
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "can not get valid callback.");
        return;
    }
    data.ReadInterfaceToken();
    std::unique_ptr<GeocodeConvertLocationRequest> request =
        GeocodeConvertLocationRequest::UnmarshallingMessageParcel(data);
    proxy->GetAddressByCoordinate(callback->AsObject(), *request);
    replyList = callback->GetResult();
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t tokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    bool flag = false;
    if (PermissionManager::CheckSystemPermission(tokenId, tokenIdEx)) {
        flag = true;
    }
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto geoAddress = *iter;
        geoAddress->SetIsSystemApp(flag);
    }
}

void LocatorImpl::GetAddressByLocationName(MessageParcel &data, std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    sptr<GeoConvertCallbackHost> callback = new (std::nothrow) GeoConvertCallbackHost();
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "can not get valid callback.");
        return;
    }
    data.ReadInterfaceToken();
    std::unique_ptr<GeocodeConvertAddressRequest> request =
        GeocodeConvertAddressRequest::UnmarshallingMessageParcel(data);
    proxy->GetAddressByLocationName(callback->AsObject(), *request);
    replyList = callback->GetResult();
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t tokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    bool flag = false;
    if (PermissionManager::CheckSystemPermission(tokenId, tokenIdEx)) {
        flag = true;
    }
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto geoAddress = *iter;
        geoAddress->SetIsSystemApp(flag);
    }
}

bool LocatorImpl::IsLocationPrivacyConfirmed(const int type)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationPrivacyConfirmed()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = false;
    proxy->IsLocationPrivacyConfirmed(type, flag);
    return flag;
}

int LocatorImpl::SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatus()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    ErrCode errorCodeValue = proxy->SetLocationPrivacyConfirmStatus(type, isConfirmed);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

int LocatorImpl::GetCachedGnssLocationsSize()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return -1;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedGnssLocationsSize()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    int size = 0;
    proxy->GetCachedGnssLocationsSize(size);
    return size;
}

int LocatorImpl::FlushCachedGnssLocations()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return -1;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::FlushCachedGnssLocations()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    ErrCode errorCodeValue = proxy->FlushCachedGnssLocations();
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

bool LocatorImpl::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SendCommand()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->SendCommand(commands->scenario, commands->command);
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMock()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    ErrCode res = proxy->EnableLocationMock();
    if (res != ERR_OK) {
        return false;
    }
    return true;
}

bool LocatorImpl::DisableLocationMock()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMock()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    ErrCode res = proxy->DisableLocationMock();
    if (res != ERR_OK) {
        return false;
    }
    return true;
}

bool LocatorImpl::SetMockedLocations(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocations()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    std::vector<Location> locationVector;
    for (const auto& it : location) {
        locationVector.push_back(*it);
    }
    ErrCode errorCodeValue = proxy->SetMockedLocations(timeInterval, locationVector);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        return false;
    }
    return true;
}

bool LocatorImpl::EnableReverseGeocodingMock()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableReverseGeocodingMock()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    ErrCode res = proxy->EnableReverseGeocodingMock();
    if (res != ERR_OK) {
        return false;
    }
    return true;
}

bool LocatorImpl::DisableReverseGeocodingMock()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableReverseGeocodingMock()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    ErrCode res = proxy->DisableReverseGeocodingMock();
    if (res != ERR_OK) {
        return false;
    }
    return true;
}

bool LocatorImpl::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetReverseGeocodingMockInfo()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    std::vector<GeocodingMockInfo> mockInfoVector;
    for (const auto& it : mockInfo) {
        mockInfoVector.push_back(*it);
    }
    ErrCode errorCodeValue = proxy->SetReverseGeocodingMockInfo(mockInfoVector);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        return false;
    }
    return true;
}

LocationErrCode LocatorImpl::IsLocationEnabledV9(bool &isEnabled)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationEnabledV9()");
    int32_t state = DEFAULT_SWITCH_STATE;
    state = LocationDataRdbManager::GetSwitchStateFromSysparaForCurrentUser();
    if (state == DISABLED || state == ENABLED) {
        isEnabled = (state == ENABLED);
        return ERRCODE_SUCCESS;
    }
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    proxy->GetSwitchState(state);
    isEnabled = (state == ENABLED);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::IsLocationEnabledForUser(bool &isEnabled, int32_t userId)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationEnabledV9()");
    int32_t state = DEFAULT_SWITCH_STATE;
    state = LocationDataRdbManager::GetSwitchStateFromSysparaForUser(userId);
    if (state == DISABLED || state == ENABLED) {
        isEnabled = (state == ENABLED);
        return ERRCODE_SUCCESS;
    }
    auto ret = LocationDataRdbManager::GetSwitchStateFromDbForUser(state, userId);
    if (ret != ERRCODE_SUCCESS) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode = CheckEdmPolicy(enable);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableAbilityV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->EnableAbility(enable);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::EnableAbilityForUser(bool enable, int32_t userId)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errorCode = CheckEdmPolicy(enable);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->EnableAbilityForUser(enable, userId);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<ILocatorCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StartLocatingV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (IsLocationCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s locatorCallback has registered", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AddLocationCallBack(requestConfig, callback);
    ErrCode errorCodeValue = proxy->StartLocating(*requestConfig, callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        RemoveLocationCallBack(callback);
    }
    return locationErrCode;
}

LocationErrCode LocatorImpl::StopLocatingV9(sptr<ILocatorCallback>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    RemoveLocationCallBack(callback);
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StopLocatingV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->StopLocating(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::GetCachedLocationV9(std::unique_ptr<Location> &loc)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedLocationV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (loc == nullptr) {
        loc = std::make_unique<Location>();
    }
    ErrCode errorCodeValue = proxy->GetCacheLocation(*loc);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        loc = nullptr;
    }
    return locationErrCode;
}

LocationErrCode LocatorImpl::RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (locationDataManager_ == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AppIdentity appIdentity;
    appIdentity.SetTokenId(IPCSkeleton::GetCallingTokenID());
    appIdentity.SetUid(IPCSkeleton::GetCallingUid());
    return locationDataManager_->
        RegisterSwitchCallback(callback, appIdentity);
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterGnssStatusCallbackV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (IsSatelliteStatusChangeCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AddSatelliteStatusChangeCallBack(callback);
    ErrCode errorCodeValue = proxy->RegisterGnssStatusCallback(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        RemoveSatelliteStatusChangeCallBack(callback);
    }
    return locationErrCode;
}

LocationErrCode LocatorImpl::UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    RemoveSatelliteStatusChangeCallBack(callback);
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterGnssStatusCallbackV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->UnregisterGnssStatusCallback(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterNmeaMessageCallbackV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (IsNmeaCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AddNmeaCallBack(callback);
    ErrCode errorCodeValue = proxy->RegisterNmeaMessageCallback(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        RemoveNmeaCallBack(callback);
    }
    return locationErrCode;
}

LocationErrCode LocatorImpl::UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    RemoveNmeaCallBack(callback);
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterNmeaMessageCallbackV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->UnregisterNmeaMessageCallback(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCountryCodeCallbackV9()");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    if (countryCodeManager == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AppIdentity identity;
    identity.SetPid(IPCSkeleton::GetCallingPid());
    identity.SetUid(IPCSkeleton::GetCallingUid());
    identity.SetTokenId(IPCSkeleton::GetCallingTokenID());
    identity.SetTokenIdEx(IPCSkeleton::GetCallingFullTokenID());
    identity.SetFirstTokenId(IPCSkeleton::GetFirstTokenID());
    std::string bundleName = "";
    if (!CommonUtils::GetBundleNameByUid(identity.GetUid(), bundleName)) {
        LBSLOGD(LOCATOR, "Fail to Get bundle name: uid = %{public}d.", identity.GetUid());
    }
    identity.SetBundleName(bundleName);
    countryCodeManager->RegisterCountryCodeCallback(callback, identity);
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCachedLocationCallbackV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->RegisterCachedLocationCallback(request->reportingPeriodSec,
        request->wakeUpCacheQueueFull, callback, "location.ILocatorService");
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCachedLocationCallbackV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->UnregisterCachedLocationCallback(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::IsGeoServiceAvailableV9(bool &isAvailable)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsGeoServiceAvailableV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->IsGeoConvertAvailable(isAvailable);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::GetAddressByCoordinateV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByCoordinateV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<GeoConvertCallbackHost> callback = new (std::nothrow) GeoConvertCallbackHost();
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "can not get valid callback.");
        return ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    data.ReadInterfaceToken();
    std::unique_ptr<GeocodeConvertLocationRequest> request =
        GeocodeConvertLocationRequest::UnmarshallingMessageParcel(data);
    ErrCode errorCodeValue = proxy->GetAddressByCoordinate(callback->AsObject(), *request);
    replyList = callback->GetResult();
    if (replyList.size() == 0) {
        return ERRCODE_REVERSE_GEOCODING_FAIL;
    }
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t tokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    bool flag = false;
    if (PermissionManager::CheckSystemPermission(tokenId, tokenIdEx)) {
        flag = true;
    }
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto geoAddress = *iter;
        geoAddress->SetIsSystemApp(flag);
    }
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::GetAddressByLocationNameV9(MessageParcel &data,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetAddressByLocationNameV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<GeoConvertCallbackHost> callback = new (std::nothrow) GeoConvertCallbackHost();
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "can not get valid callback.");
        return ERRCODE_GEOCODING_FAIL;
    }
    data.ReadInterfaceToken();
    std::unique_ptr<GeocodeConvertAddressRequest> request =
        GeocodeConvertAddressRequest::UnmarshallingMessageParcel(data);
    ErrCode errorCodeValue = proxy->GetAddressByLocationName(callback->AsObject(), *request);
    replyList = callback->GetResult();
    if (replyList.size() == 0) {
        return ERRCODE_GEOCODING_FAIL;
    }
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t tokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    bool flag = false;
    if (PermissionManager::CheckSystemPermission(tokenId, tokenIdEx)) {
        flag = true;
    }
    for (auto iter = replyList.begin(); iter != replyList.end(); ++iter) {
        auto geoAddress = *iter;
        geoAddress->SetIsSystemApp(flag);
    }
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationPrivacyConfirmedV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->IsLocationPrivacyConfirmed(type, isConfirmed);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationPrivacyConfirmStatusV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->SetLocationPrivacyConfirmStatus(type, isConfirmed);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::GetCachedGnssLocationsSizeV9(int &size)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetCachedGnssLocationsSizeV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->GetCachedGnssLocationsSize(size);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        size = 0;
    }
    return locationErrCode;
}

LocationErrCode LocatorImpl::FlushCachedGnssLocationsV9()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::FlushCachedGnssLocationsV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->FlushCachedGnssLocations();
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::SendCommandV9(std::unique_ptr<LocationCommand>& commands)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SendCommandV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (commands == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->SendCommand(commands->scenario, commands->command);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
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
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableLocationMockV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->EnableLocationMock();
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::DisableLocationMockV9()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableLocationMockV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->DisableLocationMock();
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::SetMockedLocationsV9(
    const int timeInterval, const std::vector<std::shared_ptr<Location>> &location)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetMockedLocationsV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::vector<Location> locationVector;
    for (const auto& it : location) {
        locationVector.push_back(*it);
    }
    ErrCode errorCodeValue = proxy->SetMockedLocations(timeInterval, locationVector);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::EnableReverseGeocodingMockV9()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::EnableReverseGeocodingMockV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->EnableReverseGeocodingMock();
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::DisableReverseGeocodingMockV9()
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::DisableReverseGeocodingMockV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->DisableReverseGeocodingMock();
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::SetReverseGeocodingMockInfoV9(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetReverseGeocodingMockInfoV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::vector<GeocodingMockInfo> geocodingMockInfoVector;
    for (const auto& it : mockInfo) {
        geocodingMockInfoVector.push_back(*it);
    }
    ErrCode errorCodeValue = proxy->SetReverseGeocodingMockInfo(geocodingMockInfoVector);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::ProxyForFreeze(std::set<int> pidList, bool isProxy)
{
    if (!LocationSaLoadManager::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGI(LOCATOR_STANDARD, "%{public}s locator sa is not available.", __func__);
        isServerExist_ = false;
        return ERRCODE_SUCCESS;
    }
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s init locator sa failed", __func__);
        isServerExist_ = false;
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ProxyForFreeze()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::vector<int> dataVector(pidList.begin(), pidList.end());
    ErrCode errorCodeValue = proxy->ProxyForFreeze(dataVector, isProxy);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::ResetAllProxy()
{
    if (!LocationSaLoadManager::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGI(LOCATOR_STANDARD, "%{public}s, no need reset proxy", __func__);
        isServerExist_ = false;
        return ERRCODE_SUCCESS;
    }
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        isServerExist_ = false;
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ResetAllProxy()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->ResetAllProxy();
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::RegisterLocatingRequiredDataCallback(
    std::unique_ptr<LocatingRequiredDataConfig>& dataConfig, sptr<ILocatingRequiredDataCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::%{public}s", __func__);
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->RegisterLocatingRequiredDataCallback(*dataConfig, callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::UnRegisterLocatingRequiredDataCallback(sptr<ILocatingRequiredDataCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::%{public}s", __func__);
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote callback is nullptr");
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->UnRegisterLocatingRequiredDataCallback(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::SubscribeBluetoothScanResultChange(sptr<IBluetoothScanResultCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SubscribeBluetoothScanResultChange()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote callback is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->SubscribeBluetoothScanResultChange(callback);
    if (errorCodeValue != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "SubscribeBluetoothScanResultChange failed.");
    }
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::UnSubscribeBluetoothScanResultChange(sptr<IBluetoothScanResultCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnSubscribeBluetoothScanResultChange()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "SendRegisterMsgToRemote callback is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->UnSubscribeBluetoothScanResultChange(callback);
    if (errorCodeValue != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "UnSubscribeBluetoothScanResultChange failed.");
    }
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::SubscribeLocationError(sptr<ILocatorCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StartLocatingV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->SubscribeLocationError(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "SubscribeLocationError failed.");
    }
    return locationErrCode;
}

LocationErrCode LocatorImpl::UnSubscribeLocationError(sptr<ILocatorCallback>& callback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StopLocatingV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "StopLocating callback is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->UnSubscribeLocationError(callback);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    if (locationErrCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "UnSubscribeLocationError failed.");
    }
    return locationErrCode;
}

LocationErrCode LocatorImpl::GetCurrentWifiBssidForLocating(std::string& bssid)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::GetCurrentWifiBssidForLocating() enter");
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->GetCurrentWifiBssidForLocating(bssid);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

bool LocatorImpl::IsPoiServiceSupported()
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::IsPoiServiceSupported() enter");
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return false;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool poiServiceSupportState = false;
    proxy->IsPoiServiceSupported(poiServiceSupportState);
    return poiServiceSupportState;
}

LocationErrCode LocatorImpl::GetPoiInfo(sptr<IPoiInfoCallback> &poiInfoCallback)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (poiInfoCallback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s poiInfoCallback is null", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCode = proxy->GetPoiInfo(poiInfoCallback->AsObject());
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCode);
    return locationErrCode;
}

LocationErrCode LocatorImpl::GetDistanceBetweenLocations(const Location& location1,
    const Location& location2, double& distance)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::GetDistanceBetweenLocations() enter");
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    distance = location1.GetDistanceBetweenLocations(location1.GetLatitude(), location1.GetLongitude(),
        location2.GetLatitude(), location2.GetLongitude());
    return ERRCODE_SUCCESS;
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
        if (HasGnssNetworkRequest() && SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
            g_callbackResumer->ResumeCallback();
        }
        UpdateCallbackResumingState(false);
    }
}

sptr<ILocatorService> LocatorImpl::GetProxy()
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
    client_ = iface_cast<ILocatorService>(obj);
    if (!client_ || !client_->AsObject()) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get locator service failed.", __func__);
        return nullptr;
    }
    LBSLOGI(LOCATOR_STANDARD, "%{public}s: create locator proxy", __func__);
    recipient_ = sptr<LocatorDeathRecipient>(new (std::nothrow) LocatorDeathRecipient(*this));
    if ((obj->IsProxyObject()) && (!obj->AddDeathRecipient(recipient_))) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: deathRecipient add failed.", __func__);
        return nullptr;
    }
    isServerExist_ = true;
    if (saStatusListener_ == nullptr) {
        saStatusListener_ = sptr<LocatorSystemAbilityListener>(new LocatorSystemAbilityListener());
    }
    int32_t result = sam->SubscribeSystemAbility(static_cast<int32_t>(LOCATION_LOCATOR_SA_ID), saStatusListener_);
    if (result != ERR_OK) {
        LBSLOGE(LOCATOR, "%{public}s SubcribeSystemAbility result is %{public}d!", __func__, result);
    }
    return client_;
}

void LocatorImpl::UnLoad()
{
    LBSLOGD(LOCATOR_STANDARD, "%{public}s", __func__);
    std::unique_lock<std::mutex> lock(mutex_);
    if (saStatusListener_ == nullptr && recipient_ == nullptr) {
        return;
    }
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get samgr failed.", __func__);
        return;
    }
    if (saStatusListener_ != nullptr) {
        sam->UnSubscribeSystemAbility(static_cast<int32_t>(LOCATION_LOCATOR_SA_ID), saStatusListener_);
        saStatusListener_ = nullptr;
    }

    sptr<IRemoteObject> obj = sam->CheckSystemAbility(LOCATION_LOCATOR_SA_ID);
    if (obj == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s: get remote service failed.", __func__);
        return;
    }
    if (recipient_ != nullptr) {
        obj->RemoveDeathRecipient(recipient_);
        recipient_ = nullptr;
    }
    client_ = nullptr;
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

LocationErrCode LocatorImpl::SetLocationSwitchIgnored(bool enable)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::SetLocationSwitchIgnored()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->SetLocationSwitchIgnored(enable);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode LocatorImpl::AddBeaconFence(const std::shared_ptr<BeaconFenceRequest>& beaconFenceRequest)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::AddBeaconFence()");
#ifdef BLUETOOTH_ENABLE
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (beaconFenceRequest == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s beaconFenceRequest is nullptr.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->AddBeaconFence(*beaconFenceRequest);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
#else
    return LOCATION_ERRCODE_NOT_SUPPORTED;
#endif
}

LocationErrCode LocatorImpl::RemoveBeaconFence(const std::shared_ptr<BeaconFence>& beaconFence)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::RemoveBeaconFence()");
#ifdef BLUETOOTH_ENABLE
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (beaconFence == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s beaconFenceRequest is nullptr.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->RemoveBeaconFence(*beaconFence);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
#else
    return LOCATION_ERRCODE_NOT_SUPPORTED;
#endif
}

bool LocatorImpl::IsBeaconFenceSupported()
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::IsBeaconFenceSupported() enter");
#ifdef BLUETOOTH_ENABLE
    return true;
#else
    return false;
#endif
}

LocationErrCode LocatorImpl::GetAppsPerformLocating(std::vector<AppIdentity>& performLocatingAppList)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::GetAppsPerformLocating()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->GetAppsPerformLocating(performLocatingAppList);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

void CallbackResumeManager::ResumeCallback()
{
    ResumeGnssStatusCallback();
    ResumeNmeaMessageCallback();
    ResumeLocating();
}

void CallbackResumeManager::ResumeGnssStatusCallback()
{
    sptr<ILocatorService> proxy = g_locatorImpl->GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ;
    }
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    for (auto gnssStatusCallback : g_gnssStatusInfoCallbacks) {
        if (gnssStatusCallback == nullptr) {
            continue;
        }
        proxy->RegisterGnssStatusCallback(gnssStatusCallback);
    }
}

void CallbackResumeManager::ResumeNmeaMessageCallback()
{
    sptr<ILocatorService> proxy = g_locatorImpl->GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    std::unique_lock<std::mutex> lock(g_nmeaCallbacksMutex);
    for (auto nmeaCallback : g_nmeaCallbacks) {
        if (nmeaCallback == nullptr) {
            continue;
        }
        proxy->RegisterNmeaMessageCallback(nmeaCallback);
    }
}

void CallbackResumeManager::ResumeLocating()
{
    sptr<ILocatorService> proxy = g_locatorImpl->GetProxy();
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
        LBSLOGW(LOCATOR_STANDARD, "ResumeLocating requestConfig = %{public}s", requestConfig->ToString().c_str());
        proxy->StartLocating(*requestConfig, locatorCallback);
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
