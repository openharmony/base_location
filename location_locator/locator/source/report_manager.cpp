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

#include "fusion_controller.h"
#include "i_locator_callback.h"
#include "lbs_log.h"
#include "locator_ability.h"

namespace OHOS {
namespace Location {
const long NANOS_PER_MILLI = 1000000L;
const int SECOND_TO_MILLISECOND = 1000;
const int MAX_PROVIDER_SCHEDULING_JITTER_MS = 200;
bool ReportManager::OnReportLocation(const std::unique_ptr<Location>& location, std::string abilityName)
{
    LBSLOGI(REPORT_MANAGER, "receive location : %{public}s - %{private}s",
        abilityName.c_str(), location->ToString().c_str());
    DelayedSingleton<FusionController>::GetInstance()->FuseResult(abilityName, location);
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
    if (deadRequests == nullptr) {
        return false;
    }
    for (auto iter = requestList.begin(); iter != requestList.end(); iter++) {
        auto request = *iter;
        if (request == nullptr || request->GetRequestConfig() == nullptr ||
            !request->GetIsRequesting()) {
            continue;
        }
        if (!ReportIntervalCheck(location, request) ||
            !MaxAccuracyCheck(location, request)) {
            continue;
        }
        request->SetLastLocation(location);
        auto locatorCallback = request->GetLocatorCallBack();
        if (locatorCallback != nullptr) {
            locatorCallback->OnLocationReport(location);
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

bool ReportManager::MaxAccuracyCheck(const std::unique_ptr<Location>& location,
    const std::shared_ptr<Request>& request)
{
    if (request == nullptr || request->GetRequestConfig() == nullptr) {
        return true;
    }
    float maxAcc = request->GetRequestConfig()->GetMaxAccuracy();
    LBSLOGD(REPORT_MANAGER, "MaxAccuracyCheck : maxAcc:%{public}f , location.acc:%{public}f",
        maxAcc, location->GetAccuracy());
    if (location->GetAccuracy() > maxAcc) {
        LBSLOGD(REPORT_MANAGER, "location->GetAccuracy is too big,do not report location");
        return false;
    }
    return true;
}

bool ReportManager::ReportIntervalCheck(const std::unique_ptr<Location>& location,
    const std::shared_ptr<Request>& request)
{
    if (request == nullptr || request->GetLastLocation() == nullptr) {
        return true;
    }
    int minTime = request->GetRequestConfig()->GetTimeInterval();
    long deltaMs = (location->GetTimeSinceBoot() - request->GetLastLocation()->GetTimeSinceBoot()) / NANOS_PER_MILLI;
    LBSLOGD(REPORT_MANAGER, "ReportIntervalCheck : %{public}s %{public}d - %{public}ld",
        request->GetPackageName().c_str(), minTime, deltaMs);
    if (deltaMs < (minTime * SECOND_TO_MILLISECOND - MAX_PROVIDER_SCHEDULING_JITTER_MS)) {
        LBSLOGD(REPORT_MANAGER, "do not report location");
        return false;
    }
    return true;
}
} // namespace OHOS
} // namespace Location
