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

#ifndef OHOS_LOCATION_GNSS_ABILITY_H
#define OHOS_LOCATION_GNSS_ABILITY_H

#include <mutex>
#include <singleton.h>
#include "i_gnss_status_callback.h"
#include "i_nmea_message_callback.h"
#include "if_system_ability_manager.h"
#include "system_ability.h"

#include "common_utils.h"
#include "gnss_ability_skeleton.h"
#include "gnss_vendor.h"
#include "subability_common.h"

#ifdef __aarch64__
#define VENDOR_GNSS_ADAPTER_SO_PATH "/system/lib64/vendorGnssAdapter.so"
#else
#define VENDOR_GNSS_ADAPTER_SO_PATH "/system/lib/vendorGnssAdapter.so"
#endif

namespace OHOS {
namespace Location {
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
    std::unique_ptr<Location> GetCachedLocation() override;
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
    void FlushCachedGnssLocations() override;
    void SendCommand(std::unique_ptr<LocationCommand>& commands) override;
    void AddFence(std::unique_ptr<GeofenceRequest>& request) override;
    void RemoveFence(std::unique_ptr<GeofenceRequest>& request) override;
    void ReportGnssSessionStatus(int status) override;
    void ReportNmea(const std::string &nmea) override;
    void ReportSv(const std::unique_ptr<SatelliteStatus> &sv) override;

    void RequestRecord(sptr<LocationCallbackStub> addCallback, WorkRecord &workRecord, bool isAdded) override;
    void NativeStart();
    void NativeStop();
    bool NativeInit();
    void NativeClear();
    static void StatusCallback(uint16_t* status);
    static void LocationUpdate(GnssLocation* location);
    static void NmeaCallback(int64_t timestamp, const char* nmea, int length);
    static void SvStatusCallback(GnssSatelliteStatus* svInfo);
private:
    bool nativeInitFlag_;
    void* handle;
    GnssVendorInterface* g_gpsInterface;
    std::unique_ptr<std::map<pid_t, sptr<IGnssStatusCallback>>> gnssStatusCallback_;
    std::unique_ptr<std::map<pid_t, sptr<INmeaMessageCallback>>> nmeaCallback_;
    bool Init();
    static void SaDumpInfo(std::string& result);
    bool registerToAbility_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_GNSS_ABILITY_H