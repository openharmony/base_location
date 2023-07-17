/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "passiveability_fuzzer.h"

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

#ifdef FEATURE_PASSIVE_SUPPORT
#include "passive_ability.h"
#endif

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;
const int32_t SLEEP_TIMES = 1000;

char* ParseData(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return nullptr;
    }

    if (size > MAX_MEM_SIZE) {
        return nullptr;
    }

    char* ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        return nullptr;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return nullptr;
    }
    return ch;
}

#ifdef FEATURE_PASSIVE_SUPPORT
bool PassiveAbilityFuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IPassiveAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability1 = sptr<PassiveAbility>(new (std::nothrow) PassiveAbility());
    ability1->OnRemoteRequest(static_cast<uint32_t>(PassiveInterfaceCode::SEND_LOCATION_REQUEST),
        requestParcel, reply, option);
    auto ability2 = sptr<PassiveAbility>(new (std::nothrow) PassiveAbility());
    ability2->OnRemoteRequest(static_cast<uint32_t>(PassiveInterfaceCode::SET_ENABLE),
        requestParcel, reply, option);
    auto ability3 = sptr<PassiveAbility>(new (std::nothrow) PassiveAbility());
    ability3->OnRemoteRequest(static_cast<uint32_t>(PassiveInterfaceCode::ENABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    auto ability4 = sptr<PassiveAbility>(new (std::nothrow) PassiveAbility());
    ability4->OnRemoteRequest(static_cast<uint32_t>(PassiveInterfaceCode::DISABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    auto ability5 = sptr<PassiveAbility>(new (std::nothrow) PassiveAbility());
    ability5->OnRemoteRequest(static_cast<uint32_t>(PassiveInterfaceCode::SET_MOCKED_LOCATIONS),
        requestParcel, reply, option);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    
    return true;
}
#endif
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
#ifdef FEATURE_PASSIVE_SUPPORT
        OHOS::PassiveAbilityFuzzTest(ch, size);
#endif
        free(ch);
        ch = nullptr;
    }
    return 0;
}

