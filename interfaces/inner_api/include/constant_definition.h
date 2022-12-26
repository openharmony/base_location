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

#ifndef CONSTANT_DEFINITION_H
#define CONSTANT_DEFINITION_H

#include <string>

namespace OHOS {
namespace Location {
// error type
const int ERROR_PERMISSION_NOT_GRANTED = 0x0100;
const int ERROR_SWITCH_UNOPEN = 0x0101;
const int SESSION_START = 0x0002;
const int SESSION_STOP = 0x0003;
const int SUCCESS_CALLBACK = 0;
const int FAIL_CALLBACK = 1;
const int COMPLETE_CALLBACK = 2;
const int32_t PARAM0 = 0;
const int32_t PARAM1 = 1;
const int32_t PARAM2 = 2;
const int32_t PARAM3 = 3;
const size_t RESULT_SIZE = 2;
const int DEFAULT_TIMEOUT_30S = 30000;
const int DEFAULT_APPROXIMATELY_ACCURACY = 5000;
const int STATE_OPEN = 1;
const int STATE_CLOSE = 0;
const int PRIVACY_TYPE_OTHERS = 0;
const int PRIVACY_TYPE_STARTUP = 1;
const int PRIVACY_TYPE_CORE_LOCATION = 2;
const int32_t UNKNOW_USER_ID = -1;
const int32_t SUBSCRIBE_TIME = 5;
const int32_t DEFAULT_TIME_INTERVAL = 30 * 60; // app receive location every 30 minutes in frozen state
const int32_t REQUESTS_NUM_MAX = 1;
const std::string FEATURE_SWITCH_PROP = "ro.config.locator_background";
const std::string TIME_INTERVAL_PROP = "ro.config.locator_background.timeInterval";
const std::string PROC_NAME = "system";
const std::string SEARCH_NET_WORK_STATE_CHANGE_ACTION = "com.hos.action.SEARCH_NET_WORK_STATE_CHANGE";
const std::string SIM_STATE_CHANGE_ACTION = "com.hos.action.SIM_STATE_CHANGE";
const std::string LOCALE_KEY = "persist.global.locale";

enum {
    SCENE_UNSET = 0x0300,
    SCENE_NAVIGATION = 0x0301,
    SCENE_TRAJECTORY_TRACKING = 0x0302,
    SCENE_CAR_HAILING = 0x0303,
    SCENE_DAILY_LIFE_SERVICE = 0x0304,
    SCENE_NO_POWER = 0x0305
};

enum {
    PRIORITY_UNSET = 0x0200,
    PRIORITY_ACCURACY = 0x0201,
    PRIORITY_LOW_POWER = 0x0202,
    PRIORITY_FAST_FIRST_FIX = 0x0203
};

enum {
    COUNTRY_CODE_FROM_LOCALE = 1,
    COUNTRY_CODE_FROM_SIM,
    COUNTRY_CODE_FROM_LOCATION,
    COUNTRY_CODE_FROM_NETWORK,
};

enum LocationRequestType {
    PRIORITY_TYPE_HIGH_ACCURACY = 100,            // GNSS + NLP
    PRIORITY_TYPE_BALANCED_POWER_ACCURACY = 102,  // NLP
    PRIORITY_TYPE_LOW_POWER = 104,                // NLP
    PRIORITY_TYPE_NO_POWER = 105,                 // passive
    PRIORITY_TYPE_HD_ACCURACY = 200,              // HD
    PRIORITY_TYPE_INDOOR = 300,                   // indoor
    PRIORITY_TYPE_HIGH_ACCURACY_AND_INDOOR = 400  // indoor + GNSS
};

enum LocationErrCode {
    ERRCODE_SUCCESS = 0,
    ERRCODE_PERMISSION_DENIED = 201,          /* permission denied */
    ERRCODE_INVALID_PARAM = 401,              /* invalid params */
    ERRCODE_SYSTEM_PERMISSION_DENIED,
    ERRCODE_INVALID_TOKEN,
    ERRCODE_NOT_SUPPORTED = 801,              /* capability not supported */
    ERRCODE_SERVICE_UNAVAILABLE = 3301000,    /* location service is unavailable */
    ERRCODE_SWITCH_OFF = 3301100,             /* location switch is off */
    ERRCODE_LOCATING_FAIL = 3301200,          /* Failed to obtain the geographical location. */
    ERRCODE_REVERSE_GEOCODING_FAIL = 3301300, /* Reverse geocoding query failed */
    ERRCODE_GEOCODING_FAIL = 3301400,         /* Geocoding query failed */
    ERRCODE_COUNTRYCODE_FAIL  = 3301500,      /* Failed to query the area information */
    ERRCODE_GEOFENCE_FAIL = 3301600,          /* Failed to operate the geofence */
    ERRCODE_NO_RESPONSE = 3301700,            /* No response to the request */
    ERRCODE_GNSS_FAIL,
};

enum GnssErrCode {
    GNSS_ERRCODE_SUCCESS = 0,
};

enum GeoCodeErrCode {
    GEOCODE_ERRCODE_SUCCESS = 0,
};

typedef struct {
    int reportingPeriodSec;
    bool wakeUpCacheQueueFull;
} CachedGnssLocationsRequest;

typedef struct {
    int scenario;
    std::string command;
} LocationCommand;

typedef struct {
    double latitude;
    double longitude;
    double radius;
    double expiration;
} GeoFence;

typedef struct {
    int scenario;
    GeoFence geofence;
} GeofenceRequest;

typedef struct {
    std::string locale;
    double latitude;
    double longitude;
    int maxItems;
} ReverseGeocodeRequest;
} // namespace Location
} // namespace OHOS
#endif // CONSTANT_DEFINITION_H
