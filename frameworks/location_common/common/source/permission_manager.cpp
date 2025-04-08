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

#include "permission_manager.h"

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "tokenid_kit.h"
#include "location_log.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
bool PermissionManager::CheckLocationPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId);
}

bool PermissionManager::CheckPermission(const std::string &permission, uint32_t callerToken, uint32_t tokenFirstCaller)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    int result = Security::AccessToken::PERMISSION_DENIED;
    if (tokenFirstCaller == 0) {
        if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_INVALID) {
            LBSLOGD(LOCATOR, "id = %{public}d has no permission.permission name=%{public}s",
                callerToken, permission.c_str());
            return false;
        } else {
            result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permission);
        }
    } else {
        result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, tokenFirstCaller, permission);
    }
    if (result == Security::AccessToken::PERMISSION_GRANTED) {
        return true;
    } else {
        LBSLOGD(LOCATOR, "id = %{public}d has no permission.permission name=%{public}s",
            callerToken, permission.c_str());
        return false;
    }
}

bool PermissionManager::CheckRssProcessName(uint32_t tokenId)
{
    Security::AccessToken::NativeTokenInfo callingTokenInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, callingTokenInfo);
    if (callingTokenInfo.processName != RSS_PROCESS_NAME) {
        LBSLOGE(LOCATOR, "CheckProcess failed, processName=%{public}s", callingTokenInfo.processName.c_str());
        return false;
    }
    return true;
}

bool PermissionManager::CheckBackgroundPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_BACKGROUND_LOCATION, tokenId, firstTokenId);
}

bool PermissionManager::CheckMockLocationPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_MOCK_LOCATION, tokenId, firstTokenId);
}

bool PermissionManager::CheckApproximatelyPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId);
}

bool PermissionManager::CheckSecureSettings(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(MANAGE_SECURE_SETTINGS, tokenId, firstTokenId);
}

bool PermissionManager::CheckCallingPermission(pid_t callingUid, pid_t callingPid, MessageParcel &reply)
{
    if (callingUid != static_cast<pid_t>(getuid()) || callingPid != getpid()) {
        LBSLOGE(LOCATOR, "uid pid not match locationhub process.");
        reply.WriteInt32(LOCATION_ERRCODE_PERMISSION_DENIED);
        return false;
    }
    return true;
}

int PermissionManager::GetPermissionLevel(uint32_t tokenId, uint32_t firstTokenId)
{
    int ret = PERMISSION_INVALID;
    if (CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId) &&
        CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId)) {
        ret = PERMISSION_ACCURATE;
    } else if (CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId) &&
        !CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId)) {
        ret = PERMISSION_APPROXIMATELY;
    } else if (!CheckPermission(ACCESS_APPROXIMATELY_LOCATION, tokenId, firstTokenId) &&
        CheckPermission(ACCESS_LOCATION, tokenId, firstTokenId)) {
        ret = PERMISSION_ACCURATE;
    }  else {
        ret = PERMISSION_INVALID;
    }
    return ret;
}

bool PermissionManager::CheckSystemPermission(uint32_t callerTokenId, uint64_t callerTokenIdEx)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL ||
        tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_INVALID) {
        return false;
    }
    bool isSysApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(callerTokenIdEx);
    return isSysApp;
}

bool PermissionManager::CheckIsSystemSa(uint32_t tokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    return false;
}

bool PermissionManager::CheckLocationSwitchIgnoredPermission(uint32_t tokenId, uint32_t firstTokenId)
{
    return CheckPermission(ACCESS_LOCATION_SWITCH_IGNORED, tokenId, firstTokenId);
}
} // namespace Location
} // namespace OHOS