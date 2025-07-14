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

#include "fusioncontroller_fuzzer.h"

#include "message_parcel.h"
#include "fusion_controller.h"
#include "location.h"

namespace OHOS {
    using namespace OHOS::Location;
    const int MIN_DATA_LEN = 1;
    bool FusionControllerFuzzerTest(const uint8_t* data, size_t size)
    {
        if (size < MIN_DATA_LEN) {
            return true;
        }
        std::unique_ptr<OHOS::Location::Location> location =
            std::make_unique<OHOS::Location::Location>();
        MessageParcel messageParcel;
        location->Unmarshalling(messageParcel);
        location->UnmarshallingShared(messageParcel);
        location->Marshalling(messageParcel);
        PoiInfo poiInfo;
        location->WritePoiInfoToParcel(poiInfo, messageParcel);
        location->ToString();
        location->LocationEqual(location);
        location->AdditionEqual(location);
        location->isValidLatitude(0.0);
        location->isValidLongitude(0.0);
        location->GetDistanceBetweenLocations(0.0, 0.0, 0.0, 0.0);
        std::shared_ptr<FusionController> fusionController =
            std::make_shared<FusionController>();
        int index = 0;
        fusionController->ActiveFusionStrategies(data[index++]);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FusionControllerFuzzerTest(data, size);
    return 0;
}