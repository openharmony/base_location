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

#include "i_locator_callback.h"
#include "location.h"
#include "hilog/log.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class ISubAbility : public IRemoteBroker {
public:
    enum {
        SEND_LOCATION_REQUEST = 1,
        GET_CACHED_LOCATION = 2,
        SET_ENABLE = 3,
        SELF_REQUEST = 4,
        HANDLE_REMOTE_REQUEST = 5,
        REFRESH_REQUESTS = 6,
        REG_GNSS_STATUS = 7,
        UNREG_GNSS_STATUS = 8,
        REG_NMEA = 9,
        UNREG_NMEA = 10,
        REG_CACHED = 11,
        UNREG_CACHED = 12,
        GET_CACHED_SIZE = 13,
        FLUSH_CACHED = 14,
        SEND_COMMANDS = 15,
        ADD_FENCE_INFO = 16,
        REMOVE_FENCE_INFO = 17,
        REPORT_GNSS_SESSION_STATUS = 18,
        REPORT_SV = 19,
        REPORT_NMEA = 20,
        GET_ISO_COUNTRY_CODE = 21,
        ENABLE_LOCATION_MOCK = 22,
        DISABLE_LOCATION_MOCK = 23,
        SET_MOCKED_LOCATIONS = 24,
        ENABLE_REV_GEOCODE_MOCK = 25,
        DISABLE_REV_GEOCODE_MOCK = 26,
    };
    virtual void SendLocationRequest(uint64_t interval, WorkRecord &workrecord) = 0;
    virtual void SetEnable(bool state) = 0;
    virtual bool EnableMock() = 0;
    virtual bool DisableMock() = 0;
    virtual bool SetMocked(const int timeInterval,
        const std::vector<std::shared_ptr<Location>> &location) = 0;
};

class SubAbility {
public:
    SubAbility();
    virtual ~SubAbility();
    void SetAbility(std::string name);
    void LocationRequest(uint64_t interval, WorkRecord &workrecord);
    void Enable(bool state, const sptr<IRemoteObject> ability);
    void HandleSelfRequest(pid_t pid, pid_t uid, bool state);
    void HandleRefrashRequirements();
    int GetRequestNum();
    bool EnableLocationMock();
    bool DisableLocationMock();
    bool SetMockedLocations(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location);

    int GetTimeIntervalMock();
    bool IsLocationMocked();
    std::vector<std::shared_ptr<Location>> GetLocationMock();
    void ClearLocationMock();
private:
    void HandleLocalRequest(WorkRecord &record);
    void HandleRemoveRecord(WorkRecord &newRecord);
    void HandleAddRecord(WorkRecord &newRecord);
    void CacheLocationMock(const std::vector<std::shared_ptr<Location>> &location);
    virtual void RequestRecord(WorkRecord &workRecord, bool isAdded) = 0;
    virtual void SendReportMockLocationEvent() = 0;

    OHOS::HiviewDFX::HiLogLabel label_;
    uint64_t interval_ = 0;
    std::string name_ = "";
    std::u16string capability_ = u"";
    std::unique_ptr<WorkRecord> lastRecord_;
    std::unique_ptr<WorkRecord> newRecord_;
    int mockTimeInterval_ = 0;
    bool mockEnabled_ = false;
    std::vector<std::shared_ptr<Location>> mockLoc_;
};
} // namespace Location
} // namespace OHOS
#endif // SUBABILITY_COMMON_H
