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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_common_event_subscriber.h"

#include "agnss_ni_manager.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
static constexpr int32_t DAT_MSG_RECEIVE_CODE = 1;
void GnssCommonEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    LBSLOGD(GNSS, "OnReceiveEvent");
    auto want = data.GetWant();
    std::string action = want.GetAction();

    auto agnssNiManager = AGnssNiManager::GetInstance();
    if (action == CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED && data.GetCode() == DAT_MSG_RECEIVE_CODE) {
        LBSLOGI(GNSS, "[AGNSS NI]:receive sms msg");
        agnssNiManager->CheckSmsSuplInit(want);
    } else if (action == CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED) {
        LBSLOGI(GNSS, "[AGNSS NI]:receive wap push msg");
        agnssNiManager->CheckWapSuplInit(want);
    } else if (action == CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED) {
        LBSLOGD(GNSS, "[AGNSS NI]:receive call state msg");
        agnssNiManager->OnCallStateChanged(want);
    } else if (action == AGNSS_NI_ACCEPT_EVENT) {
        LBSLOGI(GNSS, "[AGNSS NI]:receive user accept event");
        agnssNiManager->SendUserResponse(GNSS_NI_RESPONSE_CMD_ACCEPT);
    } else if (action == AGNSS_NI_REJECT_EVENT) {
        LBSLOGI(GNSS, "[AGNSS NI]:receive user reject event");
        agnssNiManager->SendUserResponse(GNSS_NI_RESPONSE_CMD_REJECT);
    }
}
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
