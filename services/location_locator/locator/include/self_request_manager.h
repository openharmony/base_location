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

#ifndef SELF_REQUEST_MANAGER_H
#define SELF_REQUEST_MANAGER_H

#include <map>
#include <singleton.h>
#include <string>
#include "event_handler.h"
#include "event_runner.h"

#include "i_locator_callback.h"
#include "request.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Location {
class SelfRequestManagerHandler : public AppExecFwk::EventHandler {
public:
    using SelfRequestManagerEventHandle = std::function<void(const AppExecFwk::InnerEvent::Pointer &)>;
    using SelfRequestManagerEventHandleMap = std::map<int, SelfRequestManagerEventHandle>;
    explicit SelfRequestManagerHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~SelfRequestManagerHandler() override;
private:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    void InitSelfRequestManagerHandlerEventMap();
    void StartLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event);
    void StopLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event);

    SelfRequestManagerEventHandleMap selfRequestManagerHandlerEventMap_;
};

class SelfRequestManager {
public:
    static SelfRequestManager* GetInstance();
    SelfRequestManager();
    ~SelfRequestManager();
    void StartSelfRequest();
    void StopSelfRequest();
    void StartLocatorThread();
    void StopLocatorThread();
    class mLocatorCallback : public IRemoteStub<ILocatorCallback> {
    public:
        void OnLocationReport(const std::unique_ptr<Location>& location);
        void OnLocatingStatusChange(const int status);
        void OnErrorReport(const int errorCode);
    };

    bool isLocating_ = false;
    bool proxySwtich_ = false;
    sptr<ILocatorCallback> callback_;
    std::shared_ptr<Request> request_;
    static std::mutex locatorMutex_;
    std::shared_ptr<SelfRequestManagerHandler> selfRequestManagerHandler_;
};
} // namespace Location
} // namespace OHOS
#endif // SELF_REQUEST_MANAGER_H
