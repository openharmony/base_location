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

#include <singleton.h>
#include "locator_impl.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "common_utils.h"
#include "country_code.h"

#include "location_data_rdb_observer.h"
#include "location_data_rdb_helper.h"
#include "location_log.h"
#include "location_sa_load_manager.h"

namespace OHOS {
namespace Location {
constexpr uint32_t WAIT_MS = 1000;
std::shared_ptr<LocatorImpl> LocatorImpl::instance_ = nullptr;
std::mutex LocatorImpl::locatorMutex_;

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
    locationDataManager_ = DelayedSingleton<LocationDataManager>::GetInstance();
    countryCodeManager_ = DelayedSingleton<CountryCodeManager>::GetInstance();
    if (countryCodeManager_ != nullptr) {
        countryCodeManager_->ReSubscribeEvent();
    }
}

LocatorImpl::~LocatorImpl()
{
    if (countryCodeManager_ != nullptr) {
        countryCodeManager_->ReUnsubscribeEvent();
    }
}

bool LocatorImpl::Init()
{
    if (CommonUtils::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGD(LOCATOR_STANDARD, "locator sa has been loaded");
        return true;
    }
    auto instance = DelayedSingleton<LocationSaLoadManager>::GetInstance();
    if (instance == nullptr || instance->LoadLocationSa(LOCATION_LOCATOR_SA_ID) != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "locator sa load failed.");
        return false;
    }
    LBSLOGI(LOCATOR_STANDARD, "init successfully");
    return true;
}

bool LocatorImpl::IsLocationEnabled()
{
    int32_t state = DISABLED;
    auto locationDataRdbHelper =
        DelayedSingleton<LocationDataRdbHelper>::GetInstance();
    if (locationDataRdbHelper == nullptr) {
        return false;
    }
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    LocationErrCode errCode =
        locationDataRdbHelper->GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "IsLocationEnabled err = %{public}d", errCode);
    }
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
    if (!Init()) {
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
    if (!Init()) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    proxy->StartLocating(requestConfig, callback, "location.ILocator", 0, 0);
}

void LocatorImpl::StopLocating(sptr<ILocatorCallback>& callback)
{
    if (!Init()) {
        return;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return;
    }
    proxy->StopLocating(callback);
}

std::unique_ptr<Location> LocatorImpl::GetCachedLocation()
{
    if (!Init()) {
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
    auto locationDataRdbHelper = DelayedSingleton<LocationDataRdbHelper>::GetInstance();
    auto dataRdbObserver = sptr<LocationDataRdbObserver>(new (std::nothrow) LocationDataRdbObserver());
    if (locationDataManager_ == nullptr || locationDataRdbHelper == nullptr || dataRdbObserver == nullptr) {
        return false;
    }
    if (!isObserverReg_) {
        Uri locationDataEnableUri(LOCATION_DATA_URI);
        locationDataRdbHelper->RegisterDataObserver(locationDataEnableUri, dataRdbObserver);
        isObserverReg_ = true;
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
    if (!Init()) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->RegisterGnssStatusCallback(callback, DEFAULT_UID);
    return true;
}

bool LocatorImpl::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    if (!Init()) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->UnregisterGnssStatusCallback(callback);
    return true;
}

bool LocatorImpl::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (!Init()) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->RegisterNmeaMessageCallback(callback, DEFAULT_UID);
    return true;
}

bool LocatorImpl::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    if (!Init()) {
        return false;
    }
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->UnregisterNmeaMessageCallback(callback);
    return true;
}

bool LocatorImpl::RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager_ is nullptr", __func__);
        return false;
    }
    countryCodeManager_->RegisterCountryCodeCallback(callback, uid);
    return true;
}

bool LocatorImpl::UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback)
{
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager_ is nullptr", __func__);
        return false;
    }
    countryCodeManager_->UnregisterCountryCodeCallback(callback);
    return true;
}

void LocatorImpl::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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

bool LocatorImpl::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    if (!Init()) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::AddFence()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->AddFence(request);
    return true;
}

bool LocatorImpl::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    if (!Init()) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RemoveFence()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    proxy->RemoveFence(request);
    return true;
}

std::shared_ptr<CountryCode> LocatorImpl::GetIsoCountryCode()
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCode()");
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager_ is nullptr", __func__);
        return nullptr;
    }
    return countryCodeManager_->GetIsoCountryCode();
}

