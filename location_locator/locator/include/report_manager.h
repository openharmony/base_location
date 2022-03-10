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

#ifndef BASE_REPORT_MANAGER_H
#define BASE_REPORT_MANAGER_H

#include <map>
#include <memory>
#include <list>

#include "iremote_broker.h"

#include "location.h"
#include "request.h"

namespace OHOS {
namespace Location {
class ReportManager {
public:
    bool ReportRemoteCallback(sptr<ILocatorCallback>& locatorCallback, int type, int result);
    bool OnReportLocation(const std::unique_ptr<Location>& location, std::string abilityName);
    bool ReportIntervalCheck(const std::unique_ptr<Location>& location, const std::shared_ptr<Request>& request);
    bool MaxAccuracyCheck(const std::unique_ptr<Location>& location, const std::shared_ptr<Request>& request);
};
} // namespace OHOS
} // namespace Location
#endif // BASE_REPORT_MANAGER_H
