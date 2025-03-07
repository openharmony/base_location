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
#include "location_data_rdb_manager.h"
#include "common_hisysevent.h"
#include "permission_manager.h"
#include "hook_utils.h"

namespace OHOS {
namespace Location {
const long NANOS_PER_MILLI = 1000000L;
const int MAX_SA_SCHEDULING_JITTER_MS = 200;
static constexpr double MAXIMUM_FUZZY_LOCATION_DISTANCE = 40.0; // Unit m
static constexpr double MINIMUM_FUZZY_LOCATION_DISTANCE = 30.0; // Unit m
static constexpr int CACHED_TIME = 25;
static constexpr int LONG_CACHE_DURATION = 60;
static constexpr int MAX_LOCATION_REPORT_DELAY_TIME = 30000; // Unit ms
static constexpr int MIN_RESET_TIME_THRESHOLD = 1 * 60 * 60 * 1000; // Unit ms

ReportManager* ReportManager::GetInstance()
{
    static ReportManager data;
    return &data;
}

ReportManager::ReportManager()
{
    clock_gettime(CLOCK_REALTIME, &lastUpdateTime_);
    offsetRandom_ = CommonUtils::DoubleRandom(0, 1);
    lastResetRecordTime_ = CommonUtils::GetSinceBootTime();
}

ReportManager::~ReportManager() {}

bool ReportManager::OnReportLocation(const std::unique_ptr<Location>& location, std::string abilityName)
{
    auto fusionController = FusionController::GetInstance();
    UpdateCacheLocation(location, abilityName);
    auto locatorAbility = LocatorAbility::GetInstance();
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
        WriteNetWorkReportEvent(abilityName, request, location);
        if (abilityName == NETWORK_ABILITY) {
            if (request->GetUuid() == location->GetUuid() || location->GetIsFromMock()) {
                ProcessRequestForReport(request, deadRequests, location, abilityName);
                break;
            }
        } else if (abilityName == GNSS_ABILITY || abilityName == PASSIVE_ABILITY) {
            ProcessRequestForReport(request, deadRequests, location, abilityName);
        }
    }
    for (auto iter = deadRequests->begin(); iter != deadRequests->end(); ++iter) {
        auto request = *iter;
        if (request == nullptr) {
            continue;
        }
        auto requestManger = RequestManager::GetInstance();
        if (requestManger != nullptr) {
            requestManger->UpdateRequestRecord(request, false);
            requestManger->UpdateUsingPermission(request, false);
        }
    }
    if (deadRequests->size() > 0) {
        locatorAbility->ApplyRequests(1);
        deadRequests->clear();
    }
    return true;
}

void ReportManager::UpdateLocationByRequest(const uint32_t tokenId, const uint64_t tokenIdEx,
    std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return;
    }
    if (!PermissionManager::CheckSystemPermission(tokenId, tokenIdEx)) {
        location->SetIsSystemApp(0);
    } else {
        location->SetIsSystemApp(1);
    }
}

