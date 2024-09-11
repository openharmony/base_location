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

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "system_ability_definition.h"
#include "locator_ability.h"
#include "locationhub_ipc_interface_code.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability.h"
#endif
#include "permission_manager.h"
#include "work_record_statistic.h"

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;
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
        .processName = "GnssAbility_FuzzTest",
        .aplStr = "system_basic",
    };
    auto tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

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

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS),
        requestParcel, reply, option);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
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

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility003FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility004FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REFRESH_REQUESTS),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility005FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_GNSS_STATUS),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility006FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_GNSS_STATUS),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility007FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_NMEA),
        requestParcel, reply, option);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility008FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_NMEA),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility009FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REG_CACHED),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility010FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::UNREG_CACHED),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility011FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::GET_CACHED_SIZE),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility012FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::FLUSH_CACHED),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility013FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility014FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::ENABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility015FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::DISABLE_LOCATION_MOCK),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility016FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::ADD_FENCE_INFO),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility017FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FENCE_INFO),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility018FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::ADD_GNSS_GEOFENCE),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility019FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::REMOVE_GNSS_GEOFENCE),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility020FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::GET_GEOFENCE_SUPPORT_COORDINATE_SYSTEM_TYPE),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}

bool GnssAbility021FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGnssAbility");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    auto ability = GnssAbility::GetInstance();
    ability->OnRemoteRequest(static_cast<uint32_t>(GnssInterfaceCode::SEND_NETWORK_LOCATION),
        requestParcel, reply, option);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIMES));
    WorkRecordStatistic::DestroyInstance();
    return true;
}
#endif
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::MockNativePermission();
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
#ifdef FEATURE_GNSS_SUPPORT
    OHOS::GnssAbility001FuzzTest(ch, size);
    OHOS::GnssAbility002FuzzTest(ch, size);
    OHOS::GnssAbility003FuzzTest(ch, size);
    OHOS::GnssAbility004FuzzTest(ch, size);
    OHOS::GnssAbility005FuzzTest(ch, size);
    OHOS::GnssAbility006FuzzTest(ch, size);
    OHOS::GnssAbility007FuzzTest(ch, size);
    OHOS::GnssAbility008FuzzTest(ch, size);
    OHOS::GnssAbility009FuzzTest(ch, size);
    OHOS::GnssAbility010FuzzTest(ch, size);
    OHOS::GnssAbility011FuzzTest(ch, size);
    OHOS::GnssAbility012FuzzTest(ch, size);
    OHOS::GnssAbility013FuzzTest(ch, size);
    OHOS::GnssAbility014FuzzTest(ch, size);
    OHOS::GnssAbility015FuzzTest(ch, size);
    OHOS::GnssAbility016FuzzTest(ch, size);
    OHOS::GnssAbility017FuzzTest(ch, size);
    OHOS::GnssAbility018FuzzTest(ch, size);
    OHOS::GnssAbility019FuzzTest(ch, size);
    OHOS::GnssAbility020FuzzTest(ch, size);
    OHOS::GnssAbility021FuzzTest(ch, size);
#endif
        free(ch);
        ch = nullptr;
    }
    return 0;
}

