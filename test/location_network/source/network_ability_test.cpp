/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "network_ability_test.h"

#include <cstdlib>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location_log.h"
#include "network_ability_skeleton.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Location;

void NetworkAbilityTest::SetUp()
{
    /*
     * @tc.setup: Get system ability's pointer and get sa proxy object.
     */
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, systemAbilityManager);
    if (systemAbilityManager->CheckSystemAbility(LOCATION_NETWORK_LOCATING_SA_ID) == nullptr) {
        LBSLOGE(NETWORK, "Can not get SA, return.");
    }
    sptr<IRemoteObject> systemAbility = systemAbilityManager->GetSystemAbility(LOCATION_NETWORK_LOCATING_SA_ID);
    EXPECT_NE(nullptr, systemAbility);
    proxy_ = new (std::nothrow) NetworkAbilityProxy(systemAbility);
    EXPECT_NE(nullptr, proxy_);
}

void NetworkAbilityTest::TearDown()
{
    /*
     * @tc.teardown: release memory.
     */
    proxy_ = nullptr;
}

/*
 * @tc.name: SendLocationRequest001
 * @tc.desc: Build Request, marshall and unmarshall data Then Send it
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, SendLocationRequest001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. build location request data.
     */
    std::unique_ptr<WorkRecord> workRecord = std::make_unique<WorkRecord>();
    int num = 2;
    for (int i = 0; i < num; i++) {
        int uid = i + 1;
        int pid = i + 2;
        std::string name = "nameForTest";
        workRecord->Add(uid, pid, name);
    }
    uint64_t interval = 1;
    
    /*
     * @tc.steps: step2. send location request
     * @tc.expected: step2. no exception happens.
     */
    proxy_->SendLocationRequest(interval, *workRecord);
    EXPECT_EQ(true, true); // always true
}

/*
 * @tc.name: SetEnableAndDisable001
 * @tc.desc: Test disable and enable system ability
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, SetEnableAndDisable001, TestSize.Level1)
{
    /*
     * @tc.steps: step1.remove SA
     * @tc.expected: step1. object1 is null.
     */
    proxy_->SetEnable(false); // if the state is false
    sptr<ISystemAbilityManager> systemAbilityManager1 =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object1 = systemAbilityManager1->GetSystemAbility(LOCATION_NETWORK_LOCATING_SA_ID);
    EXPECT_EQ(nullptr, object1); // no SA can be given

    /*
     * @tc.steps: step2. test enable SA
     * @tc.expected: step2. object2 is not null.
     */
    proxy_->SetEnable(true); // if the state is true
    sptr<ISystemAbilityManager> systemAbilityManager2 =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object2 = systemAbilityManager2->GetSystemAbility(LOCATION_NETWORK_LOCATING_SA_ID);
    EXPECT_NE(nullptr, object2); // SA can be given
}

/*
 * @tc.name: SelfRequest001
 * @tc.desc: test self request function
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, SelfRequest001, TestSize.Level1)
{
    /*
     * @tc.steps: step1. send location request
     * @tc.expected: step1. no exception happens.
     */
    proxy_->SelfRequest(true);
    EXPECT_EQ(true, true); // always true
}

/*
 * @tc.name: SelfRequest002
 * @tc.desc: test self request function
 * @tc.type: FUNC
 */
HWTEST_F(NetworkAbilityTest, SelfRequest002, TestSize.Level1)
{
    /*
     * @tc.steps: step1. send location request
     * @tc.expected: step1. no exception happens.
     */
    proxy_->SelfRequest(false);
    EXPECT_EQ(true, true); // always true
}