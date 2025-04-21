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

void ProxyFreezeManager::ProxyForFreeze(std::set<int> pidList, bool isProxy)
{
    std::unique_lock<std::mutex> lock(proxyPidsMutex_);
    if (isProxy) {
        for (auto it = pidList.begin(); it != pidList.end(); it++) {
            proxyPids_.insert(*it);
            LBSLOGI(LOCATOR, "Start locator proxy, pid: %{public}d, isProxy: %{public}d, timestamp = %{public}s",
                *it, isProxy, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
        }
    } else {
        for (auto it = pidList.begin(); it != pidList.end(); it++) {
            proxyPids_.erase(*it);
            LBSLOGI(LOCATOR, "Start locator proxy, pid: %{public}d, isProxy: %{public}d, timestamp = %{public}s",
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
