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

#include "locator_fuzzer.h"

#include "accesstoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "common_utils.h"
#include "i_locator_callback.h"
#include "locator.h"
#include "locator_callback_host.h"
#include "location_log.h"
#include "request_config.h"

namespace OHOS {
    using namespace OHOS::Location;
    auto locatorCallbackHostForTest_ =
                sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
    bool grant_ = false;
    const int FUZZ_DATA_LEN = 8;
    const int32_t MAX_CODE_LEN  = 512;
    const int32_t MAX_CODE_NUM = 40;
    const int32_t MIN_SIZE_NUM = 4;

    bool TestStartLocating(const uint8_t* data, size_t size)
    {
        bool result = false;
        if (size < FUZZ_DATA_LEN) {
            return false;
        }
        /* init locator and LocatorCallbackHost */
        std::unique_ptr<Locator> locator = Locator::GetInstance();
        int index = 0;
        locatorCallbackHostForTest_->SetFixNumber(data[index++]);
        /* init requestConfig */
        std::unique_ptr<RequestConfig> requestConfig = std::make_unique<RequestConfig>();
        requestConfig->SetScenario(data[index++]);
        requestConfig->SetPriority(data[index++]);
        requestConfig->SetTimeInterval(data[index++]);
        requestConfig->SetDistanceInterval(data[index++]);
        requestConfig->SetMaxAccuracy(data[index++]);
        requestConfig->SetFixNumber(data[index++]);
        requestConfig->SetTimeOut(data[index++]);
        /* test StartLocating */
        sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(locatorCallbackHostForTest_);
        locator->StartLocating(requestConfig, locatorCallback);
        /* test StopLocating */
        locator->StopLocating(locatorCallback);
        /* test GetCurrentLocation */
        requestConfig->SetFixNumber(1);
        locator->StartLocating(requestConfig, locatorCallback);
        locator->StopLocating(locatorCallback);
        return result;
    }
    
    bool LocatorProxySendRequestTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size > MAX_CODE_LEN) || (size < MIN_SIZE_NUM)) {
            LBSLOGE(LOCATOR, "param error");
            return false;
        }
        uint32_t cmdCode = *(reinterpret_cast<const uint32_t*>(data));
        cmdCode %= MAX_CODE_NUM;
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        sptr<IRemoteObject> object = systemAbilityManager->GetSystemAbility(LOCATION_LOCATOR_SA_ID);
        auto client = std::make_unique<LocatorProxyTestFuzzer>(object);
        if (client == nullptr) {
            LBSLOGE(LOCATOR, "client is nullptr");
            return false;
        }
        MessageParcel request;
        if (!request.WriteInterfaceToken(client->GetDescriptor())) {
            LBSLOGE(LOCATOR, "cannot write interface token");
            return false;
        }
        MessageParcel reply;
        MessageOption option;
        sptr<IRemoteObject> remote = client->GetRemote();
        if (remote == nullptr) {
            LBSLOGE(LOCATOR, "cannot get remote object");
            return false;
        }
        size -= sizeof(uint32_t);
        request.WriteBuffer(data + sizeof(uint32_t), size);
        request.RewindRead(0);
        int32_t result = remote->SendRequest(cmdCode, request, reply, option);
        return result == SUCCESS;
    }
    
    void AddPermission()
    {
        if (!grant_) {
            const char *perms[] = {
                ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
                ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
            };
            NativeTokenInfoParams infoInstance = {
                .dcapsNum = 0,
                .permsNum = 4,
                .aclsNum = 0,
                .dcaps = nullptr,
                .perms = perms,
                .acls = nullptr,
                .processName = "LocatorFuzzer",
                .aplStr = "system_basic",
            };
            uint64_t tokenId = GetAccessTokenId(&infoInstance);
            SetSelfTokenID(tokenId);
            Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
            grant_ = true;
        }
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::AddPermission();
    OHOS::TestStartLocating(data, size);
    OHOS::LocatorProxySendRequestTest(data, size);
    return 0;
}