bool ReportManager::ProcessRequestForReport(std::shared_ptr<Request>& request,
    std::unique_ptr<std::list<std::shared_ptr<Request>>>& deadRequests,
    const std::unique_ptr<Location>& location, std::string abilityName)
{
    if (location == nullptr ||
        request == nullptr || request->GetRequestConfig() == nullptr || !request->GetIsRequesting()) {
        return false;
    }
    std::unique_ptr<Location> fuseLocation;
    std::unique_ptr<Location> finalLocation;
    if (IsRequestFuse(request)) {
        if (request->GetBestLocation() == nullptr ||
            request->GetBestLocation()->GetLocationSourceType() == 0) {
            request->SetBestLocation(std::make_unique<Location>(cacheGnssLocation_));
        }
        fuseLocation = FusionController::GetInstance()->GetFuseLocation(location, request->GetBestLocation());
        if (request->GetLastLocation() != nullptr && request->GetLastLocation()->LocationEqual(fuseLocation)) {
            return false;
        }
        request->SetBestLocation(fuseLocation);
    }
    if (LocationDataRdbManager::QuerySwitchState() != ENABLED &&
        !LocatorAbility::GetInstance()->GetLocationSwitchIgnoredFlag(request->GetTokenId())) {
        LBSLOGE(REPORT_MANAGER, "QuerySwitchState is DISABLED");
        return false;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
    finalLocation = GetPermittedLocation(request, IsRequestFuse(request) ? fuseLocation : location);
    if (!ResultCheck(finalLocation, request)) {
        // add location permission using record
        int permUsedType = request->GetPermUsedType();
        locatorAbility->UpdatePermissionUsedRecord(request->GetTokenId(),
            ACCESS_APPROXIMATELY_LOCATION, permUsedType, 0, 1);
        return false;
    }
    LocationReportDelayTimeCheck(finalLocation, request);
    UpdateLocationByRequest(request->GetTokenId(), request->GetTokenIdEx(), finalLocation);
    finalLocation = ExecuteReportProcess(request, finalLocation, abilityName);
    if (finalLocation == nullptr) {
        LBSLOGE(REPORT_MANAGER, "%{public}s no need report location", __func__);
        return false;
    }
    request->SetLastLocation(finalLocation);
    if (!ReportLocationByCallback(request, finalLocation)) {
        return false;
    }
    int fixTime = request->GetRequestConfig()->GetFixNumber();
    if (fixTime > 0 && !request->GetRequestConfig()->IsRequestForAccuracy()) {
        deadRequests->push_back(request);
        return false;
    }
    return true;
}

bool ReportManager::ReportLocationByCallback(std::shared_ptr<Request>& request,
    const std::unique_ptr<Location>& finalLocation)
{
    auto locatorCallback = request->GetLocatorCallBack();
    if (locatorCallback != nullptr) {
        // add location permission using record
        auto locatorAbility = LocatorAbility::GetInstance();
        int ret = locatorAbility->UpdatePermissionUsedRecord(request->GetTokenId(),
            ACCESS_APPROXIMATELY_LOCATION, request->GetPermUsedType(), 1, 0);
        if (ret != ERRCODE_SUCCESS && locatorAbility->IsHapCaller(request->GetTokenId())) {
            LBSLOGE(REPORT_MANAGER, "UpdatePermissionUsedRecord failed ret=%{public}d", ret);
            RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_LOCATION_PERMISSION_DENIED, request);
            return false;
        }
        LBSLOGW(REPORT_MANAGER, "report location to %{public}d, uuid : %{public}s, " \
            "TimeSinceBoot : %{public}s, SourceType : %{public}d",
            request->GetTokenId(), request->GetUuid().c_str(),
            std::to_string(finalLocation->GetTimeSinceBoot()).c_str(), finalLocation->GetLocationSourceType());
        locatorCallback->OnLocationReport(finalLocation);
        RequestManager::GetInstance()->UpdateLocationError(request);
    }
    return true;
}

void ReportManager::LocationReportDelayTimeCheck(const std::unique_ptr<Location>& location,
    const std::shared_ptr<Request>& request)
{
    if (location == nullptr || request == nullptr) {
        return;
    }
    int64_t currentTime = CommonUtils::GetSinceBootTime();
    long deltaMs = (currentTime - location->GetTimeSinceBoot()) / NANOS_PER_MILLI;
    if (deltaMs > MAX_LOCATION_REPORT_DELAY_TIME) {
        long recordDeltaMs = (currentTime - lastResetRecordTime_) / NANOS_PER_MILLI;
        if (recordDeltaMs < MIN_RESET_TIME_THRESHOLD) {
            return;
        }
        LBSLOGE(REPORT_MANAGER, "%{public}s: %{public}d check fail, current deltaMs = %{public}ld", __func__,
            request->GetTokenId(), deltaMs);
        lastResetRecordTime_ = currentTime;
        _exit(0);
    }
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

std::unique_ptr<Location> ReportManager::ExecuteLocationProcess(const std::shared_ptr<Request>& request,
    const std::unique_ptr<Location>& location)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.request = *request;
    reportStruct.location = *location;
    HookUtils::ExecuteHook(
        LocationProcessStage::LOCATION_REPORT_PROCESS, (void *)&reportStruct, nullptr);
    return std::make_unique<Location>(reportStruct.location);
}

