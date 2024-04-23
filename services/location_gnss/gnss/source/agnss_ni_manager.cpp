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

#include "agnss_ni_manager.h"

#include "call_manager_client.h"
#include "gnss_ability.h"
#include "idevmgr_hdi.h"
#include "iservice_registry.h"
#include "location_data_rdb_manager.h"
#include "location_log.h"
#include "nlohmann/json.hpp"
#include "notification_helper.h"
#include "securec.h"
#include "sms_service_manager_client.h"
#include "string_utils.h"
#include "system_ability_definition.h"
#include "ui_extension_ability_connection.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"

namespace OHOS {
namespace Location {
using namespace EventFwk;
constexpr uint32_t MAX_RETRY_TIMES = 3;
constexpr uint32_t TIME_AFTER_EMERGENCY_CALL = 10 * 1000;
constexpr int32_t INVALID_SUBID = -1;
const std::string URN_APPLICATION_ID = "x-oma-application:ulp.ua";
const std::string AGNSS_NI_SERVICE_NAME = "agnss_ni";
const int32_t GNSS_AGNSS_NI_NOTIFICATION_ID = LOCATION_GNSS_SA_ID * 100;
const std::string LOCATION_DIALOG_BUNDLE_NAME = "com.ohos.locationdialog";
const std::string AGNSS_NI_DIALOG_ABILITY_NAME = "ConfirmUIExtAbility";
constexpr uint32_t NOTIFICATION_AUTO_DELETED_TIME = 1000;

AGnssNiManager::AGnssNiManager()
{}

AGnssNiManager::~AGnssNiManager()
{}

void AGnssNiManager::SubscribeSaStatusChangeListerner()
{
    LBSLOGI(GNSS, "AGNSS-NI SubscribeSaStatusChangeListerner");
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LBSLOGE(GNSS, "failed to get samgrProxy");
        return;
    }
    statusChangeListener_ = new SystemAbilityStatusChangeListener();
    if (statusChangeListener_ == nullptr) {
        LBSLOGE(GNSS, "statusChangeListener_ is nullptr!");
        return;
    }
    int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        LBSLOGE(GNSS, "subscribe systemAbilityId: call manager service failed!");
        return;
    }
}

void AGnssNiManager::RegisterAgnssNiEvent()
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_WAPPUSH_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SMS_RECEIVE_COMPLETED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);
    matchingSkills.AddEvent(AGNSS_NI_ACCEPT_EVENT);
    matchingSkills.AddEvent(AGNSS_NI_REJECT_EVENT);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<GnssCommonEventSubscriber>(subscriberInfo);

    uint32_t count = 0;
    bool result = false;
    while (!result && count <= MAX_RETRY_TIMES) {
        result = CommonEventManager::SubscribeCommonEvent(subscriber_);
        count++;
    }
    if (count > MAX_RETRY_TIMES || !result) {
        LBSLOGE(GNSS, "Failed to subscriber gnss event");
    } else {
        LBSLOGI(GNSS, "success to subscriber gnss event");
    }
}

void AGnssNiManager::Run()
{
    LBSLOGI(GNSS, "AGNSS-NI: Run");
    RegisterAgnssNiEvent();
    gnssInterface_ = HDI::Location::Gnss::V2_0::IGnssInterface::Get();
    if (gnssInterface_ == nullptr) {
        auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
        if (gnssAbility == nullptr) {
            LBSLOGE(GNSS, "AGNSS-NI: gnss ability is nullptr");
            return;
        }
        if (!gnssAbility->CheckIfHdiConnected()) {
            gnssAbility->ConnectHdi();
            gnssInterface_ = HDI::Location::Gnss::V2_0::IGnssInterface::Get();
        }
    }
}

void AGnssNiManager::UnRegisterAgnssNiEvent()
{
    if (subscriber_ == nullptr) {
        LBSLOGE(GNSS, "UnRegisterAgnssNiEvent subscriber_ is null");
        return;
    }
    bool result = CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    subscriber_ = nullptr;
    LBSLOGI(GNSS, "unSubscriber gnss event, result = %{public}d", result);
}

