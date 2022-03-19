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

#ifndef OHOS_LOCATION_CALLBACK_PROXY_H
#define OHOS_LOCATION_CALLBACK_PROXY_H

#include <list>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

#include "location.h"
#include "locator_skeleton.h"
#include "lbs_log.h"

namespace OHOS {
namespace Location {
class ILocationCallback : public IRemoteBroker {
public:
    enum {
        RECEIVE_LOCATION_CHANGE_EVENT = 1,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.location.ILocationListener");
    virtual void OnLocationUpdate(const std::unique_ptr<Location>& location) = 0;
};

class LocationCallbackStub : public IRemoteStub<ILocationCallback> {
public:
    LocationCallbackStub(std::string abilityName);
    ~LocationCallbackStub() = default;
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void OnLocationUpdate(const std::unique_ptr<Location>& location) override;
    void OnSvStatusUpdate(const std::unique_ptr<SatelliteStatus> &sv);
    void OnStatusUpdate(unsigned int gnssSessionStatus);
    void OnNmeaUpdate(int64_t timestamp, const std::string &nmea);
    std::string GetAbilityName();
    void GetRemoteLocatorProxy(std::string deviceId);
    void init(std::string abilityName);
private:
    OHOS::HiviewDFX::HiLogLabel label_;
    std::unique_ptr<LocatorProxy> proxyLocator_;
    bool isInitForProxy_;
    std::string abilityName_;
    std::string localDeviceId_;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_CALLBACK_PROXY_H
