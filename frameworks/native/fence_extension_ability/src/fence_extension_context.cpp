/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "fence_extension_context.h"
#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
namespace {
constexpr const char *PERMISSION_START_ABILITIES_FROM_BACKGROUND = "ohos.permission.START_ABILITIES_FROM_BACKGROUND";
}

FenceExtensionContext::FenceExtensionContext(){};

FenceExtensionContext::~FenceExtensionContext(){};

ErrCode FenceExtensionContext::StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId) const
{
    ErrCode err = ERR_OK;

    std::string callerBundleName = GetBundleName();
    std::string calledBundleName = want.GetBundle();
    if (calledBundleName != callerBundleName) {
        LBSLOGE(FENCE_EXTENSION, "This application won't start no-self-ability.");
        err = AAFwk::ERR_NOT_SELF_APPLICATION;
        return err;
    }

    LBSLOGI(FENCE_EXTENSION, "start %{public}s begin.", __func__);
    err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        LBSLOGE(FENCE_EXTENSION, "FenceExtensionContext::StartServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

bool FenceExtensionContext::CheckCallerIsSystemApp() const
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        return false;
    }
    return true;
}
}  // namespace Location
}  // namespace OHOS