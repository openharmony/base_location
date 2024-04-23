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

#ifndef LOCATION_GNSS_GEOFENCE_CALLBACK_PROXY_H
#define LOCATION_GNSS_GEOFENCE_CALLBACK_PROXY_H

#include <vector>

#include "iremote_proxy.h"
#include "iremote_object.h"

#include "i_gnss_geofence_callback.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
class LocationGnssGeofenceCallbackProxy : public IRemoteProxy<IGnssGeofenceCallback> {
public:
    explicit LocationGnssGeofenceCallbackProxy(const sptr<IRemoteObject> &impl);
    ~LocationGnssGeofenceCallbackProxy() = default;
    void OnFenceIdChange(int fenceId);
    void OnTransitionStatusChange(GeofenceTransition transition);
private:
    static inline BrokerDelegator<LocationGnssGeofenceCallbackProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATING_REQUIRED_DATA_CALLBACK_PROXY_H
