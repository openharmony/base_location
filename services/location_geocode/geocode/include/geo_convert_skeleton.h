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

#ifndef GEO_CONVERT_SKELETON_H
#define GEO_CONVERT_SKELETON_H
#ifdef FEATURE_GEOCODE_SUPPORT

#include <vector>

#include "iremote_broker.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include "string_ex.h"

#include "app_identity.h"
#include "geo_coding_mock_info.h"

namespace OHOS {
namespace Location {
class IGeoConvert : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"location.IGeoConvert");
    virtual int IsGeoConvertAvailable(MessageParcel &rep) = 0;
    virtual int GetAddressByCoordinate(MessageParcel &data, MessageParcel &rep) = 0;
    virtual int GetAddressByLocationName(MessageParcel &data, MessageParcel &rep) = 0;
    virtual bool EnableReverseGeocodingMock() = 0;
    virtual bool DisableReverseGeocodingMock() = 0;
    virtual LocationErrCode SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mockInfo) = 0;
};

class GeoConvertServiceStub : public IRemoteStub<IGeoConvert> {
public:
    using GeoConvertMsgHandle = int (GeoConvertServiceStub::*)(
        MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    using GeoConvertMsgHandleMap = std::map<int, GeoConvertMsgHandle>;
    GeoConvertServiceStub();
    virtual ~GeoConvertServiceStub() = default;
    void InitGeoConvertHandleMap();
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void UnloadGeoConvertSystemAbility() = 0;
private:
    int IsGeoConvertAvailableInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int GetAddressByCoordinateInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int GetAddressByLocationNameInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int EnableReverseGeocodingMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int DisableReverseGeocodingMockInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
    int SetGeocodingMockInfoInner(MessageParcel &data, MessageParcel &reply, AppIdentity &identity);
private:
    GeoConvertMsgHandleMap geoConvertMsgHandleMap_;
    std::vector<std::shared_ptr<GeocodingMockInfo>> ParseGeocodingMockInfos(MessageParcel &data);
};
} // namespace OHOS
} // namespace Location
#endif // FEATURE_GEOCODE_SUPPORT
#endif // GEO_CONVERT_SKELETON_H
