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

namespace OHOS {
namespace Location {
class ReportManager : public DelayedSingleton<ReportManager>  {
public:
    ReportManager();
    ~ReportManager();
    bool ReportRemoteCallback(sptr<ILocatorCallback>& locatorCallback, int type, int result);
    bool OnReportLocation(const std::unique_ptr<Location>& location, std::string abilityName);
    bool ResultCheck(const std::unique_ptr<Location>& location, const std::shared_ptr<Request>& request);
    void UpdateCacheLocation(const std::unique_ptr<Location>& location, std::string abilityName);
    std::unique_ptr<Location> GetLastLocation();
    std::unique_ptr<Location> GetCacheLocation(const std::shared_ptr<Request>& request);
    std::unique_ptr<Location> GetPermittedLocation(pid_t uid, uint32_t tokenId, uint32_t firstTokenId,
        uint32_t tokenIdEx, const std::unique_ptr<Location>& location);
    void UpdateRandom();
    bool IsRequestFuse(const std::shared_ptr<Request>& request);
    void UpdateLocationByRequest(const uint32_t tokenId, const uint32_t tokenIdEx,
        std::unique_ptr<Location>& location);
    bool IsAppBackground(std::string bundleName, uint32_t tokenId, uint32_t tokenIdEx, int32_t uid);
private:
    struct timespec lastUpdateTime_;
    double offsetRandom_;
    Location lastLocation_;
    Location cacheGnssLocation_;
    Location cacheNlpLocation_;
    std::unique_ptr<Location> ApproximatelyLocation(const std::unique_ptr<Location>& location);
    bool ProcessRequestForReport(std::shared_ptr<Request>& request,
        std::unique_ptr<std::list<std::shared_ptr<Request>>>& deadRequests,
        const std::unique_ptr<Location>& location, std::string abilityName);
    void WriteNetWorkReportEvent(std::string abilityName, const std::shared_ptr<Request>& request,
        const std::unique_ptr<Location>& location);
    std::unique_ptr<Location> ExecuteReportProcess(std::shared_ptr<Request>& request,
        std::unique_ptr<Location>& location, std::string abilityName);
};
} // namespace OHOS
} // namespace Location
#endif // REPORT_MANAGER_H
