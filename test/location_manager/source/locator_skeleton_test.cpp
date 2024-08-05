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
#include "nmea_message_callback_napi.h"
#endif
#include "permission_manager.h"
#include "location_data_rdb_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int32_t LOCATION_WITHOUT_PERM = 1;
const int32_t LOCATION_WITHOUT_MOCK_PERM_NUM = 5;
const int32_t LOCATION_PERM_NUM = 6;
const std::string RUNNING_STATE_OBSERVER = "ohos.permission.RUNNING_STATE_OBSERVER";
void LocatorSkeletonTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    LoadSystemAbility();
    MockNativePermission();
    MockNativeWithoutMockPermission();
    MockNativeWithoutSecurePermission();
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
        RUNNING_STATE_OBSERVER.c_str(), ACCESS_MOCK_LOCATION.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "tddTestApkname01",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] tokenId_ %{public}llu", tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void LocatorSkeletonTest::MockNativeWithoutMockPermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
        RUNNING_STATE_OBSERVER.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_WITHOUT_MOCK_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "tddTestApkname02",
        .aplStr = "system_basic",
    };
    tokenId2_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId2_);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] tokenId2_ %{public}llu", tokenId2_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void LocatorSkeletonTest::MockNativeWithoutSecurePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), RUNNING_STATE_OBSERVER.c_str(),
        ACCESS_MOCK_LOCATION.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_WITHOUT_MOCK_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "tddTestApkname03",
        .aplStr = "system_basic",
    };
    tokenId3_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId3_);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] tokenId2_ %{public}llu", tokenId3_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void LocatorSkeletonTest::MockNativeWithoutLocationPermission()
{
    const char *perms[] = {
        ACCESS_BACKGROUND_LOCATION.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_WITHOUT_PERM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorPermissionTest",
        .aplStr = "system_basic",
    };
    tokenId4_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId4_);
    LBSLOGI(LOCATOR, "[LocatorSkeletonWithoutPermissionTest] tokenId_ %{public}llu", tokenId4_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocatorSkeletonTest, LocatorInterfaceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, LocatorInterfaceTest, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] LocatorInterfaceTest begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    auto locatorHandleMap = locatorAbilityStub->locatorHandleMap_;
    for (auto it = locatorHandleMap.begin(); it != locatorHandleMap.end(); ++it) {
        LBSLOGI(LOCATOR, "[LocatorSkeletonTest] LocatorInterfaceTest code %{public}d", it->first);
        MessageParcel data;
        MessageParcel reply;
        AppIdentity identity;
        it->second(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] LocatorInterfaceTest end");
}

