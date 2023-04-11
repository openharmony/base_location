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

#include "country_code_callback_host.h"
#include "location_log.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void CountryCodeManagerTest::SetUp()
{
}

void CountryCodeManagerTest::TearDown()
{
}

HWTEST_F(CountryCodeManagerTest, GetIsoCountryCode001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, GetIsoCountryCode001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetIsoCountryCode001 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    EXPECT_NE(nullptr, countryCodeManager->GetIsoCountryCode());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] GetIsoCountryCode001 end");
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UnregisterCountryCodeCallback001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback001 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    ASSERT_TRUE(countryCodeManager != nullptr);
    countryCodeManager->UnregisterCountryCodeCallback(callback);
    EXPECT_EQ(0, countryCodeManager->countryCodeCallback_->size());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback001 end");
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, UnregisterCountryCodeCallback002, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback002 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    ASSERT_TRUE(countryCodeManager != nullptr);
    countryCodeManager->UnregisterCountryCodeCallback(nullptr);
    EXPECT_EQ(0, countryCodeManager->countryCodeCallback_->size());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] UnregisterCountryCodeCallback002 end");
}

HWTEST_F(CountryCodeManagerTest, RegisterCountryCodeCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, RegisterCountryCodeCallback001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback001 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    countryCodeManager->RegisterCountryCodeCallback(callback, 0);
    EXPECT_NE(0, countryCodeManager->countryCodeCallback_->size());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback001 end");
}

HWTEST_F(CountryCodeManagerTest, RegisterCountryCodeCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, RegisterCountryCodeCallback002, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback002 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    ASSERT_TRUE(countryCodeManager != nullptr);
    countryCodeManager->RegisterCountryCodeCallback(nullptr, 0);
    EXPECT_EQ(0, countryCodeManager->countryCodeCallback_->size());
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] RegisterCountryCodeCallback002 end");
}

HWTEST_F(CountryCodeManagerTest, ReSubscribeEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReSubscribeEvent001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent001 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    EXPECT_EQ(0, countryCodeManager->countryCodeCallback_->size());
    countryCodeManager->ReSubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent001 end");
}

HWTEST_F(CountryCodeManagerTest, ReSubscribeEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReSubscribeEvent002, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent002 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    countryCodeManager->RegisterCountryCodeCallback(callback, 0);
    EXPECT_NE(0, countryCodeManager->countryCodeCallback_->size());
    countryCodeManager->ReSubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReSubscribeEvent002 end");
}

HWTEST_F(CountryCodeManagerTest, ReUnsubscribeEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReUnsubscribeEvent001, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent001 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    EXPECT_EQ(0, countryCodeManager->countryCodeCallback_->size());
    countryCodeManager->ReUnsubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent001 end");
}

HWTEST_F(CountryCodeManagerTest, ReUnsubscribeEvent002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CountryCodeManagerTest, ReUnsubscribeEvent002, TestSize.Level1";
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent002 begin");
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    countryCodeManager->RegisterCountryCodeCallback(callback, 0);
    EXPECT_NE(0, countryCodeManager->countryCodeCallback_->size());
    countryCodeManager->ReUnsubscribeEvent();
    LBSLOGI(COUNTRY_CODE, "[CountryCodeManagerTest] ReUnsubscribeEvent002 end");
}
} // namespace Location
} // namespace OHOS