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

#ifndef OHOS_LOCATION_CONFIG_MANAGER_H
#define OHOS_LOCATION_CONFIG_MANAGER_H

#include <atomic>
#include <mutex>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "constant_definition.h"

constexpr int STATE_OPEN = 1;
constexpr int STATE_CLOSE = 0;
constexpr int32_t PER_USER_RANGE = 100000;
constexpr int PRIVACY_TYPE_OTHERS = 0;
constexpr int PRIVACY_TYPE_STARTUP = 1;
constexpr int PRIVACY_TYPE_CORE_LOCATION = 2;

namespace OHOS {
namespace Location {
class LocationConfigManager {
public:
    ~LocationConfigManager();
    static LocationConfigManager &GetInstance();

    /**
     * @Description Init the LocationConfigManager object
     *
     * @return int - init result, when 0 means success, other means some fails happened
     */
    int Init();

    /**
     * @Description Get current location switch state
     *
     * @return int - the location switch state, open/close
     */
    int GetLocationSwitchState();

    /**
     * @Description set location switch state
     *
     * @param state - the location switch state
     * @return int - 0 success
     */
    int SetLocationSwitchState(int state);

    bool IsExistFile(const std::string& filename);
    bool CreateFile(const std::string& filename, const std::string& filedata);

    std::string GetPrivacyTypeConfigPath(const LocationPrivacyType type);
    bool GetPrivacyTypeState(const LocationPrivacyType type);
    void SetPrivacyTypeState(const LocationPrivacyType type, bool isConfirmed);

private:
    LocationConfigManager();
    std::string GetLocationSwitchConfigPath();

private:
    std::atomic<int> mPrivacyTypeState[3];
    std::atomic<int> mLocationSwitchState;         /* location switch state */
    std::mutex mMutex;
};
}  // namespace Location
}  // namespace OHOS
#endif