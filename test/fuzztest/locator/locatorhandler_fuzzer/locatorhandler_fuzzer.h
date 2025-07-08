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
#ifndef LOCATORHANDLER_FUZZER_H
#define LOCATORHANDLER_FUZZER_H
#include <thread>

#include "securec.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "cached_locations_callback_napi.h"
#endif
#include "country_code_callback_napi.h"
#include "locator_callback_proxy.h"

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_status_callback_napi.h"
#endif
#include "location_switch_callback_napi.h"
#include "locator_callback_napi.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "nmea_message_callback_napi.h"
#endif


namespace OHOS {
namespace Location {
#define FUZZ_PROJECT_NAME "locatorhandler_fuzzer"

bool LocatorHandlerFuzzTest(const char* data, size_t size);
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_FUZZER_H
