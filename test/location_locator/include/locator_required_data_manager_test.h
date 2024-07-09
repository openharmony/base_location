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

#ifndef LOCATOR_REQUIRED_DATA_MANAGER_TEST_H
#define LOCATOR_REQUIRED_DATA_MANAGER_TEST_H

#include <gtest/gtest.h>

#include <map>
#include <mutex>
#include <string>

#ifdef BLUETOOTH_ENABLE
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_host.h"
#endif
#include "common_event_subscriber.h"
#include "constant_definition.h"
#include "event_handler.h"
#include "event_runner.h"
#include "iremote_stub.h"
#include "i_locating_required_data_callback.h"
#include "locating_required_data_config.h"
#ifdef WIFI_ENABLE
#include "wifi_scan.h"
#endif

namespace OHOS {
namespace Location {
class LocatorRequiredDataManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};
} // namespace Location
} // namespace OHOS
#endif // CALLBACK_TEST_H
