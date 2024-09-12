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

#include "location_log.h"
#include "cj_lambda.h"
#include "cached_locations_callback.h"
#include "location.h"

namespace OHOS {
namespace GeoLocationManager {
CachedLocationsCallback::CachedLocationsCallback()
{
    remoteDied_ = false;
}

CachedLocationsCallback::CachedLocationsCallback(int64_t callbackId)
{
    remoteDied_ = false;
    this->callbackId_ = callbackId;
    auto cFunc = reinterpret_cast<void(*)(CJLocationArr locations)>(callbackId);
    callback_ = [ lambda = CJLambda::Create(cFunc)](const std::vector<std::unique_ptr<Location::Location>>& locations)
        -> void { lambda(LocationVectorToCJLocationArr(locations)); };
}

CachedLocationsCallback::~CachedLocationsCallback()
{
}

int CachedLocationsCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(Location::CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallback::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(Location::CACHED_LOCATIONS_CALLBACK, "invalid token.");
        return -1;
    }
    if (remoteDied_) {
        LBSLOGD(Location::CACHED_LOCATIONS_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }

    switch (code) {
        case Location::ICachedLocationsCallback::RECEIVE_CACHED_LOCATIONS_EVENT: {
            int size = data.ReadInt32();
            if (size > 0 && size < Location::MAXIMUM_CACHE_LOCATIONS) {
                std::vector<std::unique_ptr<Location::Location>> locations(size);
                for (int i = 0; i < size; i++) {
                    locations.push_back(Location::Location::Unmarshalling(data));
                }
                OnCacheLocationsReport(locations);
            }
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool CachedLocationsCallback::IsRemoteDied()
{
    return remoteDied_;
}

void CachedLocationsCallback::OnCacheLocationsReport(const std::vector<std::unique_ptr<Location::Location>>& locations)
{
    LBSLOGD(Location::CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallback::OnCacheLocationsReport");
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_ != nullptr) {
        callback_(locations);
    }
}
}
}