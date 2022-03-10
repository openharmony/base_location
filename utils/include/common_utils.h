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

#ifndef OHOS_LOCATION_UTILS_H
#define OHOS_LOCATION_UTILS_H

#include "iremote_object.h"
#include "string_ex.h"
#include "constant_definition.h"
#include "lbs_log.h"

namespace OHOS {
namespace Location {
const std::string GNSS_ABILITY = "gps";
const std::string NETWORK_ABILITY = "network";
const std::string PASSIVE_ABILITY = "passive";
const std::string FUSED_ABILITY = "fused";
const std::string GEO_ABILITY = "geo";
const std::string DEFAULT_ABILITY = "default";

const std::string ACCESS_LOCATION = "ohos.permission.LOCATION";
const std::string ACCESS_BACKGROUND_LOCATION = "ohos.permission.LOCATION_IN_BACKGROUND";
const std::string MANAGE_SECURE_SETTINGS = "ohos.permission.MANAGE_SECURE_SETTINGS";

const std::string BUILD_INFO = "ro.build.characteristics";
const std::string TEST_SIMULATE_DEVICE = "persist.sys.location.simulate.device"; // value: watch
const std::string DEVICE_PHONE = "default";
const std::string DEVICE_WEAR = "watch";
const std::string DEVICE_PLATE = "tv";
const std::string DEVICE_UNKOWN = "unknown";

const uint32_t CAP_DEFAULT = 0;
const uint32_t CAP_GNSS_MIN = 0x0100;
const uint32_t CAP_GNSS_IVI = CAP_GNSS_MIN + 1;
const uint32_t CAP_GNSS_PHONE = CAP_GNSS_MIN + 2;
const uint32_t CAP_GNSS_WEAR = CAP_GNSS_MIN + 3;
const uint32_t CAP_GNSS_MAX = CAP_GNSS_MIN + 4;

const int SA_NUM = 3;
const int DEFAULT_UID = 1000;
const int DEFAULT_PID = 1000;
const int SYSTEM_UID = 1000;

const int EX_HAS_REPLY_HEADER = -128;
const int REPLY_NO_EXCEPTION = 0;
const int EXCEPTION = -1;
const int SECURITY_EXCEPTION = 1000;

const int EVENT_REGITERED_MAX_TRY_TIME = 30;
const int EVENT_REGITERED_MAX_TRY_INTERVAL = 5000;

const std::string DFT_EXCEPTION_EVENT = "GnssException";
const int DFT_IPC_CALLING_ERROR = 201;
const int DFT_DAILY_LOCATION_REQUEST_COUNT = 220;
const int DFT_DAILY_DISTRIBUTE_SESSION_COUNT = 221;

enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

enum {
    ENABLED = 1,
    DISABLED = 0
};

enum {
    ACTIVE = 1,
    SUSPEND = 0
};

enum {
    SUCCESS = 0,
    INPUT_PARAMS_ERROR = 101,
    REVERSE_GEOCODE_ERROR,
    GEOCODE_ERROR,
    LOCATOR_ERROR,
    LOCATION_SWITCH_ERROR,
    LAST_KNOWN_LOCATION_ERROR,
    LOCATION_REQUEST_TIMEOUT_ERROR,
};

class CommonUtils {
public:
    static sptr<IRemoteObject> GetLocationService();
    static bool RemoteToLocationService(uint32_t code, MessageParcel &data, MessageParcel &reply);
    static bool RemoteToLocationService(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    static void WriteInterfaceToken(const std::u16string &descriptor, MessageParcel &data);
    static bool EnforceInterface(const std::u16string &descriptor, MessageParcel &data);
    static int AbilityConvertToId(const std::string ability);
    static uint32_t GetCapabilityValue(std::string ability);
    static std::u16string GetCapabilityToString(std::string ability, uint32_t capability);
    static std::u16string GetCapability(std::string ability);
    static OHOS::HiviewDFX::HiLogLabel GetLabel(std::string name);
    static sptr<IRemoteObject> GetRemoteObject(int abilityId);
    static sptr<IRemoteObject> GetRemoteObject(int abilityId, std::string deviceId);
    static std::string InitDeviceId();
    static bool CheckLocationPermission(pid_t pid, pid_t uid);
    static bool CheckBackgroundPermission(pid_t pid, pid_t uid);
    static bool CheckSecureSettings(pid_t pid, pid_t uid);
    static bool CheckSystemCalling(pid_t uid);
    static bool CheckLocatorInterfaceToken(std::u16string descripter, MessageParcel &data);
    static int GetPrivacyType(LocationPrivacyType type);
    static LocationPrivacyType GetPrivacyTypeByInt(int type);
private:
    static bool CheckLocationPermission(const std::string& permissionName, const std::string& appIdInfo);
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_UTILS_H
