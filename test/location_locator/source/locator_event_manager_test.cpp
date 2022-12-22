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

#include "locator_event_manager_test.h"

#include "event_handler.h"
#include "event_runner.h"

#include "constant_definition.h"
#include "locator_event_manager.h"
#include "request.h"
#include "request_config.h"

#include "location_log.h"

using namespace testing::ext;
namespace OHOS {
namespace Location {
void LocatorEventManagerTest::SetUp()
{
}

void LocatorEventManagerTest::TearDown()
{
}

HWTEST_F(LocatorEventManagerTest, DftEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, DftEventTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] DftEventTest001 begin");
    std::unique_ptr<DftEvent> dftEvent = std::make_unique<DftEvent>();
    EXPECT_NE(nullptr, dftEvent);
    dftEvent->PutInt("name", 1);
    EXPECT_EQ(1, dftEvent->GetInt("name"));
    EXPECT_EQ(0, dftEvent->GetInt("name_not_exist"));
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] DftEventTest001 end");
}

HWTEST_F(LocatorEventManagerTest, DftHandlerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, DftHandlerTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] DftHandlerTest002 begin");
    std::shared_ptr<DftHandler> dftHandler =
        std::make_shared<DftHandler>(AppExecFwk::EventRunner::Create(true));
    EXPECT_NE(nullptr, dftHandler);
    AppExecFwk::InnerEvent::Pointer event =
        AppExecFwk::InnerEvent::Get(0, dftHandler, 0);
    dftHandler->ProcessEvent(event); // empty func
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] DftHandlerTest002 end");
}

HWTEST_F(LocatorEventManagerTest, LocatorDftManagerInitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, LocatorDftManagerInitTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerInitTest001 begin");
    auto locatorDftManager =
        DelayedSingleton<LocatorDftManager>::GetInstance();
    EXPECT_NE(nullptr, locatorDftManager);
    locatorDftManager->Init();
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerInitTest001 end");
}

HWTEST_F(LocatorEventManagerTest, LocatorDftManagerIpcCallingErrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, LocatorDftManagerIpcCallingErrTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerIpcCallingErrTest001 begin");
    auto locatorDftManager =
        DelayedSingleton<LocatorDftManager>::GetInstance();
    EXPECT_NE(nullptr, locatorDftManager);
    locatorDftManager->IpcCallingErr(0);
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerIpcCallingErrTest001 end");
}

HWTEST_F(LocatorEventManagerTest, LocationSessionStartTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, LocationSessionStartTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocationSessionStartTest001 begin");
    auto locatorDftManager =
        DelayedSingleton<LocatorDftManager>::GetInstance();
    EXPECT_NE(nullptr, locatorDftManager);
    locatorDftManager->LocationSessionStart(nullptr);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(0);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorEventManagerTest");
    locatorDftManager->LocationSessionStart(request1); // no request config
    auto requestConfig1 = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig1);
    requestConfig1->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig1->SetMaxAccuracy(1000.0);
    requestConfig1->SetFixNumber(1);
    request1->SetRequestConfig(*requestConfig1);
    locatorDftManager->LocationSessionStart(request1); // has request config
    locatorDftManager->LocationSessionStart(request1); // the same request
    
    std::shared_ptr<Request> request2 = std::make_shared<Request>();
    request2->SetUid(1000);
    request2->SetPid(0);
    request2->SetTokenId(0);
    request2->SetFirstTokenId(0);
    request2->SetPackageName("LocatorEventManagerTest");
    auto requestConfig2 = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig2);
    requestConfig2->SetPriority(PRIORITY_UNSET);
    requestConfig2->SetMaxAccuracy(1000.0);
    requestConfig2->SetFixNumber(1);
    request2->SetRequestConfig(*requestConfig2);
    locatorDftManager->LocationSessionStart(request2); // different request config
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocationSessionStartTest001 end");
}

HWTEST_F(LocatorEventManagerTest, LocationSessionStartTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, LocationSessionStartTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocationSessionStartTest002 begin");
    auto locatorDftManager =
        DelayedSingleton<LocatorDftManager>::GetInstance();
    EXPECT_NE(nullptr, locatorDftManager);
    std::shared_ptr<Request> request1 = std::make_shared<Request>();
    request1->SetUid(1000);
    request1->SetPid(0);
    request1->SetTokenId(0);
    request1->SetFirstTokenId(0);
    request1->SetPackageName("LocatorEventManagerTest");
    locatorDftManager->LocationSessionStart(request1); // no request config
    auto requestConfig1 = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig1);
    requestConfig1->SetScenario(SCENE_NAVIGATION);
    requestConfig1->SetMaxAccuracy(1000.0);
    requestConfig1->SetFixNumber(1);
    request1->SetRequestConfig(*requestConfig1);
    locatorDftManager->LocationSessionStart(request1); // has request config

    locatorDftManager->LocationSessionStart(request1); // the same request
  
    std::shared_ptr<Request> request2 = std::make_shared<Request>();
    request2->SetUid(1000);
    request2->SetPid(0);
    request2->SetTokenId(0);
    request2->SetFirstTokenId(0);
    request2->SetPackageName("LocatorEventManagerTest");
    auto requestConfig2 = std::make_unique<RequestConfig>();
    EXPECT_NE(nullptr, requestConfig2);
    requestConfig2->SetScenario(SCENE_UNSET);
    requestConfig2->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig2->SetMaxAccuracy(1000.0);
    requestConfig2->SetFixNumber(1);
    request2->SetRequestConfig(*requestConfig2);
    locatorDftManager->LocationSessionStart(request2); // different request config
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocationSessionStartTest002 end");
}

HWTEST_F(LocatorEventManagerTest, LocatorDftManagerDistributionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, LocatorDftManagerDistributionTest001, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerDistributionTest001 begin");
    auto locatorDftManager =
        DelayedSingleton<LocatorDftManager>::GetInstance();
    EXPECT_NE(nullptr, locatorDftManager);
    for (uint32_t i = 0; i <= COUNT_MAX; i++) {
        locatorDftManager->DistributionDisconnect();
    }
    for (uint32_t i = 0; i <= COUNT_MAX; i++) {
        locatorDftManager->DistributionSessionStart();
    }
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerDistributionTest001 end");
}

HWTEST_F(LocatorEventManagerTest, LocatorDftManagerDistributionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "LocatorEventManagerTest, LocatorDftManagerDistributionTest002, TestSize.Level1";
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerDistributionTest002 begin");
    auto locatorDftManager =
        DelayedSingleton<LocatorDftManager>::GetInstance();
    EXPECT_NE(nullptr, locatorDftManager);
    locatorDftManager->SendDistributionDailyCount();
    locatorDftManager->SendRequestDailyCount();
    LBSLOGI(LOCATOR_EVENT, "[LocatorEventManagerTest] LocatorDftManagerDistributionTest002 end");
}
} // namespace Location
} // namespace OHOS
