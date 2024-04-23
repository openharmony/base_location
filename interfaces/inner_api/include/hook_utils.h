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
    START_GNSS_PROCESS,
    STOP_GNSS_PROCESS,
    CHECK_GNSS_LOCATION_VALIDITY,
    MOCK_LOCATION_PROCESS,
    FENCE_REQUEST_PROCESS,
    REQUEST_MANAGER_HANDLE_STOP,
    LOCATOR_SA_GET_ADDRESSES_FROM_LOCATION_PROCESS,
    LOCATOR_SA_GET_ADDRESSES_FROM_LOCATIONNAME_PROCESS,
};

typedef struct {
    Location location;
    Request request;
    std::string abilityName;
    int retCode;
} LocationSupplicantInfo;

typedef struct {
    Location location;
    bool result;
} GnssLocationValidStruct;

typedef struct {
    bool enableMock;
} MockLocationStruct;

class HookUtils {
public:
    static HOOK_MGR* GetLocationExtHookMgr();
    static LocationErrCode RegisterHook(LocationProcessStage stage, int prio, OhosHook hook);
    static void UnregisterHook(LocationProcessStage stage, OhosHook hook);
    static LocationErrCode ExecuteHook(LocationProcessStage stage, void *executionContext,
        const HOOK_EXEC_OPTIONS *options);
    static void ExecuteHookWhenStartLocation(std::shared_ptr<Request> request);
    static void ExecuteHookWhenStopLocation(std::shared_ptr<Request> request);
    static void ExecuteHookWhenGetAddressFromLocation(std::string packageName);
    static void ExecuteHookWhenGetAddressFromLocationName(std::string packageName);
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_HOOK_UTILS_H