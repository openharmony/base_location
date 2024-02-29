/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "locator_agent_test.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "locator.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int32_t LOCATION_PERM_NUM = 4;
void LocatorAgentManagerTest::SetUp()
{
    MockNativePermission();
}

void LocatorAgentManagerTest::TearDown()
{
}

void LocatorAgentManagerTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorAgentManagerTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocatorAgentManagerTest, StartGnssLocatingTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, StartGnssLocatingTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] StartGnssLocatingTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    static OHOS::Location::LocationCallbackIfaces locationCallback;
    locatorAgentManager->StartGnssLocating(locationCallback);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] StartGnssLocatingTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, StopGnssLocatingTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, StopGnssLocatingTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] StopGnssLocatingTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    locatorAgentManager->StopGnssLocating();
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] StopGnssLocatingTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, RegisterGnssStatusCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, RegisterGnssStatusCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] RegisterGnssStatusCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    static OHOS::Location::SvStatusCallbackIfaces svCallback;
    locatorAgentManager->RegisterGnssStatusCallback(svCallback);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] RegisterGnssStatusCallbackTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, UnregisterGnssStatusCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, UnregisterGnssStatusCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] UnregisterGnssStatusCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    locatorAgentManager->UnregisterGnssStatusCallback();
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] UnregisterGnssStatusCallbackTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, RegisterNmeaMessageCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, RegisterNmeaMessageCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] RegisterNmeaMessageCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    static OHOS::Location::GnssNmeaCallbackIfaces nmeaCallback;
    locatorAgentManager->RegisterNmeaMessageCallback(nmeaCallback);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] RegisterNmeaMessageCallbackTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, UnregisterNmeaMessageCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, UnregisterNmeaMessageCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] UnregisterNmeaMessageCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    locatorAgentManager->UnregisterNmeaMessageCallback();
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] UnregisterNmeaMessageCallbackTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, GetLocatorAgentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, GetLocatorAgentTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] GetLocatorAgentTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    locatorAgentManager->GetLocatorAgent();
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] GetLocatorAgentTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, TryLoadLocatorSystemAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, TryLoadLocatorSystemAbilityTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] TryLoadLocatorSystemAbilityTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    locatorAgentManager->TryLoadLocatorSystemAbility();
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] TryLoadLocatorSystemAbilityTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, InitLocatorAgentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, InitLocatorAgentTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] InitLocatorAgentTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    locatorAgentManager->InitLocatorAgent(saObject);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] InitLocatorAgentTest001 end");
}

HWTEST_F(LocatorAgentManagerTest, ResetLocatorAgentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentManagerTest, ResetLocatorAgentTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] ResetLocatorAgentTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    locatorAgentManager->ResetLocatorAgent(saObject);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentManagerTest] ResetLocatorAgentTest001 end");
}

void LocatorAgentTest::SetUp()
{
    MockNativePermission();
}

void LocatorAgentTest::TearDown()
{
}

void LocatorAgentTest::MockNativePermission()
{
    const char *perms[] = {
        ACCESS_LOCATION.c_str(), ACCESS_APPROXIMATELY_LOCATION.c_str(),
        ACCESS_BACKGROUND_LOCATION.c_str(), MANAGE_SECURE_SETTINGS.c_str(),
    };
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = LOCATION_PERM_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "LocatorAgentTest",
        .aplStr = "system_basic",
    };
    tokenId_ = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId_);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(LocatorAgentTest, StartGnssLocatingTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, StartGnssLocatingTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StartGnssLocatingTest001 begin");
    auto locatorImpl = Locator::GetInstance();
    EXPECT_NE(nullptr, locatorImpl);
    locatorImpl->EnableAbility(true);
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto locationCallbackHost =
        sptr<NativeLocationCallbackHost>(new (std::nothrow) NativeLocationCallbackHost());
    auto locatorCallback = sptr<ILocatorCallback>(locationCallbackHost);
    auto errCode = locatorAgent->StartGnssLocating(locatorCallback);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StartGnssLocatingTest001 end");
}

HWTEST_F(LocatorAgentTest, StartGnssLocatingTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, StartGnssLocatingTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StartGnssLocatingTest002 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    sptr<ILocatorCallback> locatorCallback;
    auto errCode = locatorAgent->StartGnssLocating(locatorCallback);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StartGnssLocatingTest002 end");
}

