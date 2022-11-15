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

#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include <gtest/gtest.h>

#include "cached_locations_callback_proxy.h"
#include "country_code_callback_proxy.h"
#include "gnss_status_callback_proxy.h"
#include "locator_callback_proxy.h"
#include "nmea_message_callback_proxy.h"
#include "switch_callback_proxy.h"

class CallbackTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};
#endif // CALLBACK_TEST_H
