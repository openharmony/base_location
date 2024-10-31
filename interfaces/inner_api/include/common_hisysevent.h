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

#ifndef COMMON_HISYSEVENT_H
#define COMMON_HISYSEVENT_H

#include <string>

namespace OHOS {
namespace Location {
void WriteGnssStateEvent(const std::string& state, const pid_t pid, const pid_t uid);
void WriteAppLocatingStateEvent(const std::string& state, const pid_t pid, const pid_t uid);
void WriteLocationSwitchStateEvent(const std::string& state);
void WriteLocationInnerEvent(const int event, std::vector<std::string> names, std::vector<std::string>& values);
void WriteLocationInnerEvent(const int event, std::initializer_list<std::string> params);
void WriteLocationRequestEvent(const std::string& packageName, const std::string& abilityName);

}  // namespace Location
}  // namespace OHOS
#endif // COMMON_HISYSEVENT_H