/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
 
  
#ifndef BACKGROUND_MANAGER_H
#define BACKGROUND_MANAGER_H

#include <map>
#include <singleton.h>
#include <string>
#include "app_mgr_interface.h"
#include "application_state_observer_stub.h"
#include "common_event_subscriber.h"
#include "system_ability_status_change_stub.h"


namespace OHOS {
namespace Location {

class BackgroundManager {
public:
    BackgroundManager();
    ~BackgroundManager();
    static BackgroundManager* GetInstance();
    bool IsAppBackground(std::string bundleName);
    bool IsAppBackground(int uid, std::string bundleName);
    bool IsAppInLocationContinuousTasks(pid_t uid, pid_t pid);
    bool IsAppHasFormVisible(uint32_t tokenId, uint64_t tokenIdEx);
    void UpdateBackgroundAppStatues(int32_t uid, int32_t status);
private:
    void SubscribeSaStatusChangeListerner();

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
    bool isUserSwitchSubscribed_ = false;
    std::shared_ptr<UserSwitchSubscriber> subscriber_ = nullptr;
    sptr<ISystemAbilityStatusChange> statusChangeListener_ = nullptr;
    static std::mutex foregroundAppMutex_;
    std::map<int32_t, int32_t> foregroundAppMap_;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_DATA_MANAGER_H
