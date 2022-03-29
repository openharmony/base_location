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
int GeoConvertServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    LBSLOGI(GEO_CONVERT, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);
    if (callingUid > SYSTEM_UID) {
        LBSLOGE(GEO_CONVERT, "this remote request is not allowed");
        return -1;
    }
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(PASSIVE, "invalid token.");
        return EXCEPTION;
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
