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

#include "app_mgr_interface.h"
#include "app_state_data.h"
#include "application_state_observer_stub.h"
#include "common_event_subscriber.h"
#include "system_ability_status_change_stub.h"

#include "i_locator_callback.h"
#include "request.h"

namespace OHOS {
namespace Location {
class AppStateChangeCallback : public AppExecFwk::ApplicationStateObserverStub {
public:
    AppStateChangeCallback();
    ~AppStateChangeCallback() override;

    void OnForegroundApplicationChanged(const AppExecFwk::AppStateData& appStateData) override;
};

class LocatorBackgroundProxy {
public:
    static LocatorBackgroundProxy* GetInstance();
    LocatorBackgroundProxy();
    ~LocatorBackgroundProxy();
    void UpdateListOnRequestChange(const std::shared_ptr<Request>& request);
    void OnSuspend(const std::shared_ptr<Request>& request, bool active);
    void OnSaStateChange(bool enable);
    void OnDeleteRequestRecord(const std::shared_ptr<Request>& request);
    bool IsCallbackInProxy(const sptr<ILocatorCallback>& callback) const;
    bool IsAppBackground(std::string bundleName);
    bool RegisterAppStateObserver();
    bool UnregisterAppStateObserver();
    bool IsAppInLocationContinuousTasks(pid_t uid, pid_t pid);
    bool IsAppHasFormVisible(uint32_t tokenId, uint64_t tokenIdEx);
    int32_t getCurrentUserId();
    bool IsAppBackground(int uid, std::string bundleName);
    void UpdateBackgroundAppStatues(int32_t uid, int32_t status);
private:
    void StartLocator();
    void StopLocator();
    void StartLocatorThread();
    void StopLocatorThread();
    void OnUserSwitch(int32_t userId);
    void OnUserRemove(int32_t userId);
    void UpdateListOnSuspend(const std::shared_ptr<Request>& request, bool active);
    void UpdateListOnUserSwitch(int32_t userId);
    void InitArgsFromProp();
    void SubscribeSaStatusChangeListerner();

    bool CheckPermission(const std::shared_ptr<Request>& request) const;
    bool CheckMaxRequestNum(int32_t uid, const std::string& packageName) const;
    int32_t GetUserId(int32_t uid) const;
    const std::list<std::shared_ptr<Request>>& GetRequestsInProxy() const;

    class mLocatorCallback : public IRemoteStub<ILocatorCallback> {
    public:
        void OnLocationReport(const std::unique_ptr<Location>& location);
        void OnLocatingStatusChange(const int status);
        void OnErrorReport(const int errorCode);
    };

    class UserSwitchSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit UserSwitchSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info);
        ~UserSwitchSubscriber() override = default;
        static bool Subscribe();
    private:
        void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &event) override;
    };

    class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
    public:
        explicit SystemAbilityStatusChangeListener(std::shared_ptr<UserSwitchSubscriber> &subscriber);
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    private:
        std::shared_ptr<UserSwitchSubscriber> subscriber_ = nullptr;
    };

    bool isLocating_ = false;
    bool proxySwtich_ = false;
    bool featureSwitch_ = true;
    bool isWating_ = false;
    bool isUserSwitchSubscribed_ = false;
    int timeInterval_;
    int32_t curUserId_ = 0;

    sptr<ILocatorCallback> callback_;
    std::shared_ptr<Request> request_;
    std::shared_ptr<UserSwitchSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;
    std::shared_ptr<std::map<int32_t, std::shared_ptr<std::list<std::shared_ptr<Request>>>>> requestsMap_;
    std::shared_ptr<std::list<std::shared_ptr<Request>>> requestsList_;
    static std::mutex requestListMutex_;
    static std::mutex locatorMutex_;
    static std::mutex foregroundAppMutex_;
    std::map<int32_t, int32_t> foregroundAppMap_;
    sptr<AppExecFwk::IAppMgr> iAppMgr_ = nullptr;
    sptr<AppStateChangeCallback> appStateObserver_ = nullptr;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_BACKGROUND_PROXY_H
