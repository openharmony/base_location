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
#include "gnss_event_callback.h"
#include <singleton.h>
#include "ipc_skeleton.h"
#include "common_utils.h"
#include "gnss_ability.h"
#include "location_log.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
using namespace OHOS::HiviewDFX;

int32_t GnssEventCallback::ReportLocation(const LocationInfo& location)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (gnssAbility == nullptr ||locatorAbility == nullptr) {
        LBSLOGE(GNSS, "ReportLocation: gnss ability or locator ability is nullptr.");
        return ERR_OK;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::unique_ptr<Location> locationNew = std::make_unique<Location>();
    locationNew->SetLatitude(location.latitude);
    locationNew->SetLongitude(location.longitude);
    locationNew->SetAltitude(location.altitude);
    locationNew->SetAccuracy(location.accuracy);
    locationNew->SetSpeed(location.speed);
    locationNew->SetDirection(location.direction);
    locationNew->SetTimeStamp(location.timeStamp);
    locationNew->SetTimeSinceBoot(location.timeSinceBoot);
    locationNew->SetIsFromMock(false);
    if (gnssAbility->IsMockEnabled()) {
        LBSLOGE(GNSS, "location mock is enabled, do not report gnss location!");
        IPCSkeleton::SetCallingIdentity(identity);
        return ERR_OK;
    }
    locatorAbility.get()->ReportLocation(locationNew, GNSS_ABILITY);
#ifdef FEATURE_PASSIVE_SUPPORT
    locatorAbility.get()->ReportLocation(locationNew, PASSIVE_ABILITY);
#endif
    IPCSkeleton::SetCallingIdentity(identity);
    return ERR_OK;
}

int32_t GnssEventCallback::ReportGnssWorkingStatus(GnssWorkingStatus status)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ReportGnssWorkingStatus: gnss ability is nullptr.");
        return ERR_OK;
    }
    gnssAbility.get()->ReportGnssSessionStatus(static_cast<int>(status));
    return ERR_OK;
}

int32_t GnssEventCallback::ReportNmea(int64_t timestamp, const std::string& nmea, int32_t length)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ReportNmea: gnss ability is nullptr.");
        return ERR_OK;
    }
    std::string nmeaStr = nmea;
    gnssAbility.get()->ReportNmea(timestamp, nmeaStr);
    return ERR_OK;
}

int32_t GnssEventCallback::ReportGnssCapabilities(GnssCapabilities capabilities)
{
    return ERR_OK;
}

int32_t GnssEventCallback::ReportSatelliteStatusInfo(const SatelliteStatusInfo& info)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ReportSatelliteStatusInfo: gnss ability is nullptr.");
        return ERR_OK;
    }
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
    gnssAbility.get()->ReportSv(svStatus);
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
#endif
