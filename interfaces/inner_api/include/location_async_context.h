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

#ifndef LOCATION_ASYNC_CONTEXT_H
#define LOCATION_ASYNC_CONTEXT_H

#include <list>
#include <string>

#include "message_parcel.h"
#include "napi/native_api.h"

#include "async_context.h"
#include "constant_definition.h"
#include "country_code.h"
#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_address.h"
#include "geo_coding_mock_info.h"
#endif
#include "location.h"
#include "location_log.h"
#include "locator_callback_host.h"
#include "request_config.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "satellite_status.h"
#endif

namespace OHOS {
namespace Location {
class LocationAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;

    explicit LocationAsyncContext(napi_env env, napi_async_work work = nullptr,
        napi_deferred deferred = nullptr) : AsyncContext(env, work, deferred), loc(nullptr) {}

    LocationAsyncContext() = delete;

    ~LocationAsyncContext() override {}
};

#ifdef FEATURE_GEOCODE_SUPPORT
class ReverseGeocodeMockAsyncContext : public AsyncContext {
public:
    std::vector<std::shared_ptr<GeocodingMockInfo>> mockInfo;
    bool enable;

    explicit ReverseGeocodeMockAsyncContext(napi_env env, napi_async_work work = nullptr,
        napi_deferred deferred = nullptr) : AsyncContext(env, work, deferred), enable(false) {}

    ReverseGeocodeMockAsyncContext() = delete;

    ~ReverseGeocodeMockAsyncContext() override {}
};
#endif

class LocationMockAsyncContext : public AsyncContext {
public:
    int32_t timeInterval;
    bool enable;

    std::vector<std::shared_ptr<Location>> LocationNapi;
    LocationMockAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), timeInterval(0), enable(false) {}

    LocationMockAsyncContext() = delete;

    ~LocationMockAsyncContext() override {}
};

class CountryCodeContext : public AsyncContext {
public:
    std::shared_ptr<CountryCode> country;

    explicit CountryCodeContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), country(nullptr) {}

    CountryCodeContext() = delete;

    ~CountryCodeContext() override {}
};

class LocationRequestAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;
    std::unique_ptr<RequestConfig> request;

    explicit LocationRequestAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), loc(nullptr), request(nullptr) {}

    LocationRequestAsyncContext() = delete;

    ~LocationRequestAsyncContext() override {}
};

class SwitchAsyncContext : public AsyncContext {
public:
    bool enable;

    explicit SwitchAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false) {}

    SwitchAsyncContext() = delete;

    ~SwitchAsyncContext() override {}
};

#ifdef FEATURE_GNSS_SUPPORT
class NmeaAsyncContext : public AsyncContext {
public:
    std::string msg;

    explicit NmeaAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), msg("") {}

    NmeaAsyncContext() = delete;

    ~NmeaAsyncContext() override {}
};
#endif

#ifdef FEATURE_GNSS_SUPPORT
class GnssStatusAsyncContext : public AsyncContext {
public:
    std::shared_ptr<SatelliteStatus> statusInfo;

    explicit GnssStatusAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), statusInfo(nullptr) {}

    GnssStatusAsyncContext() = delete;

    ~GnssStatusAsyncContext() override {}
};
#endif

#ifdef FEATURE_GNSS_SUPPORT
class CachedLocationAsyncContext : public AsyncContext {
public:
    std::vector<std::shared_ptr<Location>> locationList;

    explicit CachedLocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    CachedLocationAsyncContext() = delete;

    ~CachedLocationAsyncContext() override {}
};
#endif

class PrivacyAsyncContext : public AsyncContext {
public:
    int type;
    bool isConfirmed;

    explicit PrivacyAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), type(PRIVACY_TYPE_OTHERS), isConfirmed(false) {}

    PrivacyAsyncContext() = delete;

    ~PrivacyAsyncContext() override {}
};

#ifdef FEATURE_GNSS_SUPPORT
class CachedAsyncContext : public AsyncContext {
public:
    bool enable;
    int locationSize;

    explicit CachedAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false), locationSize(0) {}

    CachedAsyncContext() = delete;

    ~CachedAsyncContext() override {}
};
#endif

#ifdef FEATURE_GNSS_SUPPORT
class CommandAsyncContext : public AsyncContext {
public:
    bool enable;
    std::unique_ptr<LocationCommand> command;

    explicit CommandAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false), command(nullptr) {}

    CommandAsyncContext() = delete;

    ~CommandAsyncContext() override {}
};
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
class ReverseGeoCodeAsyncContext : public AsyncContext {
public:
    MessageParcel reverseGeoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    explicit ReverseGeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    ReverseGeoCodeAsyncContext() = delete;

    ~ReverseGeoCodeAsyncContext() override {}
};
#endif

#ifdef FEATURE_GEOCODE_SUPPORT
class GeoCodeAsyncContext : public AsyncContext {
public:
    MessageParcel geoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    explicit GeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    GeoCodeAsyncContext() = delete;

    ~GeoCodeAsyncContext() override {}
};
#endif

class SingleLocationAsyncContext : public AsyncContext {
public:
    int timeout_;
    sptr<LocatorCallbackHost> callbackHost_;

    explicit SingleLocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), timeout_(0), callbackHost_(0) {}

    SingleLocationAsyncContext() = delete;

    ~SingleLocationAsyncContext() override {}
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_ASYNC_CONTEXT_H