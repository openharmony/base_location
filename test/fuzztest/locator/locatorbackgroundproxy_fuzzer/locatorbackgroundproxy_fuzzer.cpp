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

#include "locatorbackgroundproxy_fuzzer.h"

#include "i_locator_callback.h"
#include "locator_background_proxy.h"
#include "locator_callback_proxy.h"
#include "request.h"
#include "request_config.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 11;
    bool LocatorBackgroundProxyFuzzerTest(const uint8_t* data, size_t size)
    {
        int index = 0;
        auto backgroundProxy =
            DelayedSingleton<LocatorBackgroundProxy>::GetInstance().get();
        if (backgroundProxy == nullptr) {
            return false;
        }
        std::shared_ptr<Request> request =
            std::make_shared<Request>();
        std::string bundleName((const char*) data, size);
        request->SetUid(data[index++]);
        request->SetPid(data[index++]);
        request->SetPackageName(bundleName);
        std::unique_ptr<RequestConfig> requestConfig =
            std::make_unique<RequestConfig>();
        requestConfig->SetScenario(data[index++]);
        requestConfig->SetPriority(data[index++]);
        requestConfig->SetTimeInterval(data[index++]);
        requestConfig->SetDistanceInterval(data[index++]);
        requestConfig->SetMaxAccuracy(data[index++]);
        requestConfig->SetFixNumber(data[index++]);
        requestConfig->SetTimeOut(data[index++]);
        request->SetRequestConfig(*requestConfig);
        request->SetRequesting(true);
        request->SetTokenId(data[index++]);
        request->SetFirstTokenId(data[index++]);
        request->SetLocationPermState(false);
        request->SetBackgroundPermState(false);
        request->SetApproximatelyPermState(false);
        backgroundProxy->UpdateListOnRequestChange(request);
        backgroundProxy->OnSuspend(request, true);
        backgroundProxy->OnSuspend(request, false);
        backgroundProxy->OnSaStateChange(true);
        backgroundProxy->OnSaStateChange(false);
        backgroundProxy->OnDeleteRequestRecord(request);
        sptr<ILocatorCallback> callbackStub =
            new (std::nothrow) LocatorCallbackStub();
        backgroundProxy->IsCallbackInProxy(callbackStub);
        backgroundProxy->IsAppBackground(bundleName);
        backgroundProxy->RegisterAppStateObserver();
        backgroundProxy->UnregisterAppStateObserver();
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::MIN_DATA_LEN) {
        return 0;
    }
    /* Run your code on data */
    OHOS::LocatorBackgroundProxyFuzzerTest(data, size);
    return 0;
}