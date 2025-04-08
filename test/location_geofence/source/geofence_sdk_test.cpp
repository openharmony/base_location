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

#include "geofence_sdk_test.h"
#include "common_utils.h"
#include "location_log.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void GeofenceSdkTest::SetUp()
{
    geofenceManager_ = GeofenceManager::GetInstance();
}

void GeofenceSdkTest::TearDown()
{
}

HWTEST_F(GeofenceSdkTest, AddGnssGeofenceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, AddGnssGeofenceTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceSdkTest] AddGnssGeofenceTest001 begin");
    ASSERT_TRUE(geofenceManager_ != nullptr);
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    geofenceManager_->AddGnssGeofence(gnssGeofenceRequest);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceSdkTest] AddGnssGeofenceTest001 end");
}

HWTEST_F(GeofenceSdkTest, RemoveGnssGeofenceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, RemoveGnssGeofenceTest001, TestSize.Level0";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceSdkTest] RemoveGnssGeofenceTest001 begin");
    ASSERT_TRUE(geofenceManager_ != nullptr);
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    geofenceManager_->RemoveGnssGeofence(gnssGeofenceRequest);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceSdkTest] RemoveGnssGeofenceTest001 end");
}

HWTEST_F(GeofenceSdkTest, GetGeofenceSupportedCoordTypesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "GeofenceSdkTest, GetGeofenceSupportedCoordTypesTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceSdkTest] GetGeofenceSupportedCoordTypesTest001 begin");
    ASSERT_TRUE(geofenceManager_ != nullptr);
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    geofenceManager_->GetGeofenceSupportedCoordTypes(coordinateSystemTypes);
    LBSLOGI(GEOFENCE_SDK_TEST, "[GeofenceSdkTest] GetGeofenceSupportedCoordTypesTest001 end");
}
} // namespace Location
} // namespace OHOS