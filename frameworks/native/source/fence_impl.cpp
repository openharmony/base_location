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
#include "common_utils.h"
#include "location_sa_load_manager.h"

#include "iremote_object.h"
#include "location_log.h"
#include "ipc_skeleton.h"

#include "fence_impl.h"
namespace OHOS {
namespace Location {
const int FENCE_SA_ID = 4353;
const std::u16string FENCE_DESCRIPTION = u"location.IHifenceAbility";
const int REGISTER_GEOFENCE_CALLBACK = 21;
const int UNREGISTER_GEOFENCE_CALLBACK = 22;

bool FenceImpl::InitLocationExt()
{
    if (CommonUtils::CheckIfSystemAbilityAvailable(FENCE_SA_ID)) {
        LBSLOGD(LOCATOR_STANDARD, "locator sa has been loaded");
        return true;
    }
    auto instance = DelayedSingleton<LocationSaLoadManager>::GetInstance();
    if (instance == nullptr || instance->LoadLocationSa(FENCE_SA_ID) != ERRCODE_SUCCESS) {
        LBSLOGE(LOCATOR_STANDARD, "locator sa load failed.");
        return false;
    }
    LBSLOGI(LOCATOR_STANDARD, "init successfully");
    return true;
}

LocationErrCode FenceImpl::AddFenceExt(
    std::unique_ptr<GeofenceRequest> &request, const AbilityRuntime::WantAgent::WantAgent wantAgent)
{
    if (!InitLocationExt()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    LBSLOGD(LOCATOR_STANDARD, "FenceImpl::AddFenceV9()");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(FENCE_DESCRIPTION)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s WriteInterfaceToken failed", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    data.WriteParcelable(&wantAgent);
    sptr<IRemoteObject> proxyExt = CommonUtils::GetRemoteObject(FENCE_SA_ID, CommonUtils::InitDeviceId());
    if (proxyExt == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    proxyExt->SendRequest(REGISTER_GEOFENCE_CALLBACK, data, reply, option);
    return LocationErrCode(reply.ReadInt32());
}

LocationErrCode FenceImpl::RemoveFenceExt(
    std::unique_ptr<GeofenceRequest> &request, const AbilityRuntime::WantAgent::WantAgent wantAgent)
{
    if (!InitLocationExt()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    LBSLOGD(LOCATOR_STANDARD, "FenceImpl::RemoveFenceV9()");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(FENCE_DESCRIPTION)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s WriteInterfaceToken failed", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    data.WriteInt32(request->scenario);
    data.WriteDouble(request->geofence.latitude);
    data.WriteDouble(request->geofence.longitude);
    data.WriteDouble(request->geofence.radius);
    data.WriteDouble(request->geofence.expiration);
    data.WriteParcelable(&wantAgent);
    sptr<IRemoteObject> proxyExt = CommonUtils::GetRemoteObject(FENCE_SA_ID, CommonUtils::InitDeviceId());
    if (proxyExt == nullptr) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    proxyExt->SendRequest(UNREGISTER_GEOFENCE_CALLBACK, data, reply, option);
    return LocationErrCode(reply.ReadInt32());
}
}  // namespace Location
}  // namespace OHOS