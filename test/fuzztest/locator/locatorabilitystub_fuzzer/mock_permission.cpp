/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "mock_permission.h"
#include "location_log.h"
#include "permission_manager.h"
#include <sstream>
#include <thread>

namespace OHOS {
namespace Location {
using namespace Security::AccessToken;
namespace {
std::mutex g_lockSetToken;
uint64_t g_shellTokenId = 0;
}
void PrivacyTestCommon::SetTestEvironment(uint64_t shellTokenId)
{
    std::lock_guard<std::mutex> lock(g_lockSetToken);
    g_shellTokenId = shellTokenId;
}

void PrivacyTestCommon::ResetTestEvironment()
{
    std::lock_guard<std::mutex> lock(g_lockSetToken);
    g_shellTokenId = 0;
}

uint64_t PrivacyTestCommon::GetShellTokenId()
{
    std::lock_guard<std::mutex> lock(g_lockSetToken);
    return g_shellTokenId;
}

AccessTokenIDEx PrivacyTestCommon::AllocTestHapToken(const HapInfoParams& hapInfo, HapPolicyParams& hapPolicy)
{
    AccessTokenIDEx tokenIdEx = {0};
    uint64_t selfTokenId = GetSelfTokenID();
    for (auto& permissionStateFull : hapPolicy.permStateList) {
        PermissionDef permDefResult;
        if (AccessTokenKit::GetDefPermission(permissionStateFull.permissionName, permDefResult) != RET_SUCCESS) {
            continue;
        }
        hapPolicy.aclRequestedList.emplace_back(permissionStateFull.permissionName);
    }
    if (PrivacyTestCommon::GetNativeTokenIdFromProcess("foundation") == selfTokenId) {
        AccessTokenKit::InitHapToken(hapInfo, hapPolicy, tokenIdEx);
    } else {
        // set sh token for self
        MockNativeToken mock("foundation");
        AccessTokenKit::InitHapToken(hapInfo, hapPolicy, tokenIdEx);

        // restore
        SetSelfTokenID(selfTokenId);
    }
    return tokenIdEx;
}

int32_t PrivacyTestCommon::DeleteTestHapToken(AccessTokenID tokenID)
{
    uint64_t selfTokenId = GetSelfTokenID();
    if (PrivacyTestCommon::GetNativeTokenIdFromProcess("foundation") == selfTokenId) {
        return AccessTokenKit::DeleteToken(tokenID);
    }

    // set sh token for self
    MockNativeToken mock("foundation");

    int32_t ret = AccessTokenKit::DeleteToken(tokenID);
    // restore
    SetSelfTokenID(selfTokenId);
    return ret;
}

AccessTokenID PrivacyTestCommon::GetNativeTokenIdFromProcess(const std::string &process)
{
    uint64_t selfTokenId = GetSelfTokenID();
    SetSelfTokenID(PrivacyTestCommon::GetShellTokenId()); // set shell token

    std::string dumpInfo;
    AtmToolsParamInfo info;
    info.processName = process;
    AccessTokenKit::DumpTokenInfo(info, dumpInfo);
    size_t pos = dumpInfo.find("\"tokenID\": ");
    if (pos == std::string::npos) {
        return 0;
    }
    pos += std::string("\"tokenID\": ").length();
    std::string numStr;
    while (pos < dumpInfo.length() && std::isdigit(dumpInfo[pos])) {
        numStr += dumpInfo[pos];
        ++pos;
    }
    // restore
    SetSelfTokenID(selfTokenId);

    std::istringstream iss(numStr);
    AccessTokenID tokenID;
    iss >> tokenID;
    return tokenID;
}

// need call by native process
AccessTokenIDEx PrivacyTestCommon::GetHapTokenIdFromBundle(
    int32_t userID, const std::string& bundleName, int32_t instIndex)
{
    uint64_t selfTokenId = GetSelfTokenID();
    ATokenTypeEnum type = AccessTokenKit::GetTokenTypeFlag(static_cast<AccessTokenID>(selfTokenId));
    if (type != TOKEN_NATIVE) {
        AccessTokenID tokenId1 = GetNativeTokenIdFromProcess("privacy_service");
        SetSelfTokenID(tokenId1);
    }
    AccessTokenIDEx tokenIdEx = AccessTokenKit::GetHapTokenIDEx(userID, bundleName, instIndex);

    SetSelfTokenID(selfTokenId);
    return tokenIdEx;
}

MockNativeToken::MockNativeToken(const std::string& process)
{
    selfToken_ = GetSelfTokenID();
    uint32_t tokenId = PrivacyTestCommon::GetNativeTokenIdFromProcess(process);
    SetSelfTokenID(tokenId);
}

MockNativeToken::~MockNativeToken()
{
    SetSelfTokenID(selfToken_);
}

MockHapToken::MockHapToken(
    const std::string& bundle, const std::vector<std::string>& reqPerm, bool isSystemApp)
{
    selfToken_ = GetSelfTokenID();
    HapInfoParams infoParams = {
        .userID = 0,
        .bundleName = bundle,
        .instIndex = 0,
        .appIDDesc = "AccessTokenTestAppID",
        .apiVersion = PrivacyTestCommon::DEFAULT_API_VERSION,
        .isSystemApp = isSystemApp,
        .appDistributionType = "",
    };

    HapPolicyParams policyParams = {
        .apl = APL_NORMAL,
        .domain = "accesstoken_test_domain",
    };
    for (size_t i = 0; i < reqPerm.size(); ++i) {
        PermissionDef permDefResult;
        if (AccessTokenKit::GetDefPermission(reqPerm[i], permDefResult) != RET_SUCCESS) {
            continue;
        }
        PermissionStateFull permState = {
            .permissionName = reqPerm[i],
            .isGeneral = true,
            .resDeviceID = {"local3"},
            .grantStatus = {PermissionState::PERMISSION_DENIED},
            .grantFlags = {PermissionFlag::PERMISSION_DEFAULT_FLAG}
        };
        policyParams.permStateList.emplace_back(permState);
        policyParams.aclRequestedList.emplace_back(reqPerm[i]);
    }

    AccessTokenIDEx tokenIdEx = PrivacyTestCommon::AllocTestHapToken(infoParams, policyParams);
    mockToken_= tokenIdEx.tokenIdExStruct.tokenID;
    SetSelfTokenID(tokenIdEx.tokenIDEx);
    AccessTokenKit::GrantPermission(tokenIdEx.tokenIdExStruct.tokenID, ACCESS_LOCATION, PermissionFlag::PERMISSION_DEFAULT_FLAG);
    AccessTokenKit::GrantPermission(tokenIdEx.tokenIdExStruct.tokenID, ACCESS_APPROXIMATELY_LOCATION, PermissionFlag::PERMISSION_DEFAULT_FLAG);
    AccessTokenKit::GrantPermission(tokenIdEx.tokenIdExStruct.tokenID, ACCESS_APPROXIMATELY_LOCATION, PermissionFlag::PERMISSION_DEFAULT_FLAG);
}

MockHapToken::~MockHapToken()
{
    if (mockToken_ != INVALID_TOKENID) {
        PrivacyTestCommon::DeleteTestHapToken(mockToken_);
    }
    SetSelfTokenID(selfToken_);
}
}  // namespace Location
}  // namespace OHOS