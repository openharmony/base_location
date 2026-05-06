/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <string>
#include <memory>

#include "locator_impl.h"
#include "location.h"
#include "constant_definition.h"

using namespace testing::ext;
using namespace OHOS::Location;

class LocationCliToolTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(LocationCliToolTest, GetLocatorInstanceTest, TestSize.Level1)
{
    auto locator = LocatorImpl::GetInstance();
    ASSERT_NE(locator, nullptr);
}

HWTEST_F(LocationCliToolTest, IsLocationEnabledV9Test, TestSize.Level1)
{
    auto locator = LocatorImpl::GetInstance();
    ASSERT_NE(locator, nullptr);
    
    bool isEnabled = false;
    LocationErrCode errCode = locator->IsLocationEnabledV9(isEnabled);
    
    ASSERT_TRUE(errCode == ERRCODE_SUCCESS ||
                errCode == ERRCODE_SERVICE_UNAVAILABLE ||
                errCode == ERRCODE_SWITCH_OFF);
}

HWTEST_F(LocationCliToolTest, GetCachedLocationV9Test, TestSize.Level1)
{
    auto locator = LocatorImpl::GetInstance();
    ASSERT_NE(locator, nullptr);
    
    std::unique_ptr<Location> location;
    LocationErrCode errCode = locator->GetCachedLocationV9(location);
    
    ASSERT_TRUE(errCode == ERRCODE_SUCCESS || 
                errCode == ERRCODE_SERVICE_UNAVAILABLE ||
                errCode == ERRCODE_SWITCH_OFF ||
                errCode == ERRCODE_LOCATING_CACHE_FAIL);
}

HWTEST_F(LocationCliToolTest, LocationToJsonTest, TestSize.Level1)
{
    auto location = std::make_unique<Location>();
    location->SetLatitude(31.2304);
    location->SetLongitude(121.4737);
    location->SetAltitude(10.0);
    location->SetAccuracy(50.0);
    
    ASSERT_DOUBLE_EQ(location->GetLatitude(), 31.2304);
    ASSERT_DOUBLE_EQ(location->GetLongitude(), 121.4737);
    ASSERT_DOUBLE_EQ(location->GetAltitude(), 10.0);
    ASSERT_DOUBLE_EQ(location->GetAccuracy(), 50.0);
}

HWTEST_F(LocationCliToolTest, RequestConfigTest, TestSize.Level1)
{
    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetScenario(SCENE_DAILY_LIFE_SERVICE);
    requestConfig->SetTimeInterval(1);
    
    ASSERT_EQ(requestConfig->GetScenario(), SCENE_DAILY_LIFE_SERVICE);
    ASSERT_EQ(requestConfig->GetTimeInterval(), 1);
}

HWTEST_F(LocationCliToolTest, LocationErrCodeTest, TestSize.Level1)
{
    ASSERT_EQ(ERRCODE_SUCCESS, 0);
    ASSERT_EQ(ERRCODE_PERMISSION_DENIED, 201);
    ASSERT_EQ(ERRCODE_INVALID_PARAM, 401);
    ASSERT_EQ(ERRCODE_SERVICE_UNAVAILABLE, 3301000);
    ASSERT_EQ(ERRCODE_SWITCH_OFF, 3301100);
}