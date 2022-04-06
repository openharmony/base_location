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

#ifndef LOCATION_LOCATOREVENTMANAGER_H
#define LOCATION_LOCATOREVENTMANAGER_H

#include <iostream>
#include <map>
#include <singleton.h>
#include <string>

#include "event_handler.h"
#include "request.h"

namespace OHOS {
namespace Location {
constexpr uint32_t EVENT_SEND_HIVIEW_MESSAGE = 0x0001;
constexpr uint32_t EVENT_SEND_DAILY_REPORT = 0x0002;

constexpr uint32_t DAILY_INTERVAL = 24 * 60 * 60 * 1000;
constexpr uint32_t COUNT_MAX = 500;
constexpr uint32_t INVALID_VALUE = 0xFFFFFFFF;
constexpr uint32_t DFT_APP_MAX_SIZE = 10;
constexpr uint32_t DFT_TOP_REQUEST_UPLOAD_MAX_SIZE = 5;

struct AppRequestCount {
    std::string packageName;
    std::vector<int> requestType;
    std::vector<int> count;
};

class DftEvent {
public:
    void PutInt(const std::string& name, int value);
    int GetInt(const std::string& name);
private:
    std::map<std::string, int> intValues_;
};

class DftHandler : public AppExecFwk::EventHandler {
public:
    explicit DftHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~DftHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
};

class LocatorDftManager : DelayedSingleton<LocatorDftManager> {
public:
    LocatorDftManager();
    ~LocatorDftManager();
    void Init();
    void IpcCallingErr(int error);
    void LocationSessionStart(std::shared_ptr<Request> request);
    void DistributionSessionStart();
    void DistributionDisconnect();
    void SendDistributionDailyCount();
    void SendRequestDailyCount();
private:
    std::shared_ptr<AppRequestCount> GetTopRequest();

    std::shared_ptr<DftHandler> handler_;
    std::list<std::shared_ptr<AppRequestCount>> appRequests_;
    uint32_t distributeSissionCnt_;
    uint32_t distributeDisconnectCnt_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_LOCATOREVENTMANAGER_H
