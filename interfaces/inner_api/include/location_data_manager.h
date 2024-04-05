/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef LOCATION_DATA_MANAGER_H
#define LOCATION_DATA_MANAGER_H

#include <map>
#include <singleton.h>
#include "iremote_object.h"
#include "constant_definition.h"
#include "i_switch_callback.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
const std::string LOCATION_DATA_ABILITY_PREFIX = "datashare://";
const std::string LOCATION_DATA_URI_ID =
    "/com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=location_enable";
const std::string LOCATION_DATA_URI = LOCATION_DATA_ABILITY_PREFIX + LOCATION_DATA_URI_ID;
const std::string LOCATION_DATA_COLUMN_KEYWORD = "KEYWORD";
const std::string LOCATION_DATA_COLUMN_VALUE = "VALUE";
const std::string LOCATION_DATA_COLUMN_ENABLE = "location_switch_enable";
const std::string LOCATION_WORKING_STATE = "location_working_state";
class LocationDataManager : public DelayedSingleton<LocationDataManager> {
public:
    LocationDataManager();
    ~LocationDataManager();
    LocationErrCode ReportSwitchState(bool isEnabled);
    LocationErrCode RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterSwitchCallback(const sptr<IRemoteObject>& callback);
    void SetCachedSwitchState(int32_t state);
    bool IsSwitchStateReg();
    static int QuerySwitchState();
    static std::string GetLocationDataUri(std::string key);
    static LocationErrCode SetSwitchState(int modeValue);
    static bool SetLocationWorkingState(int32_t state);
    static bool GetLocationWorkingState(int32_t& state);
private:
    std::mutex mutex_;
    std::mutex switchStateMutex_;
    static ::mutex locationWorkingState_;
    std::unique_ptr<std::map<pid_t, sptr<ISwitchCallback>>> switchCallbacks_;
    int32_t cachedSwitchState_ = DISABLED;
    bool isStateCached_ = false;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_DATA_MANAGER_H
