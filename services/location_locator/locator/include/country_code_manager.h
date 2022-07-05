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

#ifndef COUNTRY_CODE_MANAGER_H
#define COUNTRY_CODE_MANAGER_H

#include <list>
#include <map>
#include <singleton.h>
#include <string>
#include "iremote_stub.h"
#include "request.h"
#include "country_code.h"
#include "location.h"
#include "common_event_subscriber.h"
#include "i_locator_callback.h"
#include "request.h"

namespace OHOS {
namespace Location {
class CountryCodeManager : public DelayedSingleton<CountryCodeManager> {
public:
    CountryCodeManager();
    ~CountryCodeManager();
    std::shared_ptr<CountryCode> GetIsoCountryCode();
    void StartPassiveLocationListen();
    std::string GetCountryCodeByLocation(std::shared_ptr<Location>& location);
    void UpdateCountryCodeByLocation(std::string countryCode, int type);
    void UpdateCountryCode(std::string countryCode, int type);
    std::string GetCountryCodeByLastLocation();
    void UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback);
    void RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    void NotifyAllListener();
    void SubscribeSimEvent();
    bool SubscribeNetworkStatusEvent();
    void UnsubscribeSimEvent();
    bool UnsubscribeNetworkStatusEvent();
private:
    class LocatorCallback : public IRemoteStub<ILocatorCallback> {
    public:
        void OnLocationReport(const std::unique_ptr<Location>& location);
        void OnLocatingStatusChange(const int status);
        void OnErrorReport(const int errorCode);
    };

    class NetworkSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit NetworkSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info);
        virtual ~NetworkSubscriber() override = default;
    private:
        virtual void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &event) override;
    };

    class SimSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
    public:
        explicit SimSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &info);
        virtual ~SimSubscriber() override = default;
    private:
        virtual void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &event) override;
    };

    sptr<LocatorCallback> callback_;
    std::shared_ptr<CountryCode> lastCountryByLocation_;
    std::shared_ptr<CountryCode> lastCountry_;
    std::unique_ptr<std::map<pid_t, sptr<ICountryCodeCallback>>> countryCodeCallback_;
    std::shared_ptr<SimSubscriber> simSubscriber_;
    std::shared_ptr<NetworkSubscriber> networkSubscriber_;
};
} // namespace Location
} // namespace OHOS
#endif // COUNTRY_CODE_MANAGER_H
