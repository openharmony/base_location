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
        int nlpRequestType = parcel.ReadInt32();

        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetTimeInterval(timeInterval);
        std::shared_ptr<Request> request = std::make_shared<Request>();
        request->SetUid(uid);
        request->SetPid(pid);
        request->SetPackageName(name);
        request->SetRequestConfig(*requestConfig);
        request->SetUuid(uuid);
        request->SetNlpRequestType(nlpRequestType);
        Add(request);
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
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    parcel.WriteInt32(num_);
    for (int i = 0; i < num_; i++) {
        parcel.WriteInt32(uids_[i]);
        parcel.WriteInt32(pids_[i]);
        parcel.WriteString(names_[i]);
        parcel.WriteInt32(timeInterval_[i]);
        parcel.WriteString(uuid_[i]);
        parcel.WriteInt32(nlpRequestType_[i]);
    }
    parcel.WriteString(deviceId_);
    return true;
}

bool WorkRecord::MarshallingWorkRecord(Parcel& parcel) const
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
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
    std::unique_lock<std::mutex> lock(workRecordMutex_);
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
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    if (index >= 0 && index < num_) {
        return names_[index];
    }
    return "";
}

int WorkRecord::GetUid(int index)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    if (index >= 0 && index < num_) {
        return uids_[index];
    }
    return -1;
}

int WorkRecord::GetPid(int index)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    if (index >= 0 && index < num_) {
        return pids_[index];
    }
    return -1;
}

int WorkRecord::GetTimeInterval(int index)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    if (index >= 0 && index < num_) {
        return timeInterval_[index];
    }
    return -1;
}

std::string WorkRecord::GetUuid(int index)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
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

bool WorkRecord::Add(const std::shared_ptr<Request>& request)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    uids_.push_back(request->GetUid());
    pids_.push_back(request->GetPid());
    names_.push_back(request->GetPackageName());
    timeInterval_.push_back(request->GetRequestConfig()->GetTimeInterval());
    uuid_.push_back(request->GetUuid());
    nlpRequestType_.push_back(request->GetNlpRequestType());
    num_++;
    return true;
}

bool WorkRecord::Add(WorkRecord &workRecord, int32_t index)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    uids_.push_back(workRecord.GetUid(index));
    pids_.push_back(workRecord.GetPid(index));
    names_.push_back(workRecord.GetName(index));
    timeInterval_.push_back(workRecord.GetTimeInterval(index));
    uuid_.push_back(workRecord.GetUuid(index));
    nlpRequestType_.push_back(workRecord.GetNlpRequestType(index));
    num_++;
    return true;
}

bool WorkRecord::Remove(int uid, int pid, std::string name, std::string uuid)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
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
    nlpRequestType_.erase(nlpRequestType_.begin() + i);
    num_--;
    return true;
}

bool WorkRecord::Remove(std::string name)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
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
    nlpRequestType_.erase(nlpRequestType_.begin() + i);
    num_--;
    return true;
}

bool WorkRecord::Find(int uid, std::string name, std::string uuid)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
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
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    uids_.clear();
    pids_.clear();
    names_.clear();
    timeInterval_.clear();
    uuid_.clear();
    nlpRequestType_.clear();
    num_ = 0;
}

void WorkRecord::Set(WorkRecord &workRecord)
{
    Clear();
    int num = workRecord.Size();
    for (int i = 0; i < num; i++) {
        Add(workRecord, i);
    }
}

int WorkRecord::GetNlpRequestType(int index)
{
    std::unique_lock<std::mutex> lock(workRecordMutex_);
    if (index >= 0 && index < num_) {
        return nlpRequestType_[index];
    }
    return -1;
}
} // namespace Location
} // namespace OHOS
