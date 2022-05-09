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

#include "geo_convert_proxy.h"
#include "ipc_skeleton.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
GeoConvertProxy::GeoConvertProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGeoConvert>(impl)
{
}

int GeoConvertProxy::IsGeoConvertAvailable(MessageParcel &data, MessageParcel &reply)
{
    int error = -1;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEO_CONVERT, "write interfaceToken fail!");
        return error;
    }
    error = Remote()->SendRequest(IS_AVAILABLE, data, reply, option);
    LBSLOGI(GEO_CONVERT, "IsGeoConvertAvailable result from server");
    return error;
}

int GeoConvertProxy::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    int error = -1;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEO_CONVERT, "write interfaceToken fail!");
        return error;
    }
    error = Remote()->SendRequest(GET_FROM_COORDINATE, data, reply, option);
    LBSLOGI(GEO_CONVERT, "GetAddressByCoordinate result from server.");
    return error;
}

int GeoConvertProxy::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    int error = -1;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEO_CONVERT, "write interfaceToken fail!");
        return error;
    }
    error = Remote()->SendRequest(GET_FROM_LOCATION_NAME_BY_BOUNDARY, data, reply, option);
    LBSLOGI(GEO_CONVERT, "GetAddressByLocationName result from server.");
    return error;
}
} // namespace Location
} // namespace OHOS