void AGnssNiManager::AgnssNiSuplInit()
{
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility != nullptr) {
        gnssAbility->SetAgnssServer();
    }
#endif
}

#ifdef SMS_MMS_ENABLE
static bool IsFromDefaultSubId(const OHOS::EventFwk::Want &want)
{
    int32_t subId = want.GetIntParam("slotId", INVALID_SUBID);
    auto msgManager = DelayedSingleton<Telephony::SmsServiceManagerClient>::GetInstance();
    if (msgManager == nullptr) {
        LBSLOGE(GNSS, "short message manager nullptr");
        return false;
    }
    int32_t defaultId = msgManager->GetDefaultSmsSlotId();
    LBSLOGD(GNSS, "current subId %{public}d, defaultSubId %{public}d", subId, defaultId);
    if (subId != INVALID_SUBID && subId != defaultId) {
        return false;
    }
    return true;
}
#endif

void AGnssNiManager::CheckWapSuplInit(const EventFwk::Want &want)
{
#ifdef SMS_MMS_ENABLE
    AgnssNiSuplInit();
    if (!IsFromDefaultSubId(want)) {
        LBSLOGE(GNSS, "supl init message does not come from default sub");
        return;
    }

    std::string applicationId = want.GetStringParam("applicationId");
    /*
     * The WAP application id SHALL be as registered with OMNA (URN: x-oma-application:ulp.ua)
     * and the assigned code value is (0x10).
     */
    if (!(applicationId == "16" || applicationId == URN_APPLICATION_ID)) {
        LBSLOGE(GNSS, "supl init message application Id %{public}s", applicationId.c_str());
        return;
    }

    std::string rawData = want.GetStringParam("rawData");
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterfacev1_0 is nullptr");
        return;
    }
    gnssInterface_->SendNetworkInitiatedMsg(rawData, rawData.length());
#endif
}

void AGnssNiManager::CheckSmsSuplInit(const EventFwk::Want &want)
{
#ifdef SMS_MMS_ENABLE
    AgnssNiSuplInit();
    if (!IsFromDefaultSubId(want)) {
        LBSLOGE(GNSS, "supl init message does not come from default sub");
        return;
    }

    std::vector<std::string> pdus = want.GetStringArrayParam(std::string("pdus"));
    if (pdus.empty()) {
        LBSLOGI(GNSS, "pdus is null");
        return;
    }
    bool isCdma = want.GetBoolParam("isCdma", false);
    std::vector<unsigned char> pdu = StringUtils::HexToByteVector(pdus[0]);
    auto message = new Telephony::ShortMessage();
    std::u16string netType = isCdma ? u"3gpp2" : u"3gpp";
    Telephony::ShortMessage::CreateMessage(pdu, netType, *message);

    if (message != nullptr) {
        std::vector<unsigned char> rawPdu = message->GetPdu();
        std::string rawPduTmp = StringUtils::StringToHex(rawPdu);
        std::vector<uint8_t> str = {0, 17, 2, 0, 0, 64, 0, 0, 0, 70, 5, 64, 1, 24, 32, 16, 8};
        std::string messageBody = StringUtils::StringToHex(str);
        if (gnssInterface_ == nullptr) {
            LBSLOGE(GNSS, "gnssInterfacev1_0 is nullptr");
            delete message;
            message = nullptr;
            return;
        }
        gnssInterface_->SendNetworkInitiatedMsg(messageBody, messageBody.length());
        delete message;
        message = nullptr;
    }
#endif
}

bool AGnssNiManager::IsInEmergency()
{
    std::unique_lock<std::mutex> lock(callStateMutex_);
    bool isInEmergencyExtension = (emergencyCallEndTime_ > 0) &&
        ((CommonUtils::GetCurrentTime() - emergencyCallEndTime_) < TIME_AFTER_EMERGENCY_CALL);

    return isInEmergencyCall_ || isInEmergencyExtension;
}

