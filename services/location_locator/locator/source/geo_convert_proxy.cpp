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
#include "location_log.h"

namespace OHOS {
namespace Location {
GeoConvertProxy::GeoConvertProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IGeoConvert>(impl)
{
}

int GeoConvertProxy::IsGeoConvertAvailable(MessageParcel &reply)
{
    return SendSimpleMsg(IS_AVAILABLE, reply);
}

int GeoConvertProxy::GetAddressByCoordinate(MessageParcel &data, MessageParcel &reply)
{
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEO_CONVERT, "write interfaceToken fail!");
        return ERRCODE_INVALID_TOKEN;
    }
    int errorCode = SendMsgWithDataReply(GET_FROM_COORDINATE, data, reply);
    LBSLOGI(GEO_CONVERT, "GetAddressByCoordinate result from server.");
    return errorCode;
}

int GeoConvertProxy::GetAddressByLocationName(MessageParcel &data, MessageParcel &reply)
{
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEO_CONVERT, "write interfaceToken fail!");
        return ERRCODE_INVALID_TOKEN;
    }
    int errorCode = SendMsgWithDataReply(GET_FROM_LOCATION_NAME_BY_BOUNDARY, data, reply);
    LBSLOGI(GEO_CONVERT, "GetAddressByLocationName result from server.");
    return errorCode;
}

int GeoConvertProxy::SendSimpleMsg(const int msgId, MessageParcel& reply)
{
    MessageParcel data;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEO_CONVERT, "write interfaceToken fail!");
        return ERRCODE_INVALID_TOKEN;
    }
    return SendMsgWithDataReply(msgId, data, reply);
}

int GeoConvertProxy::SendMsgWithDataReply(const int msgId, MessageParcel& data, MessageParcel& reply)
{
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LBSLOGE(GEO_CONVERT, "SendMsgWithDataReply remote is null");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    remote->SendRequest(msgId, data, reply, option);
    LBSLOGD(GEO_CONVERT, "Proxy::SendMsgWithDataReply result from server.");
    return reply.ReadInt32();
}

bool GeoConvertProxy::SendSimpleMsgAndParseResult(const int msgId)
{
    MessageParcel reply;
    return SendSimpleMsg(msgId, reply) == ERRCODE_SUCCESS;
}

bool GeoConvertProxy::EnableReverseGeocodingMock()
{
    return SendSimpleMsgAndParseResult(ENABLE_REVERSE_GEOCODE_MOCK);
}

bool GeoConvertProxy::DisableReverseGeocodingMock()
{
    return SendSimpleMsgAndParseResult(DISABLE_REVERSE_GEOCODE_MOCK);
}

bool GeoConvertProxy::SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LBSLOGE(GEO_CONVERT, "write interfaceToken fail!");
        return false;
    }
    data.WriteInt32(mockInfo.size());
    for (size_t i = 0; i < mockInfo.size(); i++) {
        mockInfo[i]->Marshalling(data);
    }
    int errorCode = SendMsgWithDataReply(SET_REVERSE_GEOCODE_MOCKINFO, data, reply);
    return errorCode == ERRCODE_SUCCESS;
}
} // namespace Location
} // namespace OHOS
