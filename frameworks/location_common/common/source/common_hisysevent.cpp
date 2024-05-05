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

#include "common_hisysevent.h"

#include <string>

#include <initializer_list>

#include "hisysevent.h"

#include "location_log.h"
#include "hook_utils.h"

namespace OHOS {
namespace Location {
template<typename... Types>
static void WriteEvent(const std::string& eventType, Types... args)
{
    int ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::LOCATION, eventType,
        HiviewDFX::HiSysEvent::EventType::STATISTIC, args...);
    if (ret != 0) {
        LBSLOGE(COMMON_UTILS, "Write event fail: %{public}s, ret = %{public}d", eventType.c_str(), ret);
    }
}

void WriteGnssStateEvent(const std::string& state, const pid_t pid, const pid_t uid)
{
    WriteEvent("GNSS_STATE", "STATE", state, "PID", pid, "UID", uid);
}

void WriteAppLocatingStateEvent(const std::string& state, const pid_t pid, const pid_t uid)
{
    WriteEvent("APP_LOCATING_STATE", "STATE", state, "PID", pid, "UID", uid);
}

void WriteLocationSwitchStateEvent(const std::string& state)
{
    WriteEvent("SWITCH_STATE", "STATE", state);
}

void WriteLocationInnerEvent(const int event, std::initializer_list<std::string> params)
{
    std::vector<std::string> names;
    std::vector<std::string> values;
    bool flag = true;
    for (auto x: params) {
        if (flag) {
            names.push_back(x);
        } else {
            values.push_back(x);
        }
        flag = !flag;
    }
    WriteEvent("LBS_CHR_INNER_EVENT", "EVENT", event, "NAMES", names, "VALUES", values);
    HookUtils::ExecuteHookWhenReportInnerInfo(event, names, values);
}

void WriteLocationInnerEvent(const int event, std::vector<std::string> names, std::vector<std::string>& values)
{
    WriteEvent("LBS_CHR_INNER_EVENT", "EVENT", event, "NAMES", names, "VALUES", values);
    HookUtils::ExecuteHookWhenReportInnerInfo(event, names, values);
}
}  // namespace Location
}  // namespace OHOS
