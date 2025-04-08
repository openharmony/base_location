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

#include "proxy_freeze_manager.h"


namespace OHOS {
namespace Location {

ProxyFreezeManager* ProxyFreezeManager::GetInstance()
{
    static ProxyFreezeManager data;
    return &data;
}

void ProxyFreezeManager::ProxyForFreeze(const std::vector<int32_t>& pidList, bool isProxy)
{
    size_t size = pidList.size();
    size = size > MAX_BUFF_SIZE ? MAX_BUFF_SIZE : size;
    std::set<int> pids;
    for (size_t i = 0; i < size; i++) {
        pids.insert(pidList[i]);
    }
    std::unique_lock<std::mutex> lock(proxyPidsMutex_);
    if (isProxy) {
        for (auto it = pids.begin(); it != pids.end(); it++) {
            proxyPids_.insert(*it);
            LBSLOGI(PROXY_FREEZE_MANAGER,
                "Start locator proxy, pid: %{public}d, isProxy: %{public}d, timestamp = %{public}s",
                *it, isProxy, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
        }
    } else {
        for (auto it = pids.begin(); it != pids.end(); it++) {
            proxyPids_.erase(*it);
            LBSLOGI(PROXY_FREEZE_MANAGER,
                "Start locator proxy, pid: %{public}d, isProxy: %{public}d, timestamp = %{public}s",
                *it, isProxy, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
        }
    }
}

void ProxyFreezeManager::ResetAllProxy()
{
    std::unique_lock<std::mutex> lock(proxyPidsMutex_);
    proxyPids_.clear();
}

bool ProxyFreezeManager::IsProxyPid(int32_t pid)
{
    std::unique_lock<std::mutex> lock(proxyPidsMutex_);
    return proxyPids_.find(pid) != proxyPids_.end();
}
} // namespace Location
} // namespace OHOS
