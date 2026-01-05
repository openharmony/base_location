/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
 
  
#ifndef LOCATION_ACCOUNT_MANAGER_H
#define LOCATION_ACCOUNT_MANAGER_H

#include <map>
#include <singleton.h>
#include <string>
#include "application_state_observer_stub.h"
#include "common_event_subscriber.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Location {

class LocationAccountManager {
public:
    LocationAccountManager();
    ~LocationAccountManager();
    std::vector<int> getActiveUserIds();
    void OnUserSwitch(int32_t userId);
    void OnUserRemove(int32_t userId);
    static LocationAccountManager* GetInstance();

private:
    std::vector<int> activeIds_;
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
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_DATA_MANAGER_H
