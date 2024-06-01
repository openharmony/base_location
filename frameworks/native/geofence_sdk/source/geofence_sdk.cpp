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

sptr<GeofenceSdk> GeofenceManager::GetProxy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (client_ != nullptr) {
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
    client_ = sptr<GeofenceSdk>(new (std::nothrow) GeofenceSdk(obj));
    return client_;
}

LocationErrCode GeofenceManager::AddFenceV9(std::shared_ptr<GeofenceRequest> &request)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::AddFenceV9()");
    sptr<GeofenceSdk> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->AddFenceV9(request);
    return errCode;
}

LocationErrCode GeofenceManager::RemoveFenceV9(std::shared_ptr<GeofenceRequest> &request)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::RemoveFenceV9()");
    sptr<GeofenceSdk> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->RemoveFenceV9(request);
    return errCode;
}

LocationErrCode GeofenceManager::AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::AddGnssGeofence()");
    sptr<GeofenceSdk> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->AddGnssGeofence(request);
    return errCode;
}

LocationErrCode GeofenceManager::RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::RemoveGnssGeofence()");
    sptr<GeofenceSdk> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode errCode = proxy->RemoveGnssGeofence(request);
    return errCode;
}

LocationErrCode GeofenceManager::GetGeofenceSupportedCoordTypes(
    std::vector<CoordinateSystemType>& coordinateSystemTypes)
{
    if (!LocationSaLoadManager::InitLocationSa(LOCATION_LOCATOR_SA_ID)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "GeofenceManager::%{public}s", __func__);
    sptr<GeofenceSdk> proxy = GetProxy();
    if (proxy == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s get proxy failed.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return proxy->GetGeofenceSupportedCoordTypes(coordinateSystemTypes);
}

GeofenceSdk::GeofenceSdk(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ILocator>(impl)
{
}

LocationErrCode GeofenceSdk::AddFenceV9(std::shared_ptr<GeofenceRequest>& request)
{
    return HandleGnssfenceRequest(LocatorInterfaceCode::ADD_FENCE, request);
}

LocationErrCode GeofenceSdk::RemoveFenceV9(std::shared_ptr<GeofenceRequest>& request)
{
    return HandleGnssfenceRequest(LocatorInterfaceCode::REMOVE_FENCE, request);
}

LocationErrCode GeofenceSdk::HandleGnssfenceRequest(
    LocatorInterfaceCode code, std::shared_ptr<GeofenceRequest>& request)
{
    if (request == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEOFENCE_SDK, "%{public}s WriteInterfaceToken failed", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    request->Marshalling(data);
    LocationErrCode errorCode = SendMsgWithDataReplyV9(static_cast<int>(code), data, reply);
    LBSLOGD(GEOFENCE_SDK, "Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode GeofenceSdk::AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    return HandleGnssfenceRequest(LocatorInterfaceCode::ADD_GNSS_GEOFENCE, request);
}

LocationErrCode GeofenceSdk::RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request)
{
    if (request == nullptr) {
        return ERRCODE_INVALID_PARAM;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(request->GetFenceId());
    LocationErrCode errorCode = SendMsgWithDataReplyV9(
        static_cast<int>(LocatorInterfaceCode::REMOVE_GNSS_GEOFENCE), data, reply);
    LBSLOGD(GEOFENCE_SDK, "Transact ErrCodes = %{public}d", errorCode);
    return errorCode;
}

LocationErrCode GeofenceSdk::GetGeofenceSupportedCoordTypes(
    std::vector<CoordinateSystemType>& coordinateSystemTypes)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    MessageParcel reply;
    LocationErrCode errorCode = SendMsgWithDataReplyV9(
        static_cast<int>(LocatorInterfaceCode::GET_GEOFENCE_SUPPORT_COORDINATE_SYSTEM_TYPE), data, reply);
    LBSLOGD(GEOFENCE_SDK, "Proxy::%{public}s Transact ErrCodes = %{public}d", __func__, errorCode);
    int size = reply.ReadInt32();
    size = size > COORDINATE_SYSTEM_TYPE_SIZE ? COORDINATE_SYSTEM_TYPE_SIZE : size;
    for (int i = 0; i < size; i++) {
        int coordinateSystemType = reply.ReadInt32();
        coordinateSystemTypes.push_back(static_cast<CoordinateSystemType>(coordinateSystemType));
    }
    return errorCode;
}

LocationErrCode GeofenceSdk::SendMsgWithDataReplyV9(const int msgId, MessageParcel& data, MessageParcel& reply)
{
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GEOFENCE_SDK, "SendMsgWithDataReply remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    int error = remote->SendRequest(msgId, data, reply, option);
    if (error != NO_ERROR) {
        LBSLOGE(GEOFENCE_SDK, "msgid = %{public}d, send request error: %{public}d", msgId, error);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LBSLOGD(GEOFENCE_SDK, "Proxy::SendMsgWithDataReply result from server.");
    return LocationErrCode(reply.ReadInt32());
}
}
}
