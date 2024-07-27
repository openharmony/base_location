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

#include "requestmanager_fuzzer.h"

#include "i_locator_callback.h"
#include "locator_callback_napi.h"
#include "request.h"
#include "request_config.h"
#include "request_manager.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 6;
    bool RequestManagerFuzzerTest(const uint8_t* data, size_t size)
    {
        RequestManager* requestManager =
            RequestManager::GetInstance();
        if (requestManager == nullptr) {
            return false;
        }
        std::shared_ptr<Request> request = std::make_shared<Request>();
        requestManager->InitSystemListeners();
        auto locatorCallbackHostForTest =
            sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
        sptr<ILocatorCallback> locatorCallback =
            sptr<ILocatorCallback>(locatorCallbackHostForTest);
        requestManager->HandleStopLocating(locatorCallback);
        int index = 0;
        int32_t pid = data[index++];
        int32_t uid = data[index++];
        int32_t state = data[index++];
        requestManager->HandlePowerSuspendChanged(pid, uid, state);
        requestManager->UpdateRequestRecord(request, true);
        requestManager->UpdateRequestRecord(request, false);
        requestManager->HandleRequest();
        requestManager->UpdateUsingPermission(request, true);
        requestManager->HandlePermissionChanged(data[index++]);
        return true;
    }

    bool RequestFuzzerTest(const uint8_t* data, size_t size)
    {
        int index = 0;
        std::shared_ptr<Request> request = std::make_shared<Request>();
        request->SetUid(data[index++]);
        request->SetPid(data[index++]);
        std::string packageName((const char*) data, size);
        request->SetPackageName(packageName);
        auto requestConfig = std::make_unique<RequestConfig>();
        request->SetRequestConfig(*requestConfig);
        request->SetLocatorCallBack(nullptr);
        request->SetRequesting(true);
        request->SetTokenId(data[index++]);
        request->SetFirstTokenId(data[index++]);
        request->SetLocationPermState(true);
        request->SetBackgroundPermState(data[index++]);
        request->SetApproximatelyPermState(data[index++]);

        request->GetLastLocation();
        request->GetTokenId();
        request->GetFirstTokenId();
        request->GetLocationPermState();
        request->GetBackgroundPermState();
        request->GetApproximatelyPermState();
        std::shared_ptr<std::list<std::string>> proxys =
            std::make_shared<std::list<std::string>>();
        request->GetProxyName(proxys);
        request->GetUid();
        request->GetPid();
        request->GetPackageName();
        request->GetRequestConfig();
        request->GetLocatorCallBack();
        request->GetIsRequesting();
        request->ToString();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
    OHOS::RequestFuzzerTest(data, size);
    OHOS::RequestManagerFuzzerTest(data, size);
    return 0;
}