void AGnssNiManager::OnCallStateChanged(const EventFwk::Want &want)
{
#ifdef CALL_MANAGER_ENABLE
    int32_t state = want.GetIntParam("state", (int32_t)Telephony::TelCallState::CALL_STATUS_UNKNOWN);
    if (state == (int32_t)Telephony::TelCallState::CALL_STATUS_DIALING) {
        int32_t slotId = want.GetIntParam("slotId", -1);
        std::string phoneNumber = want.GetStringParam("number");
        auto tmpPhoneNumber = Str8ToStr16(phoneNumber);
        std::unique_lock<std::mutex> lock(callStateMutex_);
        auto clientPtr = DelayedSingleton<Telephony::CallManagerClient>::GetInstance();
        if (clientPtr == nullptr) {
            return;
        }
        clientPtr->Init(OHOS::TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
        clientPtr->IsEmergencyPhoneNumber(tmpPhoneNumber, slotId, isInEmergencyCall_);
        if (isInEmergencyCall_) {
            emergencyCallEndTime_ = CommonUtils::GetCurrentTime();
            isInEmergencyCall_ = false;
        }
    }
    return;
#endif
}

std::string AGnssNiManager::BuildStartCommand(const GnssNiNotificationRequest &notif)
{
    nlohmann::json param;
    std::string uiType = "sysDialog/common";
    param["ability.want.params.uiExtensionType"] = uiType;
    std::string message = DecodeNiString(notif.supplicantInfo, notif.supplicantInfoEncoding) +
        DecodeNiString(notif.notificationText, notif.notificationTextEncoding);
    param["message"] = message;
    std::string cmdData = param.dump();
    LBSLOGD(GNSS, "cmdData is: %{public}s.", cmdData.c_str());
    return cmdData;
}

static void ConnectExtensionAbility(const AAFwk::Want &want, const std::string &commandStr)
{
    std::string bundleName = LOCATION_DIALOG_BUNDLE_NAME;
    std::string abilityName = AGNSS_NI_DIALOG_ABILITY_NAME;
    sptr<UIExtensionAbilityConnection> connection(
        new (std::nothrow) UIExtensionAbilityConnection(commandStr, bundleName, abilityName));
    if (connection == nullptr) {
        LBSLOGE(GNSS, "connect UIExtensionAbilityConnection fail");
        return;
    }

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto ret =
        AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(want, connection, nullptr, -1);
    LBSLOGI(GNSS, "connect service extension ability result = %{public}d", ret);
    IPCSkeleton::SetCallingIdentity(identity);
    return;
}

void AGnssNiManager::OpenNiDialog(const GnssNiNotificationRequest &notif)
{
    LBSLOGI(GNSS, "ConnectExtension");
    AAFwk::Want want;
    std::string bundleName = "com.ohos.sceneboard";
    std::string abilityName = "com.ohos.sceneboard.systemdialog";
    want.SetElementName(bundleName, abilityName);
    std::string connectStr = BuildStartCommand(notif);
    ConnectExtensionAbility(want, connectStr);
}

std::string AGnssNiManager::DecodeNiString(std::string original, int coding)
{
    if (coding == GNSS_NI_ENCODING_FORMAT_NULL) {
        return original;
    }

    std::string tmp = StringUtils::HexToString(original);
    switch (coding) {
        case GNSS_NI_ENCODING_FORMAT_SUPL_GSM_DEFAULT: {
            return StringUtils::Gsm7Decode(tmp);
        }
        case GNSS_NI_ENCODING_FORMAT_SUPL_UCS2: {
            std::wstring decodedUCS2String = StringUtils::Ucs2ToWstring(tmp);
            return StringUtils::WstringToString(decodedUCS2String);
        }
        case GNSS_NI_ENCODING_FORMAT_SUPL_UTF8: {
            std::wstring decodedUTF8String = StringUtils::Utf8ToWstring(tmp);
            return StringUtils::WstringToString(decodedUTF8String);
        }
        default: {
            LBSLOGE(GNSS, "unknow encoding %{public}d for NI text %{public}s", coding, original.c_str());
            return original;
        }
    }
}

void AGnssNiManager::SendNiNotification(const GnssNiNotificationRequest &notif)
{
    std::shared_ptr<Notification::NotificationNormalContent> notificationNormalContent =
        std::make_shared<Notification::NotificationNormalContent>();
    if (notificationNormalContent == nullptr) {
        LBSLOGE(GNSS, "get notification normal content nullptr");
        return;
    }

    std::string title = "Location Request";
    std::string msgBody = DecodeNiString(notif.supplicantInfo, notif.supplicantInfoEncoding) +
        DecodeNiString(notif.notificationText, notif.notificationTextEncoding);
    if (msgBody.empty()) {
        msgBody = "SUPL Service";
    }
    std::string message = "Requested by " + msgBody;
    notificationNormalContent->SetTitle(title);
    notificationNormalContent->SetText(message);
    std::shared_ptr<OHOS::Notification::NotificationContent> notificationContent =
        std::make_shared<OHOS::Notification::NotificationContent>(notificationNormalContent);

    if (notificationContent == nullptr) {
        LBSLOGE(GNSS, "get notification content nullptr");
        return;
    }

    Notification::NotificationRequest request;
    request.SetNotificationId(GNSS_AGNSS_NI_NOTIFICATION_ID);
    request.SetContent(notificationContent);
    request.SetCreatorUid(LOCATION_GNSS_SA_ID);
    request.SetAutoDeletedTime(NOTIFICATION_AUTO_DELETED_TIME);
    request.SetTapDismissed(true);
    request.SetCreatorBundleName(AGNSS_NI_SERVICE_NAME);
    request.SetSlotType(Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);

    int32_t ret = Notification::NotificationHelper::PublishNotification(request);
    if (ret != 0) {
        LBSLOGE(GNSS, "Publish Notification errorCode = %{public}d", ret);
        return;
    }
    LBSLOGI(GNSS, "GNSS service publish notification success");
}

void AGnssNiManager::SendUserResponse(GnssNiResponseCmd responseCmd)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterfacev1_0 is nullptr");
        return;
    }
    gnssInterface_->SendNiUserResponse(niNotificationId_, responseCmd);
}

