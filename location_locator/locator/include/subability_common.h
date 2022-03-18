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

#ifndef OHOS_LOCATION_SUBABILITY_COMMON_H
#define OHOS_LOCATION_SUBABILITY_COMMON_H

#include <dlfcn.h>
#include <map>
#include <unistd.h>

#include "adapter_callback_skeleton.h"
#include "common_hisysevent.h"
#include "iremote_broker.h"
#include "lbs_log.h"
#include "work_record.h"

#define HIGNSS_ADAPTER_PATH "/system/lib/libgnss.default.so"

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
    };
    virtual void SendLocationRequest(uint64_t interval, WorkRecord &workrecord) = 0;
    virtual std::unique_ptr<Location> GetCachedLocation() = 0;
    virtual void SetEnable(bool state) = 0;
};

class SubAbility {
public:
    void SetAbility(std::string name);
    void LocationRequest(uint64_t interval, WorkRecord &workrecord);
    std::unique_ptr<Location> GetCache();
    void Enable(bool state, const sptr<IRemoteObject> ability);
    void HandleSelfRequest(pid_t pid, pid_t uid, bool state);
    void HandleRemoteRequest(bool state, std::string deviceId);
    void HandleRefrashRequirements();

private:
    void HandleLocalRequest(WorkRecord &record);
    void HandleRemoveRecord(WorkRecord &record);
    void HandleAddRecord(WorkRecord &record);
    void RequestRecord(sptr<LocationCallbackStub> addCallback, WorkRecord &workRecord, bool isAdded);
    sptr<LocationCallbackStub> GetCallback(int uid);
    void WriteCallbackToParcel(sptr<LocationCallbackStub> callback, MessageParcel &data);
    void WriteInfoToParcel(WorkRecord &workRecord, MessageParcel &data);
    bool ParseReplyInfo(MessageParcel &rep);

    OHOS::HiviewDFX::HiLogLabel label_;
    sptr<IRemoteObject> ability_;
    sptr<LocationCallbackStub> callback_;
    uint64_t interval_ = 0;
    std::string name_;
    std::u16string capability_ = u"";
    std::unique_ptr<WorkRecord> lastRecord_;
    std::unique_ptr<WorkRecord> newRecord_;
    std::unique_ptr<std::map<int, sptr<LocationCallbackStub>>> requestMap_;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_SUBABILITY_COMMON_H