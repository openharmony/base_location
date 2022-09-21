/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "app_identity.h"

namespace OHOS {
namespace Location {
AppIdentity::AppIdentity()
{
    uid_ = 0;
    pid_ = 0;
    tokenId_ = 0;
    firstTokenId_ = 0;
    bundleName_ = "";
}

AppIdentity::AppIdentity(pid_t uid, pid_t pid, uint32_t tokenId, uint32_t firstTokenId)
    : uid_(uid), pid_(pid), tokenId_(tokenId), firstTokenId_(firstTokenId)
{
}

std::string AppIdentity::ToString() const
{
    std::string str = "uid : " + std::to_string(uid_) +
        ", pid : " + std::to_string(pid_) +
        ", tokenId : " + std::to_string(tokenId_) +
        ", firstTokenId : " + std::to_string(firstTokenId_) +
        ", bundleName : " + bundleName_;
    return str;
}
} // namespace Location
} // namespace OHOS