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

#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_convert_skeleton.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"
#include "locationhub_ipc_interface_code.h"
#include "permission_manager.h"

namespace OHOS {
namespace Location {
void GeoConvertServiceStub::InitGeoConvertHandleMap()
{
    if (geoConvertMsgHandleMap_.size() != 0) {
        return;
    }
    geoConvertMsgHandleMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::IS_AVAILABLE)] =
        &GeoConvertServiceStub::IsGeoConvertAvailableInner;
    geoConvertMsgHandleMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::GET_FROM_COORDINATE)] =
        &GeoConvertServiceStub::GetAddressByCoordinateInner;
    geoConvertMsgHandleMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::GET_FROM_LOCATION_NAME_BY_BOUNDARY)] =
        &GeoConvertServiceStub::GetAddressByLocationNameInner;
    geoConvertMsgHandleMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK)] =
        &GeoConvertServiceStub::EnableReverseGeocodingMockInner;
    geoConvertMsgHandleMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK)] =
        &GeoConvertServiceStub::DisableReverseGeocodingMockInner;
    geoConvertMsgHandleMap_[static_cast<uint32_t>(GeoConvertInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO)] =
        &GeoConvertServiceStub::SetGeocodingMockInfoInner;
}

GeoConvertServiceStub::GeoConvertServiceStub()
{
    InitGeoConvertHandleMap();
}

std::vector<std::shared_ptr<GeocodingMockInfo>> GeoConvertServiceStub::ParseGeocodingMockInfos(MessageParcel &data)
{
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    int arraySize = data.ReadInt32();
    arraySize = arraySize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
        arraySize;
    if (arraySize <= 0) {
        return std::vector<std::shared_ptr<GeocodingMockInfo>>();
    }
    for (int i = 0; i < arraySize; i++) {
        std::shared_ptr<GeocodingMockInfo> info = std::make_shared<GeocodingMockInfo>();
        info->ReadFromParcel(data);
        mockInfo.push_back(info);
    }
    return mockInfo;
}

int GeoConvertServiceStub::IsGeoConvertAvailableInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    IsGeoConvertAvailable(reply);
    return ERRCODE_SUCCESS;
}

int GeoConvertServiceStub::GetAddressByCoordinateInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    GetAddressByCoordinate(data, reply);
    return ERRCODE_SUCCESS;
}

int GeoConvertServiceStub::GetAddressByLocationNameInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    GetAddressByLocationName(data, reply);
    return ERRCODE_SUCCESS;
}

int GeoConvertServiceStub::EnableReverseGeocodingMockInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    EnableReverseGeocodingMock() ? reply.WriteInt32(ERRCODE_SUCCESS) :
        reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
    return ERRCODE_SUCCESS;
}

int GeoConvertServiceStub::DisableReverseGeocodingMockInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    DisableReverseGeocodingMock() ? reply.WriteInt32(ERRCODE_SUCCESS) :
        reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
    return ERRCODE_SUCCESS;
}

int GeoConvertServiceStub::SetGeocodingMockInfoInner(
    MessageParcel &data, MessageParcel &reply, AppIdentity &identity)
{
    if (!PermissionManager::CheckCallingPermission(identity.GetUid(), identity.GetPid(), reply)) {
        return ERRCODE_PERMISSION_DENIED;
    }
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo = ParseGeocodingMockInfos(data);
    reply.WriteInt32(SetReverseGeocodingMockInfo(mockInfo));
    return ERRCODE_SUCCESS;
}

int GeoConvertServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    AppIdentity identity;
    identity.SetPid(callingPid);
    identity.SetUid(callingUid);
    LBSLOGI(GEO_CONVERT,
        "cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid = %{public}d, timestamp = %{public}s",
        code, option.GetFlags(), callingPid, callingUid, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GEO_CONVERT, "invalid token.");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
 
    int ret = ERRCODE_SUCCESS;
    auto handleFunc = geoConvertMsgHandleMap_.find(code);
    if (handleFunc != geoConvertMsgHandleMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        ret = (this->*memberFunc)(data, reply, identity);
    } else {
        LBSLOGE(GEO_CONVERT, "OnReceived cmd = %{public}u, unsupport service.", code);
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    UnloadGeoConvertSystemAbility();
    return ret;
}
} // namespace Location
} // namespace OHOS
#endif
