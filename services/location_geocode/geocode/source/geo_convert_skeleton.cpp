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

namespace OHOS {
namespace Location {
int GeoConvertServiceStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    LBSLOGI(GEO_CONVERT, "OnRemoteRequest cmd = %{public}u, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), callingPid, callingUid);

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GEO_CONVERT, "invalid token.");
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(GEO_CONVERT, "uid pid not match locationhub process.");
        reply.WriteInt32(ERRCODE_PERMISSION_DENIED);
        return ERRCODE_PERMISSION_DENIED;
    }

    int ret = ERRCODE_SUCCESS;
    switch (code) {
        case IS_AVAILABLE: {
            IsGeoConvertAvailable(reply);
            break;
        }
        case GET_FROM_COORDINATE: {
            GetAddressByCoordinate(data, reply);
            break;
        }
        case GET_FROM_LOCATION_NAME_BY_BOUNDARY: {
            GetAddressByLocationName(data, reply);
            break;
        }
        case ENABLE_REVERSE_GEOCODE_MOCK: {
            EnableReverseGeocodingMock() ? reply.WriteInt32(ERRCODE_SUCCESS) :
                reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
            break;
        }
        case DISABLE_REVERSE_GEOCODE_MOCK: {
            DisableReverseGeocodingMock() ? reply.WriteInt32(ERRCODE_SUCCESS) :
                reply.WriteInt32(ERRCODE_REVERSE_GEOCODING_FAIL);
            break;
        }
        case SET_REVERSE_GEOCODE_MOCKINFO: {
            std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo = ParseGeocodingMockInfos(data);
            reply.WriteInt32(SetReverseGeocodingMockInfo(mockInfo));
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
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
} // namespace Location
} // namespace OHOS
#endif
