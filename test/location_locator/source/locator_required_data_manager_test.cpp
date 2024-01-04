/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "locator_required_data_manager_test.h"
#include "locator_required_data_manager.h"
#include "locating_required_data_callback_host.h"
#include "location_log.h"
#ifdef WIFI_ENABLE
#include "wifi_errcode.h"
#endif

using namespace testing::ext;

namespace OHOS {
namespace Location {
void LocatorRequiredDataManagerTest::SetUp()
{
}

void LocatorRequiredDataManagerTest::TearDown()
{
}

HWTEST_F(LocatorRequiredDataManagerTest, OnWifiScanStateChanged001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, OnWifiScanStateChanged001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnWifiScanStateChanged001 begin");
#ifdef WIFI_ENABLE
    auto wifiScanEventCallback =
		sptr<LocatorWifiScanEventCallback>(new (std::nothrow) LocatorWifiScanEventCallback());
    wifiScanEventCallback->OnWifiScanStateChanged(0);
    wifiScanEventCallback->OnWifiScanStateChanged(1);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] OnWifiScanStateChanged001 end");
#endif
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, RegisterCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] RegisterCallback001 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(1);
    dataConfig->SetNeedStartScan(false);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, nullptr);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errorCode);
    errorCode = locatorDataManager->UnregisterCallback(nullptr);
    EXPECT_EQ(ERRCODE_SERVICE_UNAVAILABLE, errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] RegisterCallback001 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, RegisterCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] RegisterCallback002 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(2);
    dataConfig->SetNeedStartScan(false);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    auto callback =
        sptr<LocatingRequiredDataCallbackHost>(new (std::nothrow) LocatingRequiredDataCallbackHost());
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, callback);
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] RegisterCallback002 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "CallbackTest, RegisterCallback003, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] RegisterCallback003 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(1);
    dataConfig->SetNeedStartScan(false);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    auto callback =
        sptr<LocatingRequiredDataCallbackHost>(new (std::nothrow) LocatingRequiredDataCallbackHost());
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, callback);
    EXPECT_EQ(ERRCODE_SUCCESS, errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[CallbackTest] RegisterCallback003 end");
}
}  // namespace Location
}  // namespace OHOS