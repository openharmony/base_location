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
#include "location_config_manager.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "location_log.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
LocationConfigManager &LocationConfigManager::GetInstance()
{
    static LocationConfigManager gLocationConfigManager;
    return gLocationConfigManager;
}

LocationConfigManager::LocationConfigManager()
{
    mLocationSwitchState = STATE_CLOSE;
    mPrivacyTypeState[PRIVACY_TYPE_OTHERS] = STATE_CLOSE; // for OTHERS
    mPrivacyTypeState[PRIVACY_TYPE_STARTUP] = STATE_CLOSE; // for STARTUP
    mPrivacyTypeState[PRIVACY_TYPE_CORE_LOCATION] = STATE_CLOSE; // for CORE_LOCATION
}

LocationConfigManager::~LocationConfigManager()
{
}

int LocationConfigManager::Init()
{
    LBSLOGI(LOCATION_NAPI, "LocationConfigManager::Init");
    if (!IsExistFile(GetLocationSwitchConfigPath())) {
        CreateFile(GetLocationSwitchConfigPath(), "0");
    }
    return 0;
}

bool LocationConfigManager::IsExistFile(const std::string& filename)
{
    bool bExist = false;
    std::fstream ioFile;
    char path[PATH_MAX + 1] = {0x00};
    if (strlen(filename.c_str()) > PATH_MAX || realpath(filename.c_str(), path) == NULL) {
        return false;
    }
    ioFile.open(path, std::ios::in);
    if (ioFile) {
        bExist = true;
    } else {
        return false;
    }
    ioFile.clear();
    ioFile.close();
    LBSLOGD(LOCATION_NAPI, "IsExistFile = %{public}d", bExist ? 1 : 0);
    return bExist;
}

bool LocationConfigManager::CreateFile(const std::string& filename, const std::string& filedata)
{
    LBSLOGD(LOCATION_NAPI, "CreateFile");
    std::ofstream outFile;
    outFile.open(filename.c_str());
    if (!outFile) {
        LBSLOGE(LOCATION_NAPI, "file open failed");
        return false;
    }
    outFile.flush();
    outFile << filedata << std::endl;
    outFile.clear();
    outFile.close();
    return true;
}

std::string LocationConfigManager::GetLocationSwitchConfigPath()
{
    int userId = 0;
    bool ret = CommonUtils::GetCurrentUserId(userId);
    if (!ret) {
        LBSLOGE(LOCATION_NAPI, "GetCurrentUserId failed");
    }
    std::string filePath = "/data/location/location_switch_" + std::to_string(userId) + ".conf";
    return filePath;
}

std::string LocationConfigManager::GetPrivacyTypeConfigPath(const int type)
{
    int userId = 0;
    bool ret = CommonUtils::GetCurrentUserId(userId);
    if (!ret) {
        LBSLOGE(LOCATION_NAPI, "GetCurrentUserId failed");
    }
    std::string filePath;
    switch (type) {
        case PRIVACY_TYPE_OTHERS: {
            filePath = "others_";
            break;
        }
        case PRIVACY_TYPE_STARTUP: {
            filePath = "startup_";
            break;
        }
        case PRIVACY_TYPE_CORE_LOCATION: {
            filePath = "core_location_";
            break;
        }
        default: {
            filePath = "";
            break;
        }
    }
    return "/data/location/location_pricacy_" + filePath + std::to_string(userId) + ".conf";
}

int LocationConfigManager::GetLocationSwitchState()
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (!IsExistFile(GetLocationSwitchConfigPath())) {
        CreateFile(GetLocationSwitchConfigPath(), "0");
    }
    std::ifstream fs(GetLocationSwitchConfigPath());
    if (!fs.is_open()) {
        LBSLOGE(LOCATION_NAPI, "LocationConfigManager: fs.is_open false, return");
        return -1;
    }
    std::string line;
    while (std::getline(fs, line)) {
        if (line.empty()) {
            break;
        }
        if (line[0] == '0') {
            mLocationSwitchState = STATE_CLOSE;
        } else if (line[0] == '1') {
            mLocationSwitchState = STATE_OPEN;
        }
        break;
    }
    fs.clear();
    fs.close();
    return mLocationSwitchState;
}

int LocationConfigManager::SetLocationSwitchState(int state)
{
    std::unique_lock<std::mutex> lock(mMutex);
    if (!IsExistFile(GetLocationSwitchConfigPath())) {
        CreateFile(GetLocationSwitchConfigPath(), "0");
    }
    std::fstream fs(GetLocationSwitchConfigPath());
    if (state != STATE_CLOSE && state != STATE_OPEN) {
        LBSLOGE(LOCATION_NAPI, "LocationConfigManager:SetLocationSwitchState state = %{public}d, return", state);
        return -1;
    }
    if (!fs.is_open()) {
        LBSLOGE(LOCATION_NAPI, "LocationConfigManager: fs.is_open false, return");
        return -1;
    }
    std::string content = "1";
    if (state == STATE_CLOSE) {
        content = "0";
    }
    fs.write(content.c_str(), content.length());
    fs.clear();
    fs.close();
    mLocationSwitchState = state;
    return 0;
}

bool LocationConfigManager::GetPrivacyTypeState(const int type)
{
    if (type < PRIVACY_TYPE_OTHERS || type > PRIVACY_TYPE_CORE_LOCATION) {
        LBSLOGI(LOCATION_NAPI, "GetPrivacyTypeState,invalid types");
        return false;
    }
    std::unique_lock<std::mutex> lock(mMutex);
    if (!IsExistFile(GetPrivacyTypeConfigPath(type))) {
        CreateFile(GetPrivacyTypeConfigPath(type), "0");
    }
    std::ifstream fs(GetPrivacyTypeConfigPath(type));
    if (!fs.is_open()) {
        LBSLOGE(LOCATION_NAPI, "LocationConfigManager: fs.is_open false, return");
        return -1;
    }
    std::string line;
    while (std::getline(fs, line)) {
        if (line.empty()) {
            break;
        }
        if (line[0] == '0') {
            mPrivacyTypeState[type] = STATE_CLOSE;
        } else if (line[0] == '1') {
            mPrivacyTypeState[type] = STATE_OPEN;
        }
        break;
    }
    fs.clear();
    fs.close();
    return (mPrivacyTypeState[type] == STATE_OPEN) ? true : false;
}

void LocationConfigManager::SetPrivacyTypeState(const int type, bool isConfirmed)
{
    if (type < PRIVACY_TYPE_OTHERS || type > PRIVACY_TYPE_CORE_LOCATION) {
        LBSLOGI(LOCATION_NAPI, "SetPrivacyTypeState,invalid types");
        return;
    }
    std::unique_lock<std::mutex> lock(mMutex);
    if (!IsExistFile(GetPrivacyTypeConfigPath(type))) {
        CreateFile(GetPrivacyTypeConfigPath(type), "0");
    }
    std::fstream fs(GetPrivacyTypeConfigPath(type));
    if (!fs.is_open()) {
        LBSLOGE(LOCATION_NAPI, "LocationConfigManager: fs.is_open false, return");
        return;
    }
    std::string content = "0";
    if (isConfirmed) {
        content = "1";
    }
    fs.write(content.c_str(), content.length());
    fs.clear();
    fs.close();
    mPrivacyTypeState[type] = isConfirmed ? 1 : 0;
}
}  // namespace Location
}  // namespace OHOS
