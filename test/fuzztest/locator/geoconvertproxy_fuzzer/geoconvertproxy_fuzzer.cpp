/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "geoconvertproxy_fuzzer.h"

#include "geo_convert_proxy.h"
#include "geo_convert_service.h"
#include "geo_coding_mock_info.h"
#include "message_parcel.h"

namespace OHOS {
    using namespace OHOS::Location;

    bool GeoConvertProxyFuzzerTest(const uint8_t* data, size_t size)
    {
        sptr<GeoConvertService> service =
            new (std::nothrow) GeoConvertService();
        std::shared_ptr<GeoConvertProxy> geoConvertProxy =
            std::make_shared<GeoConvertProxy>(service);
        MessageParcel request;
        MessageParcel reply;
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        geoConvertProxy->IsGeoConvertAvailable(request);
        geoConvertProxy->GetAddressByCoordinate(request, reply);
        geoConvertProxy->GetAddressByLocationName(request, reply);
        geoConvertProxy->EnableReverseGeocodingMock();
        geoConvertProxy->DisableReverseGeocodingMock();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geocodingMockInfos;
        geoConvertProxy->SetReverseGeocodingMockInfo(geocodingMockInfos);
        int msgId = *(reinterpret_cast<const int*>(data));
        geoConvertProxy->SendSimpleMsgAndParseResult(msgId);
        geoConvertProxy->SendSimpleMsg(msgId, request);
        geoConvertProxy->SendMsgWithDataReply(msgId, request, reply);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GeoConvertProxyFuzzerTest(data, size);
    return 0;
}