/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef COMMON_EVENT_SUBSCRIBER_H
#define COMMON_EVENT_SUBSCRIBER_H
#ifdef FEATURE_GNSS_SUPPORT

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

namespace OHOS {
namespace Location {
using namespace EventFwk;
using CommonEventSubscriber = OHOS::EventFwk::CommonEventSubscriber;
using CommonEventData = OHOS::EventFwk::CommonEventData;
using CommonEventSubscribeInfo = OHOS::EventFwk::CommonEventSubscribeInfo;
constexpr const char* AGNSS_NI_ACCEPT_EVENT = "usual.event.AGNSS_NI_ACCEPT";
constexpr const char* AGNSS_NI_REJECT_EVENT = "usual.event.AGNSS_NI_REJECT";

class GnssCommonEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit GnssCommonEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    ~GnssCommonEventSubscriber() {}

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
#endif // GNSS_ABILITY_COMMON_EVENT_SUBSCRIBER_H