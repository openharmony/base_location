/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "geofence_sdk.h"
#include "common_utils.h"
#include "location_sa_load_manager.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
GeofenceManager* GeofenceManager::GetInstance()
{
    static GeofenceManager data;
    return &data;
}

GeofenceManager::GeofenceManager()
{
}

GeofenceManager::~GeofenceManager()
{}

void GeofenceManager::ResetGeofenceSdkProxy(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s: remote is nullptr.", __func__);
        return;
    }
    if (client_ == nullptr || !isServerExist_) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s: proxy is nullptr.", __func__);
        return;
    }
    if (remote.promote() != nullptr) {
        remote.promote()->RemoveDeathRecipient(recipient_);
    }
    isServerExist_ = false;
    LBSLOGI(GEOFENCE_SDK, "%{public}s: finish.", __func__);
}

sptr<ILocatorService> GeofenceManager::GetProxy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (client_ != nullptr && isServerExist_) {
        return client_;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s: get samgr failed.", __func__);
        return nullptr;
    }
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(LOCATION_LOCATOR_SA_ID);
    if (obj == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s: get remote service failed.", __func__);
        return nullptr;
    }
    client_ = iface_cast<ILocatorService>(obj);
    if (!client_ || !client_->AsObject()) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s: get locator service failed.", __func__);
        return nullptr;
    }
    recipient_ = sptr<GeofenceManagerDeathRecipient>(new (std::nothrow) GeofenceManagerDeathRecipient(*this));
    if ((obj->IsProxyObject()) && (!obj->AddDeathRecipient(recipient_))) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s: deathRecipient add failed.", __func__);
        return nullptr;
    }
    isServerExist_ = true;
    return client_;
}

LocationErrCode GeofenceManager::AddFenceV9(std::shared_ptr<GeofenceRequest> &request)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::AddFenceV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (request == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->AddFence(*request);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode GeofenceManager::RemoveFenceV9(std::shared_ptr<GeofenceRequest> &request)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::RemoveFenceV9()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (request == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->RemoveFence(*request);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode GeofenceManager::AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (request == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->AddGnssGeofence(*request);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode GeofenceManager::RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::RemoveGnssGeofence()");
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (request == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    ErrCode errorCodeValue = proxy->RemoveGnssGeofence(request->GetFenceId());
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode GeofenceManager::GetGeofenceSupportedCoordTypes(
    std::vector<CoordinateSystemType>& coordinateSystemTypes)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::%{public}s", __func__);
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::vector<CoordinateType> coordinateTypes;
    ErrCode errorCodeValue = proxy->QuerySupportCoordinateSystemType(coordinateTypes);
    size_t size = coordinateTypes.size();
    size = size > COORDINATE_SYSTEM_TYPE_SIZE ? COORDINATE_SYSTEM_TYPE_SIZE : size;
    for (size_t i = 0; i < size; i++) {
        coordinateSystemTypes.push_back(static_cast<CoordinateSystemType>(coordinateTypes[i]));
    }
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}

LocationErrCode GeofenceManager::GetActiveGeoFences(std::map<int, Geofence>& fenceMap)
{
    if (!SaLoadWithStatistic::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGI(GEOFENCE_SDK, "GeofenceManager::%{public}s", __func__);
    sptr<ILocatorService> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    ErrCode errorCodeValue = proxy->GetActiveGeoFences(fenceMap);
    LocationErrCode locationErrCode = CommonUtils::ErrCodeToLocationErrCode(errorCodeValue);
    return locationErrCode;
}
}