HWTEST_F(LocatorAgentTest, StopGnssLocatingTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, StopGnssLocatingTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StopGnssLocatingTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto locationCallbackHost =
        sptr<NativeLocationCallbackHost>(new (std::nothrow) NativeLocationCallbackHost());
    auto locatorCallback = sptr<ILocatorCallback>(locationCallbackHost);
    auto errCode = locatorAgent->StopGnssLocating(locatorCallback);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StopGnssLocatingTest001 end");
}

HWTEST_F(LocatorAgentTest, StopGnssLocatingTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, StopGnssLocatingTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StopGnssLocatingTest002 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    sptr<ILocatorCallback> locatorCallback;
    auto errCode = locatorAgent->StopGnssLocating(locatorCallback);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] StopGnssLocatingTest002 end");
}

HWTEST_F(LocatorAgentTest, RegisterNmeaMessageCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, RegisterNmeaMessageCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterNmeaMessageCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto nmeaCallbackHost =
        sptr<NativeNmeaCallbackHost>(new (std::nothrow) NativeNmeaCallbackHost());
    auto nmeaCallback = sptr<INmeaMessageCallback>(nmeaCallbackHost);
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto errCode = locatorAgent->RegisterNmeaMessageCallback(nmeaCallback);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterNmeaMessageCallbackTest001 end");
}

HWTEST_F(LocatorAgentTest, RegisterNmeaMessageCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, RegisterNmeaMessageCallbackTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterNmeaMessageCallbackTest002 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto errCode = locatorAgent->RegisterNmeaMessageCallback(nullptr);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterNmeaMessageCallbackTest002 end");
}

HWTEST_F(LocatorAgentTest, UnregisterNmeaMessageCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, UnregisterNmeaMessageCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterNmeaMessageCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto nmeaCallbackHost =
        sptr<NativeNmeaCallbackHost>(new (std::nothrow) NativeNmeaCallbackHost());
    auto nmeaCallback = sptr<INmeaMessageCallback>(nmeaCallbackHost);
    auto errCode = locatorAgent->UnregisterNmeaMessageCallback(nmeaCallback);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterNmeaMessageCallbackTest001 end");
}

HWTEST_F(LocatorAgentTest, UnregisterNmeaMessageCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, UnregisterNmeaMessageCallbackTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterNmeaMessageCallbackTest002 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto errCode = locatorAgent->UnregisterNmeaMessageCallback(nullptr);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterNmeaMessageCallbackTest002 end");
}

HWTEST_F(LocatorAgentTest, RegisterGnssStatusCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, RegisterGnssStatusCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterGnssStatusCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    auto gnssCallbackHost =
        sptr<NativeSvCallbackHost>(new (std::nothrow) NativeSvCallbackHost());
    auto gnssCallback = sptr<IGnssStatusCallback>(gnssCallbackHost);
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto errCode = locatorAgent->RegisterGnssStatusCallback(gnssCallback);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterGnssStatusCallbackTest001 end");
}

HWTEST_F(LocatorAgentTest, RegisterGnssStatusCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, RegisterGnssStatusCallbackTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterGnssStatusCallbackTest002 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto errCode = locatorAgent->RegisterGnssStatusCallback(nullptr);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] RegisterGnssStatusCallbackTest002 end");
}


HWTEST_F(LocatorAgentTest, UnregisterGnssStatusCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, UnregisterGnssStatusCallbackTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterGnssStatusCallbackTest001 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto gnssCallbackHost =
        sptr<NativeSvCallbackHost>(new (std::nothrow) NativeSvCallbackHost());
    auto gnssCallback = sptr<IGnssStatusCallback>(gnssCallbackHost);
    auto errCode = locatorAgent->UnregisterGnssStatusCallback(gnssCallback);
    EXPECT_EQ(ERRCODE_SUCCESS, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterGnssStatusCallbackTest001 end");
}

HWTEST_F(LocatorAgentTest, UnregisterGnssStatusCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorAgentTest, UnregisterGnssStatusCallbackTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterGnssStatusCallbackTest002 begin");
    auto locatorAgentManager = std::make_shared<LocatorAgentManager>();
    sptr<IRemoteObject> saObject = locatorAgentManager->CheckLocatorSystemAbilityLoaded();
    auto locatorAgent =
        sptr<LocatorAgent>(new (std::nothrow) LocatorAgent(saObject));
    auto errCode = locatorAgent->UnregisterGnssStatusCallback(nullptr);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errCode);
    LBSLOGI(LOCATOR_STANDARD, "[LocatorAgentTest] UnregisterGnssStatusCallbackTest002 end");
}
}  // namespace Location
}  // namespace OHOS