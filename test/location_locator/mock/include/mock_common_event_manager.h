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

#ifndef BASE_LOCATION_MOCK_COMMON_EVENT_MANAGER_H
#define BASE_LOCATION_MOCK_COMMON_EVENT_MANAGER_H

#include "gmock/gmock.h"

#include "common_event_manager.h"

namespace OHOS {
namespace Location {
class MockCommonEventManager {
public:
    MOCK_METHOD(bool, PublishCommonEvent, (const CommonEventData &data));
    MOCK_METHOD(bool, PublishCommonEvent, (const CommonEventData &data,
        const CommonEventPublishInfo &publishInfo));
    MOCK_METHOD(bool, PublishCommonEvent, (const CommonEventData &data,
        const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber));
    MOCK_METHOD(bool, PublishCommonEvent, (const CommonEventData &data,
        const CommonEventPublishInfo &publishInfo, const std::shared_ptr<CommonEventSubscriber> &subscriber));
    MOCK_METHOD(bool, PublishCommonEvent, (const CommonEventData &data, const CommonEventPublishInfo &publishInfo,
        const std::shared_ptr<CommonEventSubscriber> &subscriber, const uid_t &uid));
    MOCK_METHOD(bool, SubscribeCommonEvent, (const std::shared_ptr<CommonEventSubscriber> &subscriber));
    MOCK_METHOD(bool, UnSubscribeCommonEvent, (const std::shared_ptr<CommonEventSubscriber> &subscriber));
    MOCK_METHOD(bool, GetStickyCommonEvent, (const std::string &event, CommonEventData &data));
    static MockCommonEventManager &GetInstance(void);

private:
    MockCommonEventManager() {}
    ~MockCommonEventManager() {}
};
} // namespace Location
} // namespace OHOS
#endif
