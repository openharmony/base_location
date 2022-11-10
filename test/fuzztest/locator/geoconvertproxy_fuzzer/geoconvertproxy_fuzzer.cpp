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
    const int MIN_DATA_LEN = 3;
    bool GeoConvertProxyFuzzerTest(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        int index = 0;
        sptr<GeoConvertService> service =
            new (std::nothrow) GeoConvertService();
        std::shared_ptr<GeoConvertProxy> geoConvertProxy =
            std::make_shared<GeoConvertProxy>(service);
        MessageParcel request;
        MessageParcel reply;
        geoConvertProxy->IsGeoConvertAvailable(request);
        geoConvertProxy->GetAddressByCoordinate(request, reply);
        geoConvertProxy->GetAddressByLocationName(request, reply);
        geoConvertProxy->EnableReverseGeocodingMock();
        geoConvertProxy->DisableReverseGeocodingMock();
        std::vector<std::shared_ptr<GeocodingMockInfo>> geocodingMockInfos;
        geoConvertProxy->SetReverseGeocodingMockInfo(geocodingMockInfos);
        geoConvertProxy->SendSimpleMsgAndParseResult(data[index++]);
        geoConvertProxy->SendSimpleMsg(data[index++], request);
        geoConvertProxy->SendMsgWithDataReply(data[index++], request, reply);
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