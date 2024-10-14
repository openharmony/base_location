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

#include "geoconvertservice_fuzzer.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "locationhub_ipc_interface_code.h"

#include "location.h"
#include "locator_ability.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_convert_service.h"
#endif
#include "permission_manager.h"

namespace OHOS {
using namespace OHOS::Location;
const int32_t MAX_MEM_SIZE = 4 * 1024 * 1024;
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
        .processName = "Geocode_FuzzTest",
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

#ifdef FEATURE_GEOCODE_SUPPORT
bool GeoConvertServiceFuzzTest001(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto service1 = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    service1->OnRemoteRequest(static_cast<uint32_t>(GeoConvertInterfaceCode::IS_AVAILABLE),
        requestParcel, reply, option);

    return true;
}

bool GeoConvertServiceFuzzTest002(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto service2 = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    service2->OnRemoteRequest(static_cast<uint32_t>(GeoConvertInterfaceCode::GET_FROM_COORDINATE),
        requestParcel, reply, option);

    return true;
}

bool GeoConvertServiceFuzzTest003(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto service3 = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    service3->OnRemoteRequest(static_cast<uint32_t>(GeoConvertInterfaceCode::GET_FROM_LOCATION_NAME_BY_BOUNDARY),
        requestParcel, reply, option);

    return true;
}

bool GeoConvertServiceFuzzTest004(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto service4 = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    service4->OnRemoteRequest(static_cast<uint32_t>(GeoConvertInterfaceCode::ENABLE_REVERSE_GEOCODE_MOCK),
        requestParcel, reply, option);

    return true;
}

bool GeoConvertServiceFuzzTest005(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto service5 = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    service5->OnRemoteRequest(static_cast<uint32_t>(GeoConvertInterfaceCode::DISABLE_REVERSE_GEOCODE_MOCK),
        requestParcel, reply, option);

    return true;
}

bool GeoConvertServiceFuzzTest006(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.IGeoConvert");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto service6 = sptr<GeoConvertService>(new (std::nothrow) GeoConvertService());
    service6->OnRemoteRequest(static_cast<uint32_t>(GeoConvertInterfaceCode::SET_REVERSE_GEOCODE_MOCKINFO),
        requestParcel, reply, option);

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
#ifdef FEATURE_GEOCODE_SUPPORT
        OHOS::GeoConvertServiceFuzzTest001(ch, size);
        OHOS::GeoConvertServiceFuzzTest002(ch, size);
        OHOS::GeoConvertServiceFuzzTest003(ch, size);
        OHOS::GeoConvertServiceFuzzTest004(ch, size);
        OHOS::GeoConvertServiceFuzzTest005(ch, size);
        OHOS::GeoConvertServiceFuzzTest006(ch, size);
#endif
        free(ch);
        ch = nullptr;
    }
    return 0;
}

