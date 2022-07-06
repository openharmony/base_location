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
        LBSLOGE(PASSIVE, "invalid token.");
        return REPLY_CODE_EXCEPTION;
    }

    int ret = REPLY_CODE_NO_EXCEPTION;
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
        case ENABLE_REVERSE_GEOCODE_MOCK: {
            bool result = EnableReverseGeocodingMock();
            reply.WriteBool(result);
            break;
        }
        case DISABLE_REVERSE_GEOCODE_MOCK: {
            bool result = DisableReverseGeocodingMock();
            reply.WriteBool(result);
            break;
        }
        case SET_REVERSE_GEOCODE_MOCKINFO: {
            std::vector<std::shared_ptr<GeocodingMockInfo>> mokeInfo =  ParseGeocodingMockInfos(data);
            bool result = SetReverseGeocodingMockInfo(mokeInfo);
            reply.WriteBool(result);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}

std::vector<std::shared_ptr<GeocodingMockInfo>> GeoConvertServiceStub::ParseGeocodingMockInfos(MessageParcel &data)
{
    std::vector<std::shared_ptr<GeocodingMockInfo>> mokeInfo;
    int arraySize = data.ReadInt32();
    if (arraySize <= 0) {
        return std::vector<std::shared_ptr<GeocodingMockInfo>>();
    }
    for (int i = 0; i < arraySize; i++) {
        std::shared_ptr<ReverseGeocodeRequest> request = std::make_shared<ReverseGeocodeRequest>();
        std::shared_ptr<GeoAddress> geoAddress = std::make_shared<GeoAddress>();
        std::shared_ptr<GeocodingMockInfo> info = std::make_shared<GeocodingMockInfo>();
        request->locale = Str16ToStr8(data.ReadString16());
        request->latitude = data.ReadDouble();
        request->longitude = data.ReadDouble();
        request->maxItems = data.ReadInt32();
        geoAddress->ReadFromParcel(data);
        info->SetReverseGeocodeRequest(request);
        info->SetGeoAddressInfo(geoAddress);
        mokeInfo.push_back(info);
    }
    return mokeInfo;
}
} // namespace Location
} // namespace OHOS
