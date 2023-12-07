/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef LOCATION_LOG_EVENT_IDS_H
#define LOCATION_LOG_EVENT_IDS_H
namespace OHOS {
namespace Location {
enum {
    ADD_REQUEST = 0,
    REMOVE_REQUEST,
    RECEIVE_GNSS_LOCATION,
    RECEIVE_SATELLITESTATUSINFO,
    START_GNSS,
    STOP_GNSS,
    RECEIVE_NETWORK_LOCATION,
    NLP_SERVICE_TIMEOUT,
    NETWORK_CALLBACK_LOCATION,
    LOCATION_REQUEST_DENY,
    HDI_EVENT,
    GEOCODE_ERROR_EVENT,
    LBS_REQUEST_TOO_MUCH,
    GEOCODE_REQUEST,
    GEOCODE_SUCCESS
};

constexpr size_t LBS_REQUEST_MAX_SIZE = 20;
} // namespace Location
} // namespace OHOS
#endif