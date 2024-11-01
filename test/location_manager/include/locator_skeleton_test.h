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

#ifndef LOCATOR_SKELETON_TEST_H
#define LOCATOR_SKELETON_TEST_H

#include <gtest/gtest.h>

#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_coding_mock_info.h"
#endif
#include "i_locator_callback.h"
#define private public
#define protected public
#include "locator_ability.h"
#undef protected
#undef private
#include "locator_background_proxy.h"
#include "locator_proxy.h"
#include "request.h"
#include "request_manager.h"
#include "mock_locator_ability_stub.h"

namespace OHOS {
namespace Location {
class LocatorSkeletonTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    void MockNativePermission();
    void MockNativeWithoutMockPermission();
    void MockNativeWithoutSecurePermission();
    void MockNativeWithoutLocationPermission();
    void LoadSystemAbility();

    uint64_t tokenId_;
    uint64_t tokenId2_;
    uint64_t tokenId3_;
    uint64_t tokenId4_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_SKELETON_TEST_H
