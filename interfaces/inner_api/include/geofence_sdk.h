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
#ifndef GEOFENCE_SDK_H
#define GEOFENCE_SDK_H

#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_broker.h"
#include "i_locator.h"
#include "geofence_request.h"
#include "constant_definition.h"
#include "locationhub_ipc_interface_code.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
class GeofenceSdk : public IRemoteProxy<ILocator> {
public:
    explicit GeofenceSdk(const sptr<IRemoteObject> &impl);
    ~GeofenceSdk() = default;
    LocationErrCode AddFenceV9(std::shared_ptr<GeofenceRequest>& request);
    LocationErrCode RemoveFenceV9(std::shared_ptr<GeofenceRequest>& request);
    LocationErrCode AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request);
    LocationErrCode RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request);
    LocationErrCode GetGeofenceSupportedCoordTypes(
        std::vector<CoordinateSystemType>& coordinateSystemTypes);
    LocationErrCode HandleGnssfenceRequest(LocatorInterfaceCode code,
        std::shared_ptr<GeofenceRequest>& request);
private:
	LocationErrCode SendMsgWithDataReplyV9(const int msgId, MessageParcel& data, MessageParcel& reply);
    static inline BrokerDelegator<GeofenceSdk> delegator_;
};

class GeofenceManager {
public:
    static GeofenceManager* GetInstance();
    explicit GeofenceManager();
    ~GeofenceManager();

    /**
     * @brief Add a geofence and subscribe geo fence status changed.
     *
     * @param request Indicates the Geofence configuration parameters.
     * @return Returns ERRCODE_SUCCESS if the fence has been added successfully.
     */
    LocationErrCode AddFenceV9(std::shared_ptr<GeofenceRequest>& request);
    /**
     * @brief Remove a geofence and unsubscribe geo fence status changed.
     *
     * @param request Indicates the Geofence configuration parameters.
     * @return Returns ERRCODE_SUCCESS if the fence has been removed successfully.
     */
    LocationErrCode RemoveFenceV9(std::shared_ptr<GeofenceRequest>& request);
    LocationErrCode AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request);
    LocationErrCode RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request);
    LocationErrCode GetGeofenceSupportedCoordTypes(
        std::vector<CoordinateSystemType>& coordinateSystemTypes);
    void ResetGeofenceSdkProxy(const wptr<IRemoteObject> &remote);
private:
    sptr<GeofenceSdk> GetProxy();

    sptr<GeofenceSdk> client_ { nullptr };
    sptr<IRemoteObject::DeathRecipient> recipient_ { nullptr };
    std::mutex mutex_;
    bool isServerExist_ = false;

private:
    class GeofenceManagerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit GeofenceManagerDeathRecipient(GeofenceManager &impl) : impl_(impl) {}
        ~GeofenceManagerDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            impl_.ResetGeofenceSdkProxy(remote);
        }
    private:
        GeofenceManager &impl_;
    };
};
}  // namespace Location
}  // namespace OHOS
#endif // GEOFENCE_SDK_H