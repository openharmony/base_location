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
#ifndef LOCATOR_AGENT_H
#define LOCATOR_AGENT_H

#include "request_config.h"
#include "i_locator_callback.h"
#include "constant_definition.h"

#include <singleton.h>
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_broker.h"
#include "i_locator.h"

#include "native_location_callback_host.h"
#include "native_sv_callback_host.h"
#include "native_nmea_callback_host.h"

namespace OHOS {
namespace Location {
class LocatorAgent : public IRemoteProxy<ILocator> {
public:
    explicit LocatorAgent(const sptr<IRemoteObject> &impl);
    ~LocatorAgent() = default;
    LocationErrCode StartGnssLocating(sptr<ILocatorCallback>& callback);
    LocationErrCode StopGnssLocating(sptr<ILocatorCallback>& callback);
    LocationErrCode RegisterNmeaMessageCallback(const sptr<INmeaMessageCallback>& callback);
    LocationErrCode UnregisterNmeaMessageCallback(const sptr<INmeaMessageCallback>& callback);
    LocationErrCode RegisterGnssStatusCallback(const sptr<IGnssStatusCallback>& callback);
    LocationErrCode UnregisterGnssStatusCallback(const sptr<IGnssStatusCallback>& callback);
private:
    LocationErrCode SendRequestToStub(const int msgId, MessageParcel& data, MessageParcel& reply);

    static inline BrokerDelegator<LocatorAgent> delegator_;
};

class LocatorAgentManager {
public:
    static LocatorAgentManager* GetInstance();
    explicit LocatorAgentManager();
    ~LocatorAgentManager();

    /**
     * @brief Subscribe location changed.
     *
     * @param callback Indicates the callback for reporting the location result.
     */
    LocationErrCode StartGnssLocating(const LocationCallbackIfaces& callback);

    /**
     * @brief Subscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     */
    LocationErrCode RegisterGnssStatusCallback(const SvStatusCallbackIfaces& callback);

    /**
     * @brief Subscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     */
    LocationErrCode RegisterNmeaMessageCallback(const GnssNmeaCallbackIfaces& callback);
    
    /**
     * @brief Unsubscribe location changed.
     */
    LocationErrCode StopGnssLocating();

    /**
     * @brief Unsubscribe nmea message changed.
     */
    LocationErrCode UnregisterNmeaMessageCallback();

    /**
     * @brief Unsubscribe satellite status changed.
     */
    LocationErrCode UnregisterGnssStatusCallback();
    void ResetLocatorAgent(const wptr<IRemoteObject> &remote);
private:
    class LocatorAgentDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit LocatorAgentDeathRecipient(LocatorAgentManager &impl) : impl_(impl) {}
        ~LocatorAgentDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            impl_.ResetLocatorAgent(remote);
        }
    private:
        LocatorAgentManager &impl_;
    };

    sptr<LocatorAgent> GetLocatorAgent();
    sptr<IRemoteObject> CheckLocatorSystemAbilityLoaded();
    bool TryLoadLocatorSystemAbility();
    sptr<LocatorAgent> InitLocatorAgent(sptr<IRemoteObject>& saObject);

    sptr<LocatorAgent> client_ { nullptr };
    sptr<IRemoteObject::DeathRecipient> recipient_ { nullptr };
    std::mutex mutex_;
    sptr<NativeLocationCallbackHost> locationCallbackHost_;
    sptr<NativeNmeaCallbackHost> nmeaCallbackHost_;
    sptr<NativeSvCallbackHost> gnssCallbackHost_;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATOR_AGENT_H