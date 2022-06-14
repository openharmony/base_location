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

#include <cstddef.h>
#include <cstdint.h>
#include "locator.h"
#include "locator_callback_host.h"
#include "request_config.h"
#include "i_locator_callback.h"

const int FUZZ_DATA_LEN = 8;

namespace OHOS {
    using namespace OHOS::Location;

    bool TestStartLocating(const uint8_t* data, size_t size)
    {
        bool result = false;
        if (size < FUZZ_DATA_LEN) {
            return false;
        }
        /* init locator and LocatorCallbackHost */
        std::unique_ptr<Locator> locator = Locator::GetInstance();
        auto locatorCallbackHost =
            sptr<LocatorCallbackHost>(new (std::nothrow) LocatorCallbackHost());
        int index = 0;
        locatorCallbackHost->m_fixNumber = data[index++];
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
        sptr<ILocatorCallback> locatorCallback = sptr<ILocatorCallback>(locatorCallbackHost);
        locator->StartLocating(requestConfig, locatorCallback);
        /* test StopLocating */
        locator->StopLocating(locatorCallback);
        /* test GetCurrentLocation */
        requestConfig->SetFixNumber(1);
        locator->StartLocating(requestConfig, locatorCallback);
        locator->StopLocating(locatorCallback);
        if (locatorCallbackHost) {
            delete locatorCallbackHost;
        }
        return result;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::TestStartLocating(data, size);
    return 0;
}

