/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "locator_skeleton_test.h"

#include <cstdlib>

#include "accesstoken_kit.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"

#include "app_identity.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator.h"
#include "location.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locator.h"
#define private public
#include "locator_skeleton.h"
#undef private
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_host.h"
#endif
#include "permission_manager.h"
#include "location_data_rdb_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 5;
const std::string RUNNING_STATE_OBSERVER = "ohos.permission.RUNNING_STATE_OBSERVER";
void LocatorSkeletonTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    LoadSystemAbility();
    MockNativePermission();
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, systemAbilityManager);
    sptr<IRemoteObject> object = systemAbilityManager->GetSystemAbility(LOCATION_LOCATOR_SA_ID);
    EXPECT_NE(nullptr, object);
}

void LocatorSkeletonTest::TearDown()
{
}

void LocatorSkeletonTest::LoadSystemAbility()
{
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_LOCATOR_SA_ID);
#ifdef FEATURE_GNSS_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_GNSS_SA_ID);
#endif
#ifdef FEATURE_PASSIVE_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_NOPOWER_LOCATING_SA_ID);
#endif
#ifdef FEATURE_NETWORK_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_NETWORK_LOCATING_SA_ID);
#endif
#ifdef FEATURE_GEOCODE_SUPPORT
    LocationSaLoadManager::GetInstance()->LoadLocationSa(LOCATION_GEO_CONVERT_SA_ID);
#endif
}

void LocatorSkeletonTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        RUNNING_STATE_OBSERVER.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocatorSkeletonTest, PreGetSwitchState, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetSwitchState, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetSwitchState begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreGetSwitchState(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetSwitchState end");
}

HWTEST_F(LocatorSkeletonTest, PreRegisterSwitchCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterSwitchCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterSwitchCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;

    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreRegisterSwitchCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterSwitchCallback end");
}

HWTEST_F(LocatorSkeletonTest, PreStartLocating, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStartLocating, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartLocating begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    LocationDataRdbManager::SetSwitchState(ENABLED);
    locatorAbilityStub->PreStartLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartLocating end");
}

HWTEST_F(LocatorSkeletonTest, PreStopLocating, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStopLocating, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopLocating begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreStopLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopLocating end");
}

