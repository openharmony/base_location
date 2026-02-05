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
 
#include <cJSON.h>
#include "poi_info_manager_test.h"
#include "poi_info_callback_napi.h"
#include "common_utils.h"
 
using namespace testing::ext;
namespace OHOS {
namespace Location {
 
void PoiInfoManagerTest::SetUp()
{
    poiInfoManager_ = PoiInfoManager::GetInstance();
    EXPECT_NE(nullptr, poiInfoManager_);
}
 
void PoiInfoManagerTest::TearDown()
{
    poiInfoManager_ = nullptr;
}
 
std::unique_ptr<Location> PoiInfoManagerTest::MockLocation()
{
    std::unique_ptr<Location> location = std::make_unique<Location>();
    MessageParcel parcel;
    parcel.WriteDouble(12.0); // latitude
    parcel.WriteDouble(13.0); // longitude
    parcel.WriteDouble(14.0); // altitude
    parcel.WriteDouble(1000.0); // accuracy
    parcel.WriteDouble(10.0); // speed
    parcel.WriteDouble(90.0); // direction
    parcel.WriteInt64(1000000000); // timeStamp
    parcel.WriteInt64(1000000100); // timeSinceBoot
    parcel.WriteString16(u"additions"); // additions
    parcel.WriteInt64(1); // additionSize
    parcel.WriteInt32(1); // isFromMock
    location->ReadFromParcel(parcel);
    std::vector<std::string> additionMap = location->GetAdditions();
    return location;
}
 
std::string PoiInfoManagerTest::MockPoiString(uint64_t delayTimeMilSec)
{
    // 创建根对象
    cJSON *root = cJSON_CreateObject();
    // 创建 pois 数组
    cJSON *poisArray = cJSON_CreateArray();
    // 创建第一个 poi 对象
    cJSON *poi1 = cJSON_CreateObject();
    cJSON_AddStringToObject(poi1, "id", "42345678");
    cJSON_AddNumberToObject(poi1, "confidence", 1);
    cJSON_AddStringToObject(poi1, "name", "静安嘉里中心NB1-03A/NAB1-03C号单元");
    cJSON_AddNumberToObject(poi1, "lon", 31.111);
    cJSON_AddNumberToObject(poi1, "lat", 124.111);
    cJSON_AddStringToObject(poi1, "address", "南京西路街道南京西路1563号");
    cJSON_AddStringToObject(poi1, "formatAddress", "上海市静安区南京西路街道南京西路1563号静安嘉里中心NB1-03A/NAB1-03C号单元");
    // 创建第二个 poi 对象
    cJSON *poi2 = cJSON_CreateObject();
    cJSON_AddStringToObject(poi2, "id", "4537867");
    cJSON_AddNumberToObject(poi2, "confidence", 1);
    cJSON_AddStringToObject(poi2, "name", "静安嘉里中心NB1-03A/NAB1-03C号单元");
    cJSON_AddNumberToObject(poi2, "lon", 31.111);
    cJSON_AddNumberToObject(poi2, "lat", 124.111);
    cJSON_AddStringToObject(poi2, "address", "南京西路街道南京西路1563号");
    cJSON_AddStringToObject(poi2, "formatAddress", "上海市静安区南京西路街道南京西路1563号静安嘉里中心NB1-03A/NAB1-03C号单元");
    // 添加 poi 对象到数组中
    cJSON_AddItemToArray(poisArray, poi1);
    cJSON_AddItemToArray(poisArray, poi2);
    // 添加 pois 数组到根对象中
    cJSON_AddItemToObject(root, "pois", poisArray);
    // 添加其他字段到根对象
    cJSON_AddStringToObject(root, "semanticDescription", "Wagas沃歌斯(静安嘉里中心店)附近");
    cJSON_AddNumberToObject(root, "time", CommonUtils::GetCurrentTimeMilSec() - delayTimeMilSec);
    // 转换为字符串
    char *jsonStr = cJSON_PrintUnformatted(root);
    // 将字符串保存为 std::string，并释放 cJSON 字符串和对象
    std::string jsonString(jsonStr);
    cJSON_free(jsonStr);
    cJSON_Delete(root);
    return jsonString;
}
 
HWTEST_F(PoiInfoManagerTest, PoiInfoManagerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, PoiInfoManagerTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PoiInfoManagerTest001 begin");
    std::unique_ptr<Location> location = MockLocation();
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->UpdateCachedPoiInfo(location);
    poiInfoManager_->UpdateLocationPoiInfo(location);

    std::string mockPoiInfo = "poiInfos:" + MockPoiString(0);
    std::u16string mockPoiInfoStr16 = Str8ToStr16(mockPoiInfo);
    std::u16string mockStr16 = Str8ToStr16("test1:{empty}");
    std::vector<std::u16string> mockAdditionsStr16;
    mockAdditionsStr16.push_back(mockPoiInfoStr16);
    mockAdditionsStr16.push_back(mockStr16);
    location->VectorString16ToVectorString8(mockAdditionsStr16);
    
    poiInfoManager_->UpdateCachedPoiInfo(location);
    poiInfoManager_->SetLatestPoiInfoTime(CommonUtils::GetCurrentTimeMilSec());
    poiInfoManager_->UpdateLocationPoiInfo(location);

    std::unique_ptr<Location> location1 = MockLocation();
    poiInfoManager_->UpdateLocationPoiInfo(location1);
    poiInfoManager_->ClearPoiInfos(location1);
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PoiInfoManagerTest001 end");
}

HWTEST_F(PoiInfoManagerTest, PoiInfoManagerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, PoiInfoManagerTest002, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PoiInfoManagerTest002 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    std::unique_ptr<Location> location = MockLocation();
    poiInfoManager_->UpdateCachedPoiInfo(location);
    poiInfoManager_->UpdateLocationPoiInfo(location);
    
    std::string mockPoiInfo = "poiInfos:" + MockPoiString(60000);
    std::u16string mockPoiInfoStr16 = Str8ToStr16(mockPoiInfo);
    std::u16string mockStr16 = Str8ToStr16("test1:{empty}");
    std::vector<std::u16string> mockAdditionsStr16;
    mockAdditionsStr16.push_back(mockPoiInfoStr16);
    mockAdditionsStr16.push_back(mockStr16);
    location->VectorString16ToVectorString8(mockAdditionsStr16);
    
    poiInfoManager_->UpdateCachedPoiInfo(location);
    poiInfoManager_->SetLatestPoiInfoTime(CommonUtils::GetCurrentTimeMilSec() - 60000);
    poiInfoManager_->UpdateLocationPoiInfo(location);
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PoiInfoManagerTest002 end");
}

HWTEST_F(PoiInfoManagerTest, PreRequestPoiInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, PreRequestPoiInfoTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PreRequestPoiInfoTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    AppIdentity identity;
    sptr<IPoiInfoCallback> callback = sptr<IPoiInfoCallback>(new PoiInfoCallbackNapi());
    poiInfoManager_->PreRequestPoiInfo(callback->AsObject(), identity);
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PreRequestPoiInfoTest001 end");
}

HWTEST_F(PoiInfoManagerTest, RequestPoiInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, RequestPoiInfoTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] RequestPoiInfoTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    AppIdentity identity;
    sptr<IPoiInfoCallback> callback = sptr<IPoiInfoCallback>(new PoiInfoCallbackNapi());
    sptr<IRemoteObject> cb = callback->AsObject();
    poiInfoManager_->RequestPoiInfo(cb, identity);
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] RequestPoiInfoTest001 end");
}

