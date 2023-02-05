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

#ifdef FEATURE_NETWORK_SUPPORT
#ifndef NETWORK_ABILITY_STUB_TEST_H
#define NETWORK_ABILITY_STUB_TEST_H

#include <gtest/gtest.h>

#include "mock_network_ability_stub.h"

namespace OHOS {
namespace Location {
class NetworkAbilityStubTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};
} // namespace Location
} // namespace OHOS
#endif // NETWORK_ABILITY_STUB_TEST_H
#endif // FEATURE_NETWORK_SUPPORT
