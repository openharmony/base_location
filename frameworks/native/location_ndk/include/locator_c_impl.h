/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef LOCATOR_C_IMPL_H
#define LOCATOR_C_IMPL_H

#include "oh_location_type.h"
#include "location_info_callback_host.h"
#include "constant_definition.h"

const int ADDITIONS_MAX_LEN = 512;

struct Location_RequestConfig {
    int32_t scenario_ = OHOS::Location::SCENE_UNSET;
    int32_t timeInterval_ = 1; // no time interval limit for reporting location
};

typedef struct Location_Info {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    double accuracy = 0.0;
    double speed = 0.0;
    double direction = 0.0;
    int64_t timeForFix = 0;
    int64_t timeSinceBoot = 0;
    char additions[ADDITIONS_MAX_LEN];
    double altitudeAccuracy = 0.0;
    double speedAccuracy = 0.0;
    double directionAccuracy = 0.0;
    int64_t uncertaintyOfTimeSinceBoot = 0;
    Location_SourceType locationSourceType;
} Location_Info;

void AddLocationCallBack(OHOS::sptr<OHOS::Location::LocationInfoCallbackHost>& callback);
void RemoveLocationCallBack(Location_InfoCallback callback);
OHOS::sptr<OHOS::Location::LocationInfoCallbackHost> GetLocationCallBack(Location_InfoCallback callback);
Location_ResultCode LocationErrCodeToLocationResultCode(OHOS::Location::LocationErrCode errCode);

#endif // LOCATOR_C_IMPL_H