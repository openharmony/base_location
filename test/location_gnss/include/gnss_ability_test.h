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

#ifndef GNSS_ABILITY_TEST_H
#define GNSS_ABILITY_TEST_H

#include <gtest/gtest.h>

#define private public
#include "agnss_event_callback.h"
#undef private

#include <v1_0/ia_gnss_interface.h>
#include <v1_0/ignss_interface.h>

#include "gnss_ability.h"
#include "gnss_ability_proxy.h"
#include "gnss_status_callback_host.h"

namespace OHOS {
namespace Location {
class GnssAbilityTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    void MockNativePermission();

    sptr<GnssAbilityProxy> proxy_;
    sptr<GnssStatusCallbackHost> callbackStub_;
    sptr<GnssAbility> ability_;
};
} // namespace Location
} // namespace OHOS
#endif  // GNSS_ABILITY_TEST_H
