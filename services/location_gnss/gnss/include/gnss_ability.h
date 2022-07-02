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
#include "i_gnss_status_callback.h"
#include "i_nmea_message_callback.h"
#include "i_cached_locations_callback.h"
#include "if_system_ability_manager.h"
#include "system_ability.h"
#include "common_utils.h"
#include "gnss_ability_skeleton.h"
#include "locator_ability.h"
#include "subability_common.h"
#include "agnss_event_callback.h"

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
    ~GnssAbility();
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    void SendLocationRequest(uint64_t interval, WorkRecord &workrecord) override;
    void SetEnable(bool state) override;
    void RemoteRequest(bool state) override;
    void RefrashRequirements() override;
    void RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) override;
    void UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) override;
    void RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback) override;
    void UnregisterCachedCallback(const sptr<IRemoteObject>& callback) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    int GetCachedGnssLocationsSize() override;
    int FlushCachedGnssLocations() override;
    void SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    void AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    void ReportGnssSessionStatus(int status);
    void ReportNmea(const std::string &nmea);
    void ReportSv(const std::unique_ptr<SatelliteStatus> &sv);
    bool EnableMock(const LocationMockConfig& config) override;
    bool DisableMock(const LocationMockConfig& config) override;
    bool SetMocked(const LocationMockConfig& config, const std::vector<std::shared_ptr<Location>> &location) override;
    void RequestRecord(WorkRecord &workRecord, bool isAdded) override;
    void SendReportMockLocationEvent() override;
    void StartGnss();
    void StopGnss();
    bool EnableGnss();
    void DisableGnss();
    bool ConnectHdi();
    void SetAgnssServer();
    void SetAgnssCallback();
    void SetSetId(const SubscriberSetId& id);
    void SetRefInfo(const AGnssRefInfo& refInfo);
    bool IsMockEnabled();
    void ProcessReportLocation();
private:
    bool Init();
    static void SaDumpInfo(std::string& result);
    bool IsGnssEnabled();
    int32_t ReportMockedLocation(const std::shared_ptr<Location> location);

    int locationIndex_ = 0;
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
};
} // namespace Location
} // namespace OHOS
#endif // GNSS_ABILITY_H
