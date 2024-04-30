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
#include "privacy_kit.h"
#include "common_utils.h"
#include "fusion_controller.h"
#include "i_locator_callback.h"
#include "location_log.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#include "location_log_event_ids.h"
#include "common_hisysevent.h"
#include "permission_manager.h"

#include "hook_utils.h"

namespace OHOS {
namespace Location {
const long NANOS_PER_MILLI = 1000000L;
const int MAX_SA_SCHEDULING_JITTER_MS = 200;
static constexpr double MAXIMUM_FUZZY_LOCATION_DISTANCE = 4000.0; // Unit m
static constexpr double MINIMUM_FUZZY_LOCATION_DISTANCE = 3000.0; // Unit m
static constexpr int GNSS_FIX_CACHED_TIME = 60;
static constexpr int NLP_FIX_CACHED_TIME = 45;
ReportManager::ReportManager()
{
    clock_gettime(CLOCK_REALTIME, &lastUpdateTime_);
    offsetRandom_ = CommonUtils::DoubleRandom(0, 1);
}

ReportManager::~ReportManager() {}

bool ReportManager::OnReportLocation(const std::unique_ptr<Location>& location, std::string abilityName)
{
    auto fusionController = DelayedSingleton<FusionController>::GetInstance();
    if (fusionController == nullptr) {
        return false;
    }
    fusionController->FuseResult(abilityName, location);
    UpdateCacheLocation(location, abilityName);
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
    auto fuseLocation = fusionController->GetFuseLocation(abilityName, location);
    for (auto iter = requestList.begin(); iter != requestList.end(); iter++) {
        auto request = *iter;
        WriteNetWorkReportEvent(abilityName, request, location);
        bool ret = true;
        if (IsRequestFuse(request)) {
            ret = ProcessRequestForReport(request, deadRequests, fuseLocation, abilityName);
        } else {
            ret = ProcessRequestForReport(request, deadRequests, location, abilityName);
        }
        if (!ret) {
            continue;
        }
    }
    for (auto iter = deadRequests->begin(); iter != deadRequests->end(); ++iter) {
        auto request = *iter;
        if (request == nullptr) {
            continue;
        }
        auto requestManger = DelayedSingleton<RequestManager>::GetInstance();
        if (requestManger != nullptr) {
            requestManger->UpdateRequestRecord(request, false);
        }
    }
    locatorAbility->ApplyRequests(1);
    deadRequests->clear();
    return true;
}

void ReportManager::UpdateLocationByRequest(const uint32_t tokenId, const uint64_t tokenIdEx,
    std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return;
    }
    if (!PermissionManager::CheckSystemPermission(tokenId, tokenIdEx)) {
        location->SetIsFromMock(-1);
    }
}

bool ReportManager::ProcessRequestForReport(std::shared_ptr<Request>& request,
    std::unique_ptr<std::list<std::shared_ptr<Request>>>& deadRequests,
    const std::unique_ptr<Location>& location, std::string abilityName)
{
    if (request == nullptr || request->GetRequestConfig() == nullptr ||
        !request->GetIsRequesting()) {
        return false;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return false;
    }
    std::unique_ptr<Location> finalLocation = GetPermittedLocation(request->GetUid(),
        request->GetTokenId(), request->GetFirstTokenId(), request->GetTokenIdEx(), location);
    if (!ResultCheck(finalLocation, request)) {
        // add location permission using record
        locatorAbility->UpdatePermissionUsedRecord(request->GetTokenId(), ACCESS_APPROXIMATELY_LOCATION, 0, 1);
        return false;
    }
    UpdateLocationByRequest(request->GetTokenId(), request->GetTokenIdEx(), finalLocation);
    finalLocation = ExecuteReportProcess(request, finalLocation, abilityName);
    if (finalLocation == nullptr) {
        LBSLOGE(REPORT_MANAGER, "%{public}s no need report location", __func__);
        return false;
    }
    request->SetLastLocation(finalLocation);
    auto locatorCallback = request->GetLocatorCallBack();
    if (locatorCallback != nullptr) {
        LBSLOGI(REPORT_MANAGER, "report location to %{public}s, TimeSinceBoot : %{public}s",
            request->GetPackageName().c_str(), std::to_string(finalLocation->GetTimeSinceBoot()).c_str());
        locatorCallback->OnLocationReport(finalLocation);
        // add location permission using record
        locatorAbility->UpdatePermissionUsedRecord(request->GetTokenId(), ACCESS_APPROXIMATELY_LOCATION, 1, 0);
    }

    int fixTime = request->GetRequestConfig()->GetFixNumber();
    if (fixTime > 0) {
        deadRequests->push_back(request);
        return false;
    }
    return true;
}

std::unique_ptr<Location> ReportManager::ExecuteReportProcess(std::shared_ptr<Request>& request,
    std::unique_ptr<Location>& location, std::string abilityName)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.request = *request;
    reportStruct.location = *location;
    reportStruct.abilityName = abilityName;
    reportStruct.retCode = true;
    HookUtils::ExecuteHook(
        LocationProcessStage::LOCATOR_SA_LOCATION_REPORT_PROCESS, (void *)&reportStruct, nullptr);
    if (!reportStruct.retCode) {
        return nullptr;
    }
    return std::make_unique<Location>(reportStruct.location);
}