HWTEST_F(LocatorSkeletonTest, PreGetCacheLocation, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetCacheLocation, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCacheLocation begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    LocationDataRdbManager::SetSwitchState(ENABLED);
    auto result = locatorAbilityStub->PreGetCacheLocation(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCacheLocation end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableAbility, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableAbility, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableAbility begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreEnableAbility(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableAbility end");
}

HWTEST_F(LocatorSkeletonTest, PreUpdateSaAbility, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUpdateSaAbility, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUpdateSaAbility begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUpdateSaAbility(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUpdateSaAbility end");
}

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreIsGeoConvertAvailable, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreIsGeoConvertAvailable, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreIsGeoConvertAvailable begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreIsGeoConvertAvailable(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreIsGeoConvertAvailable end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreGetAddressByCoordinate, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetAddressByCoordinate, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetAddressByCoordinate begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreGetAddressByCoordinate(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetAddressByCoordinate end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreGetAddressByLocationName, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetAddressByLocationName, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetAddressByLocationName begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreGetAddressByLocationName(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetAddressByLocationName end");
}
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterSwitchCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterSwitchCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterSwitchCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUnregisterSwitchCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterSwitchCallback end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreRegisterGnssStatusCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterGnssStatusCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterGnssStatusCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterGnssStatusCallback end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterGnssStatusCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterGnssStatusCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterGnssStatusCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUnregisterGnssStatusCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterGnssStatusCallback end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreRegisterNmeaMessageCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterNmeaMessageCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreRegisterNmeaMessageCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallback end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterNmeaMessageCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterNmeaMessageCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUnregisterNmeaMessageCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallback end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreRegisterNmeaMessageCallbackV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterNmeaMessageCallbackV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallbackV9 begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreRegisterNmeaMessageCallbackV9(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallbackV9 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterNmeaMessageCallbackV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterNmeaMessageCallbackV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallbackV9 begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUnregisterNmeaMessageCallbackV9(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallbackV9 end");
}
#endif

HWTEST_F(LocatorSkeletonTest, PreIsLocationPrivacyConfirmed, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreIsLocationPrivacyConfirmed, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreIsLocationPrivacyConfirmed begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreIsLocationPrivacyConfirmed(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreIsLocationPrivacyConfirmed end");
}

HWTEST_F(LocatorSkeletonTest, PreSetLocationPrivacyConfirmStatus, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetLocationPrivacyConfirmStatus, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetLocationPrivacyConfirmStatus begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreSetLocationPrivacyConfirmStatus(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetLocationPrivacyConfirmStatus end");
}

HWTEST_F(LocatorSkeletonTest, PreStartCacheLocating, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStartCacheLocating, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartCacheLocating begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreStartCacheLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartCacheLocating end");
}

HWTEST_F(LocatorSkeletonTest, PreStopCacheLocating, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStopCacheLocating, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopCacheLocating begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreStopCacheLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopCacheLocating end");
}

HWTEST_F(LocatorSkeletonTest, PreGetCachedGnssLocationsSize, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetCachedGnssLocationsSize, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCachedGnssLocationsSize begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreGetCachedGnssLocationsSize(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCachedGnssLocationsSize end");
}

HWTEST_F(LocatorSkeletonTest, PreFlushCachedGnssLocations, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreFlushCachedGnssLocations, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreFlushCachedGnssLocations begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreFlushCachedGnssLocations(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreFlushCachedGnssLocations end");
}

HWTEST_F(LocatorSkeletonTest, PreSendCommand, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSendCommand, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSendCommand begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreSendCommand(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSendCommand end");
}

HWTEST_F(LocatorSkeletonTest, PreAddFence, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreAddFence, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreAddFence begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreAddFence(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreAddFence end");
}


HWTEST_F(LocatorSkeletonTest, PreRemoveFence, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRemoveFence, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRemoveFence begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreRemoveFence(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRemoveFence end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableLocationMock, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableLocationMock, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableLocationMock begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreEnableLocationMock(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableLocationMock end");
}

HWTEST_F(LocatorSkeletonTest, PreDisableLocationMock, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreDisableLocationMock, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableLocationMock begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreDisableLocationMock(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableLocationMock end");
}

HWTEST_F(LocatorSkeletonTest, PreSetMockedLocations, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetMockedLocations, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetMockedLocations begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreSetMockedLocations(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetMockedLocations end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableReverseGeocodingMock, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableReverseGeocodingMock, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableReverseGeocodingMock begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreEnableReverseGeocodingMock(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableReverseGeocodingMock end");
}

HWTEST_F(LocatorSkeletonTest, PreDisableReverseGeocodingMock, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreDisableReverseGeocodingMock, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableReverseGeocodingMock begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreDisableReverseGeocodingMock(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableReverseGeocodingMock end");
}

HWTEST_F(LocatorSkeletonTest, PreSetReverseGeocodingMockInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetReverseGeocodingMockInfo, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetReverseGeocodingMockInfo begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreSetReverseGeocodingMockInfo(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetReverseGeocodingMockInfo end");
}

HWTEST_F(LocatorSkeletonTest, PreProxyForFreeze, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreProxyForFreeze, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreProxyForFreeze begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreProxyForFreeze(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreProxyForFreeze end");
}

HWTEST_F(LocatorSkeletonTest, PreResetAllProxy, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreResetAllProxy, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreResetAllProxy begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreResetAllProxy(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreResetAllProxy end");
}

HWTEST_F(LocatorSkeletonTest, PreReportLocation, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreReportLocation, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreReportLocation begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreReportLocation(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreReportLocation end");
}

HWTEST_F(LocatorSkeletonTest, PreRegisterLocatingRequiredDataCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterLocatingRequiredDataCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocatingRequiredDataCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreRegisterLocatingRequiredDataCallback(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocatingRequiredDataCallback end");
}

HWTEST_F(LocatorSkeletonTest, PreUnregisterLocatingRequiredDataCallback, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterLocatingRequiredDataCallback, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocatingRequiredDataCallback begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;

    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreUnregisterLocatingRequiredDataCallback(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocatingRequiredDataCallback end");
}

HWTEST_F(LocatorSkeletonTest, PreQuerySupportCoordinateSystemType, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreQuerySupportCoordinateSystemType, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreQuerySupportCoordinateSystemType begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreQuerySupportCoordinateSystemType(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreQuerySupportCoordinateSystemType end");
}

HWTEST_F(LocatorSkeletonTest, PreRegisterLocationError, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterLocationError, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocationError begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    LocationDataRdbManager::SetSwitchState(ENABLED);
    locatorAbilityStub->PreRegisterLocationError(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocationError end");
}

HWTEST_F(LocatorSkeletonTest, PreUnregisterLocationError, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterLocationError, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocationError begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;

    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreUnregisterLocationError(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocationError end");
}

HWTEST_F(LocatorSkeletonTest, PreReportLocationError, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreReportLocationError, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreReportLocationError begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;

    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreReportLocationError(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreReportLocationError end");
}

HWTEST_F(LocatorSkeletonTest, OnRemoteRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, OnRemoteRequest, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] OnRemoteRequest begin");
    auto locatorAbilityStub = sptr<LocatorAbilityStub>(new (std::nothrow) LocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    locatorAbilityStub->OnRemoteRequest(3, data, reply, option);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] OnRemoteRequest end");
}
}  // namespace Location
}  // namespace OHOS
