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

#ifndef AGNSS_NI_MANAGER_H
#define AGNSS_NI_MANAGER_H
#ifdef FEATURE_GNSS_SUPPORT

#include <atomic>
#include <mutex>
#include <singleton.h>
#include <v2_0/ignss_interface.h>

#include "common_utils.h"
#include "constant_definition.h"
#include "event_handler.h"
#include "gnss_common_event_subscriber.h"
#include "subability_common.h"
#include "system_ability_status_change_stub.h"
#ifdef NET_MANAGER_ENABLE
#include "net_conn_callback_stub.h"
#endif

namespace OHOS {
namespace Location {

enum AGPS_STATUS {
    AGPS_DATA_CONNECTION_CLOSED = 0,
    AGPS_DATA_CONNECTION_OPENING,
    AGPS_DATA_CONNECTION_OPEN
}

enum AGPS_DATA_STATUS {
    GPS_REQUEST_AGPS_DATA_CONN = 0,
    GPS_RELEASE_AGPS_DATA_CONN,
    GPS_AGPS_DATA_CONNECTED,
    GPS_AGPS_DATA_CONN_DONE,
    GPS_AGPS_DATA_CONN_FAILED
}

using HDI::Location::Gnss::V2_0::IGnssInterface;
using HDI::Location::Gnss::V2_0::GnssNiNotificationRequest;
using HDI::Location::Gnss::V2_0::GnssNiRequestCategory;
using HDI::Location::Gnss::V2_0::GnssNiResponseCmd;
using HDI::Location::Gnss::V2_0::GNSS_NI_NOTIFICATION_REQUIRE_NOTIFY;
using HDI::Location::Gnss::V2_0::GNSS_NI_NOTIFICATION_REQUIRE_VERIFY;
using HDI::Location::Gnss::V2_0::GNSS_NI_NOTIFICATION_REQUIRE_PRIVACY_OVERRIDE;
using HDI::Location::Gnss::V2_0::GNSS_NI_RESPONSE_CMD_ACCEPT;
using HDI::Location::Gnss::V2_0::GNSS_NI_RESPONSE_CMD_REJECT;
using HDI::Location::Gnss::V2_0::GNSS_NI_RESPONSE_CMD_NO_RESPONSE;
using HDI::Location::Gnss::V2_0::GNSS_NI_ENCODING_FORMAT_NULL;
using HDI::Location::Gnss::V2_0::GNSS_NI_ENCODING_FORMAT_SUPL_GSM_DEFAULT;
using HDI::Location::Gnss::V2_0::GNSS_NI_ENCODING_FORMAT_SUPL_UCS2;
using HDI::Location::Gnss::V2_0::GNSS_NI_ENCODING_FORMAT_SUPL_UTF8;

class SystemAbilityStatusChangeListener : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};

#ifdef NET_MANAGER_ENABLE
class AGnssNiNetworkCallbackEs : public NetManagerStandard::NetConnCallbackStub {
public:
    int32_t NetConnectionPropertiesChange(
        sptr<NetManagerStandard::NetHandle> &netHandle, const sptr<NetManagerStandard::NetLinkInfo> &info) override;
    int32_t NetLost(sptr<NetManagerStandard::NetHandle> &netHandle) override;
    int32_t NetUnavailable() override;
    std::string host_;
};

class AGnssNiNetworkCallback : public NetManagerStandard::NetConnCallbackStub {
public:
    int32_t NetConnectionPropertiesChange(
        sptr<NetManagerStandard::NetHandle> &netHandle, const sptr<NetManagerStandard::NetLinkInfo> &info) override;
    int32_t NetLost(sptr<NetManagerStandard::NetHandle> &netHandle) override;
    int32_t NetUnavailable() override;
    std::string host_;
};
#endif

class AGnssNiManager {
public:
    AGnssNiManager();
    ~AGnssNiManager();
    void Run();
    void SubscribeSaStatusChangeListerner();
    void RegisterAgnssNiEvent();
    void UnRegisterAgnssNiEvent();
    void RegisterNiResponseEvent();
    void UnRegisterNiResponseEvent();
    void CheckWapSuplInit(const EventFwk::Want &want);
    void CheckSmsSuplInit(const EventFwk::Want &want);
    void OnCallStateChanged(const EventFwk::Want &want);
    void HandleNiNotification(const GnssNiNotificationRequest &notif);
    void SendUserResponse(GnssNiResponseCmd responseCmd);
    static AGnssNiManager* GetInstance();

private:
    void AgnssNiSuplInit();
    bool IsInEmergency();
    std::string BuildStartCommand(const GnssNiNotificationRequest &notif);
    void OpenNiDialog(const GnssNiNotificationRequest &notif);
    void SendNiNotification(const GnssNiNotificationRequest &notif);
    std::string DecodeNiString(std::string original, int coding);

    bool isInEmergencyCall_ = false;
    std::atomic<short> niNotificationId_;
    int64_t emergencyCallEndTime_ = 0;
    std::shared_ptr<GnssCommonEventSubscriber> subscriber_ = nullptr;
    std::shared_ptr<GnssCommonEventSubscriber> niResponseSubscriber_ = nullptr;
    sptr<SystemAbilityStatusChangeListener> statusChangeListener_ = nullptr;
    std::mutex callStateMutex_;
};
} // namespace Location
} // namespace OHOS

#endif // FEATURE_GNSS_SUPPORT
#endif // AGNSS_NI_MANAGER_H
