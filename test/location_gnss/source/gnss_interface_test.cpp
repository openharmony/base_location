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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_interface_test.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
GnssInterfaceTest::GnssInterfaceTest()
{
}

GnssInterfaceTest::~GnssInterfaceTest()
{
}

int32_t GnssInterfaceTest::SetGnssConfigPara(const GnssConfigPara &para)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::EnableGnss(const sptr<IGnssCallback> &callbackObj)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::DisableGnss()
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::StartGnss(HDI::Location::Gnss::V2_0::GnssStartType type)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::StopGnss(HDI::Location::Gnss::V2_0::GnssStartType type)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::SetGnssReferenceInfo(const GnssRefInfo &refInfo)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::DeleteAuxiliaryData(unsigned short data)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::SetPredictGnssData(const std::string &data)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::GetCachedGnssLocationsSize(int32_t &size)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::GetCachedGnssLocations()
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::SendNiUserResponse(int32_t gnssNiNotificationId,
                                              HDI::Location::Gnss::V2_0::GnssNiResponseCmd userResponse)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::SendNetworkInitiatedMsg(const std::string &msg, int32_t length)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::EnableGnssMeasurement(
    const sptr<HDI::Location::Gnss::V2_0::IGnssMeasurementCallback> &callbackObj)
{
    return HDF_SUCCESS;
}

int32_t GnssInterfaceTest::DisableGnssMeasurement()
{
    return HDF_SUCCESS;
}
}  // namespace Location
}  // namespace OHOS
#endif  // FEATURE_GNSS_SUPPORT
