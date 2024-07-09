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

#ifndef AGNSS_EVENT_CALLBACK_H
#define AGNSS_EVENT_CALLBACK_H
#ifdef FEATURE_GNSS_SUPPORT
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE

#include <v2_0/ia_gnss_callback.h>
#ifdef TEL_CORE_SERVICE_ENABLE
#include "core_service_client.h"
#include "cell_information.h"
#endif

namespace OHOS {
namespace Location {
using HDI::Location::Agnss::V2_0::IAGnssCallback;
using HDI::Location::Agnss::V2_0::AGnssDataLinkRequest;
using HDI::Location::Agnss::V2_0::SubscriberSetId;
using HDI::Location::Agnss::V2_0::SubscriberSetIdType;
using HDI::Location::Agnss::V2_0::AGnssRefInfo;
using HDI::Location::Agnss::V2_0::AGnssRefInfoType;
using HDI::Location::Agnss::V2_0::AGnssUserPlaneProtocol;

#ifdef TEL_CORE_SERVICE_ENABLE
using OHOS::Telephony::CellInformation;
#endif

class AGnssEventCallback : public IAGnssCallback {
public:
    ~AGnssEventCallback() override {}
    int32_t RequestSetUpAgnssDataLink(const AGnssDataLinkRequest& request) override;
    int32_t RequestSubscriberSetId(SubscriberSetIdType type) override;
    int32_t RequestAgnssRefInfo(AGnssRefInfoType type) override;
private:
#ifdef TEL_CORE_SERVICE_ENABLE
    void JudgmentDataGsm(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem);
    void JudgmentDataUmts(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem);
    void JudgmentDataLte(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem);
    void JudgmentDataNr(AGnssRefInfo& refInfo, sptr<CellInformation> infoItem);
#endif
    __attribute__((no_sanitize("cfi"))) void GetWiFiRefInfo(AGnssRefInfo& refInfo);
    void GetCellRefInfo(AGnssRefInfo& refInfo);
};
}  // namespace Location
}  // namespace OHOS
#endif // HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
#endif // FEATURE_GNSS_SUPPORT
#endif // AGNSS_EVENT_CALLBACK_H
