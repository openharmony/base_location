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

#include "iremote_object.h"
#include "string_ex.h"
#include "constant_definition.h"
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
const std::string ACCESS_BACKGROUND_LOCATION = "ohos.permission.LOCATION_IN_BACKGROUND";
const std::string MANAGE_SECURE_SETTINGS = "ohos.permission.MANAGE_SECURE_SETTINGS";

const std::string BUILD_INFO = "ro.build.characteristics";
const int SA_NUM = 3;
const int DEFAULT_UID = 10001;
const int SYSTEM_UID = 1000;

const int EX_HAS_REPLY_HEADER = -128;
const int REPLY_NO_EXCEPTION = 0;
const int EXCEPTION = -1;
const int MSG_UNPROCESSED = -2;
const int SWITCH_OFF_EXCEPTION = -3;
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
    NO_DATA_TO_JS = 1,
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
    static void WriteInterfaceToken(const std::u16string &descriptor, MessageParcel &data);
    static int AbilityConvertToId(const std::string ability);
    static std::u16string GetCapabilityToString(std::string ability, uint32_t capability);
    static std::u16string GetCapability(std::string ability);
    static OHOS::HiviewDFX::HiLogLabel GetLabel(std::string name);
    static sptr<IRemoteObject> GetRemoteObject(int abilityId);
    static sptr<IRemoteObject> GetRemoteObject(int abilityId, std::string deviceId);
    static std::string InitDeviceId();
    static bool CheckLocationPermission();
    static bool CheckBackgroundPermission();
    static bool CheckPermission(const std::string &permission);
    static bool CheckSecureSettings();
    static bool CheckSystemCalling(pid_t uid);
    static bool GetCurrentUserId(int &userId);
};

class CountDownLatch {
public:
    CountDownLatch()
    {
        count_ = 0;
    }
    void Wait(int time);
    void CountDown();
    int GetCount() const;
    void SetCount(int count);
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<int> count_;
};
} // namespace Location
} // namespace OHOS
#endif // COMMON_UTILS_H
