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

#ifdef FEATURE_GNSS_SUPPORT
#ifndef GNSS_EVENT_CALLBACK_H
#define GNSS_EVENT_CALLBACK_H

#include <v1_0/ignss_callback.h>

namespace OHOS {
namespace Location {
using HDI::Location::Gnss::V1_0::IGnssCallback;
using HDI::Location::Gnss::V1_0::LocationInfo;
using HDI::Location::Gnss::V1_0::GnssWorkingStatus;
using HDI::Location::Gnss::V1_0::GnssCapabilities;
using HDI::Location::Gnss::V1_0::SatelliteStatusInfo;
using HDI::Location::Gnss::V1_0::GnssRefInfoType;

class GnssEventCallback : public IGnssCallback {
public:
    ~GnssEventCallback() override {}
    int32_t ReportLocation(const LocationInfo& location) override;
    int32_t ReportGnssWorkingStatus(GnssWorkingStatus status) override;
    int32_t ReportNmea(int64_t timestamp, const std::string& nmea, int32_t length) override;
    int32_t ReportGnssCapabilities(GnssCapabilities capabilities) override;
    int32_t ReportSatelliteStatusInfo(const SatelliteStatusInfo& info) override;
    int32_t RequestGnssReferenceInfo(GnssRefInfoType type) override;
    int32_t RequestPredictGnssData() override;
    int32_t ReportCachedLocation(const std::vector<LocationInfo>& gnssLocations) override;
};
}  // namespace Location
}  // namespace OHOS
#endif // GNSS_EVENT_CALLBACK_H
#endif // FEATURE_GNSS_SUPPORT
