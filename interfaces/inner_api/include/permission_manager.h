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

#ifndef PERMISSION_MANAGER_H
#define PERMISSION_MANAGER_H

#include <unistd.h>
#include <string>

#include "message_parcel.h"
namespace OHOS {
namespace Location {
const std::string ACCESS_LOCATION = "ohos.permission.LOCATION";
const std::string ACCESS_APPROXIMATELY_LOCATION = "ohos.permission.APPROXIMATELY_LOCATION";
const std::string ACCESS_BACKGROUND_LOCATION = "ohos.permission.LOCATION_IN_BACKGROUND";
const std::string MANAGE_SECURE_SETTINGS = "ohos.permission.MANAGE_SECURE_SETTINGS";
const std::string RSS_PROCESS_NAME = "resource_schedule_service";

static constexpr int PERMISSION_ACCURATE = 2;
static constexpr int PERMISSION_APPROXIMATELY = 1;
static constexpr int PERMISSION_INVALID = 0;

class PermissionManager {
public:
    static bool CheckLocationPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckApproximatelyPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckBackgroundPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckPermission(const std::string &permission, uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckSecureSettings(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckCallingPermission(pid_t callingUid, pid_t callingPid, MessageParcel &reply);
    static bool CheckRssProcessName(uint32_t tokenId);
    static bool CheckSystemPermission(uint32_t callerTokenId, uint64_t callerTokenIdEx);
    static int GetPermissionLevel(uint32_t tokenId, uint32_t firstTokenId);
};
} // namespace Location
} // namespace OHOS
#endif // PERMISSION_MANAGER_H
