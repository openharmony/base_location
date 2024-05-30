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

#include "locator_event_manager.h"

#include <vector>

#include "constant_definition.h"

namespace OHOS {
namespace Location {
void DftEvent::PutInt(const std::string& name, int value)
{
    std::unique_lock<std::mutex> lock(mutex_);
    intValues_.emplace(name, value);
}

int DftEvent::GetInt(const std::string& name)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = intValues_.find(name);
    if (it == intValues_.end()) {
        return 0;
    }
    return it->second;
}

DftHandler::DftHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

DftHandler::~DftHandler() {}

void DftHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
}

LocatorDftManager* LocatorDftManager::GetInstance()
{
    static LocatorDftManager data;
    return &data;
}

LocatorDftManager::LocatorDftManager()
{
    distributeSissionCnt_ = 0;
    distributeDisconnectCnt_ = 0;

    handler_ = std::make_shared<DftHandler>(AppExecFwk::EventRunner::Create(true));
}

LocatorDftManager::~LocatorDftManager() {}

void LocatorDftManager::Init()
{
    if (handler_ != nullptr) {
        handler_->SendHighPriorityEvent(EVENT_SEND_DAILY_REPORT, 0, DAILY_INTERVAL);
    }
}

void LocatorDftManager::IpcCallingErr(int error)
{
}

void LocatorDftManager::LocationSessionStart(std::shared_ptr<Request> request)
{
    std::shared_ptr<AppRequestCount> requestCount = nullptr;
    if (request == nullptr) {
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    for (auto appRequest : appRequests_) {
        if (appRequest->packageName.compare(request->GetPackageName()) == 0) {
            requestCount = appRequest;
            break;
        }
    }
    if (requestCount == nullptr) {
        requestCount = std::make_shared<AppRequestCount>();
        requestCount->packageName = request->GetPackageName();
        requestCount->requestType = std::vector<int>(DFT_APP_MAX_SIZE);
        requestCount->count = std::vector<int>(DFT_APP_MAX_SIZE);
        appRequests_.push_back(requestCount);
    }

    unsigned index = INVALID_VALUE;
    if (request->GetRequestConfig() == nullptr) {
        return;
    }
    int scenario = request->GetRequestConfig()->GetScenario();
    int priority = request->GetRequestConfig()->GetPriority();
    for (unsigned i = 0; i < requestCount->requestType.size(); i++) {
        if (requestCount->requestType.at(i) == scenario || requestCount->requestType.at(i) == priority) {
            index = i;
            break;
        }
    }
    if (index != INVALID_VALUE) {
        requestCount->count.at(index)++;
    } else {
        if (scenario != SCENE_UNSET) {
            requestCount->requestType.at(0) = scenario;
        } else if (priority != PRIORITY_UNSET) {
            requestCount->requestType.at(0) = priority;
        } else {
            return;
        }
        requestCount->count.at(0)++;
    }
}

void LocatorDftManager::DistributionDisconnect()
{
    if (distributeDisconnectCnt_ < COUNT_MAX) {
        distributeDisconnectCnt_++;
    }
}

void LocatorDftManager::DistributionSessionStart()
{
    if (distributeSissionCnt_ < COUNT_MAX) {
        distributeSissionCnt_++;
    }
}

void LocatorDftManager::SendDistributionDailyCount()
{
}

void LocatorDftManager::SendRequestDailyCount()
{
}

std::shared_ptr<AppRequestCount> LocatorDftManager::GetTopRequest()
{
    std::shared_ptr<AppRequestCount> topRequest;
    int topSum = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto appRequest : appRequests_) {
        int sum = 0;
        for (unsigned i = 0; i < appRequest->count.size(); i++) {
            sum = sum + appRequest->count.at(i);
        }
        if (sum > topSum) {
            topSum = sum;
            topRequest = appRequest;
        }
    }
    return topRequest;
}
} // namespace Location
} // namespace OHOS