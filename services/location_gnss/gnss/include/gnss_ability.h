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

#ifndef GNSS_ABILITY_H
#define GNSS_ABILITY_H

#include <mutex>
#include <singleton.h>
#include <v1_0/ignss_interface.h>
#include <v1_0/ia_gnss_interface.h>

#include "event_handler.h"
#include "system_ability.h"

#include "agnss_event_callback.h"
#include "common_utils.h"
#include "constant_definition.h"
#include "gnss_ability_skeleton.h"
#include "i_gnss_status_callback.h"
#include "i_nmea_message_callback.h"
#include "subability_common.h"

namespace OHOS {
namespace Location {
#ifdef __aarch64__
#define VENDOR_GNSS_ADAPTER_SO_PATH "/system/lib64/vendorGnssAdapter.so"
#else
#define VENDOR_GNSS_ADAPTER_SO_PATH "/system/lib/vendorGnssAdapter.so"
#endif

using HDI::Location::Gnss::V1_0::IGnssInterface;
using HDI::Location::Gnss::V1_0::IGnssCallback;
using HDI::Location::Gnss::V1_0::GNSS_START_TYPE_NORMAL;
using HDI::Location::Gnss::V1_0::GNSS_STATUS_NONE;
using HDI::Location::Gnss::V1_0::GNSS_STATUS_SESSION_BEGIN;
using HDI::Location::Gnss::V1_0::GNSS_STATUS_SESSION_END;
using HDI::Location::Gnss::V1_0::GNSS_STATUS_ENGINE_ON;
using HDI::Location::Gnss::V1_0::GNSS_STATUS_ENGINE_OFF;
using HDI::Location::Agnss::V1_0::IAGnssInterface;
using HDI::Location::Agnss::V1_0::IAGnssCallback;
using HDI::Location::Agnss::V1_0::AGNSS_TYPE_SUPL;
using HDI::Location::Agnss::V1_0::AGnssServerInfo;

class GnssHandler : public AppExecFwk::EventHandler {
public:
    explicit GnssHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~GnssHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
};

class GnssAbility : public SystemAbility, public GnssAbilityStub, public SubAbility, DelayedSingleton<GnssAbility> {
DECLEAR_SYSTEM_ABILITY(GnssAbility);

public:
    DISALLOW_COPY_AND_MOVE(GnssAbility);
    GnssAbility();
    ~GnssAbility() override;
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    LocationErrCode SendLocationRequest(WorkRecord &workrecord) override;
    LocationErrCode SetEnable(bool state) override;
    LocationErrCode RefrashRequirements() override;
    LocationErrCode RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    LocationErrCode UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    LocationErrCode RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback) override;
    LocationErrCode UnregisterCachedCallback(const sptr<IRemoteObject>& callback) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    LocationErrCode GetCachedGnssLocationsSize(int &size) override;
    LocationErrCode FlushCachedGnssLocations() override;
    LocationErrCode SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    LocationErrCode AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    LocationErrCode RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    void ReportGnssSessionStatus(int status);
    void ReportNmea(int64_t timestamp, const std::string &nmea);
    void ReportSv(const std::unique_ptr<SatelliteStatus> &sv);
    LocationErrCode EnableMock() override;
    LocationErrCode DisableMock() override;
    LocationErrCode SetMocked(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
    void RequestRecord(WorkRecord &workRecord, bool isAdded) override;
    void SendReportMockLocationEvent() override;
    void SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply) override;
    void StartGnss();
    void StopGnss();
    bool EnableGnss();
    void DisableGnss();
    bool ConnectHdi();
    bool RemoveHdi();
    void SetAgnssServer();
    void SetAgnssCallback();
    void SetSetId(const SubscriberSetId& id);
    void SetRefInfo(const AGnssRefInfo& refInfo);
    bool IsMockEnabled();
    void ProcessReportLocationMock();
private:
    bool Init();
    static void SaDumpInfo(std::string& result);
    bool IsGnssEnabled();
    int32_t ReportMockedLocation(const std::shared_ptr<Location> location);

    bool isHdiConnected_;
    size_t mockLocationIndex_ = 0;
    bool registerToAbility_ = false;
    int gnssWorkingStatus_ = 0;
    std::shared_ptr<GnssHandler> gnssHandler_;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::unique_ptr<std::map<pid_t, sptr<IGnssStatusCallback>>> gnssStatusCallback_;
    std::unique_ptr<std::map<pid_t, sptr<INmeaMessageCallback>>> nmeaCallback_;
    sptr<IGnssInterface> gnssInterface_;
    sptr<IGnssCallback> gnssCallback_;
    sptr<IAGnssCallback> agnssCallback_;
    sptr<IAGnssInterface> agnssInterface_;
    std::mutex gnssMutex_;
};
} // namespace Location
} // namespace OHOS
#endif // GNSS_ABILITY_H