void AGnssNiManager::HandleNiNotification(const GnssNiNotificationRequest &notif)
{
    std::unique_lock<std::mutex> lock(mutex_);
    niNotificationId_ = notif.gnssNiNotificationId;
    bool needNotify = (notif.notificationCategory & GNSS_NI_NOTIFICATION_REQUIRE_NOTIFY) != 0;
    bool needVerify = (notif.notificationCategory & GNSS_NI_NOTIFICATION_REQUIRE_VERIFY) != 0;
    bool privacyOverride = (notif.notificationCategory & GNSS_NI_NOTIFICATION_REQUIRE_PRIVACY_OVERRIDE) != 0;
    if (LocationDataRdbManager::QuerySwitchState() == DISABLED && !IsInEmergency()) {
        SendUserResponse(GNSS_NI_RESPONSE_CMD_NO_RESPONSE);
    }

    if (needNotify) {
        if (needVerify) {
            OpenNiDialog(notif);
        } else {
            SendNiNotification(notif);
            SendUserResponse(GNSS_NI_RESPONSE_CMD_ACCEPT);
        }
    }

    if (!needNotify || privacyOverride) {
        SendUserResponse(GNSS_NI_RESPONSE_CMD_ACCEPT);
    }
}

void SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LBSLOGI(GNSS, "AGNSS-NI systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(GNSS, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    auto agnssNiManager = DelayedSingleton<AGnssNiManager>::GetInstance();
    if (agnssNiManager == nullptr) {
        LBSLOGE(GNSS, "agnssNiManager nullptr");
        return;
    }
    agnssNiManager->Run();

    return;
}

void SystemAbilityStatusChangeListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LBSLOGI(GNSS, "AGNSS-NI systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LBSLOGE(GNSS, "systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }
    auto agnssNiManager = DelayedSingleton<AGnssNiManager>::GetInstance();
    if (agnssNiManager == nullptr) {
        LBSLOGE(GNSS, "agnssNiManager nullptr");
        return;
    }
    agnssNiManager->UnRegisterAgnssNiEvent();
    return;
}

}  // namespace Location
}  // namespace OHOS
