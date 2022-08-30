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

#include <map>
#include <memory>
#include <list>
#include <singleton.h>
#include <time.h>
#include "iremote_broker.h"
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
    void SetLastLocation(const std::unique_ptr<Location>& location);
    std::unique_ptr<Location> GetLastLocation();
    std::unique_ptr<Location> GetPermittedLocation(uint32_t tokenId, uint32_t firstTokenId,
        const std::unique_ptr<Location>& location);
    void UpdateRandom();
private:
    struct timespec lastUpdateTime_;
    double offsetRandom_;
    Location lastLocation_;
    std::unique_ptr<Location> ApproximatelyLocation(const std::unique_ptr<Location>& location);
};
} // namespace OHOS
} // namespace Location
#endif // REPORT_MANAGER_H
