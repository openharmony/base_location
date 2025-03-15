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
#include "permission_manager.h"

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;
const int32_t LOCATION_PERM_NUM = 6;
const int32_t WAIT_EVENT_TIME = 3;
void MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        ACCESS_CONTROL_LOCATION_SWITCH.c_str(), ACCESS_LOCATION_SWITCH_IGNORED.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorAbility_FuzzTest",
        .aplStr = "system_basic",
    };
    auto tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    LocatorAbility::GetInstance()->EnableAbility(true);
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

bool LocatorAbilityStub001FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::DISABLE_LOCATION_MOCK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub002FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_ABILITY),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub003FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_LOCATION_MOCK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub004FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_CACHE_LOCATION),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub005FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_ISO_COUNTRY_CODE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub006FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_SWITCH_STATE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub007FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::IS_PRIVACY_COMFIRMED),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub008FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::PROXY_PID_FOR_FREEZE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub009FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_COUNTRY_CODE_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub010FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_SWITCH_CALLBACK),
        requestParcel, reply, option);
    
    return true;
}

bool LocatorAbilityStub011FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REPORT_LOCATION),
        requestParcel, reply, option);
    
    return true;
}

bool LocatorAbilityStub012FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::RESET_ALL_PROXY),
        requestParcel, reply, option);
    
    return true;
}

bool LocatorAbilityStub013FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SET_MOCKED_LOCATIONS),
        requestParcel, reply, option);
    
    return true;
}

bool LocatorAbilityStub014FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SET_PRIVACY_COMFIRM_STATUS),
        requestParcel, reply, option);
    return true;
}

bool LocatorAbilityStub015FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::START_LOCATING),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub016FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::STOP_LOCATING),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub017FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UPDATE_SA_ABILITY),
        requestParcel, reply, option);
    return true;
}

bool LocatorAbilityStub018FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub019FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub020FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GEO_IS_AVAILABLE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub021FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_FROM_COORDINATE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub022FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_FROM_LOCATION_NAME),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub023FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub024FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ADD_FENCE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub025FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::FLUSH_CACHED_LOCATIONS),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub026FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_CACHED_LOCATION_SIZE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub027FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_CACHED_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub028FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_GNSS_STATUS_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub029FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_NMEA_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub030FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_NMEA_CALLBACK_V9),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub031FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REMOVE_FENCE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub032FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SEND_COMMAND),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub033FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_CACHED_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub034FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_GNSS_STATUS_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub035FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_NMEA_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub036FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_NMEA_CALLBACK_V9),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub037FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_SWITCH_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub038FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_LOCATION_ERROR),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub039FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_LOCATION_ERROR),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub040FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REPORT_LOCATION_ERROR),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub041FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ADD_GNSS_GEOFENCE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub042FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REMOVE_GNSS_GEOFENCE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub043FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_GEOFENCE_SUPPORT_COORDINATE_SYSTEM_TYPE),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub044FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::REG_LOCATING_REQUIRED_DATA_CALLBACK),
        requestParcel, reply, option);

    return true;
}

bool LocatorAbilityStub045FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::UNREG_LOCATING_REQUIRED_DATA_CALLBACK),
        requestParcel, reply, option);
    auto locatorAbility = LocatorAbility::GetInstance();
    if (locatorAbility != nullptr) {
        locatorAbility->RemoveUnloadTask(DEFAULT_CODE);
    }
    return true;
}

bool LocatorAbilityStub046FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::ENABLE_ABILITY_BY_USERID),
        requestParcel, reply, option);
    return true;
}

bool LocatorAbilityStub047FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::GET_CURRENT_WIFI_BSSID_FOR_LOCATING),
        requestParcel, reply, option);
    return true;
}

bool LocatorAbilityStub048FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"OHOS.Location.ILocatorService");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;
    auto ability = sptr<LocatorAbility>(new (std::nothrow) LocatorAbility());
    ability->OnRemoteRequest(static_cast<int>(LocatorInterfaceCode::SET_LOCATION_SETTINGS_IGNORED),
        requestParcel, reply, option);
    return true;
}
} // namespace OHOS

