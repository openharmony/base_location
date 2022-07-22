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

#include "subability_common.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "common_utils.h"
#include "gnss_ability_proxy.h"
#include "locator_ability.h"
#include "string_ex.h"

namespace OHOS {
namespace Location {
SubAbility::SubAbility()
{
}

SubAbility::~SubAbility()
{
}

void SubAbility::SetAbility(std::string name)
{
    name_ = name;
    label_ = CommonUtils::GetLabel(name);
    capability_ = CommonUtils::GetCapability(name);
    newRecord_ = std::make_unique<WorkRecord>();
    lastRecord_ = std::make_unique<WorkRecord>();
}

void SubAbility::LocationRequest(uint64_t interval, WorkRecord &workRecord)
{
    interval_ = interval;
    newRecord_->Clear();
    newRecord_->Set(workRecord);
    HandleRefrashRequirements();
}

void SubAbility::HandleRefrashRequirements()
{
    LBSLOGI(label_, "refrash requirements");

    // send local request
    HandleLocalRequest(*newRecord_);
    lastRecord_->Clear();
    lastRecord_->Set(*newRecord_);
}

int SubAbility::GetRequestNum()
{
    if (newRecord_ == nullptr) {
        return 0;
    }
    return newRecord_->Size();
}

void SubAbility::HandleLocalRequest(WorkRecord &record)
{
    HandleRemoveRecord(record);
    HandleAddRecord(record);
}

void SubAbility::HandleRemoveRecord(WorkRecord &newRecord)
{
    for (int i = 0; i < lastRecord_->Size(); i++) {
        int uid = lastRecord_->GetUid(i);
        bool isFind = newRecord.Find(uid, lastRecord_->GetName(i));
        LBSLOGD(label_, "remove record isFind:%{public}d, uid:%{public}d, lastRecord:%{public}s, newRecord:%{public}s",
            isFind, uid, lastRecord_->ToString().c_str(), newRecord.ToString().c_str());
        if (!isFind) {
            std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
            workRecord->Add(uid, lastRecord_->GetPid(i), lastRecord_->GetName(i));
            workRecord->SetDeviceId(newRecord.GetDeviceId());
            RequestRecord(*workRecord, false);
        }
    }
}

void SubAbility::HandleAddRecord(WorkRecord &newRecord)
{
    for (int i = 0; i < newRecord.Size(); i++) {
        int uid = newRecord.GetUid(i);
        bool isFind = lastRecord_->Find(uid, newRecord.GetName(i));
        LBSLOGD(label_, "add record isFind:%{public}d, uid:%{public}d, lastRecord:%{public}s, newRecord:%{public}s",
            isFind, uid, lastRecord_->ToString().c_str(), newRecord.ToString().c_str());
        if (!isFind) {
            std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
            if (workRecord == nullptr) {
                continue;
            }
            workRecord->Add(uid, newRecord.GetPid(i), newRecord.GetName(i));
            workRecord->SetDeviceId(newRecord.GetDeviceId());
            RequestRecord(*workRecord, true);
        }
    }
}

void SubAbility::Enable(bool state, const sptr<IRemoteObject> ability)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        LBSLOGE(label_, "Enable can not get SystemAbilityManager");
        return;
    }

    int saId = CommonUtils::AbilityConvertToId(name_);
    if (state) {
        if (sam->CheckSystemAbility(saId) == nullptr) {
            sam->AddSystemAbility(saId, ability, ISystemAbilityManager::SAExtraProp(true, 1, capability_, u""));
            LBSLOGI(label_, "enable %{public}s ability", name_.c_str());
        }
    } else {
        if (sam->CheckSystemAbility(saId) != nullptr) {
            sam->RemoveSystemAbility(saId);
            LBSLOGI(label_, "disable %{public}s ability", name_.c_str());
        }
    }
}

void SubAbility::HandleSelfRequest(pid_t pid, pid_t uid, bool state)
{
    std::unique_ptr<WorkRecord> records = std::make_unique<WorkRecord>();
    std::string name = Str16ToStr8(u"ohos");
    records->Set(*lastRecord_);
    if (state) {
        records->Add(uid, pid, name);
    } else {
        records->Remove(uid, pid, name);
    }
    LocationRequest(interval_, *records);
    records->Clear();
}

void SubAbility::HandleRemoteRequest(bool state, std::string deviceId)
{
    HandleRefrashRequirements();
}

bool SubAbility::EnableLocationMock(const LocationMockConfig& config)
{
    LBSLOGI(label_, "EnableLocationMock current state is %{public}d", mockEnabled_);
    mockEnabled_ = true;
    return true;
}

bool SubAbility::DisableLocationMock(const LocationMockConfig& config)
{
    LBSLOGI(label_, "DisableLocationMock current state is %{public}d", mockEnabled_);
    mockEnabled_ = false;
    return true;
}

bool SubAbility::SetMockedLocations(const LocationMockConfig& config,
    const std::vector<std::shared_ptr<Location>> &location)
{
    if (!mockEnabled_) {
        LBSLOGE(label_, "SetMockedLocations current state is %{public}d, need enbale it", mockEnabled_);
        return false;
    }
    CacheLocationMock(location);
    mockTimeInterval_ = config.GetTimeInterval();
    SendReportMockLocationEvent();
    return true;
}

void SubAbility::CacheLocationMock(const std::vector<std::shared_ptr<Location>> &location)
{
    int locationSize = location.size();
    ClearLocationMock();
    for (int i = 0; i < locationSize; i++) {
        mockLoc_.push_back(std::make_shared<Location>(*location.at(i)));
    }
}

bool SubAbility::IsLocationMocked()
{
    return mockEnabled_;
}

int SubAbility::GetTimeIntervalMock()
{
    return mockTimeInterval_;
}

std::vector<std::shared_ptr<Location>> SubAbility::GetLocationMock()
{
    return mockLoc_;
}

void SubAbility::ClearLocationMock()
{
    mockLoc_.clear();
}
} // namespace Location
} // namespace OHOS
