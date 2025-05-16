/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "location_info_callback_host.h"

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"

#include "location.h"
#include "location_log.h"
#include "oh_location_type.h"
#include "locator_c_impl.h"
#include <nlohmann/json.hpp>
#include "securec.h"

namespace OHOS {
namespace Location {
int LocationInfoCallbackHost::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_LOCATION_INFO_EVENT: {
            std::unique_ptr<Location> location = Location::UnmarshallingMakeUnique(data);
            OnLocationReport(location);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocationInfoCallbackHost::OnLocationReport(const std::unique_ptr<Location>& location)
{
    LBSLOGI(LOCATOR_CALLBACK, "LocationInfoCallbackHost::OnLocationReport");
    Location_Info location_info;
    memset_s(&location_info, sizeof(Location_Info), 0, sizeof(Location_Info));
    location_info.latitude = location->GetLatitude();
    location_info.longitude = location->GetLongitude();
    location_info.altitude = location->GetAltitude();
    location_info.accuracy = location->GetAccuracy();
    location_info.speed = location->GetSpeed();
    location_info.direction = location->GetDirection();
    location_info.timeForFix = location->GetTimeStamp();
    location_info.timeSinceBoot = location->GetTimeSinceBoot();
    nlohmann::json additionJson;
    auto additionMap = location->GetAdditionsMap();
    for (auto addition : additionMap) {
        additionJson[addition.first] = addition.second;
    }
    std::string additionStr = additionJson.dump();
    auto ret = sprintf_s(location_info.additions, sizeof(location_info.additions), "%s", additionStr.c_str());
    if (ret <= 0) {
        LBSLOGE(OHOS::Location::LOCATION_CAPI, "sprintf_s failed, ret: %{public}d", ret);
        // addition is empty, no need return
    }
    location_info.altitudeAccuracy = location->GetAltitudeAccuracy();
    location_info.speedAccuracy = location->GetSpeedAccuracy();
    location_info.directionAccuracy = location->GetDirectionAccuracy();
    location_info.uncertaintyOfTimeSinceBoot = location->GetUncertaintyOfTimeSinceBoot();
    location_info.locationSourceType = (Location_SourceType)location->GetLocationSourceType();
    if (requestConfig_ != nullptr && requestConfig_->callback_ != nullptr &&
        requestConfig_->userData_ != nullptr && IsRequestConfigValid()) {
        requestConfig_->callback_(&location_info, requestConfig_->userData_);
    }
}

void LocationInfoCallbackHost::OnLocatingStatusChange(const int status)
{
}

void LocationInfoCallbackHost::OnErrorReport(const int errorCode)
{
}
} // namespace Location
} // namespace OHOS
