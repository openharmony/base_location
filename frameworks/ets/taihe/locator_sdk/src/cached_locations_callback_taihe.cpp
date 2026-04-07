/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "cached_locations_callback_taihe.h"

#include "location_log.h"
#include "util.h"

namespace OHOS {
namespace Location {
int CachedLocationsCallbackTaihe::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackTaihe::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(CACHED_LOCATIONS_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_CACHED_LOCATIONS_EVENT: {
            int size = data.ReadInt32();
            if (size <= 0 || size > MAXIMUM_CACHE_LOCATIONS) {
                break;
            }
            std::vector<std::unique_ptr<Location>> locations;
            for (int i = 0; i < size; i++) {
                auto loc = Location::UnmarshallingMakeUnique(data);
                if (loc != nullptr) {
                    locations.push_back(std::move(loc));
                }
            }
            OnCacheLocationsReport(locations);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void CachedLocationsCallbackTaihe::SetCallback(
    ::taihe::optional<::taihe::callback<void(::taihe::array_view<::ohos::geoLocationManager::Location>)>> callback)
{
    std::unique_lock<std::mutex> guard(mutex_);
    callback_ = callback;
}


void CachedLocationsCallbackTaihe::GetCallback(
    ::taihe::optional<::taihe::callback<void(::taihe::array_view<::ohos::geoLocationManager::Location>)>>& callback)
{
    std::unique_lock<std::mutex> guard(mutex_);
    callback = callback_;
}

void CachedLocationsCallbackTaihe::OnCacheLocationsReport(const std::vector<std::unique_ptr<Location>>& locations)
{
    LBSLOGI(CACHED_LOCATIONS_CALLBACK, "CachedLocationsCallbackTaihe::OnLocatingDataChange");
    std::vector<::ohos::geoLocationManager::Location> locationList;
    for (auto &location : locations) {
        if (location == nullptr) {
            continue;
        }
        ::ohos::geoLocationManager::Location locationTaihe;
        Util::LocationToTaihe(locationTaihe, location);
        locationList.push_back(locationTaihe);
    }
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_) {
        (*callback_)(locationList);
    }
}
}  // namespace Location
}  // namespace OHOS
