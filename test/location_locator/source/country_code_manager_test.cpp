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

#include "country_code_manager_test.h"

#include "country_code_callback_napi.h"
#include "location_log.h"

#include "nmea_message_callback_napi.h"
#include "mock_i_remote_object.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void CountryCodeManagerTest::SetUp()
{
}

void CountryCodeManagerTest::TearDown()
{
    auto countryCodeManager = CountryCodeManager::GetInstance();
    countryCodeManager->lastCountryByLocation_ = std::make_shared<CountryCode>();
    countryCodeManager->lastCountry_ = std::make_shared<CountryCode>();
}

HWTEST_F(CountryCodeManagerTest, GetIsoCountryCode001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, GetIsoCountryCode001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetIsoCountryCode001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    EXPECT_NE(nullptr, countryCodeManager->GetIsoCountryCode());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetIsoCountryCode001 end");
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UnregisterCountryCodeCallback001, TestSize.Level0";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    auto callback = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    ASSERT_TRUE(countryCodeManager != nullptr);
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    EXPECT_EQ(0, countryCodeManager->countryCodeCallbacksMap_.size());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback001 end");
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UnregisterCountryCodeCallback002, TestSize.Level0";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback002 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    countryCodeManager->UnregisterCountryCodeCallback(nullptr);
    EXPECT_EQ(0, countryCodeManager->countryCodeCallbacksMap_.size());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback002 end");
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UnregisterCountryCodeCallback003, TestSize.Level0";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback003 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    auto wrongCallback = sptr<NmeaMessageCallbackNapi>(new (std::nothrow) NmeaMessageCallbackNapi());
    countryCodeManager->UnregisterCountryCodeCallback(wrongCallback->AsObject());
    EXPECT_EQ(0, countryCodeManager->countryCodeCallbacksMap_.size());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback003 end");
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UnregisterCountryCodeCallback004, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback004 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    auto callback1 = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    AppIdentity identity;
    int pid = 2;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(callback1, identity);
    countryCodeManager->UnregisterCountryCodeCallback(callback1); // size != 0, func will return
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback004 callback1 unregistered");
    
    auto callback2 = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    countryCodeManager->RegisterCountryCodeCallback(callback2, identity);
    EXPECT_EQ(1, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->UnregisterCountryCodeCallback(callback2);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback004 callback2 unregistered");
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback004 end");
}

HWTEST_F(CountryCodeManagerTest, IsCountryCodeRegistered001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, IsCountryCodeRegistered001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] IsCountryCodeRegistered001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    countryCodeManager->IsCountryCodeRegistered();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] IsCountryCodeRegistered001 end");
}

