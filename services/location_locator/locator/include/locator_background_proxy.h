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

#ifndef LOCATOR_BACKGROUND_PROXY_H
#define LOCATOR_BACKGROUND_PROXY_H

#include <map>
#include <singleton.h>
#include <string>
#include "common_event_subscriber.h"
#include "iremote_stub.h"
#include "i_locator_callback.h"
#include "request.h"

namespace OHOS {
namespace Location {
class LocatorBackgroundProxy : DelayedSingleton<LocatorBackgroundProxy> {
public:
    LocatorBackgroundProxy();
    ~LocatorBackgroundProxy();
    void OnSuspend(const std::shared_ptr<Request>& request, bool active);
    void OnPermissionChanged(int32_t uid);
    void OnSaStateChange(bool enable);
    void OnDeleteRequestRecord(const std::shared_ptr<Request>& request);
    bool IsCallbackInProxy(const sptr<ILocatorCallback>& callback) const;

private:
    void StartLocator();
    void StopLocator();
    void StartLocatorThread();
    void OnUserSwitch(int32_t userId);
    void OnUserRemove(int32_t userId);
    void UpdateListOnPermissionChanged(int32_t uid);
    void UpdateListOnSuspend(const std::shared_ptr<Request>& request, bool active);
    void UpdateListOnUserSwitch(int32_t userId);
    void SubscribeUserSwtich();
    void SubscribeUserSwtichThread();
    void InitArgsFromProp();

    bool CheckPermission() const;
    bool CheckMaxRequestNum(int32_t uid, const std::string& packageName) const;
    int32_t GetUserId(int32_t uid) const;
    const std::list<std::shared_ptr<Request>>& GetRequestsInProxy() const;

    bool isLocating_ = false;
    bool proxySwtich_ = false;
    bool featureSwitch_ = true;
    bool isWating_ = false;
    bool isSubscribed_ = false;
    int timeInterval_;
    int32_t curUserId_ = 0;

    sptr<ILocatorCallback> callback_;
    std::shared_ptr<Request> request_;
    std::shared_ptr<std::map<int32_t, std::shared_ptr<std::list<std::shared_ptr<Request>>>>> requestsMap_;
    std::shared_ptr<std::list<std::shared_ptr<Request>>> requestsList_;
    static std::mutex requestListMutex_;
    static std::mutex locatorMutex_;

    class mLocatorCallback : public IRemoteStub<ILocatorCallback> {
    public:
        void OnLocationReport(const std::unique_ptr<Location>& location);
        void OnLocatingStatusChange(const int status);
        void OnErrorReport(const int errorCode);
    };

    class UserSwitchSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit UserSwitchSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info);
        virtual ~UserSwitchSubscriber() override = default;
        static bool Subscribe();
    private:
        virtual void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &event) override;
    };
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_BACKGROUND_PROXY_H
