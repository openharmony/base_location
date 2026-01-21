/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "beacon_fence_manager_test.h"

#include "constant_definition.h"
#include "location_log.h"
#include "location_gnss_geofence_callback_napi.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void BeaconFenceManagerTest::SetUp()
{
    beaconFenceManager_ = BeaconFenceManager::GetInstance();
    EXPECT_NE(nullptr, beaconFenceManager_);
}

void BeaconFenceManagerTest::TearDown()
{
    beaconFenceManager_ = nullptr;
}

HWTEST_F(BeaconFenceManagerTest, AddBeaconFenceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, AddBeaconFenceTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] AddBeaconFenceTest001 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = nullptr;
    AppIdentity identity;
    EXPECT_EQ(ERRCODE_INVALID_PARAM, beaconFenceManager_->AddBeaconFence(beaconFenceRequest, identity));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] AddBeaconFenceTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, AddBeaconFenceTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, AddBeaconFenceTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] AddBeaconFenceTest002 begin");
    AppIdentity identity;
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("AddBeaconFenceTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");

    EXPECT_EQ(ERRCODE_SUCCESS, beaconFenceManager_->AddBeaconFence(beaconFenceRequest, identity));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] AddBeaconFenceTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, StartAddBeaconFenceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, StartAddBeaconFenceTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] StartAddBeaconFenceTest001 begin");
     AppIdentity identity;
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("StartAddBeaconFenceTest001");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");

    beaconFenceManager_->StartAddBeaconFence(beaconFenceRequest, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] StartAddBeaconFenceTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, RemoveBeaconFenceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RemoveBeaconFenceTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceTest001 begin");
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("RemoveBeaconFenceTest001");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);

    EXPECT_EQ(ERRCODE_BEACONFENCE_INCORRECT_ID, beaconFenceManager_->RemoveBeaconFence(beaconFence));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, ConstructFilterTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ConstructFilterTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ConstructFilterTest001 begin");
    std::vector<Bluetooth::BleScanFilter> filters;
    beaconFenceManager_->ConstructFilter(filters);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ConstructFilterTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, ReportFoundOrLostTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ReportFoundOrLostTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportFoundOrLostTest001 begin");
    Bluetooth::BleScanResult result;
    uint8_t type = 1;
    beaconFenceManager_->ReportFoundOrLost(result, type);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportFoundOrLostTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, ReportByFenceExtensionTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ReportByFenceExtensionTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByFenceExtensionTest001 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = nullptr;
    GeofenceTransitionEvent event = GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER;
    AppIdentity identity;
    beaconFenceManager_->ReportByFenceExtension(beaconFenceRequest, event, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByFenceExtensionTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, ReportByFenceExtensionTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ReportByFenceExtensionTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByFenceExtensionTest002 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("StartAddBeaconFenceTest001");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    GeofenceTransitionEvent event = GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER;
    AppIdentity identity;
    beaconFenceManager_->ReportByFenceExtension(beaconFenceRequest, event, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByFenceExtensionTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, IsStrValidForStoiTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsStrValidForStoiTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest001 begin");
    std::string str = "12@#$";
    EXPECT_EQ(false, beaconFenceManager_->IsStrValidForStoi(str));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, IsStrValidForStoiTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsStrValidForStoiTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest002 begin");
    std::string str = "12123";
    EXPECT_EQ(true, beaconFenceManager_->IsStrValidForStoi(str));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, GetAppIdentityByBeaconFenceRequestTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetAppIdentityByBeaconFenceRequestTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetAppIdentityByBeaconFenceRequestTest001 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = nullptr;
    beaconFenceManager_->GetAppIdentityByBeaconFenceRequest(beaconFenceRequest);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetAppIdentityByBeaconFenceRequestTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, GetAppIdentityByBeaconFenceRequestTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetAppIdentityByBeaconFenceRequestTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetAppIdentityByBeaconFenceRequestTest002 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("StartAddBeaconFenceTest001");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceManager_->GetAppIdentityByBeaconFenceRequest(beaconFenceRequest);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetAppIdentityByBeaconFenceRequestTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByCallbackTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByCallbackTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByCallbackTest001 begin");
    std::string serviceUuid = "";
    EXPECT_EQ(nullptr, beaconFenceManager_->GetBeaconFenceRequestByServiceUuid(serviceUuid));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByCallbackTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByCallbackTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByCallbackTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByCallbackTest001 begin");
    sptr<IRemoteObject> callbackObj;
    EXPECT_EQ(nullptr, beaconFenceManager_->GetBeaconFenceRequestByCallback(callbackObj));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByCallbackTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByPackageNameTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByPackageNameTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByPackageNameTest001 begin");
    std::string packageName;
    EXPECT_EQ(nullptr, beaconFenceManager_->GetBeaconFenceRequestByPackageName(packageName));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByPackageNameTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, RemoveBeaconFenceRequestByBeaconTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RemoveBeaconFenceRequestByBeaconTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceRequestByBeaconTest001 begin");
    std::shared_ptr<BeaconFence> beaconFence;
    beaconFenceManager_->RemoveBeaconFenceRequestByBeacon(beaconFence);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceRequestByBeaconTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByBeaconTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByBeaconTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByBeaconTest001 begin");
    std::shared_ptr<BeaconFence> beaconFence;
    beaconFenceManager_->GetBeaconFenceRequestByBeacon(beaconFence);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByBeaconTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, RemoveBeaconFenceRequestByCallbackTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RemoveBeaconFenceRequestByCallbackTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceRequestByCallbackTest001 begin");
    sptr<IRemoteObject> callbackObj;
    beaconFenceManager_->RemoveBeaconFenceRequestByCallback(callbackObj);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceRequestByCallbackTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, RemoveBeaconFenceByPackageNameTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RemoveBeaconFenceByPackageNameTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceByPackageNameTest001 begin");
    std::string packageName;
    beaconFenceManager_->RemoveBeaconFenceByPackageName(packageName);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceByPackageNameTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareUUIDTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareUUIDTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest001 begin");
    std::string uuid1 = "123";
    std::string uuid2 = "123";
    EXPECT_EQ(true, beaconFenceManager_->CompareUUID(uuid1, uuid2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareUUIDTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareUUIDTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest001 begin");
    std::string uuid1 = "123";
    std::string uuid2 = "456";
    EXPECT_EQ(false, beaconFenceManager_->CompareUUID(uuid1, uuid2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareBeaconFenceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareBeaconFenceTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest001 begin");
    std::shared_ptr<BeaconFence> beaconFence1;
    std::shared_ptr<BeaconFence> beaconFence2;
    EXPECT_EQ(false, beaconFenceManager_->CompareBeaconFence(beaconFence1, beaconFence2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconManufactureDataForFilterTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconManufactureDataForFilterTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconManufactureDataForFilterTest001 begin");
    beaconFenceManager_->GetBeaconManufactureDataForFilter();
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconManufactureDataForFilterTest001 end");
}
} // namespace Location
} // namespace OHOS