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

#include "report_manager.h"

#include <cmath>

#include "common_utils.h"
#include "fusion_controller.h"
#include "i_locator_callback.h"
#include "location_log.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
const long NANOS_PER_MILLI = 1000000L;
const int SECOND_TO_MILLISECOND = 1000;
const int MAX_SA_SCHEDULING_JITTER_MS = 200;
static constexpr double MAXIMUM_FUZZY_LOCATION_DISTANCE = 4000.0; // Unit m
static constexpr double MINIMUM_FUZZY_LOCATION_DISTANCE = 3000.0; // Unit m
ReportManager::ReportManager()
{
    clock_gettime(CLOCK_REALTIME, &lastUpdateTime_);
    offsetRandom_ = CommonUtils::DoubleRandom(0, 1);
}

ReportManager::~ReportManager() {}

bool ReportManager::OnReportLocation(const std::unique_ptr<Location>& location, std::string abilityName)
{
    LBSLOGI(REPORT_MANAGER, "receive location : %{public}s", abilityName.c_str());
    DelayedSingleton<FusionController>::GetInstance()->FuseResult(abilityName, location);
    SetLastLocation(location);
    LBSLOGI(REPORT_MANAGER, "after SetLastLocation");
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return false;
    }
    auto requestMap = locatorAbility->GetRequests();
    if (requestMap == nullptr) {
        return false;
    }
    auto requestListIter = requestMap->find(abilityName);
    if (requestListIter == requestMap->end()) {
        return false;
    }

    auto requestList = requestListIter->second;
    auto deadRequests = std::make_unique<std::list<std::shared_ptr<Request>>>();
    for (auto iter = requestList.begin(); iter != requestList.end(); iter++) {
        auto request = *iter;
        if (request == nullptr || request->GetRequestConfig() == nullptr ||
            !request->GetIsRequesting()) {
            continue;
        }
        uint32_t tokenId = request->GetTokenId();
        uint32_t firstTokenId = request->GetFirstTokenId();
        std::unique_ptr<Location> finalLocation = GetPermittedLocation(tokenId, firstTokenId, location);

        // if (!ResultCheck(finalLocation, request)) {
        //     continue;
        // }
        request->SetLastLocation(finalLocation);
        auto locatorCallback = request->GetLocatorCallBack();
        if (locatorCallback != nullptr) {
            locatorCallback->OnLocationReport(finalLocation);
        }

        int fixTime = request->GetRequestConfig()->GetFixNumber();
        if (fixTime > 0) {
            deadRequests->push_back(request);
            continue;
        }
    }

    for (auto iter = deadRequests->begin(); iter != deadRequests->end(); ++iter) {
        auto request = *iter;
        if (request == nullptr) {
            continue;
        }
        DelayedSingleton<RequestManager>::GetInstance()->UpdateRequestRecord(request, false);
    }
    locatorAbility->ApplyRequests();
    deadRequests->clear();
    return true;
}

std::unique_ptr<Location> ReportManager::GetPermittedLocation(uint32_t tokenId, uint32_t firstTokenId,
    const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return nullptr;
    }
    std::unique_ptr<Location> finalLocation = std::make_unique<Location>(*location);
    // for api8 and previous version, only ACCESS_LOCATION permission granted also report original location info.
    if (!CommonUtils::CheckLocationPermission(tokenId, firstTokenId) &&
        CommonUtils::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        finalLocation = ApproximatelyLocation(location);
    } else if (!CommonUtils::CheckLocationPermission(tokenId, firstTokenId) &&
        !CommonUtils::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        return nullptr;
    }
    return finalLocation;
}

bool ReportManager::ReportRemoteCallback(sptr<ILocatorCallback>& locatorCallback, int type, int result)
{
    switch (type) {
        case ILocatorCallback::RECEIVE_LOCATION_STATUS_EVENT: {
            locatorCallback->OnLocatingStatusChange(result);
            break;
        }
        case ILocatorCallback::RECEIVE_ERROR_INFO_EVENT: {
            locatorCallback->OnErrorReport(result);
            break;
        }
        default:
            return false;
    }
    return true;
}

