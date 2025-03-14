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

#ifndef SUBABILITY_COMMON_H
#define SUBABILITY_COMMON_H

#include <vector>

#include "iremote_broker.h"

#include "constant_definition.h"
#include "i_locator_callback.h"
#include "location.h"
#include "hilog/log.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class ISubAbility : public IRemoteBroker {
public:
    virtual LocationErrCode SendLocationRequest(WorkRecord &workrecord) = 0;
    virtual LocationErrCode SetEnable(bool state) = 0;
    virtual LocationErrCode EnableMock() = 0;
    virtual LocationErrCode DisableMock() = 0;
    virtual LocationErrCode SetMocked(const int timeInterval,
        const std::vector<std::shared_ptr<Location>> &location) = 0;
};

class SubAbility {
public:
    SubAbility();
    virtual ~SubAbility();
    void SetAbility(std::string name);
    void LocationRequest(WorkRecord &workrecord);
    void Enable(bool state, const sptr<IRemoteObject> ability);
    void HandleRefrashRequirements();
    int GetRequestNum();
    bool EnableLocationMock();
    bool DisableLocationMock();
    bool SetMockedLocations(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);

    int GetTimeIntervalMock();
    bool IsLocationMocked();
    std::vector<std::shared_ptr<Location>> GetLocationMock();
    void ClearLocationMock();
    void ReportLocationInfo(const std::string& systemAbility, const std::shared_ptr<Location> location);
    void ReportLocationError(int32_t errCode, std::string errMsg, std::string uuid);
    void StopAllLocationRequests();
    void RestartAllLocationRequests();
private:
    void HandleLocalRequest(WorkRecord &record);
    void HandleRemoveRecord(WorkRecord &newRecord);
    void HandleAddRecord(WorkRecord &newRecord);
    void CacheLocationMock(const std::vector<std::shared_ptr<Location>> &location);
    virtual void RequestRecord(WorkRecord &workRecord, bool isAdded) = 0;
    virtual void SendReportMockLocationEvent() = 0;

    OHOS::HiviewDFX::HiLogLabel label_;
    int interval_ = 0;
    std::string name_ = "";
    std::u16string capability_ = u"";
    std::unique_ptr<WorkRecord> lastRecord_;
    std::unique_ptr<WorkRecord> newRecord_;
    int mockTimeInterval_ = 0;
    bool mockEnabled_ = false;
    std::vector<std::shared_ptr<Location>> mockLoc_;
    std::mutex mutex_;
};
} // namespace Location
} // namespace OHOS
#endif // SUBABILITY_COMMON_H
