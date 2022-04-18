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
#include <unistd.h>
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
    requestMap_ = std::make_unique<std::map<int, sptr<LocationCallbackStub>>>();
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
            sptr<LocationCallbackStub> removeCallback = GetCallback(uid);
            if (removeCallback != nullptr) {
                std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
                workRecord->Add(uid, lastRecord_->GetPid(i), lastRecord_->GetName(i));
                workRecord->SetDeviceId(newRecord.GetDeviceId());
                RequestRecord(removeCallback, *workRecord, false);
            }
            requestMap_->erase(uid);
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
            sptr<LocationCallbackStub> addCallback = new (std::nothrow) LocationCallbackStub(name_);
            std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
            if (addCallback == nullptr || workRecord == nullptr) {
                continue;
            }
            workRecord->Add(uid, newRecord.GetPid(i), newRecord.GetName(i));
            workRecord->SetDeviceId(newRecord.GetDeviceId());
            RequestRecord(addCallback, *workRecord, true);
            requestMap_->insert(std::pair<int, sptr<LocationCallbackStub>>(uid, addCallback));
        }
    }
}

sptr<LocationCallbackStub> SubAbility::GetCallback(int uid)
{
    auto iter = requestMap_->find(uid);
    return (iter != requestMap_->end()) ? iter->second : nullptr;
}

std::unique_ptr<Location> SubAbility::GetCache()
{
    return nullptr;
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

void SubAbility::WriteCallbackToParcel(sptr<LocationCallbackStub> callback, MessageParcel &data)
{
    if (callback == nullptr) {
        return;
    }
    // generate object of ILocationListener
    data.WriteObject<IRemoteObject>(callback->AsObject());
}

void SubAbility::WriteInfoToParcel(WorkRecord &workRecord, MessageParcel &data)
{
    // generate package name
    std::string info = "zlocation";
    info += ":";
    info += std::to_string(workRecord.GetPid(0));
    info += ":";
    info += std::to_string(workRecord.GetUid(0));
    info += ":";
    info += workRecord.GetName(0);
    data.WriteString16(Str8ToStr16(info));
}

bool SubAbility::ParseReplyInfo(MessageParcel &data)
{
    int exceptionHeader = data.ReadInt32();
    LBSLOGD(label_, "get exception reply header:%{public}d", exceptionHeader);
    if (exceptionHeader != REPLY_NO_EXCEPTION) {
        std::string mgs = Str16ToStr8(data.ReadString16());
        LBSLOGE(label_, "some exceptions happened in next level service. message:%{public}s", mgs.c_str());
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS
