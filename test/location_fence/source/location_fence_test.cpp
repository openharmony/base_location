/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "location_fence_test.h"

#include "fence_impl.h"
#include "constant_definition.h"
#include "location_sa_load_manager.h"
#include "want_agent_helper.h"
#include "location_log.h"

using namespace testing::ext;

namespace OHOS {
namespace Location {
const int LOCATION_FENCE_SA_ID = 4353;
void LocationFenceTest::SetUp()
{
    fenceImpl_ = DelayedSingleton<FenceImpl>::GetInstance();
    ASSERT_TRUE(fenceImpl_ != nullptr);
}

void LocationFenceTest::TearDown()
{
}

HWTEST_F(LocationFenceTest, fenceImplAddFenceExt001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocationFenceTest, fenceImplAddFenceExt001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationFenceTest] fenceImplAddFenceExt001 begin");
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = 0x301; // scenario
    request->geofence.latitude = 34.12; // latitude
    request->geofence.longitude = 124.11; // longitude
    request->geofence.radius = 100.0; // radius
    request->geofence.expiration = 10000.0; // expiration
    AbilityRuntime::WantAgent::WantAgent wantAgent;
    auto err = DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_FENCE_SA_ID);
    if (err != ERRCODE_SUCCESS) {
        EXPECT_TRUE(fenceImpl_->AddFenceExt(request, wantAgent) == ERRCODE_NOT_SUPPORTED);
    } else {
        EXPECT_TRUE(fenceImpl_->AddFenceExt(request, wantAgent) == ERRCODE_SUCCESS);
    }
    LBSLOGI(LOCATOR, "[LocationFenceTest] fenceImplAddFenceExt001 end");
}

HWTEST_F(LocationFenceTest, fenceImplRemoveFenceExt001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocationFenceTest, fenceImplRemoveFenceExt001, TestSize.Level1";
    LBSLOGI(LOCATOR, "[LocationFenceTest] fenceImplRemoveFenceExt001 begin");
    std::unique_ptr<GeofenceRequest> request = std::make_unique<GeofenceRequest>();
    request->scenario = 0x301; // scenario
    request->geofence.latitude = 34.12; // latitude
    request->geofence.longitude = 124.11; // longitude
    request->geofence.radius = 100.0; // radius
    request->geofence.expiration = 10000.0; // expiration
    AbilityRuntime::WantAgent::WantAgent wantAgent;
    auto err = DelayedSingleton<LocationSaLoadManager>::GetInstance()->LoadLocationSa(LOCATION_FENCE_SA_ID);
    if (err != ERRCODE_SUCCESS) {
        EXPECT_TRUE(fenceImpl_->RemoveFenceExt(request, wantAgent) == ERRCODE_NOT_SUPPORTED);
    } else {
        EXPECT_TRUE(fenceImpl_->RemoveFenceExt(request, wantAgent) == ERRCODE_SUCCESS);
    }
    LBSLOGI(LOCATOR, "[LocationFenceTest] fenceImplRemoveFenceExt001 end");
}
}  // namespace Location
}  // namespace OHOS