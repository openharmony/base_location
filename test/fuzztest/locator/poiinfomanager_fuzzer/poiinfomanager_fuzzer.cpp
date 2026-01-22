/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "poiinfomanager_fuzzer.h"

#include "i_locator_callback.h"
#include "location.h"
#include "locator_callback_napi.h"
#include "request.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 6;
    bool PoiInfoManagerFuzzerTest(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        auto manager = PoiInfoManager::GetInstance();
        manager->ConnectPoiService();
        manager->PreDisconnectAbilityConnect();
        manager->DisconnectAbilityConnect();
        manager->IsConnect();
        manager->RegisterPoiServiceDeathRecipient();
        manager->UnregisterPoiServiceDeathRecipient();
        manager->PreResetServiceProxy();
        manager->ResetServiceProxy();
        manager->NotifyDisConnected();

        std::unique_ptr<OHOS::Location::Location> location =
            std::make_unique<OHOS::Location::Location>();
        int index = 0;
        location->SetTimeSinceBoot(data[index++]);

        manager->UpdateCachedPoiInfo(location);
        manager->ClearPoiInfos(location);
        manager->UpdateLocationPoiInfo(location);
        
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::PoiInfoManagerFuzzerTest(data, size);
    return 0;
}