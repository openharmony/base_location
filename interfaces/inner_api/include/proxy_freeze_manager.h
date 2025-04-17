/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef PROXY_FREEZE_MANAGER_H
#define PROXY_FREEZE_MANAGER_H

#include <string>
#include <mutex>
#include <set>

#include "location_log.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {

class ProxyFreezeManager {
public:
    static ProxyFreezeManager* GetInstance();
    void ProxyForFreeze(std::set<int> pidList, bool isProxy);
    void ResetAllProxy();
    bool IsProxyPid(int32_t pid);
private:
    std::mutex proxyPidsMutex_;
    std::set<int32_t> proxyPids_;
};
} // namespace Location
} // namespace OHOS
#endif // PROXY_FREEZE_MANAGER_H
