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

#include "work_record.h"

#include "string_ex.h"

namespace OHOS {
namespace Location {
const int MAX_RECORD_COUNT = 100;

WorkRecord::WorkRecord()
{
    num_ = 0;
}

void WorkRecord::ReadFromParcel(Parcel& parcel)
{
    int num = parcel.ReadInt32();
    if (num > MAX_RECORD_COUNT) {
        num = MAX_RECORD_COUNT;
    }
    for (int i = 0; i < num; i++) {
        int uid = parcel.ReadInt32();
        int pid = parcel.ReadInt32();
        std::string name = parcel.ReadString();
        int timeInterval = parcel.ReadInt32();
        std::string uuid = parcel.ReadString();
        Add(uid, pid, name, timeInterval, uuid);
    }
    deviceId_ = parcel.ReadString();
}

std::unique_ptr<WorkRecord> WorkRecord::Unmarshalling(Parcel& parcel)
{
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    workRecord->ReadFromParcel(parcel);
    return workRecord;
}

bool WorkRecord::Marshalling(Parcel& parcel) const
{
    parcel.WriteInt32(num_);
    for (int i = 0; i < num_; i++) {
        parcel.WriteInt32(uids_[i]);
        parcel.WriteInt32(pids_[i]);
        parcel.WriteString(names_[i]);
        parcel.WriteInt32(timeInterval_[i]);
        parcel.WriteString(uuid_[i]);
    }
    parcel.WriteString(deviceId_);
    return true;
}

bool WorkRecord::MarshallingWorkRecord(Parcel& parcel) const
{
    // write numbers
    parcel.WriteInt32(num_);
    // write uids
    parcel.WriteInt32(num_);
    for (int i = 0; i < num_; i++) {
        parcel.WriteInt32(uids_[i]);
    }
    // write names
    parcel.WriteInt32(num_);
    for (int i = 0; i < num_; i++) {
        parcel.WriteString16(Str8ToStr16(names_[i]));
    }
    return true;
}

std::string WorkRecord::ToString()
{
    std::string result = "[";
    if (!IsEmpty()) {
        for (int i = 0; i < num_; i++) {
            result += std::to_string(uids_[i]);
            result += ",";
            result += std::to_string(pids_[i]);
            result += ",";
            result += names_[i];
            result += ",";
            result += std::to_string(timeInterval_[i]);
            result += ",";
            result += uuid_[i];
            result += "; ";
        }
    }
    result += "]";
    return result;
}

std::string WorkRecord::GetName(int index)
{
    if (index >= 0 && index < num_) {
        return names_[index];
    }
    return "";
}

int WorkRecord::GetUid(int index)
{
    if (index >= 0 && index < num_) {
        return uids_[index];
    }
    return -1;
}

int WorkRecord::GetPid(int index)
{
    if (index >= 0 && index < num_) {
        return pids_[index];
    }
    return -1;
}

int WorkRecord::GetTimeInterval(int index)
{
    if (index >= 0 && index < num_) {
        return timeInterval_[index];
    }
    return -1;
}

std::string WorkRecord::GetUuid(int index)
{
    if (index >= 0 && index < num_) {
        return uuid_[index];
    }
    return "";
}

void WorkRecord::SetDeviceId(std::string deviceId)
{
    deviceId_ = deviceId;
}

std::string WorkRecord::GetDeviceId()
{
    return deviceId_;
}

int WorkRecord::Size()
{
    return num_;
}

bool WorkRecord::IsEmpty()
{
    if (num_ == 0) {
        return true;
    }
    return false;
}

bool WorkRecord::Add(int uid, int pid, std::string name, int timeInterval, std::string uuid)
{
    uids_.push_back(uid);
    pids_.push_back(pid);
    names_.push_back(name);
    timeInterval_.push_back(timeInterval);
    uuid_.push_back(uuid);
    num_++;
    return true;
}

bool WorkRecord::Remove(int uid, int pid, std::string name, std::string uuid)
{
    if (uids_.size() <= 0) {
        return false;
    }
    unsigned int i = 0;
    for (auto iterUid = uids_.begin(); iterUid != uids_.end(); iterUid++, i++) {
        if (*iterUid == uid) {
            if ((name.compare(names_[i]) == 0) && (uuid.compare(uuid_[i]) == 0)) {
                break;
            }
        }
    }
    if (uids_.size() - i == 0) {
        return false;
    }
    uids_.erase(uids_.begin() + i);
    pids_.erase(pids_.begin() + i);
    names_.erase(names_.begin() + i);
    timeInterval_.erase(timeInterval_.begin() + i);
    uuid_.erase(uuid_.begin() + i);
    num_--;
    return true;
}

bool WorkRecord::Remove(std::string name)
{
    if (uids_.size() <= 0) {
        return false;
    }
    unsigned int i = 0;
    for (auto iter = names_.begin(); iter != names_.end(); iter++, i++) {
        if (iter->compare(name) == 0) {
            break;
        }
    }
    if (names_.size() - i == 0) {
        return false;
    }
    uids_.erase(uids_.begin() + i);
    pids_.erase(pids_.begin() + i);
    names_.erase(names_.begin() + i);
    timeInterval_.erase(timeInterval_.begin() + i);
    uuid_.erase(uuid_.begin() + i);
    num_--;
    return true;
}

bool WorkRecord::Find(int uid, std::string name, std::string uuid)
{
    if (uids_.size() <= 0) {
        return false;
    }
    int i = 0;
    for (auto iterUid = uids_.begin(); iterUid != uids_.end(); iterUid++, i++) {
        if (*iterUid == uid) {
            if ((name.compare(names_[i]) == 0) && (uuid.compare(uuid_[i]) == 0)) {
                return true;
            }
        }
    }
    return false;
}

void WorkRecord::Clear()
{
    uids_.clear();
    pids_.clear();
    names_.clear();
    timeInterval_.clear();
    uuid_.clear();
    num_ = 0;
}

void WorkRecord::Set(WorkRecord &workRecord)
{
    Clear();
    int num = workRecord.Size();
    for (int i = 0; i < num; i++) {
        int uid = workRecord.GetUid(i);
        int pid = workRecord.GetPid(i);
        std::string name = workRecord.GetName(i);
        int timeInterval = workRecord.GetTimeInterval(i);
        std::string uuid = workRecord.GetUuid(i);
        Add(uid, pid, name, timeInterval, uuid);
    }
}
} // namespace Location
} // namespace OHOS