bool LocatorImpl::EnableLocationMock()
{
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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

bool LocatorImpl::ProxyUidForFreeze(int32_t uid, bool isProxy)
{
    if (!CommonUtils::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        return true;
    }
    if (!Init()) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ProxyUid()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->ProxyUidForFreeze(uid, isProxy);
    return flag;
}

bool LocatorImpl::ResetAllProxy()
{
    if (!CommonUtils::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGI(LOCATOR_STANDARD, "%{public}s, no need reset proxy", __func__);
        return true;
    }
    if (!Init()) {
        return false;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ResetAllProxy()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return false;
    }
    bool flag = proxy->ResetAllProxy();
    return flag;
}

LocationErrCode LocatorImpl::IsLocationEnabledV9(bool &isEnabled)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::IsLocationEnabledV9()");
    int32_t state = DISABLED;
    auto locationDataRdbHelper =
        DelayedSingleton<LocationDataRdbHelper>::GetInstance();
    if (locationDataRdbHelper == nullptr) {
        return ERRCODE_NOT_SUPPORTED;
    }
    Uri locationDataEnableUri(LOCATION_DATA_URI);
    LocationErrCode errCode =
        locationDataRdbHelper->GetValue(locationDataEnableUri, LOCATION_DATA_COLUMN_ENABLE, state);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "IsLocationEnabledV9 err = %{public}d", errCode);
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
    if (!Init()) {
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
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StartLocatingV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->StartLocatingV9(requestConfig, callback);
    return errCode;
}

LocationErrCode LocatorImpl::StopLocatingV9(sptr<ILocatorCallback>& callback)
{
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::StopLocatingV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->StopLocatingV9(callback);
    return errCode;
}

LocationErrCode LocatorImpl::GetCachedLocationV9(std::unique_ptr<Location> &loc)
{
    if (!Init()) {
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
    auto locationDataRdbHelper = DelayedSingleton<LocationDataRdbHelper>::GetInstance();
    auto dataRdbObserver = sptr<LocationDataRdbObserver>(new (std::nothrow) LocationDataRdbObserver());
    if (locationDataManager_ == nullptr || locationDataRdbHelper == nullptr || dataRdbObserver == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!isObserverReg_) {
        Uri locationDataEnableUri(LOCATION_DATA_URI);
        locationDataRdbHelper->RegisterDataObserver(locationDataEnableUri, dataRdbObserver);
        isObserverReg_ = true;
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
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterGnssStatusCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->RegisterGnssStatusCallbackV9(callback);
    return errCode;
}

LocationErrCode LocatorImpl::UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterGnssStatusCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->UnregisterGnssStatusCallbackV9(callback);
    return errCode;
}

LocationErrCode LocatorImpl::RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterNmeaMessageCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->RegisterNmeaMessageCallbackV9(callback);
    return errCode;
}

LocationErrCode LocatorImpl::UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback)
{
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterNmeaMessageCallbackV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->UnregisterNmeaMessageCallbackV9(callback);
    return errCode;
}

LocationErrCode LocatorImpl::RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RegisterCountryCodeCallbackV9()");
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager_ is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCodeManager_->RegisterCountryCodeCallback(callback, 0);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::UnregisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::UnregisterCountryCodeCallbackV9()");
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager_ is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCodeManager_->UnregisterCountryCodeCallback(callback);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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

LocationErrCode LocatorImpl::AddFenceV9(std::unique_ptr<GeofenceRequest>& request)
{
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::AddFenceV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->AddFenceV9(request);
    return errCode;
}

LocationErrCode LocatorImpl::RemoveFenceV9(std::unique_ptr<GeofenceRequest>& request)
{
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::RemoveFenceV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->RemoveFenceV9(request);
    return errCode;
}

LocationErrCode LocatorImpl::GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode)
{
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCodeV9()");
    if (countryCodeManager_ == nullptr) {
        LBSLOGE(LOCATOR, "%{public}s countryCodeManager_ is nullptr", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCode = countryCodeManager_->GetIsoCountryCode();
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::EnableLocationMockV9()
{
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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
    if (!Init()) {
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

LocationErrCode LocatorImpl::ProxyUidForFreezeV9(int32_t uid, bool isProxy)
{
    if (!CommonUtils::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SUCCESS;
    }
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ProxyUidForFreezeV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->ProxyUidForFreezeV9(uid, isProxy);
    return errCode;
}

LocationErrCode LocatorImpl::ResetAllProxyV9()
{
    if (!CommonUtils::CheckIfSystemAbilityAvailable(LOCATION_LOCATOR_SA_ID)) {
        LBSLOGI(LOCATOR_STANDARD, "%{public}s, no need reset proxy", __func__);
        return ERRCODE_SUCCESS;
    }
    if (!Init()) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(LOCATOR_STANDARD, "LocatorImpl::ResetAllProxyV9()");
    sptr<LocatorProxy> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->ResetAllProxyV9();
    return errCode;
}

LocationErrCode LocatorImpl::RegisterLocatingRequiredDataCallback(
    std::unique_ptr<LocatingRequiredDataConfig>& dataConfig, sptr<ILocatingRequiredDataCallback>& callback)
{
    if (!Init()) {
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
    if (!Init()) {
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
    if (resumer_ != nullptr && !IsCallbackResuming()) {
        // only the first request will be handled
        UpdateCallbackResumingState(true);
        // wait for remote died finished
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
        resumer_->ResumeCallback();
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
    return client_;
}

void LocatorImpl::SetResumer(std::shared_ptr<ICallbackResumeManager> resumer)
{
    if (resumer_ == nullptr) {
        resumer_ = resumer;
    }
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
}  // namespace Location
}  // namespace OHOS
