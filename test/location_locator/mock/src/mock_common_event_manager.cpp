/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "mock_common_event_manager.h"

namespace OHOS {
namespace EventFwk {
bool CommonEventManager::PublishCommonEvent(const CommonEventData &data)
{
    return Location::MockCommonEventManager::GetInstance().PublishCommonEvent(data);
}

bool CommonEventManager::PublishCommonEvent(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo)
{
    return Location::MockCommonEventManager::GetInstance().PublishCommonEvent(data, publishInfo);
}

bool CommonEventManager::PublishCommonEvent(const CommonEventData &data,
    const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return Location::MockCommonEventManager::GetInstance().PublishCommonEvent(data, publishInfo, subscriber);
}

bool CommonEventManager::PublishCommonEvent(const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
    const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid)
{
    return Location::MockCommonEventManager::GetInstance().PublishCommonEvent(data, publishInfo, subscriber, uid);
}

bool CommonEventManager::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return Location::MockCommonEventManager::GetInstance().SubscribeCommonEvent(subscriber);
}

bool CommonEventManager::UnSubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    return Location::MockCommonEventManager::GetInstance().UnSubscribeCommonEvent(subscriber);
}

bool CommonEventManager::GetStickyCommonEvent(const std::string &event, CommonEventData &data)
{
    return Location::MockCommonEventManager::GetInstance().GetStickyCommonEvent(event, data);
}
} // namespace EventFwk

namespace Location {
MockCommonEventManager &MockCommonEventManager::GetInstance()
{
    static MockCommonEventManager gMockCommonEventManager;
    return gMockCommonEventManager;
};
} // namespace Location
} // namespace OHOS
