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

#ifndef GNSS_INTERFACE_TEST_H
#define GNSS_INTERFACE_TEST_H
#ifdef FEATURE_GNSS_SUPPORT

#include <gtest/gtest.h>

#include <v2_0/ia_gnss_interface.h>
#include <v2_0/ignss_interface.h>

namespace OHOS {
namespace Location {
using HDI::Location::Gnss::V2_0::IGnssInterface;
using HDI::Location::Gnss::V2_0::IGnssCallback;
using HDI::Location::Gnss::V2_0::GnssConfigPara;
using HDI::Location::Gnss::V2_0::GnssRefInfo;

class GnssInterfaceTest : public IGnssInterface {
public:
    GnssInterfaceTest();
    ~GnssInterfaceTest() override;

    int32_t SetGnssConfigPara(const GnssConfigPara &para) override;

    int32_t EnableGnss(const sptr<IGnssCallback> &callbackObj) override;

    int32_t DisableGnss() override;

    int32_t StartGnss(HDI::Location::Gnss::V2_0::GnssStartType type) override;

    int32_t StopGnss(HDI::Location::Gnss::V2_0::GnssStartType type) override;

    int32_t SetGnssReferenceInfo(const GnssRefInfo &refInfo) override;

    int32_t DeleteAuxiliaryData(unsigned short data) override;

    int32_t SetPredictGnssData(const std::string &data) override;

    int32_t GetCachedGnssLocationsSize(int32_t &size) override;

    int32_t GetCachedGnssLocations() override;

    int32_t SendNiUserResponse(int32_t gnssNiNotificationId,
                               HDI::Location::Gnss::V2_0::GnssNiResponseCmd userResponse) override;

    int32_t SendNetworkInitiatedMsg(const std::string &msg, int32_t length) override;

    int32_t EnableGnssMeasurement(
        const sptr<HDI::Location::Gnss::V2_0::IGnssMeasurementCallback> &callbackObj) override;

    int32_t DisableGnssMeasurement() override;
};
}  // namespace Location
}  // namespace OHOS
#endif  // FEATURE_GNSS_SUPPORT
#endif  // GNSS_INTERFACE_TEST_H
