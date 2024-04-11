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
        << "LocatorRequiredDataManagerTest, OnWifiScanStateChanged001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] OnWifiScanStateChanged001 begin");
#ifdef WIFI_ENABLE
    auto wifiScanEventCallback =
		sptr<LocatorWifiScanEventCallback>(new (std::nothrow) LocatorWifiScanEventCallback());
    wifiScanEventCallback->OnWifiScanStateChanged(0);
    wifiScanEventCallback->OnWifiScanStateChanged(1);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] OnWifiScanStateChanged001 end");
#endif
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, RegisterCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback001 begin");
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
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback001 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, RegisterCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback002 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(2);
    dataConfig->SetNeedStartScan(false);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    auto callback =
        sptr<LocatingRequiredDataCallbackHost>(new (std::nothrow) LocatingRequiredDataCallbackHost());
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, callback->AsObject());
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback002 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, RegisterCallback003, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback003 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(1);
    dataConfig->SetNeedStartScan(false);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    auto callback =
        sptr<LocatingRequiredDataCallbackHost>(new (std::nothrow) LocatingRequiredDataCallbackHost());
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, callback->AsObject());
    EXPECT_EQ(ERRCODE_SUCCESS, errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback003 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost001 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    auto ret = locatorBluetoothHost->GetLocatingRequiredDataByBtHost(device);
    EXPECT_EQ(1, ret.size());
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost001 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost002 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    locatorBluetoothHost->OnStateChanged(0, 0);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost002 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost003, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost003 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    locatorBluetoothHost->OnDiscoveryStateChanged(0);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost003 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost005, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost005 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    locatorBluetoothHost->OnPairRequested(device);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost005 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost006, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost006 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    locatorBluetoothHost->OnScanModeChanged(0);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost006 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost007, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost007 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    locatorBluetoothHost->OnDeviceNameChanged("device");
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost007 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost008, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost008, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost008 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    locatorBluetoothHost->OnDeviceAddrChanged("device");
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost008 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost009, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost009 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    locatorBluetoothHost->OnPairConfirmed(device, 0, 0);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost009 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBleCallbackWapper001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBleCallbackWapper001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper001 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    Bluetooth::BleScanResult result;
    auto ret = callback->GetLocatingRequiredDataByBle(result);
    EXPECT_EQ(1, ret.size());
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper001 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBleCallbackWapper003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBleCallbackWapper003, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper003 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    std::vector<Bluetooth::BleScanResult> results;
    callback->OnBleBatchScanResultsEvent(results);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper003 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBleCallbackWapper004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBleCallbackWapper004, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper004 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    callback->OnStartOrStopScanEvent(0, true);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper004 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBleCallbackWapper005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBleCallbackWapper005, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper005 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    std::vector<uint8_t> vecs;
    Bluetooth::UUID uuid;
    callback->OnNotifyMsgReportFromLpDevice(uuid, 0, vecs);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper005 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, StartWifiScan001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, StartWifiScan001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] StartWifiScan001 begin");
    auto locatorDataManager = DelayedSingleton<LocatorRequiredDataManager>::GetInstance();
    locatorDataManager->StartWifiScan(true);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] StartWifiScan001 end");
}
}  // namespace Location
}  // namespace OHOS