std::unique_ptr<Location> ReportManager::GetPermittedLocation(const std::shared_ptr<Request>& request,
    const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return nullptr;
    }
    std::string bundleName = request->GetPackageName();
    auto tokenId = request->GetTokenId();
    auto firstTokenId = request->GetFirstTokenId();
    auto tokenIdEx = request->GetTokenIdEx();
    auto uid =  request->GetUid();
    if (bundleName.length() == 0) {
        if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
            LBSLOGD(REPORT_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
        }
    }
    if (request->GetRequestConfig()->GetFixNumber() == 0 &&
        IsAppBackground(bundleName, tokenId, tokenIdEx, uid, request->GetPid()) &&
        !PermissionManager::CheckBackgroundPermission(tokenId, firstTokenId)) {
        //app background, no background permission, not ContinuousTasks
        RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_BACKGROUND_PERMISSION_DENIED, request);
        return nullptr;
    }
    AppIdentity identity;
    identity.SetUid(request->GetUid());
    identity.SetTokenId(request->GetTokenId());
    identity.SetBundleName(bundleName);
    int currentUserId = LocatorBackgroundProxy::GetInstance()->getCurrentUserId();
    if (!CommonUtils::IsAppBelongCurrentAccount(identity, currentUserId)) {
        //app is not in current user, not need to report
        LBSLOGI(REPORT_MANAGER, "GetPermittedLocation uid: %{public}d CheckAppForUser fail", tokenId);
        auto locationErrorCallback = request->GetLocationErrorCallBack();
        if (locationErrorCallback != nullptr) {
            locationErrorCallback->OnErrorReport(LOCATING_FAILED_LOCATION_PERMISSION_DENIED);
        }
        return nullptr;
    }
    std::unique_ptr<Location> finalLocation = ExecuteLocationProcess(request, location);
    // for api8 and previous version, only ACCESS_LOCATION permission granted also report original location info.
    if (PermissionManager::CheckLocationPermission(tokenId, firstTokenId)) {
        return finalLocation;
    }
    if (PermissionManager::CheckApproximatelyPermission(tokenId, firstTokenId)) {
        LBSLOGI(REPORT_MANAGER, "%{public}d has ApproximatelyLocation permission", tokenId);
        finalLocation = ApproximatelyLocation(location);
        return finalLocation;
    }
    LBSLOGE(REPORT_MANAGER, "%{public}d has no location permission failed", tokenId);
    RequestManager::GetInstance()->ReportLocationError(LOCATING_FAILED_LOCATION_PERMISSION_DENIED, request);
    return nullptr;
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
    if (request == nullptr || location == nullptr) {
        return false;
    }
    auto locatorAbility = LocatorAbility::GetInstance();
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
        auto locatorCallback = request->GetLocatorCallBack();
        if (locatorCallback != nullptr) {
            ReportRemoteCallback(locatorCallback, ILocatorCallback::RECEIVE_ERROR_INFO_EVENT,
                LocationErrCode::ERRCODE_LOCATING_ACC_FAIL);
        } else {
            LBSLOGE(REPORT_MANAGER, "ReportManager null LocatorCallback");
        }
        LBSLOGE(REPORT_MANAGER, "accuracy check fail, do not report location");
        return false;
    }
    if (CommonUtils::DoubleEqual(request->GetLastLocation()->GetLatitude(), MIN_LATITUDE - 1) ||
        request->GetLastLocation()->GetIsFromMock() != location->GetIsFromMock()) {
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
        if (HookUtils::CheckGnssLocationValidity(location)) {
            cacheGnssLocation_ = *location;
            UpdateLastLocation(location);
        }
    } else if (abilityName == NETWORK_ABILITY) {
        cacheNlpLocation_ = *location;
        if (location->GetLocationSourceType() != LocationSourceType::INDOOR_TYPE) {
            UpdateLastLocation(location);
        }
    } else {
        UpdateLastLocation(location);
    }
}

void ReportManager::UpdateLastLocation(const std::unique_ptr<Location>& location)
{
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    int currentUserId = locatorBackgroundProxy->getCurrentUserId();
    std::unique_lock<std::mutex> lock(lastLocationMutex_);
    lastLocationsMap_[currentUserId] = std::make_shared<Location>(*location);
}

std::unique_ptr<Location> ReportManager::GetLastLocation()
{
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    int currentUserId = locatorBackgroundProxy->getCurrentUserId();
    LBSLOGI(LOCATOR_STANDARD, "GetCacheLocation GetLastLocation currentUserId = %{public}d ", currentUserId);
    std::unique_lock<std::mutex> lock(lastLocationMutex_);
    auto iter = lastLocationsMap_.find(currentUserId);
    if (iter == lastLocationsMap_.end()) {
        return nullptr;
    }
    std::unique_ptr<Location> lastLocation = std::make_unique<Location>(*(iter->second));
    if (CommonUtils::DoubleEqual(lastLocation->GetLatitude(), MIN_LATITUDE - 1)) {
        return nullptr;
    }
    return lastLocation;
}

