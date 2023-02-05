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

#ifdef FEATURE_GNSS_SUPPORT
#ifndef GNSS_ABILITY_FUZZER_H
#define GNSS_ABILITY_FUZZER_H

#include "message_option.h"
#include "message_parcel.h"

#include "gnss_ability.h"

namespace OHOS {
namespace Location {
class GnssAbilityTestFuzzer : public GnssAbility {
public:
    explicit GnssAbilityTestFuzzer() : GnssAbility()
    {}
    ~GnssAbilityTestFuzzer() = default;
};
} // namespace Location
} // namespace OHOS
#endif // GNSS_ABILITY_FUZZER_H
#endif // FEATURE_GNSS_SUPPORT
