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

#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include <singleton.h>
#include <string>
#include <time.h>
#include <map>

#include "i_locator_callback.h"
#include "location.h"
#include "request.h"
#include <mutex>

namespace OHOS {
namespace Location {

class ReportManager {
public:
    ReportManager();
    ~ReportManager();
    bool ReportRemoteCallback(const sptr<ILocatorCallback>& locatorCallback, int type, int result);
    bool OnReportLocation(const std::unique_ptr<Location>& location, std::string abilityName);
    bool ResultCheck(const std::unique_ptr<Location>& location, const std::shared_ptr<Request>& request);
    void UpdateCacheLocation(const std::unique_ptr<Location>& location, std::string abilityName);
    std::unique_ptr<Location> GetLastLocation();
    std::unique_ptr<Location> GetCacheLocation(const std::shared_ptr<Request>& request);
    std::unique_ptr<Location> GetPermittedLocation(const std::shared_ptr<Request>& request,
        const std::unique_ptr<Location>& location);
    void UpdateRandom();
    bool IsRequestFuse(const std::shared_ptr<Request>& request);
    void UpdateLocationByRequest(const uint32_t tokenId, const uint64_t tokenIdEx,
        std::unique_ptr<Location>& location);
    bool IsAppBackground(std::string bundleName, uint32_t tokenId, uint64_t tokenIdEx, pid_t uid, pid_t pid);
    static ReportManager* GetInstance();
    bool IsCacheGnssLocationValid();

private:
    struct timespec lastUpdateTime_;
    double offsetRandom_;
    std::map<int, std::shared_ptr<Location>> lastLocationsMap_;
    Location cacheGnssLocation_;
    Location cacheNlpLocation_;
    std::mutex lastLocationMutex_;
    std::mutex cacheGnssLocationMutex_;
    std::mutex cacheNlpLocationMutex_;
    std::atomic<int64_t> lastResetRecordTime_;
    std::unique_ptr<Location> ApproximatelyLocation(const std::unique_ptr<Location>& location,
        const std::shared_ptr<Request>& request);
    bool ProcessRequestForReport(std::shared_ptr<Request>& request,
        std::unique_ptr<std::list<std::shared_ptr<Request>>>& deadRequests,
        const std::unique_ptr<Location>& location, std::string abilityName);
    bool ReportLocationByCallback(std::shared_ptr<Request>& request,
        const std::unique_ptr<Location>& finalLocation);
    void WriteNetWorkReportEvent(std::string abilityName, const std::shared_ptr<Request>& request,
        const std::unique_ptr<Location>& location);
    std::unique_ptr<Location> ExecuteReportProcess(std::shared_ptr<Request>& request,
        std::unique_ptr<Location>& location, std::string abilityName);
    std::unique_ptr<Location> ExecuteLocationProcess(const std::shared_ptr<Request>& request,
        const std::unique_ptr<Location>& location);
    void UpdateLastLocation(const std::unique_ptr<Location>& location);
    void LocationReportDelayTimeCheck(const std::unique_ptr<Location>& location,
        const std::shared_ptr<Request>& request);
    bool NeedUpdateTimeStamp(std::unique_ptr<Location>& fuseLocation, const std::shared_ptr<Request>& request);
    void UpdateCacheGnssLocation(Location& location);
    void UpdateCacheNlpLocation(Location& location);
    Location& GetCacheGnssLocation();
    Location& GetCacheNlpLocation();
};
} // namespace OHOS
} // namespace Location
#endif // REPORT_MANAGER_H
