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

#include <map>
#include <random>
#include <sys/time.h>
#include <sstream>
#include <chrono>
#include <cJSON.h>
#include <fstream>

#include "common_utils.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "constant_definition.h"
#include "parameter.h"
#include "location_sa_load_manager.h"
#include "hook_utils.h"
#include "accesstoken_kit.h"
#include "os_account_manager.h"
#include "os_account_info.h"
#include "permission_manager.h"
#include "file_ex.h"
#include <charconv>

namespace OHOS {
namespace Location {
static std::shared_ptr<std::map<int, sptr<IRemoteObject>>> g_proxyMap =
    std::make_shared<std::map<int, sptr<IRemoteObject>>>();
std::mutex g_proxyMutex;
static std::random_device g_randomDevice;
static std::mt19937 g_gen(g_randomDevice());
static std::uniform_int_distribution<> g_dis(0, 15);   // random between 0 and 15
static std::uniform_int_distribution<> g_dis2(8, 11);  // random between 8 and 11
const int32_t STOI_BYTE_LIMIT = 8;
const int32_t MAX_INT_LENGTH = 9;
const size_t MAX_ULL_SIZE = 19;
const int64_t SEC_TO_NANO = 1000 * 1000 * 1000;
const int DEFAULT_USERID = 100;

int CommonUtils::AbilityConvertToId(const std::string ability)
{
    if (GNSS_ABILITY.compare(ability) == 0) {
        return LOCATION_GNSS_SA_ID;
    }
    if (NETWORK_ABILITY.compare(ability) == 0) {
        return LOCATION_NETWORK_LOCATING_SA_ID;
    }
    if (PASSIVE_ABILITY.compare(ability) == 0) {
        return LOCATION_NOPOWER_LOCATING_SA_ID;
    }
    if (GEO_ABILITY.compare(ability) == 0) {
        return LOCATION_GEO_CONVERT_SA_ID;
    }
    return -1;
}

std::u16string CommonUtils::GetCapabilityToString(std::string ability, uint32_t capability)
{
    std::string value = "{\"Capabilities\":{\"" + ability + "\":" + std::to_string(capability) + "}}";
    return Str8ToStr16(value);
}

std::u16string CommonUtils::GetCapability(std::string ability)
{
    uint32_t capability = 0x102;
    return GetCapabilityToString(ability, capability);
}

OHOS::HiviewDFX::HiLogLabel CommonUtils::GetLabel(std::string name)
{
    if (GNSS_ABILITY.compare(name) == 0) {
        return GNSS;
    }
    if (NETWORK_ABILITY.compare(name) == 0) {
        return NETWORK;
    }
    if (PASSIVE_ABILITY.compare(name) == 0) {
        return PASSIVE;
    }
    if (GEO_ABILITY.compare(name) == 0) {
        return GEO_CONVERT;
    }
    OHOS::HiviewDFX::HiLogLabel label = { LOG_CORE, LOCATION_LOG_DOMAIN, "unknown" };
    return label;
}

sptr<IRemoteObject> CommonUtils::GetRemoteObject(int abilityId)
{
    return GetRemoteObject(abilityId, InitDeviceId());
}

sptr<IRemoteObject> CommonUtils::GetRemoteObject(int abilityId, std::string deviceId)
{
    std::unique_lock<std::mutex> lock(g_proxyMutex);
    auto objectGnss = g_proxyMap->find(abilityId);
    if (objectGnss == g_proxyMap->end()) {
        auto manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (manager == nullptr) {
            LBSLOGE(COMMON_UTILS, "GetSystemAbilityManager is null.");
            return nullptr;
        }
        sptr<IRemoteObject> object = manager->GetSystemAbility(abilityId, deviceId);
        if (object == nullptr) {
            LBSLOGE(COMMON_UTILS, "GetSystemAbility is null.");
            return nullptr;
        }
        g_proxyMap->insert(std::make_pair(abilityId, object));
        return object;
    } else {
        sptr<IRemoteObject> remoteObject = objectGnss->second;
        return remoteObject;
    }
}

std::string CommonUtils::InitDeviceId()
{
    std::string deviceId;
    return deviceId;
}

bool CommonUtils::GetActiveUserIds(std::vector<int>& activeIds)
{
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        activeIds.push_back(DEFAULT_USERID);
        LBSLOGI(COMMON_UTILS, "GetCurrentUserId failed ret:%{public}d", ret);
        return false;
    }
    if (activeIds.empty()) {
        activeIds.push_back(DEFAULT_USERID);
        LBSLOGE(COMMON_UTILS, "QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    userId = activeIds[0];
    return true;
}

int CommonUtils::GetUserIdByUid(int uid)
{
    int userId = 0;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (userId == 0) {
        GetCurrentUserId(userId);
        LBSLOGE(COMMON_UTILS, "GetUserIdByUid userId = %{public}d", userId);
    }
    return userId;
}

bool CommonUtils::GetCurrentUserId(int &userId)
{
    std::vector<int> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        userId = DEFAULT_USERID;
        LBSLOGI(COMMON_UTILS, "GetCurrentUserId failed ret:%{public}d", ret);
        return false;
    }
    if (activeIds.empty()) {
        userId = DEFAULT_USERID;
        LBSLOGE(COMMON_UTILS, "QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    userId = activeIds[0];
    return true;
}

bool CommonUtils::GetAllUserId(std::vector<int>& activeIds)
{
    std::vector<AccountSA::OsAccountInfo> accountInfos;
    int ret = AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(accountInfos);
    if (ret != 0) {
        LBSLOGE(COMMON_UTILS, "GetAllUserId failed ret:%{public}d", ret);
        return false;
    }
    for (auto &info : accountInfos) {
        activeIds.push_back(info.GetLocalId());
    }
    if (activeIds.empty()) {
        LBSLOGE(COMMON_UTILS, "QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    return true;
}

void CountDownLatch::Wait(int time)
{
    LBSLOGD(LOCATOR_STANDARD, "enter wait, time = %{public}d", time);
    std::unique_lock<std::mutex> lock(mutex_);
    if (count_ == 0) {
        LBSLOGE(LOCATOR_STANDARD, "count_ = 0");
        return;
    }
    condition_.wait_for(lock, std::chrono::seconds(time / MILLI_PER_SEC), [&]() {return count_ == 0;});
}

void CountDownLatch::CountDown()
{
    LBSLOGD(LOCATOR_STANDARD, "enter CountDown");
    std::unique_lock<std::mutex> lock(mutex_);
    int oldC = count_.load();
    while (oldC > 0) {
        if (count_.compare_exchange_strong(oldC, oldC - 1)) {
            if (oldC == 1) {
                LBSLOGD(LOCATOR_STANDARD, "notify_all");
                condition_.notify_all();
            }
            break;
        }
        oldC = count_.load();
    }
}

int CountDownLatch::GetCount()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return count_;
}

void CountDownLatch::SetCount(int count)
{
    std::unique_lock<std::mutex> lock(mutex_);
    count_ = count;
}

std::string CommonUtils::Str16ToStr8(std::u16string str)
{
    if (str == DEFAULT_USTRING) {
        return DEFAULT_STRING;
    }
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert(DEFAULT_STRING);
    std::string result = convert.to_bytes(str);
    return result == DEFAULT_STRING ? "" : result;
}

bool CommonUtils::DoubleEqual(double a, double b)
{
    if (fabs(a - b) < 1e-6) {
        return true;
    } else {
        return false;
    }
}

double CommonUtils::CalDistance(const double lat1, const double lon1, const double lat2, const double lon2)
{
    double radLat1 = lat1 * PI / DEGREE_PI;
    double radLat2 = lat2 * PI / DEGREE_PI;
    double radLon1 = lon1 * PI / DEGREE_PI;
    double radLon2 = lon2 * PI / DEGREE_PI;

    double latDiff = radLat1 - radLat2;
    double lonDiff = radLon1 - radLon2;
    double temp = sqrt(pow(sin(latDiff / DIS_FROMLL_PARAMETER), DIS_FROMLL_PARAMETER) +
        cos(radLat1) * cos(radLat2) * pow(sin(lonDiff / DIS_FROMLL_PARAMETER), DIS_FROMLL_PARAMETER));
    double disRad = asin(temp) * DIS_FROMLL_PARAMETER;
    double dis = disRad * EARTH_RADIUS;
    return dis;
}

double CommonUtils::DoubleRandom(double min, double max)
{
    double param = 0.0;
    std::random_device rd;
    static std::uniform_real_distribution<double> u(min, max);
    static std::default_random_engine e(rd());
    param = u(e);
    return param;
}

int CommonUtils::IntRandom(int min, int max)
{
    int param = 0;
    std::random_device rd;
    static std::uniform_int_distribution<int> u(min, max);
    static std::default_random_engine e(rd());
    param = u(e);
    return param;
}

bool CommonUtils::GetBundleNameByUid(int32_t uid, std::string& bundleName)
{
    AppExecFwk::BundleMgrClient bundleMgrClient;
    int32_t error = bundleMgrClient.GetNameForUid(uid, bundleName);
    if (error != ERR_OK) {
        return false;
    }
    return true;
}

/*
 * Check whether the application is installed by bundleName
 * @param bundleName
 * @return true if app is installed
 * @return false if app is not installed
 */
bool CommonUtils::CheckAppInstalled(const std::string& bundleName)
{
    int userId = 0;
    bool ret = GetCurrentUserId(userId);
    if (!ret) {
        LBSLOGE(COMMON_UTILS, "GetCurrentUserId failed");
        return false;
    }
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        LBSLOGE(COMMON_UTILS, "fail to get system ability manager!");
        return false;
    }
    auto bundleMgrSa = systemManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleMgrSa == nullptr) {
        LBSLOGE(COMMON_UTILS, "fail to get bundle manager system ability!");
        return false;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bundleMgrSa);
    if (bundleMgr == nullptr) {
        LBSLOGE(COMMON_UTILS, "Bundle mgr is nullptr.");
        return false;
    }
    AppExecFwk::ApplicationInfo info;
    bundleMgr->GetApplicationInfoV9(bundleName, 0, userId, info);
    if (info.name.empty() || info.bundleName.empty()) {
        return false;
    }
    return true;
}

int64_t CommonUtils::GetCurrentTime()
{
    struct timespec times = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &times);
    int64_t second = static_cast<int64_t>(times.tv_sec);
    return second;
}

int64_t CommonUtils::GetCurrentTimeMilSec()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return milliSeconds;
}

int64_t CommonUtils::GetCurrentTimeStamp()
{
    struct timeval currentTime;
    gettimeofday(&currentTime, nullptr);
    return static_cast<int64_t>(currentTime.tv_sec);
}

std::vector<std::string> CommonUtils::Split(std::string str, std::string pattern)
{
    std::vector<std::string> result;
    str += pattern;
    size_t size = str.size();
    size_t i = 0;
    while (i < size) {
        size_t pos = str.find(pattern, i);
        if (pos != std::string::npos && pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
        i++;
    }
    return result;
}

uint8_t CommonUtils::ConvertStringToDigit(std::string str)
{
    uint8_t res = 0;
    constexpr int bitWidth = 4;
    constexpr int numDiffForHexAlphabet = 10;
    for (auto ch : str) {
        res = res << bitWidth;
        if (ch >= '0' && ch <= '9') {
            res += (ch - '0');
        }
        if (ch >= 'A' && ch <= 'F') {
            res += (ch - 'A' + numDiffForHexAlphabet);
        }
        if (ch >= 'a' && ch <= 'f') {
            res += (ch - 'a' + numDiffForHexAlphabet);
        }
    }
    return res;
}

bool CommonUtils::isValidInteger(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    if (str.length() > MAX_INT_LENGTH) {
        return false;
    }
    size_t digitStartIndex = 0;
    if (str[0] == '-') {
        if (str.length() == 1) {
            return false;
        }
        digitStartIndex = 1;
    }

    for (size_t i = digitStartIndex; i < str.length(); i++) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

errno_t CommonUtils::GetMacArray(const std::string& strMac, uint8_t mac[MAC_LEN])
{
    std::vector<std::string> strVec = Split(strMac, ":");
    for (size_t i = 0; i < strVec.size() && i < MAC_LEN; i++) {
        mac[i] = ConvertStringToDigit(strVec[i]);
    }
    return EOK;
}

bool CommonUtils::GetStringParameter(const std::string& type, std::string& value)
{
    char result[MAX_BUFF_SIZE] = {0};
    auto res = GetParameter(type.c_str(), "", result, MAX_BUFF_SIZE);
    if (res <= 0) {
        LBSLOGE(COMMON_UTILS, "%{public}s get para value failed, res: %{public}d",
            __func__, res);
        return false;
    }
    value = result;
    return true;
}

bool CommonUtils::GetEdmPolicy(std::string& name)
{
    return GetStringParameter(EDM_POLICY_NAME, name);
}

std::string CommonUtils::GenerateUuid()
{
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {  // first group 8 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {  // second group 4 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {  // third group 3 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-";
    ss << g_dis2(g_gen);
    for (i = 0; i < 3; i++) {  // fourth group 3 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {  // fifth group 12 bit for UUID
        ss << g_dis(g_gen);
    };
    return ss.str();
}

bool CommonUtils::CheckAppForUser(int32_t uid, std::string& bundleName)
{
    std::vector<int> activeIds;
    if (!GetActiveUserIds(activeIds)) {
        currentUserId = DEFAULT_USERID;
    }
    return CommonUtils::CheckAppForUser(uid, currentUserId, bundleName);
}

bool CommonUtils::CheckAppForUsers(int32_t uid, std::vector<int> activeIds, std::string& bundleName)
{
    int userId = 0;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    bool containsActiveId = std::find(activeIds.begin(), activeIds.end(), userId) != activeIds.end();
    if (containsActiveId || userId == 0) {
        return true;
    }
    if (bundleName.length() == 0) {
        if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
            LBSLOGE(REPORT_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
        }
    }
    if (bundleName.length() > 0 && HookUtils::ExecuteHookWhenCheckAppForUser(bundleName)) {
        return true;
    }
    return false;
}

bool CommonUtils::CheckAppForUser(int32_t uid, std::string& bundleName)
{
    int currentUserId = 0;
    if (!GetCurrentUserId(currentUserId)) {
        currentUserId = DEFAULT_USERID;
    }
    return CommonUtils::CheckAppForUser(uid, currentUserId, bundleName);
}

bool CommonUtils::CheckAppForUser(int32_t uid, int32_t currentUserId, std::string& bundleName)
{
    int userId = 0;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    if (userId == currentUserId || userId == 0) {
        return true;
    }
    if (bundleName.length() == 0) {
        if (!CommonUtils::GetBundleNameByUid(uid, bundleName)) {
            LBSLOGE(REPORT_MANAGER, "Fail to Get bundle name: uid = %{public}d.", uid);
        }
    }
    if (bundleName.length() > 0 && HookUtils::ExecuteHookWhenCheckAppForUser(bundleName)) {
        return true;
    }
    return false;
}

int64_t CommonUtils::GetSinceBootTime()
{
    int result;
    struct timespec ts;
    result = clock_gettime(CLOCK_BOOTTIME, &ts);
    if (result == 0) {
        return ts.tv_sec * SEC_TO_NANO + ts.tv_nsec;
    } else {
        return 0;
    }
}

bool CommonUtils::GetConfigFromJson(const std::string &key, std::string &value)
{
    std::string content;
    if (!LoadStringFromFile(LOCATION_SERVICE_CONFIG_PATH, content) || content.empty()) {
        LBSLOGE(COMMON_UTILS, "Fail to Load json file.");
        return false;
    }
    cJSON *json = cJSON_Parse(content.c_str());
    if (json == nullptr) {
        LBSLOGE(COMMON_UTILS, "json is nullptr.");
        return false;
    }
    if (!cJSON_IsObject(json)) {
        LBSLOGE(COMMON_UTILS, "reader is not cJSON object.");
        cJSON_Delete(json);
        return false;
    }
    cJSON *cJsonObject = cJSON_GetObjectItem(json, key.c_str());
    if (cJsonObject == nullptr) {
        LBSLOGE(COMMON_UTILS, "cJsonObject is nullptr.");
        cJSON_Delete(json);
        return false;
    }
    value = cJSON_GetStringValue(cJsonObject);
    cJSON_Delete(json);
    return true;
}

bool CommonUtils::IsAppBelongCurrentAccounts(AppIdentity &identity, std::vector<int> activeIds)
{
    std::string bundleName = identity.GetBundleName();
    if (CommonUtils::CheckAppForUser(identity.GetUid(), activeIds, bundleName)) {
        return true;
    }
    if (PermissionManager::CheckIsSystemSa(identity.GetTokenId())) {
        return true;
    }
    return false;
}

bool CommonUtils::IsAppBelongCurrentAccount(AppIdentity &identity)
{
    std::vector<int> activeIds;
    if (!CommonUtils::GetActiveUserIds(activeIds)) {
        LBSLOGE(COMMON_UTILS, "Fail to GetCurrentUserId.");
    }
    return CommonUtils::IsAppBelongCurrentAccounts(identity, activeIds);
}

bool CommonUtils::IsValidForStoull(const std::string input, size_t size)
{
    if (size > MAX_ULL_SIZE) {
        return false; // 设定的最大位数大于uint64最大位数
    }
    if (input.size() > size) {
        return false; // 大于设定的最大位数
    }
    if (input.empty()) {
        return false; // 字符串为空
    }
    for (char c : input) {
        if (!std::isdigit(c)) {
            return false; // 检查是否全为数字
        }
    }
    return true;
}

LocationErrCode CommonUtils::ErrCodeToLocationErrCode(ErrCode errorCode)
{
    LocationErrCode locationErrCode = ERRCODE_SERVICE_UNAVAILABLE;
    if (errorCode == ERRCODE_SUCCESS || errorCode >= ERRCODE_SERVICE_UNAVAILABLE) {
        locationErrCode = static_cast<LocationErrCode>(errorCode);
    } else if (errorCode > LOCATION_ERRCODE_MIN) {
        locationErrCode = static_cast<LocationErrCode>(errorCode - LOCATION_ERRCODE_MIN);
    } else {
        locationErrCode = ERRCODE_SERVICE_UNAVAILABLE;
    }
    return locationErrCode;
}

bool CommonUtils::IsStrValidForStoi(const std::string &str)
{
    if (str.length() > STOI_BYTE_LIMIT) {
        return false;
    }
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

bool CommonUtils::IsExistFile(const std::string& filename)
{
    bool bExist = false;
    std::fstream ioFile;
    char path[PATH_MAX + 1] = {0x00};
    if (strlen(filename.c_str()) > PATH_MAX || realpath(filename.c_str(), path) == NULL) {
        LBSLOGI(GNSS, "IsExistFile false,realpath fail");
        return false;
    }
    ioFile.open(path, std::ios::in);
    if (ioFile) {
        bExist = true;
    } else {
        LBSLOGI(GNSS, "IsExistFile false,open fail");
        return false;
    }
    ioFile.clear();
    ioFile.close();
    LBSLOGI(GNSS, "IsExistFile = %{public}d", bExist ? 1 : 0);
    return bExist;
}

bool CommonUtils::CreateFile(const std::string& filename, const std::string& filedata)
{
    std::ofstream outFile;
    outFile.open(filename.c_str());
    if (!outFile) {
        LBSLOGE(GNSS, "file open failed");
        return false;
    }
    outFile.flush();
    outFile << filedata << std::endl;
    outFile.clear();
    outFile.close();
    LBSLOGI(GNSS, "CreateFile success");
    return true;
}

bool common_utils::ConvertStringToDigit(const std::string& str, int32_t &ret)
{
    std::from_chars_result res =
       std::from_chars(str.data(), str.data() + str.size(), ret);
    if (res.ec != std::errc{} || res.ptr != str.data() + str.size ()) {
        LBSLOGI(GNSS, "FromString failed, error string is %{public}s", str.c_str());
        return false;
    }
    return true;
}


} // namespace Location
} // namespace OHOS
