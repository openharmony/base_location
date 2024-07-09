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

#include "location_gnss_geofence_callback_host_test.h"

#include "location_gnss_geofence_callback_napi.h"
#include "location_log.h"
#include "geofence_definition.h"
#include "i_gnss_geofence_callback.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void LocationGnssGeofenceCallbackHostTest::SetUp()
{
}

void LocationGnssGeofenceCallbackHostTest::TearDown()
{
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, OnRemoteRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, OnRemoteRequestTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] OnRemoteRequestTest001 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    MessageParcel dataParcel;
    dataParcel.WriteInt32(DEFAULT_NUM_VALUE);
    dataParcel.WriteInt32(static_cast<int>(GeofenceTransitionEvent::GEOFENCE_TRANSITION_EVENT_ENTER));
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    callback->OnRemoteRequest(IGnssGeofenceCallback::RECEIVE_TRANSITION_STATUS_EVENT, dataParcel, reply, option);
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] OnRemoteRequestTest001 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, OnRemoteRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, OnRemoteRequestTest002, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] OnRemoteRequestTest002 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    MessageParcel dataParcel;
    dataParcel.WriteInt32(DEFAULT_NUM_VALUE);
    dataParcel.WriteInt32(static_cast<int>(GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD));
    dataParcel.WriteInt32(static_cast<int>(GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS));
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    callback->OnRemoteRequest(IGnssGeofenceCallback::REPORT_OPERATION_RESULT_EVENT, dataParcel, reply, option);
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] OnRemoteRequestTest002 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, IsRemoteDiedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, IsRemoteDiedTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] IsRemoteDiedTest001 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    callback->IsRemoteDied();
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] IsRemoteDiedTest001 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, DeleteHandlerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, DeleteHandlerTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] DeleteHandlerTest001 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    callback->DeleteHandler();
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] DeleteHandlerTest001 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, LatchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, LatchTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] LatchTest001 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    callback->SetCount(1);
    int countNum = callback->GetCount();
    ASSERT_TRUE(countNum == DEFAULT_NUM_VALUE);
    callback->Wait(DEFAULT_CALLBACK_WAIT_TIME);
    countNum = callback->GetCount();
    ASSERT_TRUE(countNum == DEFAULT_NUM_VALUE);
    callback->CountDown();
    countNum = callback->GetCount();
    ASSERT_TRUE(countNum != DEFAULT_NUM_VALUE);
    callback->SetCount(1);
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] LatchTest001 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, FenceIdOperationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, FenceIdOperationTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] FenceIdOperationTest001 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    callback->SetFenceId(DEFAULT_NUM_VALUE);
    int fenceId = callback->GetFenceId();
    ASSERT_TRUE(fenceId == DEFAULT_NUM_VALUE);
    callback->ClearFenceId();
    fenceId = callback->GetFenceId();
    ASSERT_TRUE(fenceId != DEFAULT_NUM_VALUE);
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] FenceIdOperationTest001 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, GeofenceOperationTypeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, GeofenceOperationTypeTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] GeofenceOperationTypeTest001 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    GnssGeofenceOperateType optType = static_cast<GnssGeofenceOperateType>(0);
    callback->SetGeofenceOperationType(optType);
    GnssGeofenceOperateType optResultType = callback->GetGeofenceOperationType();
    ASSERT_TRUE(optResultType == optType);
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] GeofenceOperationTypeTest001 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, GeofenceOperationResultTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, GeofenceOperationResultTest001, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] GeofenceOperationResultTest001 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    GnssGeofenceOperateResult optValue = static_cast<GnssGeofenceOperateResult>(0);
    callback->SetGeofenceOperationResult(optValue);
    GnssGeofenceOperateResult optResult = callback->GetGeofenceOperationResult();
    ASSERT_TRUE(optValue == optResult);
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] GeofenceOperationResultTest001 end");
}

HWTEST_F(LocationGnssGeofenceCallbackHostTest, GeofenceOperationResultTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocationGnssGeofenceCallbackHostTest, GeofenceOperationResultTest002, TestSize.Level1";
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] GeofenceOperationResultTest002 begin");
    std::shared_ptr<LocationGnssGeofenceCallbackNapi> callback =
        std::make_shared<LocationGnssGeofenceCallbackNapi>();
    ASSERT_TRUE(callback != nullptr);
    callback->SetGeofenceOperationResult(GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_SUCCESS);
    callback->DealGeofenceOperationResult();
    callback->SetGeofenceOperationResult(GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_UNKNOWN);
    callback->DealGeofenceOperationResult();
    callback->SetGeofenceOperationResult(GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_TOO_MANY_GEOFENCES);
    callback->DealGeofenceOperationResult();
    callback->SetGeofenceOperationResult(GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_EXISTS);
    callback->DealGeofenceOperationResult();
    callback->SetGeofenceOperationResult(GnssGeofenceOperateResult::GNSS_GEOFENCE_OPERATION_ERROR_PARAMS_INVALID);
    callback->DealGeofenceOperationResult();
    LBSLOGI(GEOFENCE_SDK_TEST, "[LocationGnssGeofenceCallbackHostTest] GeofenceOperationResultTest002 end");
}
} // namespace Location
} // namespace OHOS