std::unique_ptr<Location> ReportManager::GetPermittedLocation(pid_t uid, uint32_t tokenId, uint32_t firstTokenId,
    uint64_t tokenIdEx, const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return nullptr;
    }
    std::string bundleName = "";
    if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
        LBSLOGD(REPORT_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
    }
    if (IsAppBackground(bundleName, tokenId, tokenIdEx, uid) &&
        !PermissionManager::CheckBackgroundPermission(tokenId, firstTokenId)) {
        //app background, no background permission, not ContinuousTasks
        return nullptr;
    }
    if (!PermissionManager::CheckLocationPermission(tokenId, firstTokenId) &&
        !PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        LBSLOGE(REPORT_MANAGER, "%{public}d has no location permission failed", tokenId);
        return nullptr;
    }
    std::unique_ptr<Location> finalLocation = std::make_unique<Location>(*location);
    // for api8 and previous version, only ACCESS_LOCATION permission granted also report original location info.
    if (!PermissionManager::CheckLocationPermission(tokenId, firstTokenId) &&
        PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        LBSLOGI(REPORT_MANAGER, "%{public}d has ApproximatelyLocation permission", tokenId);
        finalLocation = ApproximatelyLocation(location);
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
        return false;
    }
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return false;
    }
    if (locatorAbility->IsProxyPid(request->GetPid())) {
        LBSLOGE(REPORT_MANAGER, "pid:%{public}d is proxy by freeze, no need to report", request->GetPid());
        return false;
    }
    int permissionLevel = PermissionManager::GetPermissionLevel(request->GetTokenId(), request->GetFirstTokenId());
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
        LBSLOGD(REPORT_MANAGER, "no valid cache location, no need to check");
        return true;
    }
    int minTime = request->GetRequestConfig()->GetTimeInterval();
    long deltaMs = (location->GetTimeSinceBoot() - request->GetLastLocation()->GetTimeSinceBoot()) / NANOS_PER_MILLI;
    if (deltaMs < (minTime * MILLI_PER_SEC - MAX_SA_SCHEDULING_JITTER_MS)) {
        LBSLOGE(REPORT_MANAGER,
            "%{public}d timeInterval check fail, do not report location, current deltaMs = %{public}ld",
            request->GetTokenId(), deltaMs);
        return false;
    }

    double distanceInterval = request->GetRequestConfig()->GetDistanceInterval();
    double deltaDis = CommonUtils::CalDistance(location->GetLatitude(), location->GetLongitude(),
        request->GetLastLocation()->GetLatitude(), request->GetLastLocation()->GetLongitude());
    if (deltaDis - distanceInterval < 0) {
        LBSLOGE(REPORT_MANAGER, "%{public}d distanceInterval check fail, do not report location",
            request->GetTokenId());
        return false;
    }
    return true;
}

void ReportManager::UpdateCacheLocation(const std::unique_ptr<Location>& location, std::string abilityName)
{
    if (abilityName == GNSS_ABILITY) {
        if (CommonUtils::CheckGnssLocationValidity(location)) {
            cacheGnssLocation_ = *location;
            lastLocation_ = *location;
        }
    } else if (abilityName == NETWORK_ABILITY) {
        cacheNlpLocation_ = *location;
        lastLocation_ = *location;
    } else {
        lastLocation_ = *location;
    }
}

