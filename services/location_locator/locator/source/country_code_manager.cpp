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

#include "request_manager.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "fusion_controller.h"
#include "location_log.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#include "locator_event_manager.h"
#include "subability_common.h"
#include "core_service_client.h"

namespace OHOS {
namespace Location {
CountryCodeManager::CountryCodeManager()
{
}

CountryCodeManager::~CountryCodeManager()
{
}

int CountryCodeManager::GetIsoCountryCode(std::string& code)
{
    LBSLOGI(GNSS, "CountryCodeManager::GetIsoCountryCode");
    std::u16string isoCode =
        DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetISOCountryCodeForSim(0);
    if (isoCode.empty()) {
        isoCode = DelayedRefSingleton<Telephony::CoreServiceClient>::GetInstance().GetISOCountryCodeForSim(1);
    }
    code = Str16ToStr8(isoCode);
    return 0;
}
} // namespace Location
} // namespace OHOS
