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

#ifndef LOCATOR_ABILITY_H
#define LOCATOR_ABILITY_H

#include <map>
#include <mutex>
#include <singleton.h>

#include "event_handler.h"
#include "if_system_ability_manager.h"
#include "system_ability.h"

#include "common_hisysevent.h"
#include "common_utils.h"
#include "geo_convert_proxy.h"
#include "locator_callback_proxy.h"
#include "locator_event_subscriber.h"
#include "locator_skeleton.h"
#include "switch_callback_proxy.h"
#include "request.h"
#include "request_manager.h"
#include "report_manager.h"

namespace OHOS {
namespace Location {
class LocatorHandler : public AppExecFwk::EventHandler {
public:
    explicit LocatorHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~LocatorHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
};

class LocatorAbility : public SystemAbility, public LocatorAbilityStub, DelayedSingleton<LocatorAbility> {
DECLEAR_SYSTEM_ABILITY(LocatorAbility);

public:
    DISALLOW_COPY_AND_MOVE(LocatorAbility);
    LocatorAbility();
    ~LocatorAbility();
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    void InitSaAbility();
    void InitRequestManagerMap();

    void UpdateSaAbility() override;
    int GetSwitchState() override;
    void EnableAbility(bool isEnabled) override;
    void RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterSwitchCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    int StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback, std::string bundleName, pid_t pid, pid_t uid) override;
    int StopLocating(sptr<ILocatorCallback>& callback) override;
    int GetCacheLocation(MessageParcel& data, MessageParcel& replay) override;
    int IsGeoConvertAvailable(MessageParcel &data, MessageParcel &replay) override;
    int GetAddressByCoordinate(MessageParcel &data, MessageParcel &replay) override;
    int GetAddressByLocationName(MessageParcel &data, MessageParcel &replay) override;

    bool IsLocationPrivacyConfirmed(const int type) override;
    void SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) override;

    int RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback, std::string bundleName) override;
    int UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) override;

    int GetCachedGnssLocationsSize() override;
    void FlushCachedGnssLocations() override;
    void SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    void AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;

    int ReportLocation(const std::unique_ptr<Location>& location, std::string abilityName);
    int ReportLocationStatus(sptr<ILocatorCallback>& callback, int result);
    int ReportErrorStatus(sptr<ILocatorCallback>& callback, int result);

    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> GetRequests();
    std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> GetReceivers();
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> GetProxyMap();
    void UpdateSaAbilityHandler();
    void ApplyRequests();
    void RegisterAction();
private:
    bool Init();
    bool CheckSaValid();
    static int QuerySwitchState();
    int SendGeoRequest(int type, MessageParcel &data, MessageParcel &replay);

    static void SaDumpInfo(std::string& result);

    bool registerToAbility_ = false;
    bool isActionRegistered = false;
    bool isEnabled_ = false;
    std::string deviceId_;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::shared_ptr<LocatorEventSubscriber> locatorEventSubscriber_;
    std::unique_ptr<std::map<pid_t, sptr<ISwitchCallback>>> switchCallbacks_;
    std::shared_ptr<std::map<std::string, std::list<std::shared_ptr<Request>>>> requests_;
    std::shared_ptr<std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<Request>>>> receivers_;
    std::shared_ptr<std::map<std::string, sptr<IRemoteObject>>> proxyMap_;
    std::shared_ptr<LocatorHandler> locatorHandler_;
    std::shared_ptr<RequestManager> requestManager_;
    std::shared_ptr<ReportManager> reportManager_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_ABILITY_H