std::unique_ptr<Location> ReportManager::GetLastLocation()
{
    auto lastLocation = std::make_unique<Location>(lastLocation_);
    if (CommonUtils::DoubleEqual(lastLocation->GetLatitude(), MIN_LATITUDE - 1)) {
        LBSLOGE(REPORT_MANAGER, "%{public}s no valid cache location", __func__);
        return nullptr;
    }
    return lastLocation;
}

std::unique_ptr<Location> ReportManager::GetCacheLocation(const std::shared_ptr<Request>& request)
{
    int64_t curTime = CommonUtils::GetCurrentTimeStamp();
    std::unique_ptr<Location> cacheLocation = nullptr;
    if (!CommonUtils::DoubleEqual(cacheGnssLocation_.GetLatitude(), MIN_LATITUDE - 1) &&
        (curTime - cacheGnssLocation_.GetTimeStamp() / MILLI_PER_SEC) <= GNSS_FIX_CACHED_TIME) {
        cacheLocation = std::make_unique<Location>(cacheGnssLocation_);
    } else if (!CommonUtils::DoubleEqual(cacheNlpLocation_.GetLatitude(), MIN_LATITUDE - 1) &&
        (curTime - cacheNlpLocation_.GetTimeStamp() / MILLI_PER_SEC) <= NLP_FIX_CACHED_TIME) {
        cacheLocation = std::make_unique<Location>(cacheNlpLocation_);
    }
    std::unique_ptr<Location> finalLocation = GetPermittedLocation(request->GetUid(),
        request->GetTokenId(), request->GetFirstTokenId(), request->GetTokenIdEx(), cacheLocation);
    if (!ResultCheck(finalLocation, request)) {
        return nullptr;
    }
    UpdateLocationByRequest(request->GetTokenId(), request->GetTokenIdEx(), finalLocation);
    return finalLocation;
}

void ReportManager::UpdateRandom()
{
    auto locatorAbility = DelayedSingleton<LocatorAbility>::GetInstance();
    if (locatorAbility == nullptr) {
        return;
    }
    int num = locatorAbility->GetActiveRequestNum();
    if (num > 0) {
        LBSLOGD(REPORT_MANAGER, "Exists %{public}d active request, cannot refresh offset", num);
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
    std::vector<std::string> emptyAdds;
    coarseLocation->SetAdditions(emptyAdds, false);
    coarseLocation->SetAdditionSize(0);
    return coarseLocation;
}

bool ReportManager::IsRequestFuse(const std::shared_ptr<Request>& request)
{
    if (request == nullptr || request->GetRequestConfig() == nullptr) {
        return false;
    }
    if (request->GetRequestConfig()->GetScenario() == SCENE_UNSET &&
        request->GetRequestConfig()->GetPriority() == PRIORITY_FAST_FIRST_FIX) {
        return true;
    }
    return false;
}

void ReportManager::WriteNetWorkReportEvent(std::string abilityName, const std::shared_ptr<Request>& request,
    const std::unique_ptr<Location>& location)
{
    if (abilityName == NETWORK_ABILITY && request != nullptr) {
        WriteLocationInnerEvent(RECEIVE_NETWORK_LOCATION, {
            "PackageName", request->GetPackageName(),
            "abilityName", abilityName,
            "requestAddress", request->GetUuid(),
            "latitude", std::to_string(location->GetLatitude()),
            "longitude", std::to_string(location->GetLongitude()),
            "accuracy", std::to_string(location->GetAccuracy())
        });
    }
}

bool ReportManager::IsAppBackground(std::string bundleName, uint32_t tokenId, uint64_t tokenIdEx, int32_t uid)
{
    auto locatorBackgroundProxy = DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
    if (locatorBackgroundProxy == nullptr) {
        return false;
    }
    if (!locatorBackgroundProxy->IsAppBackground(bundleName)) {
        return false;
    }
    if (locatorBackgroundProxy->IsAppHasFormVisible(tokenId, tokenIdEx)) {
        return false;
    }
    if (locatorBackgroundProxy->IsAppInLocationContinuousTasks(uid)) {
        return false;
    }
    return true;
}
} // namespace OHOS
} // namespace Location
