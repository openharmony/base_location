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

#ifndef LOCATION_HOOK_UTILS_H
#define LOCATION_HOOK_UTILS_H

#include "hookmgr.h"
#include "constant_definition.h"
#include "request.h"
#include "location.h"

namespace OHOS {
namespace Location {
enum class LocationProcessStage {
    LOCATOR_SA_START_LOCATING = 0,
    LOCATOR_SA_REQUEST_PROCESS,
    GNSS_SA_REQUEST_PROCESS,
    NETWORK_SA_REQUEST_PROCESS,
    GNSS_SA_LOCATION_REPORT_PROCESS,
    NETWORK_SA_LOCATION_REPORT_PROCESS,
    LOCATOR_SA_LOCATION_REPORT_PROCESS,
};

typedef struct {
    Location location;
    Request request;
    std::string abilityName;
    int retCode;
} LocationSupplicantInfo;

class HookUtils {
public:
    static HOOK_MGR* GetLocationExtHookMgr();
    static LocationErrCode RegisterHook(LocationProcessStage stage, int prio, OhosHook hook);
    static void UnregisterHook(LocationProcessStage stage, OhosHook hook);
    static LocationErrCode ExecuteHook(LocationProcessStage stage, void *executionContext,
        const HOOK_EXEC_OPTIONS *options);
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_HOOK_UTILS_H