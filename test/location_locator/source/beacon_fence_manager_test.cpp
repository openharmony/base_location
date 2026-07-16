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
const int BEACON_FENCE_DATA_LENGTH = 18;
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

HWTEST_F(BeaconFenceManagerTest, RegisterBeaconFenceCallbackTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RegisterBeaconFenceCallbackTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RegisterBeaconFenceCallbackTest001 begin");
    AppIdentity identity;
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("RegisterBeaconFenceCallbackTest001");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceManager_->RegisterBeaconFenceCallback(beaconFenceRequest, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RegisterBeaconFenceCallbackTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, RegisterBeaconFenceCallbackTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RegisterBeaconFenceCallbackTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RegisterBeaconFenceCallbackTest002 begin");
    AppIdentity identity;
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("RegisterBeaconFenceCallbackTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost = new LocationGnssGeofenceCallbackNapi();
    beaconFenceRequest->SetBeaconFenceTransitionCallback(callbackHost->AsObject());
    beaconFenceManager_->RegisterBeaconFenceCallback(beaconFenceRequest, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RegisterBeaconFenceCallbackTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, IsBeaconFenceRequestExistsTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsBeaconFenceRequestExistsTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsBeaconFenceRequestExistsTest001 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("isBeaconFenceRequestExistsTest001");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceRequest->SetServiceUuid("121");

    EXPECT_EQ(false, beaconFenceManager_->IsBeaconFenceRequestExists(beaconFenceRequest));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsBeaconFenceRequestExistsTest001 end");
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

    EXPECT_EQ(ERRCODE_BEACONFENCE_INCORRECT_ID, beaconFenceManager_->RemoveBeaconFence(
        beaconFence, "RemoveBeaconFenceTest001"));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, StartBluetoothScanTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, StartBluetoothScanTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] StartBluetoothScanTest001 begin");
    beaconFenceManager_->StartBluetoothScan();
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] StartBluetoothScanTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, StopBluetoothScanTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, StopBluetoothScanTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] StopBluetoothScanTest001 begin");
    beaconFenceManager_->StopBluetoothScan();
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] StopBluetoothScanTest001 end");
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

HWTEST_F(BeaconFenceManagerTest, OnReportOperationResultByCallbackTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, OnReportOperationResultByCallbackTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] OnReportOperationResultByCallbackTest001 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("OnReportOperationResultByCallbackTest001");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    GnssGeofenceOperateType type = GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD;
    GnssGeofenceOperateResult result = GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS;

    beaconFenceManager_->OnReportOperationResultByCallback(beaconFenceRequest, type, result);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] OnReportOperationResultByCallbackTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, ExtractiBeaconUUIDTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ExtractiBeaconUUIDTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ExtractiBeaconUUIDTest001 begin");
    std::vector<uint8_t> data;
    beaconFenceManager_->ExtractiBeaconUUID(data);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ExtractiBeaconUUIDTest001 end");
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

