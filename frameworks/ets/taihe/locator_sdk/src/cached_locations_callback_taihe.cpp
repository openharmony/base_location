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
            if (size > 0 && size < MAXIMUM_CACHE_LOCATIONS) {
                std::vector<std::unique_ptr<Location>> locations;
                for (int i = 0; i < size; i++) {
                    locations.push_back(Location::UnmarshallingMakeUnique(data));
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

void CachedLocationsCallbackTaihe::OnCacheLocationsReport(const std::vector<std::unique_ptr<Location>>& locations)
{
    LBSLOGI(CACHED_LOCATIONS_CALLBACK, "LocatingRequiredDataCallbackTaihe::OnLocatingDataChange");
    std::vector<::ohos::geoLocationManager::Location> locationList;
    for (auto &location : locations) {
        std::unique_ptr<Location> locationReport = std::make_unique<Location>(*location);
        ::ohos::geoLocationManager::Location locationTaihe;
        Util::LocationToTaihe(locationTaihe, locationReport);
        locationList.push_back(locationTaihe);
    }
    if (callback_) {
        (*callback_)(locationList);
    }
}
}  // namespace Location
}  // namespace OHOS
