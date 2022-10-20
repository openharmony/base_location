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

#ifndef PERMISSION_STATUS_CHANGE_CB_H
#define PERMISSION_STATUS_CHANGE_CB_H

#include "permission_state_change_info.h"
#include "perm_state_change_callback_customize.h"

namespace OHOS {
namespace Location {
using namespace Security::AccessToken;
class PermissionStatusChangeCb : public PermStateChangeCallbackCustomize {
public:
    explicit PermissionStatusChangeCb(const PermStateChangeScope &scopeInfo)
        : PermStateChangeCallbackCustomize(scopeInfo) {}
    ~PermissionStatusChangeCb() {}
    void PermStateChangeCallback(PermStateChangeInfo& result) override;
};
} // namespace Location
} // namespace OHOS
#endif // PERMISSION_STATUS_CHANGE_CB_H
