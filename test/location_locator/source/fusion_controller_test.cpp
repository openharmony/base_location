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

#include "fusion_controller_test.h"

#include <singleton.h>

#include "common_utils.h"
#include "constant_definition.h"
#include "fusion_controller.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
const int UNKNOWN_TYPE = 0;
const uint32_t FUSION_DEFAULT_FLAG = 0;
const uint32_t FUSION_BASE_FLAG = 1;
const uint32_t REPORT_FUSED_LOCATION_FLAG = FUSION_BASE_FLAG;
void FusionControllerTest::SetUp()
{
    fusionController_ = DelayedSingleton<FusionController>::GetInstance();
    EXPECT_NE(nullptr, fusionController_);
}

void FusionControllerTest::TearDown()
{
    fusionController_ = nullptr;
    DelayedSingleton<FusionController>::DestroyInstance();
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies001 begin");
    fusionController_->ActiveFusionStrategies(SCENE_NAVIGATION);
    EXPECT_EQ(FUSION_DEFAULT_FLAG, fusionController_->fusedFlag_);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies001 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies002 begin");
    fusionController_->ActiveFusionStrategies(SCENE_TRAJECTORY_TRACKING);
    EXPECT_EQ(FUSION_DEFAULT_FLAG, fusionController_->fusedFlag_);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies002 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies003, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies003 begin");
    fusionController_->ActiveFusionStrategies(PRIORITY_FAST_FIRST_FIX);
    EXPECT_EQ(REPORT_FUSED_LOCATION_FLAG, fusionController_->fusedFlag_);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies003 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies004, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies004 begin");
    fusionController_->ActiveFusionStrategies(UNKNOWN_TYPE);
    EXPECT_EQ(FUSION_DEFAULT_FLAG, fusionController_->fusedFlag_);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies004 end");
}

HWTEST_F(FusionControllerTest, ActiveFusionStrategies005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ActiveFusionStrategies005, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies005 begin");
    EXPECT_EQ(true, fusionController_->needReset_);
    fusionController_->ActiveFusionStrategies(SCENE_NAVIGATION); // when needReset is true
    EXPECT_EQ(false, fusionController_->needReset_);
    fusionController_->ActiveFusionStrategies(SCENE_NAVIGATION); // when needReset is false
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ActiveFusionStrategies005 end");
}

HWTEST_F(FusionControllerTest, Process001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, Process001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process001 begin");
    fusionController_->Process(GNSS_ABILITY);
    EXPECT_EQ(true, fusionController_->needReset_);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process001 end");
}

HWTEST_F(FusionControllerTest, Process002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, Process002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process002 begin");
    fusionController_->Process(PASSIVE_ABILITY); // is not gnss ability
    EXPECT_EQ(true, fusionController_->needReset_);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] Process002 end");
}

HWTEST_F(FusionControllerTest, FuseResult001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, FuseResult001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult001 begin");
    auto location = std::make_unique<Location>();
    ASSERT_TRUE(fusionController_ != nullptr);
    fusionController_->FuseResult(GNSS_ABILITY, location);
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult001 end");
}

HWTEST_F(FusionControllerTest, FuseResult002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, FuseResult002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult002 begin");
    auto location = std::make_unique<Location>();
    ASSERT_TRUE(fusionController_ != nullptr);
    fusionController_->FuseResult(NETWORK_ABILITY, location); // is not gnss ability
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] FuseResult002 end");
}

HWTEST_F(FusionControllerTest, ChooseBestLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ChooseBestLocation001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ChooseBestLocation001 begin");
    EXPECT_EQ(nullptr, fusionController_->chooseBestLocation(nullptr, nullptr));

    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->ReadFromParcel(parcel);
    EXPECT_NE(nullptr, fusionController_->chooseBestLocation(nullptr, location));

    std::unique_ptr<Location> location_gnss = std::make_unique<Location>();
    location_gnss->ReadFromParcel(parcel);
    std::unique_ptr<Location> location_network = std::make_unique<Location>();
    location_network->ReadFromParcel(parcel);
    location_network->SetTimeSinceBoot(5000000000);
    EXPECT_NE(nullptr, fusionController_->chooseBestLocation(location_gnss, location_network));

    location_gnss->SetAccuracy(20.0);
    EXPECT_NE(nullptr, fusionController_->chooseBestLocation(location_gnss, location_network));

    location_gnss->SetTimeSinceBoot(10000000000);
    EXPECT_NE(nullptr, fusionController_->chooseBestLocation(location_gnss, location_network));
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ChooseBestLocation001 end");
}

HWTEST_F(FusionControllerTest, ChooseBestLocation002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, ChooseBestLocation002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ChooseBestLocation002 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc

    std::unique_ptr<Location> location_gnss = std::make_unique<Location>();
    location_gnss->ReadFromParcel(parcel);
    std::unique_ptr<Location> location_network = std::make_unique<Location>(*location_gnss);
    auto bestLocation = fusionController_->chooseBestLocation(location_gnss, location_network);
    location_gnss->SetAccuracy(20.0);
    EXPECT_EQ(20.0, location_gnss->GetAccuracy());
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] ChooseBestLocation002 end");
}

HWTEST_F(FusionControllerTest, GetFuseLocation001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, GetFuseLocation001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] GetFuseLocation001 begin");
    std::unique_ptr<Location> location = std::make_unique<Location>();
    
    EXPECT_NE(nullptr, fusionController_->GetFuseLocation(GNSS_ABILITY, location));
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] GetFuseLocation001 end");
}

HWTEST_F(FusionControllerTest, LocationEqual001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, LocationEqual001, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual001 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc

    std::unique_ptr<Location> bestLocation = std::make_unique<Location>();
    std::unique_ptr<Location> fuseLocation = std::make_unique<Location>();
    EXPECT_NE(nullptr, fusionController_->GetFuseLocation(GNSS_ABILITY, bestLocation));
    bestLocation->ReadFromParcel(parcel);
    fuseLocation = std::make_unique<Location>(*bestLocation);

    EXPECT_EQ(true, fusionController_->LocationEqual(bestLocation, fuseLocation));

    bestLocation->SetLatitude(12.1);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetLatitude(12.1);
    bestLocation->SetLongitude(13.1);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetLongitude(13.1);
    bestLocation->SetAltitude(14.1);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetAltitude(14.1);
    bestLocation->SetAccuracy(20.0);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetAccuracy(20.0);
    bestLocation->SetSpeed(10.1);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual001 end");
}

HWTEST_F(FusionControllerTest, LocationEqual002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, LocationEqual002, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual002 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc

    std::unique_ptr<Location> bestLocation = std::make_unique<Location>();
    std::unique_ptr<Location> fuseLocation = std::make_unique<Location>();
    EXPECT_NE(nullptr, fusionController_->GetFuseLocation(GNSS_ABILITY, bestLocation));
    bestLocation->ReadFromParcel(parcel);
    fuseLocation = std::make_unique<Location>(*bestLocation);

    EXPECT_EQ(true, fusionController_->LocationEqual(bestLocation, fuseLocation));

    bestLocation->SetDirection(80.0);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetDirection(80.0);
    bestLocation->SetTimeStamp(1000000002);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetTimeStamp(1000000002);
    bestLocation->SetTimeSinceBoot(1000000002);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetTimeSinceBoot(1000000002);
    bestLocation->SetAdditions("addition");
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetAdditions("addition");
    bestLocation->SetAdditionSize(2);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetAdditionSize(2);
    bestLocation->SetIsFromMock(true);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual002 end");
}

HWTEST_F(FusionControllerTest, LocationEqual003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, LocationEqual003, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual003 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc

    std::unique_ptr<Location> bestLocation = std::make_unique<Location>();
    std::unique_ptr<Location> fuseLocation = std::make_unique<Location>();
    EXPECT_NE(nullptr, fusionController_->GetFuseLocation(GNSS_ABILITY, bestLocation));
    bestLocation->ReadFromParcel(parcel);
    fuseLocation = std::make_unique<Location>(*bestLocation);

    EXPECT_EQ(true, fusionController_->LocationEqual(bestLocation, fuseLocation));

    bestLocation->SetSourceType(2);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));

    fuseLocation->SetSourceType(2);
    bestLocation->SetFloorNo(1);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual003 end");
}

HWTEST_F(FusionControllerTest, LocationEqual004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "FusionControllerTest, LocationEqual004, TestSize.Level1";
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual004 begin");
    MessageParcel parcel;
    parcel.WriteDouble(12.0);         // latitude
    parcel.WriteDouble(13.0);         // longitude
    parcel.WriteDouble(14.0);         // altitude
    parcel.WriteDouble(1000.0);       // accuracy
    parcel.WriteDouble(10.0);         // speed
    parcel.WriteDouble(90.0);         // direction
    parcel.WriteInt64(1000000000);    // timeStamp
    parcel.WriteInt64(1000000000);    // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1);             // additionSize
    parcel.WriteBool(false);          // isFromMock
    parcel.WriteInt32(1); // source type
    parcel.WriteInt32(0); // floor no.
    parcel.WriteDouble(1000.0); // floor acc

    std::unique_ptr<Location> bestLocation = std::make_unique<Location>();
    std::unique_ptr<Location> fuseLocation = std::make_unique<Location>();
    EXPECT_NE(nullptr, fusionController_->GetFuseLocation(GNSS_ABILITY, bestLocation));
    bestLocation->ReadFromParcel(parcel);
    fuseLocation = std::make_unique<Location>(*bestLocation);
    bestLocation->SetFloorAccuracy(500.0);
    EXPECT_EQ(false, fusionController_->LocationEqual(bestLocation, fuseLocation));
    LBSLOGI(FUSION_CONTROLLER, "[FusionControllerTest] LocationEqual004 end");
}
} // namespace Location
} // namespace OHOS