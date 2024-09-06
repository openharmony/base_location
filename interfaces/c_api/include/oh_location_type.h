/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Location
 * @{
 *
 * @brief Provide functions for querying the status of location switch, starting and stopping locating.
 *
 * @since 13
 */

/**
 * @file oh_location_type.h
 * @kit LocationKit
 * @brief Declares the common location attributes.
 * @library libohlocation.so
 * @syscap SystemCapability.Location.Location.Core
 * @since 13
 */

#ifndef OH_LOCATION_TYPE_H
#define OH_LOCATION_TYPE_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerates the location result codes.
 *
 * @since 13
 */
typedef enum Location_ResultCode {
    /**
     * @error The operation is successful.
     */
    LOCATION_SUCCESS = 0,
    /**
     * @error Permission verification failed. The application does not have the
     * permission required to call the API.
     */
    LOCATION_PERMISSION_DENIED = 201,
    /**
     * @error Parameter error. Possible reasons: 1. The input parameter is a null pointer;\n
     * 2. Parameter values exceed the defined range.\n
     */
    LOCATION_INVALID_PARAM = 401,
    /**
     * @error Capability not supported. Failed to call function due to limited device capabilities.
     */
    LOCATION_NOT_SUPPORTED = 801,
    /**
     * @error The location service is unavailable.
     * Possible reasons: Abnormal startup of location services.\n
     */
    LOCATION_SERVICE_UNAVAILABLE = 3301000,
    /**
     * @error The location switch is off.
     */
    LOCATION_SWITCH_OFF = 3301100
} Location_ResultCode;

/**
 * @brief Enumeration values of use scenarios.
 *
 * @since 13
 */
typedef enum Location_UseScene {
    /**
     * Indicates the navigation scenario.
     * This feature applies to outdoor scenarios where real-time device locations need
     * to be obtained, such as vehicle-mounted and pedestrian navigation.
     * The GNSS positioning technology is used to provide positioning services, and the
     * power consumption is high.
     */
    LOCATION_USE_SCENE_NAVIGATION = 0x0401,
    /**
     * Indicates the sport scenario.
     * This feature applies to scenarios where user location tracks are recorded,
     * for example, sports apps. The GNSS positioning technology is used to provide
     * positioning services, and the power consumption is high.
     */
    LOCATION_USE_SCENE_SPORT = 0x0402,
    /**
     * Indicates a travel scenario.
     * This mode applies to travel scenarios, such as taxi and public transportation.
     * The GNSS positioning technology is used to provide positioning services, and
     * the power consumption is high.
     */
    LOCATION_USE_SCENE_TRANSPORT = 0x0403,
    /**
     * Indicates the daily service usage scenario.
     * This mode applies to scenarios where precise user location is not required,
     * such as news, online shopping, and ordering applications.
     * In this scenario, only a network positioning technology is used to provide a
     * positioning service, and power consumption is relatively low.
     */
    LOCATION_USE_SCENE_DAILY_LIFE_SERVICE = 0x0404
} Location_UseScene;

/**
 * @brief Enumerates the power consumption scenario.
 *
 * @since 13
 */
typedef enum Location_PowerConsumptionScene {
    /**
     * High power consumption.
     * GNSS positioning technology is mainly used. We will use network positioning
     * technology to provide services before GNSS provides stable location results;
     * During continuous positioning, if the GNSS positioning result cannot be obtained
     * for more than 30 seconds, the network positioning technology is used to obtain
     * the location. Consumes a large number of hardware resources and power.
     */
    LOCATION_HIGH_POWER_CONSUMPTION = 0x0601,
    /**
     * Low power consumption.
     * This mode applies to scenarios that do not require high user location precision,
     * such as news, online shopping, and meal ordering.
     * In this scenario, only a network positioning technology is used to provide a
     * positioning service, and power consumption is relatively low.
     */
    LOCATION_LOW_POWER_CONSUMPTION = 0x0602,
    /**
     * No power consumption.
     * In this scenario, the location is not proactively triggered. The location is
     * returned to the current app only when other apps are located.
     */
    LOCATION_NO_POWER_CONSUMPTION = 0x0603
} Location_PowerConsumptionScene;

/**
 * @brief Enumerates the source type of location.
 *
 * @since 13
 */
typedef enum Location_SourceType {
    /**
     * The positioning result is based on the GNSS positioning technology.
     */
    LOCATION_SOURCE_TYPE_GNSS = 1,
    /**
     * The positioning result comes from the network positioning technology.
     */
    LOCATION_SOURCE_TYPE_NETWORK = 2,
    /**
     * The positioning result comes from the high-precision indoor positioning technology.
     */
    LOCATION_SOURCE_TYPE_INDOOR = 3,
    /**
     * The positioning result comes from the high-precision positioning technology.
     */
    LOCATION_SOURCE_TYPE_RTK = 4
} Location_SourceType;

/**
 * @brief Defines the location information.
 *
 * @since 13
 */
typedef struct Location_BasicInfo {
    /**
     * Indicates latitude information, with positive values indicating north latitude\n
     * and negative values indicating south latitude. The value range is -90 to 90.\n
     * Only supports WGS84 coordinate system.\n
     */
    double latitude;
    /**
     * Indicates longitude information, positive values indicate east longitude,\n
     * and negative values indicate west longitude. The value range is -180 to 180.\n
     * Only supports WGS84 coordinate system.\n
     */
    double longitude;
    /**
     * Altitude in meters.
     */
    double altitude;
    /**
     * Horizontal location accuracy in meters.
     */
    double accuracy;
    /**
     * Speed in meters per second.
     */
    double speed;
    /**
     * Heading in degrees. The value range is 0 to 360.
     */
    double direction;
    /**
     * Timestamp for the location fix. Number of milliseconds since January 1, 1970.
     */
    int64_t timeForFix;
    /**
     * Time since the system was booted, and include deep sleep. The unit is nanosecond.
     */
    int64_t timeSinceBoot;
    /**
     * Vertical position accuracy in meters.
     */
    double altitudeAccuracy;
    /**
     * Speed accuracy in meter per seconds.
     */
    double speedAccuracy;
    /**
     * Direction accuracy in degrees. The value range is 0 to 360.
     */
    double directionAccuracy;
    /**
     * Time uncertainty in nanosecond.
     */
    int64_t uncertaintyOfTimeSinceBoot;
    /**
     * Indicates the source of the location result.
     */
    Location_SourceType locationSourceType;
} Location_BasicInfo;

/**
 * @brief Define the structure of location information.
 * @since 13
 */
typedef struct Location_Info Location_Info;

/**
 * @brief Obtain basic location information.
 *
 * @param location - Pointer to the location information structure.\n
 * A non-null pointer is required. The pointer can be obtained from {@link Location_InfoCallback}.\n
 * @return Return the basic information structure of the location.\n
 * For a detailed definition, please refer to {@link Location_BasicInfo}.\n
 * @since 13
 */
Location_BasicInfo OH_LocationInfo_GetBasicInfo(Location_Info* location);

/**
 * @brief Obtain additional information from the location information.
 *
 * @param location - Pointer to the location information structure.\n
 * A non-null pointer is required. The pointer can be obtained from {@link Location_InfoCallback}.\n
 * @param additionalInfo - Non null pointers of char type; This variable is used to store additional\n
 * information strings. The string is in JSON format.\n
 * The pointer and the corresponding memory are created by the caller.\n
 * You are advised to apply for a memory of 256 bytes or more.\n
 * If a null pointer is passed, an error code is returned.\n
 * @param length - Memory size of additionalInfo.
 * @return Location functions result code.\n
 *     For a detailed definition, please refer to {@link Location_ResultCode}.\n
 *     {@link LOCAION_SUCCESS} Successfully obtained additional information.\n
 *     {@link LOCATION_INVALID_PARAM} 1.The input parameter location or additionalInfo is a null pointer.\n
 *         2.The input parameter length is too small to store additional information.\n
 * @since 13
 */
Location_ResultCode OH_LocationInfo_GetAdditionalInfo(Location_Info* location,
    char* additionalInfo, uint32_t length);

/**
 * @brief Defines the callback function used to report location data.
 *
 * @param location - Pointer to the {@link Location_Info} instance. Carry the latest location information.\n
 * The memory of the location instance is recycled at the end of {@link Location_InfoCallback}.\n
 * Before that, call {@link OH_LocationInfo_GetBasicInfo} and other interfaces to obtain location information.\n
 * @param userData - Pointer to an application data structure, this parameter is passed in\n
 * through {@link OH_LocationRequestConfig_SetCallback}.\n
 * @since 13
 */
typedef void (*Location_InfoCallback)(Location_Info* location, void* userData);

/**
 * @brief Define the structure of location request parameters.
 * @since 13
 */
typedef struct Location_RequestConfig Location_RequestConfig;

/**
 * @brief Create a location request parameter structure instance.
 *
 * @return Return a pointer to the {@ link Location_RequestConfig} instance. \n
 * If NULL is returned, it indicates that the creation failed. \n
 * The possible reason is that the application address space is full,\n
 * resulting in the inability to allocate space. \n
 * @since 13
 */
Location_RequestConfig* OH_Location_CreateRequestConfig(void);

/**
 * @brief Destroy the location request parameter instance and reclaim memory.
 *
 * @param requestConfig - Pointer to {@link Location_RequestConfig} instance.\n
 * The instance was created by {@link OH_Location_CreateRequestConfig}.\n
 * @since 13
 */
void OH_Location_DestroyRequestConfig(Location_RequestConfig* requestConfig);

/**
 * @brief Set the use scenario in the location request parameter.\n
 * Prioritize useScene in the location request parameter {@link Location_RequestConfig}.\n
 * If useScene is set, powerConsumptionScene becomes invalid.\n
 * If useScene is not set and powerConsumptionScene is set, this parameter takes effect.\n
 * If both parameters are not set, the default useScene is\n
 * {@link LOCATION_USE_SCENE_DAILY_LIFE_SERVICE},\n
 * and the powerConsumptionCenario parameter is invalid.\n
 *
 * @param requestConfig - Pointer to the {@link Location_RequestConfig} instance.\n
 * The instance was created by {@link OH_Location_CreateRequestConfig}.\n
 * @param useScene - Representing the use scenario during location requests.\n
 * The default value is {@link LOCATION_USE_SCENE_DAILY_LIFE_SERVICE}\n.
  * For a detailed definition, please refer to {@link Location_UseScene}.\n
 * @since 13
 */
void OH_LocationRequestConfig_SetUseScene(Location_RequestConfig* requestConfig,
    Location_UseScene useScene);

/**
 * @brief Set the power consumption scenario in the location request parameters.
 *
 * @param requestConfig - Pointer to the {@link Location_RequestConfig} instance.\n
 * The instance was created by {@link OH_Location_CreateRequestConfig}.\n
 * @param powerConsumptionScene - Represents the power consumption scenario for location requests.\n
 * The recognition value is {@link LOCATION_LOW_POWER_CONSUMPTION}.\n
 * For a detailed definition, please refer to {@link Location_PowerConsumptionScene}.\n
 * @since 13
 */
void OH_LocationRequestConfig_SetPowerConsumptionScene(Location_RequestConfig* requestConfig,
    Location_PowerConsumptionScene powerConsumptionScene);

/**
 * @brief Set the location reporting interval in the location request parameter.
 *
 * @param requestConfig - Pointer to the {@link Location_RequestConfig} instance.\n
 * The instance was created by {@link OH_Location_CreateRequestConfig}.\n
 * @param interval - Indicates the time interval for location reporting, in seconds.\n
 * The value is greater than or equal to 1. The default value is 1.\n
 * @since 13
 */
void OH_LocationRequestConfig_SetInterval(Location_RequestConfig* requestConfig,
    int interval);

/**
 * @brief Set up a callback function for receiving location information.
 *
 * @param requestConfig - Pointer to the {@link Location_RequestConfig} instance.\n
 * The instance was created by {@link OH_Location_CreateRequestConfig}.\n
 * @param callback - Pointer to the callback function for receiving the location.\n
 * For details, see {@link Location_InfoCallback}.\n
 * @param userData - Pointer to the application data structure, which will be\n
 * carried in the callback function.\n
 * @since 13
 */
void OH_LocationRequestConfig_SetCallback(Location_RequestConfig* requestConfig,
    Location_InfoCallback callback, void* userData);
#ifdef __cplusplus
}
#endif
/** @} */
#endif // OH_LOCATION_TYPE_H