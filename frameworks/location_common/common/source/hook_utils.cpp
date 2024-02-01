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

LocationErrCode HookUtils::ExecuteHook(LocationProcessStage stage, void *executionContext, const HOOK_EXEC_OPTIONS *options)
{
    auto ret = HookMgrExecute(GetLocationExtHookMgr(), static_cast<int>(stage), executionContext, options);
    if (ret == 0) {
        return ERRCODE_SUCCESS;
    }
    LBSLOGE(LOCATOR, "%{public}s stage = %{public}d execute failed ret = %{public}d",
        __func__, static_cast<int>(stage), ret);
    return ERRCODE_SERVICE_UNAVAILABLE;
}
} // namespace Location
} // namespace OHOS
