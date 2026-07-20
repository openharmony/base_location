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

#ifdef FEATURE_GNSS_SUPPORT
#include "agnss_ni_manager_test.h"

#include "location_log.h"
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t WAIT_EVENT_TIME = 3;

void AGnssNiManagerTest::SetUp()
{
}

void AGnssNiManagerTest::TearDown()
{
}

void AGnssNiManagerTest::TearDownTestCase()
{
    sleep(WAIT_EVENT_TIME);
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerGetInstance001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerGetInstance001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerGetInstance001 begin");
    auto instance = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, instance);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerGetInstance001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerSubscribeSaStatusChangeListerner001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerSubscribeSaStatusChangeListerner001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerSubscribeSaStatusChangeListerner001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->SubscribeSaStatusChangeListerner();
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerSubscribeSaStatusChangeListerner001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerRegisterAgnssNiEvent001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerRegisterAgnssNiEvent001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerRegisterAgnssNiEvent001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->RegisterAgnssNiEvent();
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerRegisterAgnssNiEvent001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerUnRegisterAgnssNiEvent001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerUnRegisterAgnssNiEvent001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerUnRegisterAgnssNiEvent001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->UnRegisterAgnssNiEvent();
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerUnRegisterAgnssNiEvent001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerRegisterNiResponseEvent001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerRegisterNiResponseEvent001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerRegisterNiResponseEvent001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->RegisterNiResponseEvent();
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerRegisterNiResponseEvent001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerUnRegisterNiResponseEvent001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerUnRegisterNiResponseEvent001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerUnRegisterNiResponseEvent001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->UnRegisterNiResponseEvent();
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerUnRegisterNiResponseEvent001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerDecodeNiString001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerDecodeNiString001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerDecodeNiString001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    std::string result = agnssNiManager->DecodeNiString("test_string", GNSS_NI_ENCODING_FORMAT_NULL);
    EXPECT_EQ("test_string", result);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerDecodeNiString001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerDecodeNiString002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerDecodeNiString002, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerDecodeNiString002 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    std::string result = agnssNiManager->DecodeNiString("test_string", GNSS_NI_ENCODING_FORMAT_SUPL_GSM_DEFAULT);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerDecodeNiString002 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerDecodeNiString005, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerDecodeNiString005, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerDecodeNiString005 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    std::string result = agnssNiManager->DecodeNiString("test_string", 99);
    EXPECT_EQ("test_string", result);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerDecodeNiString005 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerBuildStartCommand001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerBuildStartCommand001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerBuildStartCommand001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    GnssNiNotificationRequest notif;
    notif.gnssNiNotificationId = 1;
    notif.notificationText = "test_notification";
    notif.supplicantInfo = "test_supplicant";
    notif.notificationTextEncoding = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.defaultResponseCmd = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.notificationCategory = 0;
    std::string cmdData = agnssNiManager->BuildStartCommand(notif);
    EXPECT_NE("", cmdData);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerBuildStartCommand001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerOpenNiDialog001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerOpenNiDialog001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerOpenNiDialog001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    GnssNiNotificationRequest notif;
    notif.gnssNiNotificationId = 1;
    notif.notificationText = "test_notification";
    notif.supplicantInfo = "test_supplicant";
    notif.notificationTextEncoding = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.defaultResponseCmd = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.notificationCategory = 0;
    agnssNiManager->OpenNiDialog(notif);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerOpenNiDialog001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerSendNiNotification001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerSendNiNotification001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerSendNiNotification001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    GnssNiNotificationRequest notif;
    notif.gnssNiNotificationId = 1;
    notif.notificationText = "";
    notif.supplicantInfo = "";
    notif.notificationTextEncoding = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.defaultResponseCmd = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.notificationCategory = 0;
    agnssNiManager->SendNiNotification(notif);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerSendNiNotification001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerHandleNiNotification001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerHandleNiNotification001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    GnssNiNotificationRequest notif;
    notif.gnssNiNotificationId = 1;
    notif.notificationText = "test_notification";
    notif.supplicantInfo = "test_supplicant";
    notif.notificationTextEncoding = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.defaultResponseCmd = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.notificationCategory = GNSS_NI_NOTIFICATION_REQUIRE_NOTIFY | GNSS_NI_NOTIFICATION_REQUIRE_VERIFY;
    agnssNiManager->HandleNiNotification(notif);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerHandleNiNotification002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerHandleNiNotification002, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification002 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    GnssNiNotificationRequest notif;
    notif.gnssNiNotificationId = 2;
    notif.notificationText = "test_notification";
    notif.supplicantInfo = "test_supplicant";
    notif.notificationTextEncoding = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.defaultResponseCmd = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.notificationCategory = GNSS_NI_NOTIFICATION_REQUIRE_NOTIFY;
    agnssNiManager->HandleNiNotification(notif);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification002 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerHandleNiNotification003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerHandleNiNotification003, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification003 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    GnssNiNotificationRequest notif;
    notif.gnssNiNotificationId = 3;
    notif.notificationText = "test_notification";
    notif.supplicantInfo = "test_supplicant";
    notif.notificationTextEncoding = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.defaultResponseCmd = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.notificationCategory = GNSS_NI_NOTIFICATION_REQUIRE_NOTIFY | GNSS_NI_NOTIFICATION_REQUIRE_PRIVACY_OVERRIDE;
    agnssNiManager->HandleNiNotification(notif);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification003 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerHandleNiNotification004, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerHandleNiNotification004, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification004 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    GnssNiNotificationRequest notif;
    notif.gnssNiNotificationId = 4;
    notif.notificationText = "test_notification";
    notif.supplicantInfo = "test_supplicant";
    notif.notificationTextEncoding = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.defaultResponseCmd = GNSS_NI_ENCODING_FORMAT_NULL;
    notif.notificationCategory = GNSS_NI_NOTIFICATION_REQUIRE_PRIVACY_OVERRIDE;
    agnssNiManager->HandleNiNotification(notif);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerHandleNiNotification004 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerSendUserResponse001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerSendUserResponse001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerSendUserResponse001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->SendUserResponse(GNSS_NI_RESPONSE_CMD_ACCEPT);
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerSendUserResponse001 end");
}

