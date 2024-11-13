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
#include <sys/time.h>
#include "ipc_skeleton.h"
#include "common_utils.h"
#include "gnss_ability.h"
#include "location_log.h"
#include "location_log_event_ids.h"
#include "common_hisysevent.h"
#include "agnss_ni_manager.h"

#ifdef TIME_SERVICE_ENABLE
#include "ntp_time_check.h"
#include "time_service_client.h"
#endif
namespace OHOS {
namespace Location {
using namespace OHOS::HiviewDFX;
const int WEAK_GPS_SIGNAL_SCENARIO_COUNT = 3;
const int MAX_SV_COUNT = 64;
const int GPS_DUMMY_SV_COUNT = 5;
const int AZIMUTH_DEGREES = 60;
const int ELEVATION_DEGREES = 90;
const int SATELLITES_ADDITIONAL = 4;
bool g_hasLocation = false;
bool g_svIncrease = false;
std::unique_ptr<SatelliteStatus> g_svInfo = nullptr;

static void SetGpsTime(int64_t gpsTime)
{
#ifdef TIME_SERVICE_ENABLE
    auto ntpTimeCheck = NtpTimeCheck::GetInstance();
    if (ntpTimeCheck != nullptr) {
        auto elapsedRealTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
        ntpTimeCheck->SetGpsTime(gpsTime, elapsedRealTime);
    }
#endif
}

int32_t GnssEventCallback::ReportLocation(const LocationInfo& location)
{
    auto gnssAbility = GnssAbility::GetInstance();
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::shared_ptr<Location> locationNew = std::make_shared<Location>();
    locationNew->SetLatitude(location.latitude);
    locationNew->SetLongitude(location.longitude);
    locationNew->SetAltitude(location.altitude);
    locationNew->SetAccuracy(location.horizontalAccuracy);
    locationNew->SetSpeed(location.speed);
    locationNew->SetDirection(location.bearing);
    locationNew->SetAltitudeAccuracy(location.verticalAccuracy);
    locationNew->SetSpeedAccuracy(location.speedAccuracy);
    locationNew->SetDirectionAccuracy(location.bearingAccuracy);
    locationNew->SetTimeStamp(location.timeForFix);
    locationNew->SetTimeSinceBoot(location.timeSinceBoot);
    locationNew->SetUncertaintyOfTimeSinceBoot(location.timeUncertainty);
    locationNew->SetIsFromMock(false);
    locationNew->SetLocationSourceType(LocationSourceType::GNSS_TYPE);
    locationNew->SetFieldValidity(location.fieldValidity);
    if (gnssAbility->IsMockEnabled()) {
        LBSLOGE(GNSS, "location mock is enabled, do not report gnss location!");
        IPCSkeleton::SetCallingIdentity(identity);
        return ERR_OK;
    }
    // add dummy sv if needed
    SendDummySvInfo();
    struct timeval now;
    gettimeofday(&now, NULL);
    WriteLocationInnerEvent(RECEIVE_GNSS_LOCATION, {
        "speed", std::to_string(location.speed),
        "accuracy", std::to_string(location.horizontalAccuracy),
        "locationTimestamp", std::to_string(location.timeForFix / MILLI_PER_SEC),
        "receiveTimestamp", std::to_string(CommonUtils::GetCurrentTimeStamp()),
        "latitude", std::to_string(location.latitude),
        "longitude", std::to_string(location.longitude)});
    gnssAbility->ReportLocationInfo(GNSS_ABILITY, locationNew);
    SetGpsTime(locationNew->GetTimeStamp());
    IPCSkeleton::SetCallingIdentity(identity);
    return ERR_OK;
}

int32_t GnssEventCallback::ReportGnssWorkingStatus(GnssWorkingStatus status)
{
    auto gnssAbility = GnssAbility::GetInstance();
    gnssAbility->ReportGnssSessionStatus(static_cast<int>(status));
    return ERR_OK;
}

int32_t GnssEventCallback::ReportNmea(int64_t timestamp, const std::string& nmea, int32_t length)
{
    auto gnssAbility = GnssAbility::GetInstance();
    std::string nmeaStr = nmea;
    gnssAbility->ReportNmea(timestamp, nmeaStr);
    return ERR_OK;
}

int32_t GnssEventCallback::ReportGnssCapabilities(unsigned int capabilities)
{
    return ERR_OK;
}

int32_t GnssEventCallback::ReportSatelliteStatusInfo(const SatelliteStatusInfo& info)
{
    auto gnssAbility = GnssAbility::GetInstance();
    std::unique_ptr<SatelliteStatus> svStatus = std::make_unique<SatelliteStatus>();
    if (info.satellitesNumber < 0) {
        LBSLOGD(GNSS, "SvStatusCallback, satellites_num < 0!");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> names;
    std::vector<std::string> satelliteStatusInfos;
    names.push_back("SatelliteStatusInfo");
    satelliteStatusInfos.push_back(std::to_string(info.satellitesNumber));

    svStatus->SetSatellitesNumber(info.satellitesNumber);
    for (unsigned int i = 0; i < info.satellitesNumber; i++) {
        svStatus->SetAltitude(info.elevation[i]);
        svStatus->SetAzimuth(info.azimuths[i]);
        svStatus->SetCarrierFrequencie(info.carrierFrequencies[i]);
        svStatus->SetCarrierToNoiseDensity(info.carrierToNoiseDensitys[i]);
        svStatus->SetSatelliteId(info.satelliteIds[i]);
        svStatus->SetConstellationType(info.constellation[i]);
        svStatus->SetSatelliteAdditionalInfo(info.additionalInfo[i]);
        std::string str_info = "satelliteId : " + std::to_string(info.satelliteIds[i]) +
            ", carrierToNoiseDensity : " + std::to_string(info.carrierToNoiseDensitys[i]) +
            ", elevation : " + std::to_string(info.elevation[i]) +
            ", azimuth : " + std::to_string(info.azimuths[i]) +
            ", carrierFrequencie : " + std::to_string(info.carrierFrequencies[i]);
        names.push_back(std::to_string(i));
        satelliteStatusInfos.push_back(str_info);
    }
    // save sv info
    std::unique_lock<std::mutex> lock(svInfoMutex_, std::defer_lock);
    lock.lock();
    g_svInfo = nullptr;
    g_svInfo = std::make_unique<SatelliteStatus>(*svStatus);
    lock.unlock();
    WriteLocationInnerEvent(RECEIVE_SATELLITESTATUSINFO, names, satelliteStatusInfos);
    gnssAbility->ReportSv(svStatus);
    return ERR_OK;
}

void GnssEventCallback::SendDummySvInfo()
{
    std::unique_lock<std::mutex> lock(svInfoMutex_);
    if (g_svInfo == nullptr) {
        LBSLOGE(GNSS, "%{public}s: sv is nullptr.", __func__);
        return;
    }
    // indicates location is coming
    g_hasLocation = true;
    int usedSvCount = 0;
    int svListSize = g_svInfo->GetSatellitesNumber();
    // calculate the num of used GPS satellites
    for (int svSize = 0; svSize < svListSize; svSize++) {
        if (IsSvTypeGps(g_svInfo, svSize) && IsSvUsed(g_svInfo, svSize)) {
            usedSvCount++;
        }
    }
    LBSLOGD(GNSS, "%{public}s: the USED GPS SV Count is %{public}d", __func__, usedSvCount);
    // weak gps signal scenario
    if (usedSvCount <= WEAK_GPS_SIGNAL_SCENARIO_COUNT) {
        // indicates the need for dummy satellites
        g_svIncrease = true;
        LBSLOGD(GNSS, "%{public}s: start increase dummy sv", __func__);

        if (MAX_SV_COUNT - svListSize >= GPS_DUMMY_SV_COUNT) {
            AddDummySv(g_svInfo, 4, 6); // sv1: svid = 4, cN0Dbhz = 6
            AddDummySv(g_svInfo, 7, 15); // sv2: svid = 7, cN0Dbhz = 15
            AddDummySv(g_svInfo, 1, 2); // sv3: svid = 1, cN0Dbhz = 2
            AddDummySv(g_svInfo, 11, 10); // sv4: svid = 11, cN0Dbhz = 10
            AddDummySv(g_svInfo, 17, 5); // sv5: svid = 17, cN0Dbhz = 5
            g_svInfo->
                SetSatellitesNumber(g_svInfo->GetSatellitesNumber() + GPS_DUMMY_SV_COUNT);
            ReportDummySv(g_svInfo);
        } else {
            LBSLOGD(GNSS, "%{public}s: sv number > 58, no need send dummy satellites", __func__);
        }
        LBSLOGD(GNSS, "%{public}s: increase sv finished", __func__);
    } else {
        // indicates no need for dummy satellites
        g_svIncrease = false;
    }
}

void GnssEventCallback::ReportDummySv(const std::unique_ptr<SatelliteStatus> &sv)
{
    auto gnssAbility = GnssAbility::GetInstance();
    if (sv == nullptr) {
        LBSLOGE(GNSS, "%{public}s sv is nullptr.", __func__);
        return;
    }
    gnssAbility->ReportSv(sv);
}

bool GnssEventCallback::IsNeedSvIncrease()
{
    if (g_hasLocation && g_svIncrease) {
        return true;
    }
    return false;
}

bool GnssEventCallback::IsSvTypeGps(const std::unique_ptr<SatelliteStatus> &sv, int index)
{
    if (sv == nullptr) {
        return false;
    }
    return sv->GetConstellationTypes()[index] == HDI::Location::Gnss::V2_0::CONSTELLATION_CATEGORY_GPS;
}

bool GnssEventCallback::IsSvUsed(const std::unique_ptr<SatelliteStatus> &sv, int index)
{
    if (sv == nullptr) {
        return false;
    }
    return static_cast<uint32_t>(sv->GetSatelliteAdditionalInfoList()[index]) &
        static_cast<uint8_t>(HDI::Location::Gnss::V2_0::SATELLITES_ADDITIONAL_INFO_USED_IN_FIX);
}

void GnssEventCallback::AddDummySv(std::unique_ptr<SatelliteStatus> &sv, int svid, int cN0Dbhz)
{
    if (sv == nullptr) {
        return;
    }
    sv->SetSatelliteId(svid);
    sv->SetConstellationType(HDI::Location::Gnss::V2_0::CONSTELLATION_CATEGORY_GPS);
    sv->SetCarrierToNoiseDensity(cN0Dbhz);
    sv->SetAltitude(ELEVATION_DEGREES); // elevationDegrees
    sv->SetAzimuth(AZIMUTH_DEGREES); // azimuthDegrees
    sv->SetCarrierFrequencie(0); // carrierFrequencyHz
    sv->SetSatelliteAdditionalInfo(SATELLITES_ADDITIONAL); // satellites_additional
}

int32_t GnssEventCallback::RequestGnssReferenceInfo(GnssRefInfoType type)
{
    LBSLOGI(GNSS, "RequestGnssReferenceInfo: request type %{public}d", static_cast<int>(type));
    auto gnssAbility = GnssAbility::GetInstance();
    switch (type) {
        case GnssRefInfoType::GNSS_REF_INFO_TIME:
            gnssAbility->InjectTime();
            break;
        case GnssRefInfoType::GNSS_REF_INFO_LOCATION:
            gnssAbility->InjectLocation();
            break;
        default:
            LBSLOGI(GNSS, "RequestGnssReferenceInfo: request type not support now");
            break;
    }
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

int32_t GnssEventCallback::ReportGnssNiNotification(const GnssNiNotificationRequest& notification)
{
    auto agnssNiManager = AGnssNiManager::GetInstance();
    agnssNiManager->HandleNiNotification(notification);
    return ERR_OK;
}
}  // namespace Location
}  // namespace OHOS
#endif