HWTEST_F(BeaconFenceManagerTest, MatchesDataTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, MatchesDataTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] MatchesDataTest001 begin");
    std::vector<uint8_t> fData;
    std::string scanData;
    EXPECT_EQ(false, beaconFenceManager_->MatchesData(fData, scanData));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] MatchesDataTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, MatchesDataTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, MatchesDataTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] MatchesDataTest002 begin");
    std::vector<uint8_t> fData;
    fData.push_back(8);
    std::string scanData = "scanData";
    EXPECT_EQ(false, beaconFenceManager_->MatchesData(fData, scanData));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] MatchesDataTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, TransitionStatusChangeTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, TransitionStatusChangeTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] TransitionStatusChangeTest001 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = nullptr;
    GeofenceTransitionEvent event = GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER;
    AppIdentity identity;
    beaconFenceManager_->TransitionStatusChange(beaconFenceRequest, event, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] TransitionStatusChangeTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, TransitionStatusChangeTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, TransitionStatusChangeTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] TransitionStatusChangeTest002 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("TransitionStatusChangeTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    GeofenceTransitionEvent event = GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER;
    AppIdentity identity;
    beaconFenceManager_->TransitionStatusChange(beaconFenceRequest, event, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] TransitionStatusChangeTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, ReportByCallbackTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ReportByCallbackTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByCallbackTest001 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = nullptr;
    GeofenceTransitionEvent event = GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER;
    AppIdentity identity;
    beaconFenceManager_->ReportByCallback(beaconFenceRequest, event, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByCallbackTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, ReportByCallbackTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ReportByCallbackTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByCallbackTest002 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("ReportByCallbackTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    GeofenceTransitionEvent event = GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER;
    AppIdentity identity;
    beaconFenceManager_->ReportByCallback(beaconFenceRequest, event, identity);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ReportByCallbackTest002 end");
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
    beaconFence->SetIdentifier("ReportByFenceExtensionTest002");
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

HWTEST_F(BeaconFenceManagerTest, GetAppIdentityByBeaconFenceRequestTest001, TestSize.Level0)
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
    beaconFence->SetIdentifier("GetAppIdentityByBeaconFenceRequestTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceManager_->GetAppIdentityByBeaconFenceRequest(beaconFenceRequest);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetAppIdentityByBeaconFenceRequestTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByServiceUuidTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByServiceUuidTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByServiceUuidTest001 begin");
    std::string serviceUuid = "405";
    EXPECT_EQ(nullptr, beaconFenceManager_->GetBeaconFenceRequestByServiceUuid(serviceUuid));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByServiceUuidTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByCallbackTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByCallbackTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByCallbackTest001 begin");
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost = new LocationGnssGeofenceCallbackNapi();
    EXPECT_EQ(nullptr, beaconFenceManager_->GetBeaconFenceRequestByCallback(callbackHost->AsObject()));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByCallbackTest001 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByPackageNameTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByPackageNameTest001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByPackageNameTest001 begin");
    std::string packageName = "GetBeaconFenceRequestByPackageNameTest001";
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

HWTEST_F(BeaconFenceManagerTest, CompareUUIDTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareUUIDTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest002 begin");
    std::string uuid1 = "123";
    std::string uuid2 = "456";
    EXPECT_EQ(false, beaconFenceManager_->CompareUUID(uuid1, uuid2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest002 end");
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

HWTEST_F(BeaconFenceManagerTest, ExtractiBeaconUUIDTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, ExtractiBeaconUUIDTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ExtractiBeaconUUIDTest002 begin");
    std::vector<uint8_t> data;
    for (int i = 0; i < BEACON_FENCE_DATA_LENGTH; i++) {
        data.push_back(static_cast<uint8_t>(i + 1));
    }
    std::string uuid = beaconFenceManager_->ExtractiBeaconUUID(data);
    EXPECT_NE("", uuid);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] ExtractiBeaconUUIDTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, MatchesDataTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, MatchesDataTest003, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] MatchesDataTest003 begin");
    std::vector<uint8_t> fData;
    fData.push_back(0x4C);
    fData.push_back(0x00);
    fData.push_back(0x12);
    fData.push_back(0xFB);
    fData.push_back(0x5B);
    fData.push_back(0xBF);
    fData.push_back(0x8D);
    fData.push_back(0x9E);
    fData.push_back(0x4E);
    fData.push_back(0x14);
    fData.push_back(0x9F);
    fData.push_back(0xBB);
    fData.push_back(0x62);
    fData.push_back(0x00);
    fData.push_back(0x00);
    fData.push_back(0x00);
    fData.push_back(0x00);
    fData.push_back(0xC5);
    std::string scanData = "\x4C\x00\x12\xFB\x5B\xBF\x8D\x9E\x4E\x14\x9F\xBB\x62\x00\x00\x00\x00\xC5";
    beaconFenceManager_->MatchesData(fData, scanData);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] MatchesDataTest003 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareBeaconFenceTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareBeaconFenceTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest002 begin");
    std::shared_ptr<BeaconFence> beaconFence1 = std::make_shared<BeaconFence>();
    std::shared_ptr<BeaconFence> beaconFence2 = nullptr;
    EXPECT_EQ(false, beaconFenceManager_->CompareBeaconFence(beaconFence1, beaconFence2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareBeaconFenceTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareBeaconFenceTest003, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest003 begin");
    std::shared_ptr<BeaconFence> beaconFence1 = std::make_shared<BeaconFence>();
    std::shared_ptr<BeaconFence> beaconFence2 = std::make_shared<BeaconFence>();
    beaconFence1->SetIdentifier("CompareTest003_1");
    beaconFence2->SetIdentifier("CompareTest003_2");
    EXPECT_EQ(false, beaconFenceManager_->CompareBeaconFence(beaconFence1, beaconFence2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest003 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareBeaconFenceTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareBeaconFenceTest005, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest005 begin");
    std::shared_ptr<BeaconFence> beaconFence1 = std::make_shared<BeaconFence>();
    std::shared_ptr<BeaconFence> beaconFence2 = std::make_shared<BeaconFence>();
    beaconFence1->SetIdentifier("CompareTest005");
    beaconFence2->SetIdentifier("CompareTest005");
    beaconFence1->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence2->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    BeaconManufactureData manufactureData1;
    manufactureData1.manufactureId = 76;
    BeaconManufactureData manufactureData2;
    manufactureData2.manufactureId = 77;
    beaconFence1->SetBeaconManufactureData(manufactureData1);
    beaconFence2->SetBeaconManufactureData(manufactureData2);
    EXPECT_EQ(false, beaconFenceManager_->CompareBeaconFence(beaconFence1, beaconFence2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest005 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareBeaconFenceTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareBeaconFenceTest006, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest006 begin");
    std::shared_ptr<BeaconFence> beaconFence1 = std::make_shared<BeaconFence>();
    std::shared_ptr<BeaconFence> beaconFence2 = std::make_shared<BeaconFence>();
    beaconFence1->SetIdentifier("CompareTest006");
    beaconFence2->SetIdentifier("CompareTest006");
    beaconFence1->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence2->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    BeaconManufactureData manufactureData1;
    manufactureData1.manufactureId = 76;
    manufactureData1.manufactureData = {0x4C, 0x00, 0x12};
    BeaconManufactureData manufactureData2;
    manufactureData2.manufactureId = 76;
    manufactureData2.manufactureData = {0x4C, 0x00, 0x13};
    beaconFence1->SetBeaconManufactureData(manufactureData1);
    beaconFence2->SetBeaconManufactureData(manufactureData2);
    EXPECT_EQ(false, beaconFenceManager_->CompareBeaconFence(beaconFence1, beaconFence2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest006 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareBeaconFenceTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareBeaconFenceTest007, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest007 begin");
    std::shared_ptr<BeaconFence> beaconFence1 = std::make_shared<BeaconFence>();
    std::shared_ptr<BeaconFence> beaconFence2 = std::make_shared<BeaconFence>();
    beaconFence1->SetIdentifier("CompareTest007");
    beaconFence2->SetIdentifier("CompareTest007");
    beaconFence1->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence2->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    BeaconManufactureData manufactureData1;
    manufactureData1.manufactureId = 76;
    manufactureData1.manufactureData = {0x4C, 0x00, 0x12};
    manufactureData1.manufactureDataMask = {0xFF, 0xFF, 0xFF};
    BeaconManufactureData manufactureData2;
    manufactureData2.manufactureId = 76;
    manufactureData2.manufactureData = {0x4C, 0x00, 0x12};
    manufactureData2.manufactureDataMask = {0xFF, 0xFF, 0x00};
    beaconFence1->SetBeaconManufactureData(manufactureData1);
    beaconFence2->SetBeaconManufactureData(manufactureData2);
    EXPECT_EQ(false, beaconFenceManager_->CompareBeaconFence(beaconFence1, beaconFence2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest007 end");
}

HWTEST_F(BeaconFenceManagerTest, IsStrValidForStoiTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsStrValidForStoiTest003, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest003 begin");
    std::string str = "12345678";
    EXPECT_EQ(false, beaconFenceManager_->IsStrValidForStoi(str));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest003 end");
}

HWTEST_F(BeaconFenceManagerTest, IsStrValidForStoiTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsStrValidForStoiTest004, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest004 begin");
    std::string str = "";
    EXPECT_EQ(false, beaconFenceManager_->IsStrValidForStoi(str));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest004 end");
}

HWTEST_F(BeaconFenceManagerTest, IsStrValidForStoiTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsStrValidForStoiTest005, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest005 begin");
    std::string str = "12345678";
    EXPECT_EQ(false, beaconFenceManager_->IsStrValidForStoi(str));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest005 end");
}

HWTEST_F(BeaconFenceManagerTest, IsStrValidForStoiTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsStrValidForStoiTest006, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest006 begin");
    std::string str = "12345a";
    EXPECT_EQ(false, beaconFenceManager_->IsStrValidForStoi(str));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsStrValidForStoiTest006 end");
}

HWTEST_F(BeaconFenceManagerTest, GenerateBeaconFenceId001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GenerateBeaconFenceId001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GenerateBeaconFenceId001 begin");
    int32_t id1 = beaconFenceManager_->GenerateBeaconFenceId();
    int32_t id2=  beaconFenceManager_->GenerateBeaconFenceId();
    EXPECT_GT(id2, id1);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GenerateBeaconFenceId001 end");
}

HWTEST_F(BeaconFenceManagerTest, CheckIfExceedsLimitForOneApp001, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CheckIfExceedsLimitForOneApp001, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CheckIfExceedsLimitForOneApp001 begin");
    bool result = beaconFenceManager_->CheckIfExceedsLimitForOneApp("test_bundle_name");
    EXPECT_EQ(false, result);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CheckIfExceedsLimitForOneApp001 end");
}

HWTEST_F(BeaconFenceManagerTest, IsBeaconFenceRequestExists002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, IsBeaconFenceRequestExists002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsBeaconFenceRequestExists002 begin");
    AppIdentity identity;
    identity.SetBundleName("test_bundle_name");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    manufactureData.manufactureData = {0x4C, 0x00, 0x12, 0xFB, 0x5B, 0xBF, 0x8D, 0x9E, 0x4E, 0x14, 0x9F, 0xBB, 0x62, 0x00, 0x00, 0x00, 0x00, 0xC5};
    beaconFence->SetIdentifier("IsBeaconFenceRequestExists002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetBundleName("test_bundle_name");
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceManager_->RegisterBeaconFenceCallback(beaconFenceRequest, identity);
    bool exists = beaconFenceManager_->IsBeaconFenceRequestExists(beaconFenceRequest);
    EXPECT_EQ(true, exists);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] IsBeaconFenceRequestExists002 end");
}

HWTEST_F(BeaconFenceManagerTest, AddBeaconFenceTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, AddBeaconFenceTest003, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] AddBeaconFenceTest003 begin");
    AppIdentity identity;
    identity.SetBundleName("test_bundle_name");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    manufactureData.manufactureData = {0x4C, 0x00, 0x12, 0xFB, 0x5B, 0xBF, 0x8D, 0x9E, 0x4E, 0x14, 0x9F, 0xBB, 0x62, 0x00, 0x00, 0x00, 0x00, 0xC5};
    beaconFence->SetIdentifier("AddBeaconFenceTest003");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetBundleName("test_bundle_name");
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    EXPECT_EQ(ERRCODE_SUCCESS, beaconFenceManager_->AddBeaconFence(beaconFenceRequest, identity));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] AddBeaconFenceTest003 end");
}

HWTEST_F(BeaconFenceManagerTest, RemoveBeaconFenceTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RemoveBeaconFenceTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceTest002 begin");
    AppIdentity identity;
    identity.SetBundleName("test_bundle_name_remove");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    manufactureData.manufactureData = {0x4C, 0x00, 0x12, 0xFB, 0x5B, 0xBF, 0x8D, 0x9E, 0x4E, 0x14, 0x9F, 0xBB, 0x62, 0x00, 0x00, 0x00, 0x00, 0xC5};
    beaconFence->SetIdentifier("RemoveBeaconFenceTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetBundleName("test_bundle_name_remove");
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceManager_->AddBeaconFence(beaconFenceRequest, identity);
    ErrCode result = beaconFenceManager_->RemoveBeaconFence(beaconFence, "test_bundle_name_remove");
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, RemoveBeaconFenceTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, RemoveBeaconFenceTest003, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceTest003 begin");
    AppIdentity identity;
    identity.SetBundleName("test_bundle_name_wrong");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    manufactureData.manufactureData = {0x4C, 0x00, 0x12, 0xFB, 0x5B, 0xBF, 0x8D, 0x9E, 0x4E, 0x14, 0x9F, 0xBB, 0x62, 0x00, 0x00, 0x00, 0x00, 0xC5};
    beaconFence->SetIdentifier("RemoveBeaconFenceTest003");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetBundleName("test_bundle_name_wrong");
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceManager_->AddBeaconFence(beaconFenceRequest, identity);
    ErrCode result = beaconFenceManager_->RemoveBeaconFence(beaconFence, "wrong_bundle_name");
    EXPECT_EQ(ERRCODE_BEACONFENCE_INCORRECT_ID, result);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] RemoveBeaconFenceTest003 end");
}

HWTEST_F(BeaconFenceManagerTest, OnReportOperationResultByCallbackTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, OnReportOperationResultByCallbackTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] OnReportOperationResultByCallbackTest002 begin");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    beaconFence->SetIdentifier("OnReportOperationResultByCallbackTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceRequest->SetFenceId("123");
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost = new LocationGnssGeofenceCallbackNapi();
    beaconFenceRequest->SetBeaconFenceTransitionCallback(callbackHost->AsObject());
    GnssGeofenceOperateType type = GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD;
    GnssGeofenceOperateResult result = GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS;
    beaconFenceManager_->OnReportOperationResultByCallback(beaconFenceRequest, type, result);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] OnReportOperationResultByCallbackTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareUUIDTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareUUIDTest003, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest003 begin");
    std::string uuid1 = "ABCDEF12-1234-5678-9ABC-DEF012345678";
    std::string uuid2 = "abcdef12-1234-5678-9abc-def012345678";
    EXPECT_EQ(true, beaconFenceManager_->CompareUUID(uuid1, uuid2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest003 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareUUIDTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareUUIDTest004, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest004 begin");
    std::string uuid1 = "ABCDEF12-1234-5678-9ABC-DEF012345678";
    std::string uuid2 = "ABCDEF12-1234-5678-9ABC-DEF012345679";
    EXPECT_EQ(false, beaconFenceManager_->CompareUUID(uuid1, uuid2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareUUIDTest004 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconFenceRequestByServiceUuidTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconFenceRequestByServiceUuidTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByServiceUuidTest002 begin");
    AppIdentity identity;
    identity.SetBundleName("test_bundle_name_uuid");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    manufactureData.manufactureData = {0x4C, 0x00, 0x12, 0xFB, 0x5B, 0xBF, 0x8D, 0x9E, 0x4E, 0x14, 0x9F, 0xBB, 0x62, 0x00, 0x00, 0x00, 0x00, 0xC5};
    beaconFence->SetIdentifier("GetBeaconFenceRequestByServiceUuidTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetBundleName("test_bundle_name_uuid");
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceRequest->SetFenceId("123");
    beaconFenceRequest->SetServiceUuid("e2c56db5-dffb-48d2-b060-d0f5a71096e0");
    beaconFenceManager_->RegisterBeaconFenceCallback(beaconFenceRequest, identity);
    auto result = beaconFenceManager_->GetBeaconFenceRequestByServiceUuid("e2c56db5-dffb-48d2-b060-d0f5a71096e0");
    EXPECT_NE(nullptr, result);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconFenceRequestByServiceUuidTest002 end");
}

HWTEST_F(BeaconFenceManagerTest, GetAppIdentityByBeaconFenceRequestTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetAppIdentityByBeaconFenceRequestTest003, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetAppIdentityByBeaconFenceRequestTest003 begin");
    AppIdentity identity;
    identity.SetBundleName("test_bundle_name_identity");
    identity.SetUid(12345);
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    manufactureData.manufactureData = {0x4C, 0x00, 0x12, 0xFB, 0x5B, 0xBF, 0x8D, 0x9E, 0x4E, 0x14, 0x9F, 0xBB, 0x62, 0x00, 0x00, 0x00, 0x00, 0xC5};
    beaconFence->SetIdentifier("GetAppIdentityByBeaconFenceRequestTest003");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetBundleName("test_bundle_name_identity");
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceRequest->SetFenceId("123");
    beaconFenceRequest->SetServiceUuid("e2c56db5-dffb-48d2-b060-d0f5a71096e0");
    beaconFenceManager_->RegisterBeaconFenceCallback(beaconFenceRequest, identity);
    beaconFenceManager_->GetAppIdentityByBeaconFenceRequest(beaconFenceRequest);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetAppIdentityByBeaconFenceRequestTest003 end");
}

HWTEST_F(BeaconFenceManagerTest, CompareBeaconFenceTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, CompareBeaconFenceTest008, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest008 begin");
    std::shared_ptr<BeaconFence> beaconFence1 = std::make_shared<BeaconFence>();
    std::shared_ptr<BeaconFence> beaconFence2 = std::make_shared<BeaconFence>();
    beaconFence1->SetIdentifier("CompareTest008");
    beaconFence2->SetIdentifier("CompareTest008");
    beaconFence1->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence2->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    BeaconManufactureData manufactureData1;
    manufactureData1.manufactureId = 76;
    manufactureData1.manufactureData = {0x4C, 0x00, 0x12};
    manufactureData1.manufactureDataMask = {0xFF, 0xFF, 0xFF};
    BeaconManufactureData manufactureData2;
    manufactureData2.manufactureId = 76;
    manufactureData2.manufactureData = {0x4C, 0x00, 0x12};
    manufactureData2.manufactureDataMask = {0xFF, 0xFF, 0xFF};
    beaconFence1->SetBeaconManufactureData(manufactureData1);
    beaconFence2->SetBeaconManufactureData(manufactureData2);
    EXPECT_EQ(true, beaconFenceManager_->CompareBeaconFence(beaconFence1, beaconFence2));
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] CompareBeaconFenceTest008 end");
}

HWTEST_F(BeaconFenceManagerTest, GetBeaconManufactureDataForFilterTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO)
        << "BeaconFenceManagerTest, GetBeaconManufactureDataForFilterTest002, TestSize.Level0";
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconManufactureDataForFilterTest002 begin");
    AppIdentity identity;
    identity.SetBundleName("test_bundle_name_filter");
    std::shared_ptr<BeaconFenceRequest> beaconFenceRequest = std::make_shared<BeaconFenceRequest>();
    std::shared_ptr<BeaconFence> beaconFence = std::make_shared<BeaconFence>();
    BeaconManufactureData manufactureData;
    manufactureData.manufactureId = 76;
    manufactureData.manufactureData = {0x4C, 0x00, 0x12, 0xFB, 0x5B, 0xBF, 0x8D, 0x9E, 0x4E, 0x14, 0x9F, 0xBB, 0x62, 0x00, 0x00, 0x00, 0x00, 0xC5};
    beaconFence->SetIdentifier("GetBeaconManufactureDataForFilterTest002");
    beaconFence->SetBeaconFenceInfoType(BeaconFenceInfoType::BEACON_MANUFACTURE_DATA);
    beaconFence->SetBeaconManufactureData(manufactureData);
    beaconFenceRequest->SetBeaconFence(beaconFence);
    beaconFenceRequest->SetBundleName("test_bundle_name_filter");
    beaconFenceRequest->SetFenceExtensionAbilityName("ExtensionAbility");
    beaconFenceRequest->SetFenceId("123");
    beaconFenceManager_->RegisterBeaconFenceCallback(beaconFenceRequest, identity);
    auto filters = beaconFenceManager_->GetBeaconManufactureDataForFilter();
    EXPECT_GT(filters.size(), 0);
    LBSLOGI(BEACON_FENCE_MANAGER, "[BeaconFenceManagerTest] GetBeaconManufactureDataForFilterTest002 end");
}
} // namespace Location
} // namespace OHOS