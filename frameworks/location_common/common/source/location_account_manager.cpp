/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "location_account_manager.h"
#include <vector>
#include <algorithm>
#include <mutex>
#include "common_utils.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"


namespace OHOS {
namespace Location {
LocationAccountManager* LocationAccountManager::GetInstance()
{
    static LocationAccountManager manager;
    return &manager;
}

LocationAccountManager::LocationAccountManager()
{
    SubscribeSaStatusChangeListerner();
    isUserSwitchSubscribed_ = LocationAccountManager::UserSwitchSubscriber::Subscribe();
}

LocationAccountManager::~LocationAccountManager()
{
}

std::vector<int> LocationAccountManager::getActiveUserIds()
{
    return activeIds_;
}

void LocationAccountManager::OnUserSwitch(int32_t userId)
{
    bool containsActiveId = std::find(activeIds_.begin(), activeIds_.end(), userId) != activeIds_.end();
    if (!containsActiveId) {
        activeIds_.push_back(userId);
    }
}

void LocationAccountManager::OnUserRemove(int32_t userId)
{
    auto iter = std::find(activeIds_.begin(), activeIds_.end(), userId);
    if (iter != activeIds_.end()) {
        activeIds_.erase(iter);
    }
}

void LocationAccountManager::SubscribeSaStatusChangeListerner()
{
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    if (subscriber_ == nullptr) {
        subscriber_ = std::make_shared<UserSwitchSubscriber>(subscriberInfo);
    }
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    statusChangeListener_ = new (std::nothrow) SystemAbilityStatusChangeListener(subscriber_);
    if (samgrProxy == nullptr || statusChangeListener_ == nullptr) {
        LBSLOGE(ACCOUNT_MANAGER,
            "SubscribeSaStatusChangeListerner samgrProxy or statusChangeListener_ is nullptr");
        return;
    }
    int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    LBSLOGI(ACCOUNT_MANAGER,
        "SubscribeSaStatusChangeListerner SubscribeSystemAbility COMMON_EVENT_SERVICE_ID result:%{public}d", ret);
}

LocationAccountManager::UserSwitchSubscriber::UserSwitchSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &info)
    : CommonEventSubscriber(info)
{
    LBSLOGD(ACCOUNT_MANAGER, "create UserSwitchEventSubscriber");
}

void LocationAccountManager::UserSwitchSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData& event)
{
    int32_t userId = event.GetCode();
    const auto action = event.GetWant().GetAction();
    auto accountManager = LocationAccountManager::GetInstance();
    LBSLOGD(ACCOUNT_MANAGER, "action = %{public}s, userId = %{public}d", action.c_str(), userId);
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        accountManager->OnUserSwitch(userId);
    } else if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        accountManager->OnUserRemove(userId);
    }
}

bool LocationAccountManager::UserSwitchSubscriber::Subscribe()
{
    LBSLOGD(ACCOUNT_MANAGER, "subscribe common event");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<UserSwitchSubscriber> subscriber = std::make_shared<UserSwitchSubscriber>(subscriberInfo);
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    if (result) {
    } else {
        LBSLOGE(ACCOUNT_MANAGER, "Subscribe service event error.");
    }
    return result;
}

LocationAccountManager::SystemAbilityStatusChangeListener::SystemAbilityStatusChangeListener(
    std::shared_ptr<UserSwitchSubscriber> &subscriber) : subscriber_(subscriber)
{}

void LocationAccountManager::SystemAbilityStatusChangeListener::OnAddSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(ACCOUNT_MANAGER, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (subscriber_ == nullptr) {
        LBSLOGE(ACCOUNT_MANAGER, "OnAddSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    LBSLOGI(ACCOUNT_MANAGER, "SubscribeCommonEvent subscriber_ result = %{public}d", result);
}

void LocationAccountManager::SystemAbilityStatusChangeListener::OnRemoveSystemAbility(
    int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(ACCOUNT_MANAGER, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    if (subscriber_ == nullptr) {
        LBSLOGE(ACCOUNT_MANAGER, "OnRemoveSystemAbility subscribeer is nullptr");
        return;
    }
    bool result = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    LBSLOGE(ACCOUNT_MANAGER, "UnSubscribeCommonEvent subscriber_ result = %{public}d", result);
}

} // namespace Location
} // namespace OHOS