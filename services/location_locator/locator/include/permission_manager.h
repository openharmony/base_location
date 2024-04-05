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

namespace OHOS {
namespace Location {
class PermissionManager {
public:
    static bool CheckLocationPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckApproximatelyPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckBackgroundPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckPermission(const std::string &permission, uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckSecureSettings(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckCallingPermission(pid_t callingUid, pid_t callingPid, MessageParcel &reply);
    static bool CheckRssProcessName(uint32_t tokenId);
};
} // namespace Location
} // namespace OHOS
#endif // PERMISSION_MANAGER_H
