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

#ifndef NETWORK_ABILITY_H
#define NETWORK_ABILITY_H
#ifdef FEATURE_NETWORK_SUPPORT

#include <string>
#include <singleton.h>

#include "ability_connect_callback_interface.h"
#include "event_handler.h"
#include "event_runner.h"
#include "ffrt.h"
#include "system_ability.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "network_ability_skeleton.h"
#include "subability_common.h"

namespace OHOS {
namespace Location {
static constexpr int REQUEST_NETWORK_LOCATION = 1;
static constexpr int REMOVE_NETWORK_LOCATION = 2;
class NetworkHandler : public AppExecFwk::EventHandler {
public:
    explicit NetworkHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~NetworkHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
};

class NLPServiceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    NLPServiceDeathRecipient();
    ~NLPServiceDeathRecipient() override;
};

class NetworkAbility : public SystemAbility, public NetworkAbilityStub, public SubAbility {
DECLEAR_SYSTEM_ABILITY(NetworkAbility);

public:
    DISALLOW_COPY_AND_MOVE(NetworkAbility);
    NetworkAbility();
    ~NetworkAbility() override;
    static NetworkAbility* GetInstance();
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    LocationErrCode SendLocationRequest(WorkRecord &workrecord) override;
    LocationErrCode SetEnable(bool state) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void RequestRecord(WorkRecord &workRecord, bool isAdded) override;
    LocationErrCode EnableMock() override;
    LocationErrCode DisableMock() override;
    LocationErrCode SetMocked(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
    void SendReportMockLocationEvent() override;
    bool CancelIdleState() override;
    void UnloadNetworkSystemAbility() override;
    void ProcessReportLocationMock();
    bool ConnectNlpService();
    bool ReConnectNlpService();
    bool ResetServiceProxy();
    void NotifyConnected(const sptr<IRemoteObject>& remoteObject);
    void NotifyDisConnected();
    bool IsMockEnabled();
    void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) override;
    void ReportLocationError(int32_t errCode, std::string errMsg, std::string uuid);
private:
    bool Init();
    static void SaDumpInfo(std::string& result);
    int32_t ReportMockedLocation(const std::shared_ptr<Location> location);
    bool CheckIfNetworkConnecting();
    bool RequestNetworkLocation(WorkRecord &workRecord);
    bool RemoveNetworkLocation(WorkRecord &workRecord);
    void RegisterNLPServiceDeathRecipient();
    void UnRegisterNLPServiceDeathRecipient();
    bool IsConnect();

    ffrt::mutex mutex_;
    sptr<IRemoteObject> nlpServiceProxy_;
    ffrt::condition_variable connectCondition_;
    std::shared_ptr<NetworkHandler> networkHandler_;
    size_t mockLocationIndex_ = 0;
    bool registerToAbility_ = false;
    sptr<IRemoteObject::DeathRecipient> nlpServiceRecipient_ = sptr<NLPServiceDeathRecipient>(new
        (std::nothrow) NLPServiceDeathRecipient());
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    sptr<AAFwk::IAbilityConnection> conn_;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_NETWORK_SUPPORT
#endif // NETWORK_ABILITY_H
