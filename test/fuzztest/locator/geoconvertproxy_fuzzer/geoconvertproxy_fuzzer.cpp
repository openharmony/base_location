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

#ifdef FEATURE_GEOCODE_SUPPORT
#include "geoconvertproxy_fuzzer.h"

#include "geo_convert_proxy.h"
#include "geo_convert_service.h"
#include "geocoding_mock_info.h"
#include "message_parcel.h"
#include "geocode_convert_location_request.h"
#include "geocode_convert_address_request.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 3;
    bool GeoConvertProxyFuzzerTest1(const uint8_t* data, size_t size)
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
        auto geoConvertRequest = std::make_unique<GeocodeConvertAddressRequest>();
        MessageParcel convertRequest;
        geoConvertRequest->SetLocale("zh");
        geoConvertRequest->SetMaxItems(1);
        geoConvertRequest->SetDescription("zh");
        geoConvertRequest->SetMaxLatitude(0.0);
        geoConvertRequest->SetMaxLongitude(0.0);
        geoConvertRequest->SetMinLatitude(0.0);
        geoConvertRequest->SetMinLongitude(0.0);
        geoConvertRequest->SetTransId("zh");
        geoConvertRequest->SetCountry("zh");
        geoConvertRequest->UnmarshallingMessageParcel(convertRequest);
        geoConvertRequest->GetLocale();
        geoConvertRequest->GetMaxItems();
        geoConvertRequest->GetDescription();
        geoConvertRequest->GetMaxLatitude();
        geoConvertRequest->GetMaxLongitude();
        geoConvertRequest->GetMinLatitude();
        geoConvertRequest->GetMinLongitude();
        geoConvertRequest->GetTransId();
        geoConvertRequest->GetCountry();
        return true;
    }

    bool GeoConvertProxyFuzzerTest2(const uint8_t* data, size_t size)
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
        auto geoConvertRequest = std::make_unique<GeocodeConvertLocationRequest>();
        MessageParcel convertRequest;
        geoConvertRequest->SetLocale("zh");
        geoConvertRequest->SetMaxItems(1);
        geoConvertRequest->SetTransId("zh");
        geoConvertRequest->SetCountry("zh");
        geoConvertRequest->UnmarshallingMessageParcel(convertRequest);
        geoConvertRequest->GetLocale();
        geoConvertRequest->GetMaxItems();
        geoConvertRequest->GetTransId();
        geoConvertRequest->GetCountry();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GeoConvertProxyFuzzerTest1(data, size);
    OHOS::GeoConvertProxyFuzzerTest2(data, size);
    return 0;
}
#endif // FEATURE_GEOCODE_SUPPORT