HWTEST_F(LocatorSkeletonTest, PreGetSwitchState, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetSwitchState, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetSwitchState begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteDouble(1.0);
    data.WriteFloat(1.0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteRemoteObject(callback->AsObject());
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreStartLocating(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        locatorAbilityStub->PreStartLocating(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreStartLocating(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreStartLocating(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartLocating end");
}

HWTEST_F(LocatorSkeletonTest, PreStopLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStopLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopLocating001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    data.WriteRemoteObject(callback->AsObject());
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreStopLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopLocating001 end");
}

HWTEST_F(LocatorSkeletonTest, PreStopLocating002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStopLocating002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopLocating002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    data.WriteRemoteObject(callback->AsObject());
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreStopLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopLocating002 end");
}

HWTEST_F(LocatorSkeletonTest, PreGetCacheLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetCacheLocation001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCacheLocation001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    MessageParcel reply;
    MessageParcel data;
    data.WriteInt32(1);
    data.WriteBool(true);
    data.WriteRemoteObject(callback->AsObject());
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreGetCacheLocation(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreGetCacheLocation(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreGetCacheLocation(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCacheLocation001 end");
}

HWTEST_F(LocatorSkeletonTest, PreGetCacheLocation002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetCacheLocation002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCacheLocation002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    MessageParcel reply;
    MessageParcel data;
    data.WriteInt32(1);
    data.WriteBool(true);
    data.WriteRemoteObject(callback->AsObject());
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreGetCacheLocation(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreGetCacheLocation(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreGetCacheLocation(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCacheLocation002 end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableAbility001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableAbility001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreEnableAbility(data, reply, identity);
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    data.WriteBool(true);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreEnableAbility(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableAbility001 end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableAbility002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableAbility002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableAbility002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreEnableAbility(data, reply, identity);
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    data.WriteBool(true);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId3_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreEnableAbility(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableAbility002 end");
}

HWTEST_F(LocatorSkeletonTest, PreUpdateSaAbility, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUpdateSaAbility, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUpdateSaAbility begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreUpdateSaAbility(data, reply, identity);
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreIsGeoConvertAvailable(data, reply, identity);
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterGnssStatusCallback end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterGnssStatusCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterGnssStatusCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterGnssStatusCallback001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUnregisterGnssStatusCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterGnssStatusCallback001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterGnssStatusCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterGnssStatusCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterGnssStatusCallback002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUnregisterGnssStatusCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterGnssStatusCallback002 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreRegisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallback001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreRegisterNmeaMessageCallback(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreRegisterNmeaMessageCallback(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreRegisterNmeaMessageCallback(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallback001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreRegisterNmeaMessageCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterNmeaMessageCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallback002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreRegisterGnssStatusCallback(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreRegisterNmeaMessageCallback(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreRegisterNmeaMessageCallback(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreRegisterNmeaMessageCallback(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallback002 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterNmeaMessageCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterNmeaMessageCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallback001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallback001 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterNmeaMessageCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterNmeaMessageCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallback002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreUnregisterNmeaMessageCallback(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallback002 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreRegisterNmeaMessageCallbackV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterNmeaMessageCallbackV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallbackV9 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreRegisterNmeaMessageCallbackV9(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreRegisterNmeaMessageCallbackV9(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreRegisterNmeaMessageCallbackV9(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreRegisterNmeaMessageCallbackV9(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterNmeaMessageCallbackV9 end");
}
#endif

#ifdef FEATURE_GNSS_SUPPORT
HWTEST_F(LocatorSkeletonTest, PreUnregisterNmeaMessageCallbackV9, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterNmeaMessageCallbackV9, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterNmeaMessageCallbackV9 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreUnregisterNmeaMessageCallbackV9(data, reply, identity);
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreIsLocationPrivacyConfirmed(data, reply, identity);
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

HWTEST_F(LocatorSkeletonTest, PreSetLocationPrivacyConfirmStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetLocationPrivacyConfirmStatus001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetLocationPrivacyConfirmStatus001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreSetLocationPrivacyConfirmStatus(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreSetLocationPrivacyConfirmStatus(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetLocationPrivacyConfirmStatus001 end");
}

HWTEST_F(LocatorSkeletonTest, PreSetLocationPrivacyConfirmStatus002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetLocationPrivacyConfirmStatus002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetLocationPrivacyConfirmStatus002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreSetLocationPrivacyConfirmStatus(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId3_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto result = locatorAbilityStub->PreSetLocationPrivacyConfirmStatus(data, reply, identity);
    EXPECT_EQ(ERRCODE_SUCCESS, result);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetLocationPrivacyConfirmStatus002 end");
}

HWTEST_F(LocatorSkeletonTest, PreStartCacheLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStartCacheLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartCacheLocating001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    data.WriteInt32(1);
    data.WriteBool(true);
    data.WriteRemoteObject(callback->AsObject());
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        locatorAbilityStub->PreStartCacheLocating(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreStartCacheLocating(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreStartCacheLocating(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartCacheLocating001 end");
}

HWTEST_F(LocatorSkeletonTest, PreStartCacheLocating002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStartCacheLocating002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartCacheLocating002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    data.WriteInt32(1);
    data.WriteBool(true);
    data.WriteRemoteObject(callback->AsObject());
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        locatorAbilityStub->PreStartCacheLocating(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreStartCacheLocating(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreStartCacheLocating(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStartCacheLocating002 end");
}

HWTEST_F(LocatorSkeletonTest, PreStopCacheLocating001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStopCacheLocating001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopCacheLocating001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    data.WriteInt32(1);
    data.WriteBool(true);
    data.WriteRemoteObject(callback->AsObject());
    locatorAbilityStub->PreStopCacheLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopCacheLocating001 end");
}

HWTEST_F(LocatorSkeletonTest, PreStopCacheLocating002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreStopCacheLocating002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopCacheLocating002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    auto callback = sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    data.WriteInt32(1);
    data.WriteBool(true);
    data.WriteRemoteObject(callback->AsObject());
    locatorAbilityStub->PreStopCacheLocating(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreStopCacheLocating002 end");
}

HWTEST_F(LocatorSkeletonTest, PreGetCachedGnssLocationsSize, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreGetCachedGnssLocationsSize, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCachedGnssLocationsSize begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreGetCachedGnssLocationsSize(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreGetCachedGnssLocationsSize(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreGetCachedGnssLocationsSize(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    }
    locatorAbilityStub->PreGetCachedGnssLocationsSize(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreGetCachedGnssLocationsSize end");
}

HWTEST_F(LocatorSkeletonTest, PreFlushCachedGnssLocations, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreFlushCachedGnssLocations, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreFlushCachedGnssLocations begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreFlushCachedGnssLocations(data, reply, identity);
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreFlushCachedGnssLocations(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreFlushCachedGnssLocations(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreFlushCachedGnssLocations(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreFlushCachedGnssLocations end");
}

HWTEST_F(LocatorSkeletonTest, PreSendCommand, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSendCommand, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSendCommand begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreSendCommand(data, reply, identity);
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    locatorAbilityStub->PreAddFence(data, reply, identity);
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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

HWTEST_F(LocatorSkeletonTest, DoProcessFenceRequest, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, DoProcessFenceRequest, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] DoProcessFenceRequest begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        locatorAbilityStub->DoProcessFenceRequest(LocatorInterfaceCode::ADD_FENCE, data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->DoProcessFenceRequest(LocatorInterfaceCode::ADD_FENCE, data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->DoProcessFenceRequest(LocatorInterfaceCode::ADD_FENCE, data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] DoProcessFenceRequest end");
}

HWTEST_F(LocatorSkeletonTest, PreAddGnssGeofence, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreAddGnssGeofence, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreAddGnssGeofence begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreAddGnssGeofence(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreAddGnssGeofence(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    }
    locatorAbilityStub->PreAddGnssGeofence(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreAddGnssGeofence end");
}

HWTEST_F(LocatorSkeletonTest, PreRemoveGnssGeofence, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRemoveGnssGeofence, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRemoveGnssGeofence begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        auto result = locatorAbilityStub->PreRemoveGnssGeofence(data, reply, identity);
        EXPECT_EQ(ERRCODE_SUCCESS, result);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreRemoveGnssGeofence(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreRemoveGnssGeofence(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRemoveGnssGeofence end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableLocationMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableLocationMock001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreEnableLocationMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableLocationMock001 end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableLocationMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableLocationMock002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableLocationMock002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId2_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreEnableLocationMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableLocationMock002 end");
}

HWTEST_F(LocatorSkeletonTest, PreDisableLocationMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreDisableLocationMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableLocationMock001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreDisableLocationMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableLocationMock001 end");
}

HWTEST_F(LocatorSkeletonTest, PreDisableLocationMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreDisableLocationMock002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableLocationMock002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId2_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreDisableLocationMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableLocationMock002 end");
}

HWTEST_F(LocatorSkeletonTest, PreSetMockedLocations001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetMockedLocations001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetMockedLocations001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreSetMockedLocations(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetMockedLocations001 end");
}

HWTEST_F(LocatorSkeletonTest, PreSetMockedLocations002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetMockedLocations002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetMockedLocations002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId2_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreSetMockedLocations(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetMockedLocations002 end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableReverseGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableReverseGeocodingMock001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreEnableReverseGeocodingMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableReverseGeocodingMock001 end");
}

HWTEST_F(LocatorSkeletonTest, PreEnableReverseGeocodingMock002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreEnableReverseGeocodingMock002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableReverseGeocodingMock002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId2_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreEnableReverseGeocodingMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreEnableReverseGeocodingMock002 end");
}

HWTEST_F(LocatorSkeletonTest, PreDisableReverseGeocodingMock001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreDisableReverseGeocodingMock001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableReverseGeocodingMock001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreDisableReverseGeocodingMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableReverseGeocodingMock001 end");
}

HWTEST_F(LocatorSkeletonTest, PreDisableReverseGeocodingMock, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreDisableReverseGeocodingMock, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableReverseGeocodingMock begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId2_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreDisableReverseGeocodingMock(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreDisableReverseGeocodingMock end");
}

HWTEST_F(LocatorSkeletonTest, PreSetReverseGeocodingMockInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetReverseGeocodingMockInfo001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetReverseGeocodingMockInfo001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreSetReverseGeocodingMockInfo(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetReverseGeocodingMockInfo001 end");
}

HWTEST_F(LocatorSkeletonTest, PreSetReverseGeocodingMockInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreSetReverseGeocodingMockInfo002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetReverseGeocodingMockInfo002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId2_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreSetReverseGeocodingMockInfo(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreSetReverseGeocodingMockInfo002 end");
}

HWTEST_F(LocatorSkeletonTest, PreProxyForFreeze, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreProxyForFreeze, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreProxyForFreeze begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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

HWTEST_F(LocatorSkeletonTest, PreRegisterLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocationError001 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        locatorAbilityStub->PreRegisterLocationError(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreRegisterLocationError(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreRegisterLocationError(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocationError001 end");
}

HWTEST_F(LocatorSkeletonTest, PreRegisterLocationError002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreRegisterLocationError002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocationError002 begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    bool isSwitchEnable = LocationDataRdbManager::QuerySwitchState() == ENABLED ? true : false;
    if (isSwitchEnable) {
        locatorAbilityStub->PreRegisterLocationError(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(DISABLED);
        locatorAbilityStub->PreRegisterLocationError(data, reply, identity);
        LocationDataRdbManager::SetSwitchStateToSyspara(ENABLED);
    } else {
        locatorAbilityStub->PreRegisterLocationError(data, reply, identity);
    }
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreRegisterLocationError002 end");
}

HWTEST_F(LocatorSkeletonTest, PreUnregisterLocationError001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterLocationError001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocationError001 begin");
    
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocationError001 end");
}

HWTEST_F(LocatorSkeletonTest, PreUnregisterLocationError002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreUnregisterLocationError002, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocationError002 begin");
    
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    MessageParcel reply;
    AppIdentity identity;
    identity.SetPid(1);
    identity.SetUid(2);
    uint32_t tokenId = static_cast<uint32_t>(tokenId4_);
    identity.SetTokenId(tokenId);
    identity.SetFirstTokenId(0);
    identity.SetBundleName("bundleName");
    locatorAbilityStub->PreUnregisterLocationError(data, reply, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreUnregisterLocationError002 end");
}

HWTEST_F(LocatorSkeletonTest, PreReportLocationError, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PreReportLocationError, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PreReportLocationError begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
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
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    MessageParcel data;
    data.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    locatorAbilityStub->OnRemoteRequest(3, data, reply, option);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] OnRemoteRequest end");
}

HWTEST_F(LocatorSkeletonTest, RemoveUnloadTask, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, RemoveUnloadTask TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] RemoveUnloadTask begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    locatorAbilityStub->RemoveUnloadTask(3);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] RemoveUnloadTask end");
}

HWTEST_F(LocatorSkeletonTest, PostUnloadTask, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, PostUnloadTask, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PostUnloadTask begin");
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    locatorAbilityStub->PostUnloadTask(3);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] PostUnloadTask end");
}

HWTEST_F(LocatorSkeletonTest, WriteLocationDenyReportEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, WriteLocationDenyReportEvent TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] WriteLocationDenyReportEvent begin");
    MessageParcel data;
    data.WriteInterfaceToken(LocatorProxy::GetDescriptor());
    AppIdentity identity;
    auto locatorAbilityStub = sptr<MockLocatorAbilityStub>(new (std::nothrow) MockLocatorAbilityStub());
    locatorAbilityStub->WriteLocationDenyReportEvent(0, 0, data, identity);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] WriteLocationDenyReportEvent end");
}

HWTEST_F(LocatorSkeletonTest, OnRemoteDied, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, OnRemoteDied TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] OnRemoteDied begin");
    const wptr<IRemoteObject> remote;
    auto switchCallbackDeathRecipient =
        sptr<SwitchCallbackDeathRecipient>(new (std::nothrow) SwitchCallbackDeathRecipient());
    switchCallbackDeathRecipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] OnRemoteDied end");
}

HWTEST_F(LocatorSkeletonTest, ScanCallbackOnRemoteDied, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorSkeletonTest, ScanCallbackOnRemoteDied TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] ScanCallbackOnRemoteDied begin");
    const wptr<IRemoteObject> remote;
    auto scanCallbackDeathRecipient =
        sptr<ScanCallbackDeathRecipient>(new (std::nothrow) ScanCallbackDeathRecipient());
    scanCallbackDeathRecipient->OnRemoteDied(remote);
    LBSLOGI(LOCATOR, "[LocatorSkeletonTest] ScanCallbackOnRemoteDied end");
}
}  // namespace Location
}  // namespace OHOS
