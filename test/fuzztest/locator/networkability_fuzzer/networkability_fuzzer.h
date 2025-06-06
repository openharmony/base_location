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
#ifndef NETWORKABILITY_FUZZER_H
#define NETWORKABILITY_FUZZER_H
#include <thread>

#include "securec.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "country_code_callback_napi.h"
#include "locator_callback_proxy.h"
#include "location_switch_callback_napi.h"
#include "locator_callback_napi.h"


namespace OHOS {
namespace Location {
#define FUZZ_PROJECT_NAME "networkability_fuzzer"

#ifdef FEATURE_NETWORK_SUPPORT
bool NetworkAbilityFuzzTest(const char* data, size_t size);
#endif
} // namespace Location
} // namespace OHOS
#endif // NETWORKABILITY_FUZZER_H