HWTEST_F(PoiInfoManagerTest, ConnectPoiServiceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, ConnectPoiServiceTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] ConnectPoiServiceTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->ConnectPoiService();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] ConnectPoiServiceTest001 end");
}

HWTEST_F(PoiInfoManagerTest, PreDisconnectAbilityConnectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, PreDisconnectAbilityConnectTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PreDisconnectAbilityConnectTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->PreDisconnectAbilityConnect();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PreDisconnectAbilityConnectTest001 end");
}

HWTEST_F(PoiInfoManagerTest, DisconnectAbilityConnectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, DisconnectAbilityConnectTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] DisconnectAbilityConnectTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->DisconnectAbilityConnect();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] DisconnectAbilityConnectTest001 end");
}

HWTEST_F(PoiInfoManagerTest, IsConnectTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, IsConnectTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] IsConnectTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->IsConnect();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] IsConnectTest001 end");
}

HWTEST_F(PoiInfoManagerTest, RegisterPoiServiceDeathRecipientTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, RegisterPoiServiceDeathRecipientTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] RegisterPoiServiceDeathRecipientTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->RegisterPoiServiceDeathRecipient();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] RegisterPoiServiceDeathRecipientTest001 end");
}

HWTEST_F(PoiInfoManagerTest, UnregisterPoiServiceDeathRecipientTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, UnregisterPoiServiceDeathRecipientTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] UnregisterPoiServiceDeathRecipientTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->UnregisterPoiServiceDeathRecipient();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] UnregisterPoiServiceDeathRecipientTest001 end");
}

HWTEST_F(PoiInfoManagerTest, NotifyDisConnectedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, NotifyDisConnectedTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] NotifyDisConnectedTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->NotifyDisConnected();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] NotifyDisConnectedTest001 end");
}

HWTEST_F(PoiInfoManagerTest, PreResetServiceProxyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, PreResetServiceProxyTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PreResetServiceProxyTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->PreResetServiceProxy();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] PreResetServiceProxyTest001 end");
}

HWTEST_F(PoiInfoManagerTest, ResetServiceProxyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "PoiInfoManagerTest, ResetServiceProxyTest001, TestSize.Level1";
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] ResetServiceProxyTest001 begin");
    ASSERT_TRUE(poiInfoManager_ != nullptr);
    poiInfoManager_->ResetServiceProxy();
    LBSLOGI(REPORT_MANAGER, "[PoiInfoManagerTest] ResetServiceProxyTest001 end");
}
}  // namespace Location
}  // namespace OHOS