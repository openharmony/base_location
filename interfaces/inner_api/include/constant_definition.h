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
const int32_t MAXIMUM_JS_PARAMS = 10;
const int32_t MAX_CALLBACK_NUM = 3;
const size_t RESULT_SIZE = 2;
const int DEFAULT_TIMEOUT_30S = 30000;
const double DEFAULT_APPROXIMATELY_ACCURACY = 5000.0;
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
const int MODE_STANDALONE = 1;
const int MODE_MS_BASED = 2;
const int MODE_MS_ASSISTED = 3;
const int DEFAULT_CALLBACK_WAIT_TIME = 10000;

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
    LOCATION_SCENE_NAVIGATION = 0x0401,
    LOCATION_SCENE_SPORT = 0x0402,
    LOCATION_SCENE_TRANSPORT = 0x0403,
    LOCATION_SCENE_DAILY_LIFE_SERVICE = 0x0404,
};

enum {
    LOCATION_PRIORITY_ACCURACY = 0x0501,
    LOCATION_PRIORITY_LOCATING_SPEED = 0x0502,
};

enum {
    LOCATION_SCENE_HIGH_POWER_CONSUMPTION = 0x0601,
    LOCATION_SCENE_LOW_POWER_CONSUMPTION = 0x0602,
    LOCATION_SCENE_NO_POWER_CONSUMPTION = 0x0603,
};


enum {
    COUNTRY_CODE_FROM_LOCALE = 1,
    COUNTRY_CODE_FROM_SIM,
    COUNTRY_CODE_FROM_LOCATION,
    COUNTRY_CODE_FROM_NETWORK,
};

enum NlpRequestType {
    PRIORITY_TYPE_BALANCED_POWER_ACCURACY = 102,  // NLP
    PRIORITY_TYPE_INDOOR = 300,                   // indoor
};

enum LocationErrCode {
    ERRCODE_SUCCESS = 0,                      /* SUCCESS. */
    ERRCODE_PERMISSION_DENIED = 201,          /* Permission denied. */
    ERRCODE_SYSTEM_PERMISSION_DENIED = 202,   /* System API is not allowed called by third HAP. */
    ERRCODE_EDM_POLICY_ABANDON = 203,         /* This feature is prohibited by enterprise management policies. */
    ERRCODE_INVALID_PARAM = 401,              /* Parameter error. */
    ERRCODE_NOT_SUPPORTED = 801,              /* Capability not supported. */
    ERRCODE_SERVICE_UNAVAILABLE = 3301000,    /* Location service is unavailable. */
    ERRCODE_SWITCH_OFF = 3301100,             /* The location switch is off. */
    ERRCODE_LOCATING_FAIL = 3301200,          /* Failed to obtain the geographical location. */
    ERRCODE_REVERSE_GEOCODING_FAIL = 3301300, /* Reverse geocoding query failed */
    ERRCODE_GEOCODING_FAIL = 3301400,         /* Geocoding query failed */
    ERRCODE_COUNTRYCODE_FAIL  = 3301500,      /* Failed to query the area information */
    ERRCODE_GEOFENCE_FAIL = 3301600,          /* Failed to operate the geofence */
    ERRCODE_NO_RESPONSE = 3301700,            /* No response to the request */
    ERRCODE_SCAN_FAIL = 3301800,              /* Failed to start WiFi or Bluetooth scanning. */
    ERRCODE_GEOFENCE_EXCEED_MAXIMUM = 3301601 /* The number of geofences exceeds the maximum. */
};

enum SatelliteConstellation {
    SV_CONSTELLATION_CATEGORY_UNKNOWN = 0,
    SV_CONSTELLATION_CATEGORY_GPS,
    SV_CONSTELLATION_CATEGORY_SBAS,
    SV_CONSTELLATION_CATEGORY_GLONASS,
    SV_CONSTELLATION_CATEGORY_QZSS,
    SV_CONSTELLATION_CATEGORY_BEIDOU,
    SV_CONSTELLATION_CATEGORY_GALILEO,
    SV_CONSTELLATION_CATEGORY_IRNSS,
};

enum SatelliteAdditionalInfo {
    SV_ADDITIONAL_INFO_NULL = 0,
    SV_ADDITIONAL_INFO_EPHEMERIS_DATA_EXIST = 1,
    SV_ADDITIONAL_INFO_ALMANAC_DATA_EXIST = 2,
    SV_ADDITIONAL_INFO_USED_IN_FIX = 4,
    SV_ADDITIONAL_INFO_CARRIER_FREQUENCY_EXIST = 8,
};

enum LocationErr {
    LOCATING_FAILED_DEFAULT = -1,
    LOCATING_FAILED_LOCATION_PERMISSION_DENIED = -2,
    LOCATING_FAILED_BACKGROUND_PERMISSION_DENIED = -3,
    LOCATING_FAILED_LOCATION_SWITCH_OFF = -4,
    LOCATING_FAILED_INTERNET_ACCESS_FAILURE = -5,
};

enum LocatingRequiredDataType {
    WIFI = 1,
    BLUE_TOOTH,
};

enum LocationSourceType {
    GNSS_TYPE = 1,
    NETWORK_TYPE = 2,
    INDOOR_TYPE = 3,
    RTK_TYPE = 4,
};

enum GeofenceTransitionEvent {
    GEOFENCE_TRANSITION_INIT = -1,
    GEOFENCE_TRANSITION_EVENT_ENTER = 1,
    GEOFENCE_TRANSITION_EVENT_EXIT = 2,
    GEOFENCE_TRANSITION_EVENT_DWELL = 4,
};

enum CoordinateSystemType {
    WGS84 = 1,
    GCJ02,
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
    std::string locale;
    double latitude;
    double longitude;
    int maxItems;
} ReverseGeocodeRequest;

typedef struct {
    int fenceId;
    GeofenceTransitionEvent event;
} GeofenceTransition;
} // namespace Location
} // namespace OHOS
#endif // CONSTANT_DEFINITION_H
