/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "hook_utils.h"
#include "location_log.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
static HOOK_MGR* locatorHookMgr_ = nullptr;

HOOK_MGR* HookUtils::GetLocationExtHookMgr()
{
    if (locatorHookMgr_ != nullptr) {
        return locatorHookMgr_;
    }

    locatorHookMgr_ = HookMgrCreate("locationHookMgr");
    return locatorHookMgr_;
}

LocationErrCode HookUtils::RegisterHook(LocationProcessStage stage, int prio, OhosHook hook)
{
    auto ret = HookMgrAdd(GetLocationExtHookMgr(), static_cast<int>(stage), prio, hook);
    if (ret == 0) {
        return ERRCODE_SUCCESS;
    }
    LBSLOGE(LOCATOR, "%{public}s stage = %{public}d add failed ret = %{public}d",
        __func__, static_cast<int>(stage), ret);
    return ERRCODE_SERVICE_UNAVAILABLE;
}

void HookUtils::UnregisterHook(LocationProcessStage stage, OhosHook hook)
{
    HookMgrDel(GetLocationExtHookMgr(), static_cast<int>(stage), hook);
}

LocationErrCode HookUtils::ExecuteHook(
    LocationProcessStage stage, void *executionContext, const HOOK_EXEC_OPTIONS *options)
{
    auto ret = HookMgrExecute(GetLocationExtHookMgr(), static_cast<int>(stage), executionContext, options);
    if (ret == 0) {
        return ERRCODE_SUCCESS;
    }
    LBSLOGE(LOCATOR, "%{public}s stage = %{public}d execute failed ret = %{public}d",
        __func__, static_cast<int>(stage), ret);
    return ERRCODE_SERVICE_UNAVAILABLE;
}

void HookUtils::ExecuteHookWhenStartLocation(std::shared_ptr<Request> request)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.request = *request;
    reportStruct.retCode = true;
    ExecuteHook(LocationProcessStage::LOCATOR_SA_START_LOCATING, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenStopLocation(std::shared_ptr<Request> request)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.request = *request;
    reportStruct.retCode = true;
    ExecuteHook(LocationProcessStage::REQUEST_MANAGER_HANDLE_STOP, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenGetAddressFromLocation(std::string packageName)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.abilityName = packageName;
    reportStruct.retCode = true;
    ExecuteHook(
        LocationProcessStage::LOCATOR_SA_GET_ADDRESSES_FROM_LOCATION_PROCESS, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenGetAddressFromLocationName(std::string packageName)
{
    LocationSupplicantInfo reportStruct;
    reportStruct.abilityName = packageName;
    reportStruct.retCode = true;
    ExecuteHook(
        LocationProcessStage::LOCATOR_SA_GET_ADDRESSES_FROM_LOCATIONNAME_PROCESS, (void *)&reportStruct, nullptr);
}

void HookUtils::ExecuteHookWhenReportInnerInfo(
    int32_t event, std::vector<std::string>& names, std::vector<std::string>& values)
{
    DfxInnerInfo innerInfo;
    innerInfo.eventId = event;
    innerInfo.names = names;
    innerInfo.values = values;
    ExecuteHook(LocationProcessStage::WRITE_DFX_INNER_EVENT_PROCESS, (void *)&innerInfo, nullptr);
}
} // namespace Location
} // namespace OHOS
