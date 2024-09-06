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

#include <fstream>

#include "common_utils.h"
#include "constant_definition.h"
#include "location_log.h"

#include "parameter.h"
#include "location_data_rdb_manager.h"
#include "ipc_skeleton.h"
#include "ui_extension_ability_connection.h"

namespace OHOS {
namespace Location {
const int UNKNOW_ERROR = -1;
const int MAX_SIZE = 100;
const char* LOCATION_PRIVACY_MODE = "persist.location.privacy_mode";
LocationConfigManager* LocationConfigManager::GetInstance()
{
    static LocationConfigManager gLocationConfigManager;
    return &gLocationConfigManager;
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
    LBSLOGI(LOCATOR, "LocationConfigManager::Init");
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
    LBSLOGD(LOCATOR, "IsExistFile = %{public}d", bExist ? 1 : 0);
    return bExist;
}

bool LocationConfigManager::CreateFile(const std::string& filename, const std::string& filedata)
{
    LBSLOGD(LOCATOR, "CreateFile");
    std::ofstream outFile;
    outFile.open(filename.c_str());
    if (!outFile) {
        LBSLOGE(LOCATOR, "file open failed");
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
        LBSLOGE(LOCATOR, "%{public}s GetCurrentUserId failed", __func__);
    }
    std::string filePath = LOCATION_DIR + SWITCH_CONFIG_NAME + "_" + std::to_string(userId) + ".conf";
    return filePath;
}

std::string LocationConfigManager::GetPrivacyTypeConfigPath(const int type)
{
    int userId = 0;
    bool ret = CommonUtils::GetCurrentUserId(userId);
    if (!ret) {
        LBSLOGE(LOCATOR, "%{public}s GetCurrentUserId failed", __func__);
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
    return LOCATION_DIR + PRIVACY_CONFIG_NAME + "_" + filePath + std::to_string(userId) + ".conf";
}

int LocationConfigManager::GetLocationSwitchState()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!IsExistFile(GetLocationSwitchConfigPath())) {
        CreateFile(GetLocationSwitchConfigPath(), "0");
    }
    std::ifstream fs(GetLocationSwitchConfigPath());
    if (!fs.is_open()) {
        LBSLOGE(LOCATOR, "LocationConfigManager: fs.is_open false, return");
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

bool LocationConfigManager::GetStringParameter(const std::string& type, std::string& value)
{
    char result[MAX_BUFF_SIZE] = {0};
    auto res = GetParameter(type.c_str(), "", result, MAX_BUFF_SIZE);
    if (res <= 0) {
        LBSLOGE(LOCATOR, "%{public}s get para value failed, res: %{public}d",
            __func__, res);
        return false;
    }
    value = result;
    return true;
}

int LocationConfigManager::GetIntParameter(const std::string& type)
{
    char result[MAX_BUFF_SIZE] = {0};
    std::string value = "";
    auto res = GetParameter(type.c_str(), "", result, MAX_BUFF_SIZE);
    if (res < 0 || strlen(result) == 0) {
        LBSLOGE(LOCATOR, "%{public}s get para value failed, res: %{public}d",
            __func__, res);
        return UNKNOW_ERROR;
    }
    value = result;
    for (auto ch : value) {
        if (std::isdigit(ch) == 0) {
            LBSLOGE(LOCATOR, "wrong para");
            return UNKNOW_ERROR;
        }
    }
    if (value.size() == 0) {
        return UNKNOW_ERROR;
    }
    return std::stoi(value);
}

bool LocationConfigManager::GetSettingsBundleName(std::string& name)
{
    return GetStringParameter(SETTINGS_BUNDLE_NAME, name);
}

bool LocationConfigManager::GetNlpServiceName(std::string& name)
{
    return GetStringParameter(NLP_SERVICE_NAME, name);
}

bool LocationConfigManager::GetNlpAbilityName(std::string& name)
{
    return GetStringParameter(NLP_ABILITY_NAME, name);
}

bool LocationConfigManager::GetGeocodeServiceName(std::string& name)
{
    return GetStringParameter(GEOCODE_SERVICE_NAME, name);
}

bool LocationConfigManager::GetGeocodeAbilityName(std::string& name)
{
    return GetStringParameter(GEOCODE_ABILITY_NAME, name);
}

int LocationConfigManager::GetSuplMode()
{
    return GetIntParameter(SUPL_MODE_NAME);
}

bool LocationConfigManager::GetAgnssServerAddr(std::string& name)
{
    return GetStringParameter(AGNSS_SERVER_ADDR, name);
}

int LocationConfigManager::GetAgnssServerPort()
{
    return GetIntParameter(AGNSS_SERVER_PORT);
}

int LocationConfigManager::SetLocationSwitchState(int state)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!IsExistFile(GetLocationSwitchConfigPath())) {
        CreateFile(GetLocationSwitchConfigPath(), "0");
    }
    std::fstream fs(GetLocationSwitchConfigPath());
    if (state != STATE_CLOSE && state != STATE_OPEN) {
        LBSLOGE(LOCATOR, "LocationConfigManager:SetLocationSwitchState state = %{public}d, return", state);
        return -1;
    }
    if (!fs.is_open()) {
        LBSLOGE(LOCATOR, "LocationConfigManager: fs.is_open false, return");
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

LocationErrCode LocationConfigManager::GetPrivacyTypeState(const int type, bool& isConfirmed)
{
    int status = 0;
    int cacheState = GetCachePrivacyType();
    if (cacheState == DISABLED || cacheState == ENABLED) {
        isConfirmed = (status == 1);
        return ERRCODE_SUCCESS;
    }
    if (!LocationDataRdbManager::GetLocationEnhanceStatus(status)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    isConfirmed = (status == 1);
    return ERRCODE_SUCCESS;
}

int LocationConfigManager::GetCachePrivacyType()
{
    return GetIntParameter(LOCATION_PRIVACY_MODE);
}

bool LocationConfigManager::SetCachePrivacyType(int value)
{
    char valueArray[MAX_SIZE] = {0};
    (void)sprintf_s(valueArray, sizeof(valueArray), "%d", value);
    int res = SetParameter(LOCATION_PRIVACY_MODE, valueArray);
    if (res < 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s failed, res: %{public}d", __func__, res);
        return false;
    }
    return true;
}

LocationErrCode LocationConfigManager::SetPrivacyTypeState(const int type, bool isConfirmed)
{
    int status = isConfirmed ? 1 : 0;
    if (!LocationDataRdbManager::SetLocationEnhanceStatus(status)) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    SetCachePrivacyType(status);
    return ERRCODE_SUCCESS;
}

std::string LocationConfigManager::GenerateStartCommand()
{
    nlohmann::json param;
    std::string uiType = "sysDialog/common";
    param["ability.want.params.uiExtensionType"] = uiType;
    std::string cmdData = param.dump();
    LBSLOGD(GNSS, "cmdData is: %{public}s.", cmdData.c_str());
    return cmdData;
}

void LocationConfigManager::OpenPrivacyDialog()
{
    LBSLOGI(LOCATOR, "ConnectExtension");
    AAFwk::Want want;
    std::string bundleName = "com.ohos.sceneboard";
    std::string abilityName = "com.ohos.sceneboard.systemdialog";
    want.SetElementName(bundleName, abilityName);
    std::string connectStr = GenerateStartCommand();
    ConnectExtensionAbility(want, connectStr);
}

void LocationConfigManager::ConnectExtensionAbility(const AAFwk::Want &want, const std::string &commandStr)
{
    std::string bundleName = "com.ohos.locationdialog";
    std::string abilityName = "LocationPrivacyExtAbility";
    sptr<UIExtensionAbilityConnection> connection(
        new (std::nothrow) UIExtensionAbilityConnection(commandStr, bundleName, abilityName));
    if (connection == nullptr) {
        LBSLOGE(LOCATOR, "connect UIExtensionAbilityConnection fail");
        return;
    }

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto ret =
        AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(want, connection, nullptr, -1);
    LBSLOGI(LOCATOR, "connect service extension ability result = %{public}d", ret);
    IPCSkeleton::SetCallingIdentity(identity);
    return;
}
}  // namespace Location
}  // namespace OHOS