bool ReportManager::ResultCheck(const std::unique_ptr<Location>& location,
    const std::shared_ptr<Request>& request)
{
    if (request == nullptr) {
        return false;
    }
    if (location == nullptr) {
        LBSLOGE(REPORT_MANAGER, "%{public}s has no access permission", request->GetPackageName().c_str());
        return false;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return false;
    }
    if (locatorAbility->IsProxyUid(request->GetUid())) {
        LBSLOGD(REPORT_MANAGER, "uid:%{public}d is proxy by freeze, no need to report", request->GetUid());
        return false;
    }
    int permissionLevel = CommonUtils::GetPermissionLevel(request->GetTokenId(), request->GetFirstTokenId());
    if (request->GetLastLocation() == nullptr || request->GetRequestConfig() == nullptr) {
        return true;
    }
    float maxAcc = request->GetRequestConfig()->GetMaxAccuracy();
    LBSLOGD(REPORT_MANAGER, "acc ResultCheck :  %{public}f - %{public}f", maxAcc, location->GetAccuracy());
    if ((permissionLevel == PERMISSION_ACCURATE) &&
        (maxAcc > 0) && (location->GetAccuracy() > maxAcc)) {
        LBSLOGE(REPORT_MANAGER, "accuracy check fail, do not report location");
        return false;
    }
    if (CommonUtils::DoubleEqual(request->GetLastLocation()->GetLatitude(), MIN_LATITUDE - 1)) {
        LBSLOGE(REPORT_MANAGER, "no valid cache location, no need to check");
        return true;
    }
    int minTime = request->GetRequestConfig()->GetTimeInterval();
    long deltaMs = (location->GetTimeSinceBoot() - request->GetLastLocation()->GetTimeSinceBoot()) / NANOS_PER_MILLI;
    LBSLOGD(REPORT_MANAGER, "timeInterval ResultCheck : %{public}s %{public}d - %{public}ld",
        request->GetPackageName().c_str(), minTime, deltaMs);
    if (deltaMs < (minTime * SECOND_TO_MILLISECOND - MAX_SA_SCHEDULING_JITTER_MS)) {
        LBSLOGE(REPORT_MANAGER, "timeInterval check fail, do not report location");
        return false;
    }
    
    int distanceInterval = request->GetRequestConfig()->GetDistanceInterval();
    double deltaDis = CommonUtils::CalDistance(location->GetLatitude(), location->GetLongitude(),
        request->GetLastLocation()->GetLatitude(), request->GetLastLocation()->GetLongitude());
    LBSLOGD(REPORT_MANAGER, "distanceInterval ResultCheck :  %{public}lf - %{public}d", deltaDis, distanceInterval);
    if (deltaDis - distanceInterval < 0) {
        LBSLOGE(REPORT_MANAGER, "distanceInterval check fail, do not report location");
        return false;
    }
    return true;
}

void ReportManager::SetLastLocation(const std::unique_ptr<Location>& location)
{
    lastLocation_.SetLatitude(location->GetLatitude());
    lastLocation_.SetLongitude(location->GetLongitude());
    lastLocation_.SetAltitude(location->GetAltitude());
    lastLocation_.SetAccuracy(location->GetAccuracy());
    lastLocation_.SetSpeed(location->GetSpeed());
    lastLocation_.SetDirection(location->GetDirection());
    lastLocation_.SetTimeStamp(location->GetTimeStamp());
    lastLocation_.SetTimeSinceBoot(location->GetTimeSinceBoot());
}

std::unique_ptr<Location> ReportManager::GetLastLocation()
{
    auto lastLocation = std::make_unique<Location>(lastLocation_);
    if (CommonUtils::DoubleEqual(lastLocation->GetLatitude(), MIN_LATITUDE - 1)) {
        LBSLOGE(REPORT_MANAGER, "no valid cache location");
        return nullptr;
    }
    return lastLocation;
}

void ReportManager::UpdateRandom()
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return;
    }
    int num = locatorAbility->GetActiveRequestNum();
    if (num > 0) {
        LBSLOGE(REPORT_MANAGER, "Exists %{public}d active request, cannot refresh offset", num);
        return;
    }
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    if (abs(now.tv_sec - lastUpdateTime_.tv_sec) > LONG_TIME_INTERVAL) {
        offsetRandom_ = CommonUtils::DoubleRandom(0, 1);
    }
}

std::unique_ptr<Location> ReportManager::ApproximatelyLocation(const std::unique_ptr<Location>& location)
{
    std::unique_ptr<Location> coarseLocation = std::make_unique<Location>(*location);
    double startLat = coarseLocation->GetLatitude();
    double startLon = coarseLocation->GetLongitude();
    double brg = offsetRandom_ * DIS_FROMLL_PARAMETER * M_PI; // 2PI
    double dist = offsetRandom_ * (MAXIMUM_FUZZY_LOCATION_DISTANCE -
        MINIMUM_FUZZY_LOCATION_DISTANCE) + MINIMUM_FUZZY_LOCATION_DISTANCE;
    double perlat = (DIS_FROMLL_PARAMETER * M_PI * EARTH_RADIUS) / DEGREE_DOUBLE_PI; // the radian value of per degree

    double lat = startLat + (dist * sin(brg)) / perlat;
    double lon;
    if (cos(brg) < 0) {
        lon = startLon - (dist * DEGREE_DOUBLE_PI) / (DIS_FROMLL_PARAMETER * M_PI * EARTH_RADIUS);
    } else {
        lon = startLon + (dist * DEGREE_DOUBLE_PI) / (DIS_FROMLL_PARAMETER * M_PI * EARTH_RADIUS);
    }
    if (lat < -MAX_LATITUDE) {
        lat = -MAX_LATITUDE;
    } else if (lat > MAX_LATITUDE) {
        lat = MAX_LATITUDE;
    } else {
        lat = std::round(lat * std::pow(10, 8)) / std::pow(10, 8); // 8 decimal
    }
    if (lon < -MAX_LONGITUDE) {
        lon = -MAX_LONGITUDE;
    } else if (lon > MAX_LONGITUDE) {
        lon = MAX_LONGITUDE;
    } else {
        lon = std::round(lon * std::pow(10, 8)) / std::pow(10, 8); // 8 decimal
    }
    coarseLocation->SetLatitude(lat);
    coarseLocation->SetLongitude(lon);
    coarseLocation->SetAccuracy(DEFAULT_APPROXIMATELY_ACCURACY); // approximately location acc
    return coarseLocation;
}
} // namespace OHOS
} // namespace Location
