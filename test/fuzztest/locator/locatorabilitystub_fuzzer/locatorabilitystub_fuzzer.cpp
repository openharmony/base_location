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

#include "locatorabilitystub_fuzzer.h"

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

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;

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

bool LocatorAbilityStub001FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocator");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability1 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability1->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    auto ability2 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability2->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_ABILITY),
        requestParcel, reply, option);
    auto ability3 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability3->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    auto ability4 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability4->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_CACHE_LOCATION),
        requestParcel, reply, option);
    auto ability5 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability5->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_ISO_COUNTRY_CODE),
        requestParcel, reply, option);
    auto ability6 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability6->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_SWITCH_STATE),
        requestParcel, reply, option);
    auto ability7 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability7->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::IS_PRIVACY_COMFIRMED),
        requestParcel, reply, option);
    auto ability8 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability8->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::PROXY_UID_FOR_FREEZE),
        requestParcel, reply, option);
    auto ability9 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability9->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_COUNTRY_CODE_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub002FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocator");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability10 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability10->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_SWITCH_CALLBACK),
        requestParcel, reply, option);
    auto ability11 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability11->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REPORT_LOCATION),
        requestParcel, reply, option);
    auto ability12 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability12->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::RESET_ALL_PROXY),
        requestParcel, reply, option);
    auto ability13 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability13->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS),
        requestParcel, reply, option);
    auto ability14 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability14->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SET_PRIVACY_COMFIRM_STATUS),
        requestParcel, reply, option);
    auto ability15 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability15->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::START_LOCATING),
        requestParcel, reply, option);
    auto ability16 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability16->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::STOP_LOCATING),
        requestParcel, reply, option);
    auto ability17 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability17->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_COUNTRY_CODE_CALLBACK),
        requestParcel, reply, option);
    auto ability18 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability18->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_SWITCH_CALLBACK),
        requestParcel, reply, option);
    auto ability19 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability19->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UPDATE_SA_ABILITY),
        requestParcel, reply, option);
    return true;
}

#ifdef FEATURE_GEOCODE_SUPPORT
bool LocatorAbilityStub003FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocator");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability1 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability1->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK),
        requestParcel, reply, option);
    auto ability2 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability2->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK),
        requestParcel, reply, option);
    auto ability3 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability3->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GEO_IS_AVAILABLE),
        requestParcel, reply, option);
    auto ability4 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability4->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_FROM_COORDINATE),
        requestParcel, reply, option);
    auto ability5 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability5->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_FROM_LOCATION_NAME),
        requestParcel, reply, option);
    auto ability6 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability6->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO),
        requestParcel, reply, option);

    return true;
}
#endif

bool LocatorAbilityStub004FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocator");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability1 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability1->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ADD_FENCE),
        requestParcel, reply, option);
    auto ability2 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability2->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::FLUSH_CACHED_LOCATIONS),
        requestParcel, reply, option);
    auto ability3 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability3->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_CACHED_LOCATION_SIZE),
        requestParcel, reply, option);
    auto ability4 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability4->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_CACHED_CALLBACK),
        requestParcel, reply, option);
    auto ability5 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability5->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_GNSS_STATUS_CALLBACK),
        requestParcel, reply, option);
    auto ability6 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability6->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_NMEA_CALLBACK),
        requestParcel, reply, option);
    auto ability7 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability7->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_NMEA_CALLBACK_v9),
        requestParcel, reply, option);
    auto ability8 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability8->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REMOVE_FENCE),
        requestParcel, reply, option);
    auto ability9 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability9->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SEND_COMMAND),
        requestParcel, reply, option);
    auto ability10 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability10->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_CACHED_CALLBACK),
        requestParcel, reply, option);
    auto ability11 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability11->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_GNSS_STATUS_CALLBACK),
        requestParcel, reply, option);
    auto ability12 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability12->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_NMEA_CALLBACK),
        requestParcel, reply, option);
    auto ability13 = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    ability13->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_NMEA_CALLBACK_v9),
        requestParcel, reply, option);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
        OHOS::LocatorAbilityStub001FuzzTest(ch, size);
        OHOS::LocatorAbilityStub002FuzzTest(ch, size);
        OHOS::LocatorAbilityStub003FuzzTest(ch, size);
        OHOS::LocatorAbilityStub004FuzzTest(ch, size);
        free(ch);
        ch = nullptr;
    }
    return 0;
}

