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
#include "country_code_manager.h"
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
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    EXPECT_NE(nullptr, countryCodeManager->GetIsoCountryCode());
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback001, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    countryCodeManager->UnregisterCountryCodeCallback(callback);
}

HWTEST_F(CountryCodeManagerTest, UnregisterCountryCodeCallback002, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    countryCodeManager->UnregisterCountryCodeCallback(nullptr);
}

HWTEST_F(CountryCodeManagerTest, RegisterCountryCodeCallback001, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    countryCodeManager->RegisterCountryCodeCallback(callback, 0);
}

HWTEST_F(CountryCodeManagerTest, RegisterCountryCodeCallback002, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    countryCodeManager->RegisterCountryCodeCallback(nullptr, 0);
}

HWTEST_F(CountryCodeManagerTest, ReSubscribeEvent001, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    countryCodeManager->ReSubscribeEvent();
}

HWTEST_F(CountryCodeManagerTest, ReSubscribeEvent002, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    countryCodeManager->RegisterCountryCodeCallback(callback, 0);
    countryCodeManager->ReSubscribeEvent();
}

HWTEST_F(CountryCodeManagerTest, ReUnsubscribeEvent001, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    countryCodeManager->ReUnsubscribeEvent();
}

HWTEST_F(CountryCodeManagerTest, ReUnsubscribeEvent002, TestSize.Level1)
{
    std::shared_ptr<CountryCodeManager> countryCodeManager =
        std::make_shared<CountryCodeManager>();
    auto callback = sptr<CountryCodeCallbackHost>(new (std::nothrow) CountryCodeCallbackHost());
    countryCodeManager->RegisterCountryCodeCallback(callback, 0);
    countryCodeManager->ReUnsubscribeEvent();
}
} // namespace Location
} // namespace OHOS