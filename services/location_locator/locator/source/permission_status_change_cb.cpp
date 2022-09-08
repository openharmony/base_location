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

#include "permission_status_change_cb.h"
#include "location_log.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
void PermissionStatusChangeCb::PermStateChangeCallback(PermStateChangeInfo& result)
{
    LBSLOGD(LOCATOR, "%{public}s changed.", result.permissionName.c_str());
    std::string permissionName = result.permissionName;
    int32_t type = result.PermStateChangeType;
    uint32_t tokenID = result.tokenID;
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ApplyRequests();
    DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get()->OnPermissionChanged(type, tokenID, permissionName);
}
} // namespace Location
} // namespace OHOS