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

#include "networkabilityproxy_fuzzer.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "locator_ability.h"
#include "locationhub_ipc_interface_code.h"

#ifdef FEATURE_NETWORK_SUPPORT
#include "network_ability.h"
#include "network_ability_proxy.h"
#endif
#include "permission_manager.h"

namespace OHOS {
using namespace OHOS::Location;
const int32_t SLEEP_TIMES = 1000;
const int32_t LOCATION_PERM_NUM = 4;
void MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "NetworkAbilityProxy_FuzzTest",
        .aplStr = "system_basic",
    };
    auto tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    LocatorAbility::GetInstance()->EnableAbility(true);
}

#ifdef FEATURE_NETWORK_SUPPORT
bool NetworkAbilityProxy001FuzzTest(const uint8_t* data, size_t size)
{
    int index = 0;
    sptr<OHOS::Location::NetworkAbility> ability = new (std::nothrow) NetworkAbility();
    sptr<OHOS::Location::NetworkAbilityProxy> proxy =
        new (std::nothrow) NetworkAbilityProxy(ability);
    proxy->SetEnable(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    proxy->SetEnable(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    std::vector<std::shared_ptr<OHOS::Location::Location>> locations;
    proxy->EnableMock();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    proxy->DisableMock();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    proxy->SetMocked(data[index++], locations);
    ability = nullptr;
    return true;
}
#endif
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::MockNativePermission();
#ifdef FEATURE_NETWORK_SUPPORT
    OHOS::NetworkAbilityProxy001FuzzTest(data, size);
#endif
    return 0;
}

