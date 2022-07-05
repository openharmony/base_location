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

#ifndef GEO_CONVERT_PROXY_H
#define GEO_CONVERT_PROXY_H

#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "common_utils.h"
#include "geo_coding_mock_info.h"
#include "geo_convert_skeleton.h"

namespace OHOS {
namespace Location {
class GeoConvertProxy : public IRemoteProxy<IGeoConvert> {
public:
    explicit GeoConvertProxy(const sptr<IRemoteObject> &impl);
    ~GeoConvertProxy() = default;
    int IsGeoConvertAvailable(MessageParcel &data, MessageParcel &rep) override;
    int GetAddressByCoordinate(MessageParcel &data, MessageParcel &rep) override;
    int GetAddressByLocationName(MessageParcel &data, MessageParcel &rep) override;
    bool EnableReverseGeocodingMock() override;
    bool DisableReverseGeocodingMock() override;
    bool SetReverseGeocodingMockInfo(std::vector<std::shared_ptr<GeocodingMockInfo>>& mokeInfo) override;
private:
    static inline BrokerDelegator<GeoConvertProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // GEO_CONVERT_PROXY_H
