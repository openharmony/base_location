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

#include "gnss_event_callback.h"
#include <singleton.h>
#include "common_utils.h"
#include "gnss_ability.h"
#include "location_log.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
using namespace OHOS::HiviewDFX;

int32_t GnssEventCallback::ReportLocation(const LocationInfo& location)
{
    std::unique_ptr<Location> locationNew = std::make_unique<Location>();
    locationNew->SetLatitude(location.latitude);
    locationNew->SetLongitude(location.longitude);
    locationNew->SetAltitude(location.altitude);
    locationNew->SetAccuracy(location.accuracy);
    locationNew->SetSpeed(location.speed);
    locationNew->SetDirection(location.direction);
    locationNew->SetTimeStamp(location.timeStamp);
    locationNew->SetTimeSinceBoot(location.timeSinceBoot);
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocation(locationNew, GNSS_ABILITY);
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocation(locationNew, PASSIVE_ABILITY);
    return ERR_OK;
}

int32_t GnssEventCallback::ReportGnssWorkingStatus(GnssWorkingStatus status)
{
    DelayedSingleton<GnssAbility>::GetInstance().get()->ReportGnssSessionStatus(static_cast<int>(status));
    return ERR_OK;
}

int32_t GnssEventCallback::ReportNmea(int64_t timestamp, const std::string& nmea, int32_t length)
{
    std::string nmeaStr = nmea;
    DelayedSingleton<GnssAbility>::GetInstance().get()->ReportNmea(nmeaStr);
    return ERR_OK;
}

int32_t GnssEventCallback::ReportGnssCapabilities(GnssCapabilities capabilities)
{
    return ERR_OK;
}

int32_t GnssEventCallback::ReportSatelliteStatusInfo(const SatelliteStatusInfo& info)
{
    std::unique_ptr<SatelliteStatus> svStatus = std::make_unique<SatelliteStatus>();
    if (info.satellitesNumber <= 0) {
        LBSLOGD(GNSS, "SvStatusCallback, satellites_num <= 0!");
        return ERR_INVALID_VALUE;
    }

    svStatus->SetSatellitesNumber(info.satellitesNumber);
    for (unsigned int i = 0; i < info.satellitesNumber; i++) {
        svStatus->SetAltitude(info.elevation[i]);
        svStatus->SetAzimuth(info.azimuths[i]);
        svStatus->SetCarrierFrequencie(info.carrierFrequencies[i]);
        svStatus->SetCarrierToNoiseDensity(info.carrierToNoiseDensitys[i]);
        svStatus->SetSatelliteId(info.satelliteIds[i]);
    }
    DelayedSingleton<GnssAbility>::GetInstance().get()->ReportSv(svStatus);
    return ERR_OK;
}

int32_t GnssEventCallback::RequestGnssReferenceInfo(GnssRefInfoType type)
{
    return ERR_OK;
}

int32_t GnssEventCallback::RequestPredictGnssData()
{
    return ERR_OK;
}

int32_t GnssEventCallback::ReportCachedLocation(const std::vector<LocationInfo>& gnssLocations)
{
    return ERR_OK;
}
}  // namespace Location
}  // namespace OHOS