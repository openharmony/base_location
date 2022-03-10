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

#include "geo_convert_skeleton.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "lbs_log.h"

namespace OHOS {
namespace Location {
GeoConvertProxy::GeoConvertProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGeoConvert>(impl)
{
}

int GeoConvertProxy::IsGeoConvertAvailable(MessageParcel &data, MessageParcel &reply)
{
    int error;
    MessageOption option;
    error = Remote()->SendRequest(IS_AVAILABLE, data, reply, option);
    LBSLOGI(GEO_CONVERT, "IsGeoConvertAvailable result from server");
    return error;
}

int GeoConvertProxy::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    int error;
    MessageOption option;
    error = Remote()->SendRequest(GET_FROM_COORDINATE, data, reply, option);
    LBSLOGI(GEO_CONVERT, "GetAddressByCoordinate result from server.");
    return error;
}

int GeoConvertProxy::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    int error;
    MessageOption option;
    error = Remote()->SendRequest(GET_FROM_LOCATION_NAME_BY_BOUNDARY, data, reply, option);
    LBSLOGI(GEO_CONVERT, "GetAddressByLocationName result from server.");
    return error;
}

int GeoConvertServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    LBSLOGI(GEO_CONVERT, "OnRemoteRequest cmd = %{public}d, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);
    if (callingUid > SYSTEM_UID) {
        LBSLOGE(GEO_CONVERT, "this remote request is not allowed");
        return -1;
    }

    int ret;
    switch (code) {
        case IS_AVAILABLE: {
            ret = IsGeoConvertAvailable(data, reply);
            break;
        }
        case GET_FROM_COORDINATE: {
            ret = GetAddressByCoordinate(data, reply);
            break;
        }
        case GET_FROM_LOCATION_NAME_BY_BOUNDARY: {
            ret = GetAddressByLocationName(data, reply);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}
} // namespace Location
} // namespace OHOS
