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
#define private public
#include "locator_required_data_manager.h"
#include "locating_required_data_callback_napi.h"
#undef private
#include "location_log.h"
#ifdef WIFI_ENABLE
#include "wifi_errcode.h"
#endif

using namespace testing::ext;

namespace OHOS {
namespace Location {
const uint32_t EVENT_START_SCAN = 0x0100;
const uint32_t EVENT_STOP_SCAN = 0x0200;
const uint32_t EVENT_GET_WIFI_LIST = 0x0300;
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
    wifiScanEventCallback->OnWifiScanStateChanged(0, 0);
    wifiScanEventCallback->OnWifiScanStateChanged(1, 1);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] OnWifiScanStateChanged001 end");
#endif
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, RegisterCallback001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback001 begin");
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();
    
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(1);
    dataConfig->SetNeedStartScan(true);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, nullptr);
    EXPECT_EQ(ERRCODE_INVALID_PARAM, errorCode);
    errorCode = locatorDataManager->UnregisterCallback(nullptr);

    auto callback =
        sptr<LocatingRequiredDataCallbackNapi>(new (std::nothrow) LocatingRequiredDataCallbackNapi());
    locatorDataManager->UnregisterCallback(callback->AsObject());
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback001 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, RegisterCallback002, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback002 begin");
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();
    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(2);
    dataConfig->SetNeedStartScan(false);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    auto callback =
        sptr<LocatingRequiredDataCallbackNapi>(new (std::nothrow) LocatingRequiredDataCallbackNapi());
    locatorDataManager->isWifiCallbackRegistered_ = true;
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, callback->AsObject());
    EXPECT_EQ(ERRCODE_NOT_SUPPORTED, errorCode);
    locatorDataManager->RegisterCallback(dataConfig, callback->AsObject());
    dataConfig->SetType(1);
    locatorDataManager->isWifiCallbackRegistered_ = true;
    locatorDataManager->RegisterCallback(dataConfig, callback->AsObject());
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback002 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, RegisterCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, RegisterCallback003, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback003 begin");
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();

    std::shared_ptr<LocatingRequiredDataConfig> dataConfig = std::make_shared<LocatingRequiredDataConfig>();
    dataConfig->SetType(1);
    dataConfig->SetNeedStartScan(false);
    dataConfig->SetScanIntervalMs(1);
    dataConfig->SetScanTimeoutMs(1);
    auto callback =
        sptr<LocatingRequiredDataCallbackNapi>(new (std::nothrow) LocatingRequiredDataCallbackNapi());
    LocationErrCode errorCode = locatorDataManager->RegisterCallback(dataConfig, callback->AsObject());
    EXPECT_EQ(ERRCODE_SUCCESS, errorCode);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] RegisterCallback003 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost001 begin");
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

    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    locatorBluetoothHost->OnStateChanged(0, 0);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost002 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost003, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost003, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost003 begin");

    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    locatorBluetoothHost->OnDiscoveryStateChanged(0);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost003 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost004, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost004, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost004 begin");
    
    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    Bluetooth::BluetoothRemoteDevice device;
    locatorBluetoothHost->OnDiscoveryResult(device, 0, "devicename", 0);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost004 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost005, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost005 begin");

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

    std::shared_ptr<LocatorBluetoothHost> locatorBluetoothHost = std::make_shared<LocatorBluetoothHost>();
    locatorBluetoothHost->OnDeviceAddrChanged("device");
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost008 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBluetoothHost009, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBluetoothHost009, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBluetoothHost009 begin");

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

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    callback->OnStartOrStopScanEvent(0, true);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper004 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBleCallbackWapper005, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBleCallbackWapper005, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper005 begin");

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    std::vector<uint8_t> vecs;
    Bluetooth::UUID uuid;
    callback->OnNotifyMsgReportFromLpDevice(uuid, 0, vecs);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper005 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBleCallbackWapper006, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBleCallbackWapper006, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper006 begin");

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    Bluetooth::BleScanResult result;
    callback->OnScanCallback(result);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper006 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, LocatorBleCallbackWapper007, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, LocatorBleCallbackWapper007, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper007 begin");

    std::shared_ptr<LocatorBleCallbackWapper> callback = std::make_shared<LocatorBleCallbackWapper>();
    Bluetooth::BleScanResult result;
    callback->OnFoundOrLostCallback(result, 1);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] LocatorBleCallbackWapper007 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, ProcessEvent001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, ProcessEvent001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] ProcessEvent001 begin");
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();

    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(EVENT_START_SCAN, 0);
    locatorDataManager->scanHandler_->ProcessEvent(event);
    sleep(1);
    AppExecFwk::InnerEvent::Pointer event1 =
        AppExecFwk::InnerEvent::Get(EVENT_STOP_SCAN, 0);
    locatorDataManager->scanHandler_->ProcessEvent(event1);
    sleep(1);
    AppExecFwk::InnerEvent::Pointer event2 =
        AppExecFwk::InnerEvent::Get(EVENT_GET_WIFI_LIST, 0);
    locatorDataManager->scanHandler_->ProcessEvent(event2);
    sleep(1);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] ProcessEvent001 end");
}

HWTEST_F(LocatorRequiredDataManagerTest, WifiServiceStatusChange001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorRequiredDataManagerTest, WifiServiceStatusChange001, TestSize.Level1";
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] WifiServiceStatusChange001 begin");
    int32_t systemAbilityId = 0;
    const std::string& deviceId = "id";
    auto locatorDataManager = LocatorRequiredDataManager::GetInstance();
    locatorDataManager->saStatusListener_->OnAddSystemAbility(systemAbilityId, deviceId);
    locatorDataManager->saStatusListener_->OnRemoveSystemAbility(systemAbilityId, deviceId);
    locatorDataManager->wifiScanPtr_ = nullptr;
    locatorDataManager->saStatusListener_->OnAddSystemAbility(systemAbilityId, deviceId);
    LBSLOGI(LOCATOR_CALLBACK, "[LocatorRequiredDataManagerTest] WifiServiceStatusChange001 end");
}
}  // namespace Location
}  // namespace OHOS