HWTEST_F(AGnssNiManagerTest, AGnssNiManagerAgnssNiSuplInit001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, AGnssNiManagerAgnssNiSuplInit001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerAgnssNiSuplInit001 begin");
    auto agnssNiManager = AGnssNiManager::GetInstance();
    EXPECT_NE(nullptr, agnssNiManager);
    agnssNiManager->AgnssNiSuplInit();
    LBSLOGI(GNSS, "[AGnssNiManagerTest] AGnssNiManagerAgnssNiSuplInit001 end");
}

HWTEST_F(AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnAdd001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnAdd001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnAdd001 begin");
    sptr<SystemAbilityStatusChangeListener> listener = new (std::nothrow) SystemAbilityStatusChangeListener();
    EXPECT_NE(nullptr, listener);
    listener->OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "device123");
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnAdd001 end");
}

HWTEST_F(AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnAdd002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnAdd002, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnAdd002 begin");
    sptr<SystemAbilityStatusChangeListener> listener = new (std::nothrow) SystemAbilityStatusChangeListener();
    EXPECT_NE(nullptr, listener);
    listener->OnAddSystemAbility(12345, "device123");
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnAdd002 end");
}

HWTEST_F(AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnRemove001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnRemove001, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnRemove001 begin");
    sptr<SystemAbilityStatusChangeListener> listener = new (std::nothrow) SystemAbilityStatusChangeListener();
    EXPECT_NE(nullptr, listener);
    listener->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, "device123");
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnRemove001 end");
}

HWTEST_F(AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnRemove002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "AGnssNiManagerTest, SystemAbilityStatusChangeListenerOnRemove002, TestSize.Level0";
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnRemove002 begin");
    sptr<SystemAbilityStatusChangeListener> listener = new (std::nothrow) SystemAbilityStatusChangeListener();
    EXPECT_NE(nullptr, listener);
    listener->OnRemoveSystemAbility(12345, "device123");
    LBSLOGI(GNSS, "[AGnssNiManagerTest] SystemAbilityStatusChangeListenerOnRemove002 end");
}
}  // namespace Location
}  // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT