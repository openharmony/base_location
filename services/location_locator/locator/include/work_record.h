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

#ifndef WORK_RECORD_H
#define WORK_RECORD_H

#include <mutex>
#include <parcel.h>
#include <string>
#include <vector>

#include "request.h"

namespace OHOS {
namespace Location {
class WorkRecord : public Parcelable {
public:
    WorkRecord();
    ~WorkRecord() override = default;
    bool Add(const std::shared_ptr<Request>& request);
    bool Add(WorkRecord &workRecord, int32_t index);
    bool Remove(int uid, int pid, std::string name, std::string uuid);
    bool Find(int uid, std::string name, std::string uuid);
    void Clear();
    void Set(WorkRecord &workRecord);
    bool Remove(std::string name);
    std::string GetDeviceId();
    void SetDeviceId(std::string deviceId);
    int Size();
    bool IsEmpty();
    std::string GetName(int index);
    int GetUid(int index);
    int GetPid(int index);
    int GetTimeInterval(int index);
    int GetNlpRequestType(int index);
    std::string GetUuid(int index);
    void ReadFromParcel(Parcel& parcel);
    bool Marshalling(Parcel& parcel) const override;
    bool MarshallingWorkRecord(Parcel& parcel) const;
    static std::unique_ptr<WorkRecord> Unmarshalling(Parcel& parcel);
    std::string ToString();
private:
    int num_;
    std::vector<int> uids_;
    std::vector<int> pids_;
    std::vector<std::string> names_;
    std::vector<int> timeInterval_;
    std::vector<std::string> uuid_;
    std::vector<int> nlpRequestType_;
    std::string deviceId_;
    mutable std::mutex workRecordMutex_;
};
} // namespace Location
} // namespace OHOS
#endif // WORK_RECORD_H
