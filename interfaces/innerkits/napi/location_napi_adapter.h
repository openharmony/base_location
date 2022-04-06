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

#ifndef LOCATION_NAPI_ADAPTER_H
#define LOCATION_NAPI_ADAPTER_H

#include <array>
#include "constant_definition.h"
#include "location.h"
#include "location_util.h"
#include "message_parcel.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "request_config.h"
#include "common_utils.h"
#include "ipc_skeleton.h"
#include "lbs_log.h"
#include "locator.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Location {
napi_value GetLastLocation(napi_env env, napi_callback_info info);
napi_value IsLocationEnabled(napi_env env, napi_callback_info info);
napi_value EnableLocation(napi_env env, napi_callback_info info);
napi_value DisableLocation(napi_env env, napi_callback_info info);
napi_value RequestEnableLocation(napi_env env, napi_callback_info info);
napi_value IsGeoServiceAvailable(napi_env env, napi_callback_info info);
napi_value GetAddressesFromLocation(napi_env env, napi_callback_info info);
napi_value GetAddressesFromLocationName(napi_env env, napi_callback_info info);
napi_value IsLocationPrivacyConfirmed(napi_env env, napi_callback_info info);
napi_value SetLocationPrivacyConfirmStatus(napi_env env, napi_callback_info info);
napi_value GetCachedGnssLocationsSize(napi_env env, napi_callback_info info);
napi_value FlushCachedGnssLocations(napi_env env, napi_callback_info info);
napi_value SendCommand(napi_env env, napi_callback_info info);

class LocationRequestAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;
    std::unique_ptr<RequestConfig> request;

    LocationRequestAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred)
        {
            loc = nullptr;
            request = nullptr;
        }

    LocationRequestAsyncContext() = delete;

    virtual ~LocationRequestAsyncContext(){}
};

class LocationAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;

    LocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred)
        {
            loc = nullptr;
        }

    LocationAsyncContext() = delete;

    virtual ~LocationAsyncContext(){}
};

class LocatorAsyncContext : public AsyncContext {
public:
    std::shared_ptr<Location> loc;

    LocatorAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred)
        {
            loc = nullptr;
        }

    LocatorAsyncContext() = delete;

    virtual ~LocatorAsyncContext(){}
};

class SwitchAsyncContext : public AsyncContext {
public:
    bool enable;

    SwitchAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred)
        {
            enable = false;
        }

    SwitchAsyncContext() = delete;

    virtual ~SwitchAsyncContext(){}
};

class PrivacyAsyncContext : public AsyncContext {
public:
    LocationPrivacyType type;
    bool isConfirmed;

    PrivacyAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred)
        {
            type = LocationPrivacyType::OTHERS;
            isConfirmed = false;
        }

    PrivacyAsyncContext() = delete;

    virtual ~PrivacyAsyncContext(){}
};

class CachedAsyncContext : public AsyncContext {
public:
    bool enable;
    int locationSize;

    CachedAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred)
        {
            locationSize = 0;
            enable = false;
        }

    CachedAsyncContext() = delete;

    virtual ~CachedAsyncContext(){}
};

class CommandAsyncContext : public AsyncContext {
public:
    bool enable;
    std::unique_ptr<LocationCommand> command;

    CommandAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred)
        {
            command = nullptr;
            enable = false;
        }

    CommandAsyncContext() = delete;

    virtual ~CommandAsyncContext(){}
};

class ReverseGeoCodeAsyncContext : public AsyncContext {
public:
    MessageParcel reverseGeoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    ReverseGeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred) {}

    ReverseGeoCodeAsyncContext() = delete;

    virtual ~ReverseGeoCodeAsyncContext(){}
};

class GeoCodeAsyncContext : public AsyncContext {
public:
    MessageParcel geoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    GeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred) {}

    GeoCodeAsyncContext() = delete;

    virtual ~GeoCodeAsyncContext(){}
};
}  // namespace Location
}  // namespace OHOS

#endif