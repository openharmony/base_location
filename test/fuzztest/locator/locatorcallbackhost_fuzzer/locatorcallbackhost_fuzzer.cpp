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

#include "locatorcallbackhost_fuzzer.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "locator_ability.h"

#include "locator_callback_host.h"

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

bool LocatorCallbackHost001FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocatorCallback");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto callback = sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    callback->OnRemoteRequest(ILocatorCallback::RECEIVE_LOCATION_INFO_EVENT, requestParcel, reply, option);

    return true;
}

bool LocatorCallbackHost002FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocatorCallback");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto callback = sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    callback->OnRemoteRequest(ILocatorCallback::RECEIVE_LOCATION_STATUS_EVENT, requestParcel, reply, option);

    return true;
}

bool LocatorCallbackHost003FuzzTest(const char* data, size_t size)
{
    MessageParcel requestParcel;
    requestParcel.WriteInterfaceToken(u"location.ILocatorCallback");
    requestParcel.WriteBuffer(data, size);
    requestParcel.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    auto callback = sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    callback->OnRemoteRequest(ILocatorCallback::RECEIVE_ERROR_INFO_EVENT, requestParcel, reply, option);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::MockNativePermission();
    char* ch = OHOS::ParseData(data, size);
    if (ch != nullptr) {
        OHOS::LocatorCallbackHost001FuzzTest(ch, size);
        OHOS::LocatorCallbackHost002FuzzTest(ch, size);
        OHOS::LocatorCallbackHost003FuzzTest(ch, size);
        free(ch);
        ch = nullptr;
    }
    return 0;
}