std::unique_ptr<Location> ReportManager::GetCacheLocation(const std::shared_ptr<Request>& request)
{
    int64_t curTime = CommonUtils::GetCurrentTimeStamp();
    std::unique_ptr<Location> cacheLocation = nullptr;
    std::string packageName = request->GetPackageName();
    int cachedTime = 0;
    bool indoorFlag = false;
    if (HookUtils::ExecuteHookReportManagerGetCacheLocation(packageName, indoorFlag)) {
        cachedTime = LONG_CACHE_DURATION;
    } else {
        cachedTime = CACHED_TIME;
    }
    if (!indoorFlag && !CommonUtils::DoubleEqual(cacheGnssLocation_.GetLatitude(), MIN_LATITUDE - 1) &&
        (curTime - cacheGnssLocation_.GetTimeStamp() / MILLI_PER_SEC) <= cachedTime) {
        cacheLocation = std::make_unique<Location>(cacheGnssLocation_);
    } else if (!CommonUtils::DoubleEqual(cacheNlpLocation_.GetLatitude(), MIN_LATITUDE - 1) &&
        (curTime - cacheNlpLocation_.GetTimeStamp() / MILLI_PER_SEC) <= cachedTime) {
        cacheLocation = std::make_unique<Location>(cacheNlpLocation_);
    }
    if (indoorFlag && cacheLocation != nullptr && cacheLocation->GetLocationSourceType() == NETWORK_TYPE) {
        return nullptr;
    }
    if (!indoorFlag && cacheLocation->GetLocationSourceType() == INDOOR_TYPE) {
        auto additionMap = cacheLocation->GetAdditionMap();
        std::vector<std::string> emptyAdds;
        std::map<std::string, std::string> emptyMap;
        auto iter = additionMap.find("requestId");
        if (iter != additionMap.end()) {
            emptyMap["requestId"] = additionMap["requestId"];
            emptyAdds.push_back("requestId:" + additionMap["requestId"]);
        }
        auto iter = additionMap.find("inHdArea");
        if (iter != additionMap.end()) {
            emptyMap["inHdArea"] = additionMap["inHdArea"];
            emptyAdds.push_back("inHdArea:" + additionMap["inHdArea"]);
        }
        coarseLocation->SetAdditions(emptyAdds, false);
        coarseLocation->SetAdditionSize(emptyAdds.size());
        cacheLocation->SetAdditionsMap(emptyMap);
    }
    std::unique_ptr<Location> finalLocation = GetPermittedLocation(request, cacheLocation);
    if (!ResultCheck(finalLocation, request)) {
        return nullptr;
    }
    UpdateLocationByRequest(request->GetTokenId(), request->GetTokenIdEx(), finalLocation);
    return finalLocation;
}

void ReportManager::UpdateRandom()
{
    auto locatorAbility = LocatorAbility::GetInstance();
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
    if (request->GetRequestConfig()->GetFixNumber() == 1 && request->GetRequestConfig()->IsRequestForAccuracy()) {
        return false;
    }
    if ((request->GetRequestConfig()->GetScenario() == SCENE_UNSET &&
        request->GetRequestConfig()->GetPriority() == PRIORITY_LOW_POWER) ||
        request->GetRequestConfig()->GetScenario() == SCENE_NO_POWER ||
        request->GetRequestConfig()->GetScenario() == SCENE_DAILY_LIFE_SERVICE ||
        request->GetRequestConfig()->GetScenario() == LOCATION_SCENE_DAILY_LIFE_SERVICE ||
        request->GetRequestConfig()->GetScenario() == LOCATION_SCENE_LOW_POWER_CONSUMPTION ||
        request->GetRequestConfig()->GetScenario() == LOCATION_SCENE_NO_POWER_CONSUMPTION) {
        return false;
    }
    return true;
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

bool ReportManager::IsAppBackground(std::string bundleName, uint32_t tokenId, uint64_t tokenIdEx, pid_t uid, pid_t pid)
{
    auto locatorBackgroundProxy = LocatorBackgroundProxy::GetInstance();
    if (!locatorBackgroundProxy->IsAppBackground(uid, bundleName)) {
        return false;
    }
    if (locatorBackgroundProxy->IsAppHasFormVisible(tokenId, tokenIdEx)) {
        return false;
    }
    if (locatorBackgroundProxy->IsAppInLocationContinuousTasks(uid, pid)) {
        return false;
    }
    return true;
}

bool ReportManager::IsCacheGnssLocationValid()
{
    int64_t curTime = CommonUtils::GetCurrentTimeStamp();
    if (!CommonUtils::DoubleEqual(cacheGnssLocation_.GetLatitude(), MIN_LATITUDE - 1) &&
        (curTime - cacheGnssLocation_.GetTimeStamp() / MILLI_PER_SEC) <= CACHED_TIME) {
        return true;
    }
    return false;
}
} // namespace OHOS
} // namespace Location