void GeoCodeFuzzTest(const char* ch, size_t size)
{
    OHOS::LocatorAbilityStub018FuzzTest(ch, size);
    OHOS::LocatorAbilityStub019FuzzTest(ch, size);
    OHOS::LocatorAbilityStub020FuzzTest(ch, size);
    OHOS::LocatorAbilityStub021FuzzTest(ch, size);
    OHOS::LocatorAbilityStub022FuzzTest(ch, size);
    OHOS::LocatorAbilityStub023FuzzTest(ch, size);
}

void SwitchFuzzTest(const char* ch, size_t size)
{
    OHOS::LocatorAbilityStub002FuzzTest(ch, size);
    OHOS::LocatorAbilityStub006FuzzTest(ch, size);
    OHOS::LocatorAbilityStub046FuzzTest(ch, size);
    OHOS::LocatorAbilityStub048FuzzTest(ch, size);
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
        OHOS::MockNativePermission();
        GeoCodeFuzzTest(ch, size);
        SwitchFuzzTest(ch, size);
        OHOS::LocatorAbilityStub001FuzzTest(ch, size);
        OHOS::LocatorAbilityStub003FuzzTest(ch, size);
        OHOS::LocatorAbilityStub004FuzzTest(ch, size);
        OHOS::LocatorAbilityStub005FuzzTest(ch, size);
        OHOS::LocatorAbilityStub007FuzzTest(ch, size);
        OHOS::LocatorAbilityStub008FuzzTest(ch, size);
        OHOS::LocatorAbilityStub009FuzzTest(ch, size);
        OHOS::LocatorAbilityStub010FuzzTest(ch, size);
        OHOS::LocatorAbilityStub011FuzzTest(ch, size);
        OHOS::LocatorAbilityStub012FuzzTest(ch, size);
        OHOS::LocatorAbilityStub013FuzzTest(ch, size);
        OHOS::LocatorAbilityStub014FuzzTest(ch, size);
        OHOS::LocatorAbilityStub015FuzzTest(ch, size);
        OHOS::LocatorAbilityStub016FuzzTest(ch, size);
        OHOS::LocatorAbilityStub017FuzzTest(ch, size);
        OHOS::LocatorAbilityStub024FuzzTest(ch, size);
        OHOS::LocatorAbilityStub025FuzzTest(ch, size);
        OHOS::LocatorAbilityStub026FuzzTest(ch, size);
        OHOS::LocatorAbilityStub027FuzzTest(ch, size);
        OHOS::LocatorAbilityStub028FuzzTest(ch, size);
        OHOS::LocatorAbilityStub029FuzzTest(ch, size);
        OHOS::LocatorAbilityStub030FuzzTest(ch, size);
        OHOS::LocatorAbilityStub031FuzzTest(ch, size);
        OHOS::LocatorAbilityStub032FuzzTest(ch, size);
        OHOS::LocatorAbilityStub033FuzzTest(ch, size);
        OHOS::LocatorAbilityStub034FuzzTest(ch, size);
        OHOS::LocatorAbilityStub035FuzzTest(ch, size);
        OHOS::LocatorAbilityStub036FuzzTest(ch, size);
        OHOS::LocatorAbilityStub037FuzzTest(ch, size);
        OHOS::LocatorAbilityStub038FuzzTest(ch, size);
        OHOS::LocatorAbilityStub039FuzzTest(ch, size);
        OHOS::LocatorAbilityStub040FuzzTest(ch, size);
        OHOS::LocatorAbilityStub041FuzzTest(ch, size);
        OHOS::LocatorAbilityStub042FuzzTest(ch, size);
        OHOS::LocatorAbilityStub043FuzzTest(ch, size);
        OHOS::LocatorAbilityStub044FuzzTest(ch, size);
        OHOS::LocatorAbilityStub045FuzzTest(ch, size);
        OHOS::LocatorAbilityStub047FuzzTest(ch, size);
        sleep(OHOS::WAIT_EVENT_TIME);
        free(ch);
        ch = nullptr;
    }
    return 0;
}

