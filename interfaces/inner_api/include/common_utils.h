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
#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unistd.h>
#include <string>

#include "constant_definition.h"
#include "iremote_object.h"
#include "string_ex.h"

#include "location_log.h"

namespace OHOS {
namespace Location {
const std::string GNSS_ABILITY = "gps";
const std::string NETWORK_ABILITY = "network";
const std::string PASSIVE_ABILITY = "passive";
const std::string FUSED_ABILITY = "fused";
const std::string GEO_ABILITY = "geo";
const std::string DEFAULT_ABILITY = "default";

const std::string ACCESS_LOCATION = "ohos.permission.LOCATION";
const std::string ACCESS_APPROXIMATELY_LOCATION = "ohos.permission.APPROXIMATELY_LOCATION";
const std::string ACCESS_BACKGROUND_LOCATION = "ohos.permission.LOCATION_IN_BACKGROUND";
const std::string MANAGE_SECURE_SETTINGS = "ohos.permission.MANAGE_SECURE_SETTINGS";
const std::string LOCATION_DIR = "/data/service/el1/public/location/";
const std::string SWITCH_CONFIG_NAME = "location_switch";
const std::string PRIVACY_CONFIG_NAME = "location_privacy";
const std::string LOCATION_DATA_ABILITY_PREFIX = "datashare://";
const std::string LOCATION_DATA_URI_ID =
    "/com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=location_enable";
const std::string LOCATION_DATA_URI = LOCATION_DATA_ABILITY_PREFIX + LOCATION_DATA_URI_ID;
const std::string LOCATION_DATA_COLUMN_KEYWORD = "KEYWORD";
const std::string LOCATION_DATA_COLUMN_VALUE = "VALUE";
const std::string LOCATION_DATA_COLUMN_ENABLE = "location_switch_enable";

const std::string BUILD_INFO = "ro.build.characteristics";
const int SA_NUM = 3;
const int DEFAULT_UID = 10001;
const int SYSTEM_UID = 1000;
const int ROOT_UID = 0;
static constexpr int MIN_INT_RANDOM = 10000;
static constexpr int MAX_INT_RANDOM = 99999;

const int EX_HAS_REPLY_HEADER = -128;
const int REPLY_CODE_NO_EXCEPTION = 0;
const int REPLY_CODE_EXCEPTION = -1;
const int REPLY_CODE_MSG_UNPROCESSED = -2;
const int REPLY_CODE_SWITCH_OFF_EXCEPTION = -3;
const int REPLY_CODE_SECURITY_EXCEPTION = -4;
const int REPLY_CODE_UNSUPPORT = -5;

const int INPUT_ARRAY_LEN_MAX = 100;

const int EVENT_REGITERED_MAX_TRY_TIME = 30;
const int EVENT_REGITERED_MAX_TRY_INTERVAL = 5000;

const std::string DFT_EXCEPTION_EVENT = "GnssException";
const int DFT_IPC_CALLING_ERROR = 201;
const int DFT_DAILY_LOCATION_REQUEST_COUNT = 220;
const int DFT_DAILY_DISTRIBUTE_SESSION_COUNT = 221;
const int SEC_TO_MILLI_SEC = 1000;
const int CONNECT_TIME_OUT = 10;

const char DEFAULT_STRING[] = "error";
const std::wstring DEFAULT_WSTRING = L"error";
const std::u16string DEFAULT_USTRING = u"error";

const double PI = 3.1415926;
const double DEGREE_PI = 180.0;
const double DIS_FROMLL_PARAMETER = 2;
const double EARTH_RADIUS = 6378137.0; // earth semimajor axis (WGS84) (m)

static constexpr double MIN_LATITUDE = -90.0;
static constexpr double MAX_LATITUDE = 90.0;
static constexpr double MIN_LONGITUDE = -180.0;
static constexpr double MAX_LONGITUDE = 180.0;
static constexpr double DEGREE_DOUBLE_PI = 360.0;
static constexpr long LONG_TIME_INTERVAL = 24 * 60 * 60;
static constexpr int32_t LOCATION_LOADSA_TIMEOUT_MS = 5000;

static constexpr int PERMISSION_ACCURATE = 2;
static constexpr int PERMISSION_APPROXIMATELY = 1;
static constexpr int PERMISSION_INVALID = 0;
static constexpr int LOCATOR_UID = 1021;
static constexpr int MAXIMUM_INTERATION = 100;
static constexpr int MAXIMUM_CACHE_LOCATIONS = 1000;

#define CHK_PARCEL_RETURN_VALUE(ret) \
{ \
    if ((ret) != true) { \
        return false; \
    } \
}

#define CHK_ERRORCODE_RETURN_VALUE(ret) \
{                                       \
    if ((ret) != ERRCODE_SUCCESS) {     \
        return (ret);                   \
    }                                   \
}

enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

enum {
    ENABLED = 1,
    DISABLED = 0
};

enum {
    SUCCESS = 0,
    COMMON_ERROR = 98,
    PARAM_IS_EMPTY = 99,
    NOT_SUPPORTED = 100,
    INPUT_PARAMS_ERROR = 101,
    REVERSE_GEOCODE_ERROR,
    GEOCODE_ERROR,
    LOCATOR_ERROR,
    LOCATION_SWITCH_ERROR,
    LAST_KNOWN_LOCATION_ERROR,
    LOCATION_REQUEST_TIMEOUT_ERROR,
    QUERY_COUNTRY_CODE_ERROR,
};

enum {
    PERMISSION_REVOKED_OPER = 0,
    PERMISSION_GRANTED_OPER = 1
};

class CommonUtils {
public:
    static void WriteInterfaceToken(const std::u16string &descriptor, MessageParcel &data);
    static int AbilityConvertToId(const std::string ability);
    static std::u16string GetCapabilityToString(std::string ability, uint32_t capability);
    static std::u16string GetCapability(std::string ability);
    static OHOS::HiviewDFX::HiLogLabel GetLabel(std::string name);
    static sptr<IRemoteObject> GetRemoteObject(int abilityId);
    static sptr<IRemoteObject> GetRemoteObject(int abilityId, std::string deviceId);
    static std::string InitDeviceId();
    static bool CheckLocationPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckApproximatelyPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckBackgroundPermission(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckPermission(const std::string &permission, uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckSecureSettings(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckCallingPermission(pid_t callingUid, pid_t callingPid, MessageParcel &reply);
    static bool GetCurrentUserId(int &userId);
    static std::string Str16ToStr8(std::u16string str);
    static bool DoubleEqual(double a, double b);
    static double CalDistance(const double lat1, const double lon1, const double lat2, const double lon2);
    static double DoubleRandom(double min, double max);
    static int IntRandom(int min, int max);
    static int GetPermissionLevel(uint32_t tokenId, uint32_t firstTokenId);
    static bool CheckSystemPermission(uint32_t callerTokenId, uint64_t callerTokenIdEx);
    static bool GetBundleNameByUid(int32_t uid, std::string& bundleName);
    static bool CheckAppInstalled(const std::string& bundleName);
    static bool CheckIfSystemAbilityAvailable(int32_t systemAbilityId);
};

class CountDownLatch {
public:
    CountDownLatch()
    {
        count_ = 0;
    }
    ~CountDownLatch() = default;
    void Wait(int time);
    void CountDown();
    int GetCount();
    void SetCount(int count);
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<int> count_;
};
} // namespace Location
} // namespace OHOS
#endif // COMMON_UTILS_H