HWTEST_F(CountryCodeManagerTest, RegisterCountryCodeCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, RegisterCountryCodeCallback001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    auto callback = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    AppIdentity identity;
    int pid = 10;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(callback, identity);
    EXPECT_NE(0, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback001 end");
}

HWTEST_F(CountryCodeManagerTest, RegisterCountryCodeCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, RegisterCountryCodeCallback002, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback002 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    AppIdentity identity;
    int pid = 3;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(nullptr, identity);
    EXPECT_EQ(0, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->UnregisterCountryCodeCallback(nullptr);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback002 end");
}

HWTEST_F(CountryCodeManagerTest, RegisterCountryCodeCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, RegisterCountryCodeCallback003, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback003 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    sptr<MockIRemoteObject> wrongCallback = sptr<MockIRemoteObject>(new (std::nothrow) MockIRemoteObject());
    AppIdentity identity;
    int pid = 4;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(wrongCallback, identity);
    EXPECT_EQ(1, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->UnregisterCountryCodeCallback(wrongCallback);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback003 end");
}

HWTEST_F(CountryCodeManagerTest, ReSubscribeEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReSubscribeEvent001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    EXPECT_EQ(0, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->ReSubscribeEvent();
    countryCodeManager->ReUnsubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent001 end");
}

HWTEST_F(CountryCodeManagerTest, ReSubscribeEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReSubscribeEvent002, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent002 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    auto callback = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    AppIdentity identity;
    int pid = 3;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(callback, identity);
    EXPECT_NE(0, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    countryCodeManager->ReSubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent002 end");
}

HWTEST_F(CountryCodeManagerTest, ReSubscribeEvent003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReSubscribeEvent003, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent003 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    auto callback = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    AppIdentity identity;
    int pid = 3;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(callback, identity);
    EXPECT_NE(0, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->ReSubscribeEvent();
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent003 end");
}

HWTEST_F(CountryCodeManagerTest, ReUnsubscribeEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReUnsubscribeEvent001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    countryCodeManager->ReUnsubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent001 end");
}

HWTEST_F(CountryCodeManagerTest, ReUnsubscribeEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReUnsubscribeEvent002, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent002 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    auto callback = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    AppIdentity identity;
    int pid = 3;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(callback, identity);
    EXPECT_NE(0, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    countryCodeManager->ReUnsubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent002 end");
}

HWTEST_F(CountryCodeManagerTest, ReUnsubscribeEvent003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReUnsubscribeEvent003, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent003 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    auto callback = sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    AppIdentity identity;
    int pid = 3;
    identity.SetPid(pid);
    identity.SetUid(pid);
    countryCodeManager->RegisterCountryCodeCallback(callback, identity);
    EXPECT_NE(0, countryCodeManager->countryCodeCallbacksMap_.size());
    countryCodeManager->ReUnsubscribeEvent();
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent003 end");
}

HWTEST_F(CountryCodeManagerTest, GetCountryCodeByLastLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, GetCountryCodeByLastLocation001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetCountryCodeByLastLocation001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    countryCodeManager->lastCountryByLocation_ = nullptr;
    EXPECT_EQ("", countryCodeManager->GetCountryCodeByLastLocation());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetCountryCodeByLastLocation001 end");
}

HWTEST_F(CountryCodeManagerTest, UpdateCountryCode001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UpdateCountryCode001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UpdateCountryCode001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    countryCodeManager->lastCountry_->SetCountryCodeType(1);
    countryCodeManager->UpdateCountryCode("zh", 0); // last type is more reliable

    countryCodeManager->lastCountry_ = nullptr;
    countryCodeManager->UpdateCountryCode("zh", 0); // lastCountry_ is nullptr
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UpdateCountryCode001 end");
}

HWTEST_F(CountryCodeManagerTest, UpdateCountryCodeByLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UpdateCountryCodeByLocation001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UpdateCountryCodeByLocation001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    countryCodeManager->lastCountryByLocation_->SetCountryCodeStr("zh");
    EXPECT_EQ(false, countryCodeManager->UpdateCountryCodeByLocation("zh", 1));

    countryCodeManager->lastCountryByLocation_->SetCountryCodeStr("us");
    EXPECT_EQ(true, countryCodeManager->UpdateCountryCodeByLocation("zh", 1));

    countryCodeManager->lastCountryByLocation_ = nullptr;
    EXPECT_EQ(false, countryCodeManager->UpdateCountryCodeByLocation("zh", 1));
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UpdateCountryCodeByLocation001 end");
}

HWTEST_F(CountryCodeManagerTest, GetCountryCodeByLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, GetCountryCodeByLocation001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetCountryCodeByLocation001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    EXPECT_EQ("", countryCodeManager->GetCountryCodeByLocation(nullptr));
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetCountryCodeByLocation001 end");
}

HWTEST_F(CountryCodeManagerTest, OnLocationReport001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, OnLocationReport001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] OnLocationReport001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    sptr<ILocatorCallback> callback =
        sptr<ILocatorCallback>(new (std::nothrow) CountryCodeManager::LocatorCallback());
    ASSERT_TRUE(callback != nullptr);
    callback->OnLocationReport(nullptr);

    std::unique_ptr<Location> location = std::make_unique<Location>();
    callback->OnLocationReport(location);
    callback->OnLocatingStatusChange(1);
    callback->OnErrorReport(1);

    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] OnLocationReport001 end");
}

HWTEST_F(CountryCodeManagerTest, NetworkSubscriber001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, NetworkSubscriber001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] NetworkSubscriber001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    OHOS::EventFwk::MatchingSkills matchingSkills;
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto networkSubscriber = std::make_shared<CountryCodeManager::NetworkSubscriber>(subscriberInfo);
    OHOS::EventFwk::CommonEventData event;
    networkSubscriber->OnReceiveEvent(event);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] NetworkSubscriber001 end");
}

HWTEST_F(CountryCodeManagerTest, SimSubscriber001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, SimSubscriber001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] SimSubscriber001 begin");
    auto countryCodeManager = CountryCodeManager::GetInstance();
    ASSERT_TRUE(countryCodeManager != nullptr);
    OHOS::EventFwk::MatchingSkills matchingSkills;
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    auto simSubscriber = std::make_shared<CountryCodeManager::SimSubscriber>(subscriberInfo);
    OHOS::EventFwk::CommonEventData event;
    simSubscriber->OnReceiveEvent(event);
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] SimSubscriber001 end");
}
} // namespace Location
} // namespace OHOS