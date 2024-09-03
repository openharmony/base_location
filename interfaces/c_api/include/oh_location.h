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
 * @since 13
 */
/**
 * @file oh_location.h
 * @kit LocationKit
 * @brief Define interfaces for querying location switch status, starting locating, and stopping locating.
 * @library libohlocation.so
 * @syscap SystemCapability.Location.Location.Core
 * @since 13
 */

#ifndef OH_LOCATION_H
#define OH_LOCATION_H

#include "oh_location_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check whether the location switch is enabled.
 *
 * @param enabled - It is a boolean pointer used to receive location switch status values.\n
 * Equal to true indicates that the location switch is turned on, false indicates that\n
 * the location switch is turned off.\n
 * The caller needs to pass in a non empty boolean pointer, otherwise an error will be returned.\n
 * @return Location functions result code.\n
 *     For a detailed definition, please refer to {@link Location_ResultCode}.\n
 *     {@link LOCAION_SUCCESS} Successfully obtained the location switch status.\n
 *     {@link LOCATION_INVALID_PARAM} The input parameter enabled is a null pointer.\n
 *     {@link LOCATION_SERVICE_UNAVAILABLE} Abnormal startup of location services.\n
 * @since 13
 */
Location_ResultCode OH_Location_IsLocatingEnabled(bool* enabled);

/**
 * @brief Start locating and subscribe location changed.
 *
 * @param requestConfig - Pointer to the locating request parameters.\n
 * For details, see {@link Location_RequestConfig}.\n
 * You can use {@link OH_Location_CreateRequestConfig} to create an instance.\n
 * @param callback - Pointer to the callback function for reporting the location.\n
 * For details, see {@link Location_InfoCallback}.\n
 * A non-null pointer is required, otherwise an error is returned.\n
 * @return Location functions result code.\n
 *     For a detailed definition, please refer to {@link Location_ResultCode}.\n
 *     {@link LOCAION_SUCCESS} Successfully start locating.\n
 *     {@link LOCATION_INVALID_PARAM} The input parameter requestConfig or callback is a null pointer.\n
 *     {@link LOCATION_PERMISSION_DENIED} Permission verification failed. The application does not have the\n
 *         permission required to call the API.\n
 *     {@link LOCATION_NOT_SUPPORTED} Capability not supported.\n
 *         Failed to call function due to limited device capabilities.\n
 *     {@link LOCATION_SERVICE_UNAVAILABLE} Abnormal startup of location services.\n
 *     {@link LOCATION_SWITCH_OFF} The location switch is off.\n
 * @permission ohos.permission.APPROXIMATELY_LOCATION
 * @since 13
 */
Location_ResultCode OH_Location_StartLocating(const Location_RequestConfig* requestConfig,
    Location_InfoCallback callback);

/**
 * @brief Stop locating and unsubscribe location changed.
 *
 * @param callback - Pointer to the callback function for reporting the location.\n
 * This parameter needs to be the same as the callback pointer passed in {@link OH_Location_StartLocating}.\n
 * For details, see {@link Location_InfoCallback}.\n
 * A non-null pointer is required, otherwise an error is returned.\n
 * @return Location functions result code.\n
 *     For a detailed definition, please refer to {@link Location_ResultCode}.\n
 *     {@link LOCAION_SUCCESS} Successfully stop locationg.\n
 *     {@link LOCATION_INVALID_PARAM} 1.The input parameter callback is a null pointer.
 *         2.Different from the callback passed from {@link OH_Location_StartLocating}.\n
 *     {@link LOCATION_PERMISSION_DENIED} Permission verification failed. The application does not have the\n
 *         permission required to call the API.\n
 *     {@link LOCATION_NOT_SUPPORTED} Capability not supported.\n
 *         Failed to call function due to limited device capabilities.\n
 *     {@link LOCATION_SERVICE_UNAVAILABLE} Possible reasons: 1. Abnormal startup of location services.\n
 *     {@link LOCATION_SWITCH_OFF} The location switch is off.\n
 * @permission ohos.permission.APPROXIMATELY_LOCATION
 * @since 13
 */
Location_ResultCode OH_Location_StopLocating(Location_InfoCallback callback);
#ifdef __cplusplus
}
#endif
/** @} */
#endif // OH_LOCATION_H