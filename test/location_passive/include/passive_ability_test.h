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

#ifndef PASSIVE_ABILITY_TEST_H
#define PASSIVE_ABILITY_TEST_H
#ifdef FEATURE_PASSIVE_SUPPORT

#include <gtest/gtest.h>

#include "passive_ability.h"
#include "passive_ability_proxy.h"

namespace OHOS {
namespace Location {
class PassiveAbilityTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void MockNativePermission();
    
    static sptr<PassiveAbility> ability_;
    static sptr<PassiveAbilityProxy> proxy_;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_PASSIVE_SUPPORT
#endif // PASSIVE_ABILITY_TEST_H
