/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef COMMON_EVENT_HELPER_H
#define COMMON_EVENT_HELPER_H

#include <string>
#include <cstdint>

namespace OHOS {
namespace Location {
const std::string COMMON_EVENT_LOCATION_MODE_STATE_CHANGED = "usual.event.location.MODE_STATE_CHANGED";

class CommonEventHelper {
public:
    static bool PublishLocationModeChangeCommonEventAsUser(const int &modeValue, const int32_t &userId);
};

} // namespace Location
} // namespace OHOS
#endif // COMMON_EVENT_HELPER_H
