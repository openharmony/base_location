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

#ifndef OHOS_LOCATION_CONSTANT_DEFINITION_H
#define OHOS_LOCATION_CONSTANT_DEFINITION_H

#include <string>

namespace OHOS {
namespace Location {
// error type
const int ERROR_PERMISSION_NOT_GRANTED = 0x0100;
const int ERROR_SWITCH_UNOPEN = 0x0101;
const int SESSION_START = 0x0002;
const int SESSION_STOP = 0x0003;

const int PRIVACY_TYPE_OTHERS = 0;
const int PRIVACY_TYPE_STARTUP = 1;
const int PRIVACY_TYPE_CORE_LOCATION = 2;

const int DEFAULT_UID = 10001;
const int DEFAULT_TIMEOUT_30S = 30000;

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

enum class LocationPrivacyType {
    OTHERS = 0,
    STARTUP,
    CORE_LOCATION
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
    int priority;
    int scenario;
    GeoFence geofence;
} GeofenceRequest;
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_CONSTANT_DEFINITION_H