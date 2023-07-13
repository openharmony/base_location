/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "gnssability_fuzzer.h"

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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability.h"
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

#ifdef FEATURE_GNSS_SUPPORT
bool GnssAbility001FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability1 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability1->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST),
        requestParcel, reply, option);
    auto ability2 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability2->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS),
        requestParcel, reply, option);
    auto ability3 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability3->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE),
        requestParcel, reply, option);
    auto ability4 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability4->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REFRESH_REQUESTS),
        requestParcel, reply, option);
    auto ability5 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability5->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_GNSS_STATUS),
        requestParcel, reply, option);
    auto ability6 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability6->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_GNSS_STATUS),
        requestParcel, reply, option);
    auto ability7 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability7->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_NMEA),
        requestParcel, reply, option);
    auto ability8 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability8->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_NMEA),
        requestParcel, reply, option);
    auto ability9 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability9->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_CACHED),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));

    return true;
}

bool GnssAbility002FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability10 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability10->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_CACHED),
        requestParcel, reply, option);
    auto ability11 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability11->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::GET_CACHED_SIZE),
        requestParcel, reply, option);
    auto ability12 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability12->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::FLUSH_CACHED),
        requestParcel, reply, option);
    auto ability13 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability13->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS),
        requestParcel, reply, option);
    auto ability14 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability14->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::ENABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    auto ability15 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability15->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::DISABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    auto ability16 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability16->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::ADD_FENCE_INFO),
        requestParcel, reply, option);
    auto ability17 = sptr<GnssAbility>(new (std::nothrow) GnssAbility());
    ability17->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FENCE_INFO),
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
#ifdef FEATURE_GNSS_SUPPORT
        OHOS::GnssAbility001FuzzTest(ch, size);
        OHOS::GnssAbility002FuzzTest(ch, size);
#endif
        free(ch);
        ch = nullptr;
    }
    return 0;
}

