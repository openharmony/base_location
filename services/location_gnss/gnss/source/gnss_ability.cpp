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

#include "gnss_ability.h"
#include <file_ex.h>
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"
#include "location_log.h"
#include "location_dumper.h"

#include "gnss_status_callback_proxy.h"
#include "nmea_message_callback_proxy.h"

namespace OHOS {
namespace Location {
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<GnssAbility>::GetInstance().get());
GnssBasicCallbackIfaces g_gnssBasicCb = {
    .size = sizeof(GnssBasicCallbackIfaces),
    .locationUpdate = GnssAbility::LocationUpdate,
    .statusUpdate = GnssAbility::StatusCallback,
    .svStatusUpdate = GnssAbility::SvStatusCallback,
    .nmeaUpdate = GnssAbility::NmeaCallback,
    .capabilitiesUpdate = nullptr,
    .refInfoRequest = nullptr,
    .downloadRequestCb = nullptr,
};

GnssCacheCallbackIfaces g_gnssCacheCb = {
    .size = 0,
    .cachedLocationCb = nullptr,
};

GnssCallbackStruct g_callbacks = {
    .size = sizeof(GnssCallbackStruct),
    .gnssCb = g_gnssBasicCb,
    .gnssCacheCb = g_gnssCacheCb,
};

GnssAbility::GnssAbility() : SystemAbility(LOCATION_GNSS_SA_ID, true)
{
    gnssStatusCallback_ = std::make_unique<std::map<pid_t, sptr<IGnssStatusCallback>>>();
    nmeaCallback_ = std::make_unique<std::map<pid_t, sptr<INmeaMessageCallback>>>();
    SetAbility(GNSS_ABILITY);
    nativeInitFlag_ = false;
    handle = nullptr;
    g_gpsInterface = nullptr;
    LBSLOGI(GNSS, "ability constructed.");
}

GnssAbility::~GnssAbility()
{
    NativeClear();
    dlclose(handle);
}

void GnssAbility::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LBSLOGI(GNSS, "ability has already started.");
        return;
    }
    if (!Init()) {
        LBSLOGE(GNSS, "failed to init ability");
        OnStop();
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    LBSLOGI(GNSS, "OnStart start ability success.");
}

void GnssAbility::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToAbility_ = false;
    LBSLOGI(GNSS, "OnStop ability stopped.");
}

bool GnssAbility::Init()
{
    if (!registerToAbility_) {
        bool ret = Publish(AsObject());
        if (!ret) {
            LBSLOGE(GNSS, "Init Publish failed!");
            return false;
        }
        registerToAbility_ = true;
    }
    return true;
}

void GnssAbility::SendLocationRequest(uint64_t interval, WorkRecord &workrecord)
{
    LocationRequest(interval, workrecord);
}

std::unique_ptr<Location> GnssAbility::GetCachedLocation()
{
    return GetCache();
}

void GnssAbility::SetEnable(bool state)
{
    Enable(state, AsObject());
}

void GnssAbility::RemoteRequest(bool state)
{
    std::string deviceId = IPCSkeleton::GetCallingDeviceID();
    HandleRemoteRequest(state, deviceId);
}

void GnssAbility::RefrashRequirements()
{
    HandleRefrashRequirements();
}

void GnssAbility::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid gnssStatus callback");
        return;
    }

    sptr<IGnssStatusCallback> gnssStatusCallback = iface_cast<IGnssStatusCallback>(callback);
    if (gnssStatusCallback == nullptr) {
        LBSLOGE(GNSS, "cast switch callback fail!");
        return;
    }
    gnssStatusCallback_->erase(uid);
    gnssStatusCallback_->insert(std::make_pair(uid, gnssStatusCallback));
    LBSLOGD(GNSS, "after uid:%{public}d register, gnssStatusCallback size:%{public}s",
        uid, std::to_string(gnssStatusCallback_->size()).c_str());
}

void GnssAbility::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "unregister an invalid gnssStatus callback");
        return;
    }
    sptr<IGnssStatusCallback> gnssStatusCallback = iface_cast<IGnssStatusCallback>(callback);
    if (gnssStatusCallback == nullptr) {
        LBSLOGE(GNSS, "cast gnssStatus callback fail!");
        return;
    }

    pid_t uid = -1;
    for (auto iter = gnssStatusCallback_->begin(); iter != gnssStatusCallback_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        if (remoteObject == callback) {
            uid = iter->first;
            break;
        }
    }
    gnssStatusCallback_->erase(uid);
    LBSLOGD(GNSS, "after uid:%{public}d unregister, gnssStatus callback size:%{public}s",
        uid, std::to_string(gnssStatusCallback_->size()).c_str());
}

void GnssAbility::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid nmea callback");
        return;
    }

    sptr<INmeaMessageCallback> nmeaCallback = iface_cast<INmeaMessageCallback>(callback);
    if (nmeaCallback == nullptr) {
        LBSLOGE(GNSS, "cast nmea callback fail!");
        return;
    }
    nmeaCallback_->erase(uid);
    nmeaCallback_->insert(std::make_pair(uid, nmeaCallback));
    LBSLOGD(GNSS, "after uid:%{public}d register, nmeaCallback size:%{public}s",
        uid, std::to_string(nmeaCallback_->size()).c_str());
}

void GnssAbility::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "unregister an invalid nmea callback");
        return;
    }
    sptr<INmeaMessageCallback> nmeaCallback = iface_cast<INmeaMessageCallback>(callback);
    if (nmeaCallback == nullptr) {
        LBSLOGE(GNSS, "cast nmea callback fail!");
        return;
    }

    pid_t uid = -1;
    for (auto iter = nmeaCallback_->begin(); iter != nmeaCallback_->end(); iter++) {
        sptr<IRemoteObject> remoteObject = (iter->second)->AsObject();
        if (remoteObject == callback) {
            uid = iter->first;
            break;
        }
    }
    nmeaCallback_->erase(uid);
    LBSLOGD(GNSS, "after uid:%{public}d unregister, nmea callback size:%{public}s",
        uid, std::to_string(nmeaCallback_->size()).c_str());
}

void GnssAbility::RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
    const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid cached location callback");
        return;
    }

    sptr<ICachedLocationsCallback> cachedCallback = iface_cast<ICachedLocationsCallback>(callback);
    if (cachedCallback == nullptr) {
        LBSLOGE(GNSS, "cast cached location callback fail!");
        return;
    }
    LBSLOGD(GNSS, "request:%{public}d %{public}d",
        request->reportingPeriodSec, request->wakeUpCacheQueueFull ? 1 : 0);
}

void GnssAbility::UnregisterCachedCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid cached location callback");
        return;
    }

    sptr<ICachedLocationsCallback> cachedCallback = iface_cast<ICachedLocationsCallback>(callback);
    if (cachedCallback == nullptr) {
        LBSLOGE(GNSS, "cast cached location callback fail!");
        return;
    }
}

void GnssAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    LBSLOGE(GNSS, "enter RequestRecord");
    if (isAdded) {
        NativeStart();
    } else {
        NativeStop();
    }
    std::string state = isAdded ? "start" : "stop";
    WriteGnssStateEvent(state);
}

int GnssAbility::GetCachedGnssLocationsSize()
{
    int size = 0;
    return size;
}

void GnssAbility::FlushCachedGnssLocations()
{
}

void GnssAbility::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
}

void GnssAbility::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
}

void GnssAbility::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
}

void GnssAbility::ReportGnssSessionStatus(int status)
{
}

void GnssAbility::ReportNmea(const std::string &nmea)
{
    for (auto iter = nmeaCallback_->begin(); iter != nmeaCallback_->end(); iter++) {
        sptr<INmeaMessageCallback> nmeaCallback = (iter->second);
        nmeaCallback->OnMessageChange(nmea);
    }
}

void GnssAbility::ReportSv(const std::unique_ptr<SatelliteStatus> &sv)
{
    for (auto iter = gnssStatusCallback_->begin(); iter != gnssStatusCallback_->end(); iter++) {
        sptr<IGnssStatusCallback> callback = (iter->second);
        callback->OnStatusChange(sv);
    }
}

void GnssAbility::LocationUpdate(GnssLocation* location)
{
    std::unique_ptr<Location> locationNew = std::make_unique<Location>();
    if (location == nullptr || locationNew == nullptr) {
        LBSLOGE(GNSS, "LocationUpdate : location or callback is nullptr");
        return;
    }
    LBSLOGI(GNSS, "LocationUpdate");
    locationNew->SetLatitude(location->latitude);
    locationNew->SetLongitude(location->longitude);
    locationNew->SetAltitude(location->altitude);
    locationNew->SetAccuracy(location->horizontalAccuracy);
    locationNew->SetSpeed(location->speed);
    locationNew->SetDirection(0);
    locationNew->SetTimeStamp(location->timestamp);
    locationNew->SetTimeSinceBoot(location->timestampSinceBoot);
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocation(locationNew, GNSS_ABILITY);
    DelayedSingleton<LocatorAbility>::GetInstance().get()->ReportLocation(locationNew, PASSIVE_ABILITY);
}

void GnssAbility::StatusCallback(uint16_t* status)
{
    if (status == nullptr) {
        LBSLOGE(GNSS, "StatusCallback: param is nullptr");
        return;
    }
    LBSLOGI(GNSS, "StatusCallback, status : %{public}d", *status);
    DelayedSingleton<GnssAbility>::GetInstance().get()->ReportGnssSessionStatus(*status);
}

void GnssAbility::SvStatusCallback(GnssSatelliteStatus* svInfo)
{
    std::unique_ptr<SatelliteStatus> svStatus = std::make_unique<SatelliteStatus>();
    if (svInfo == nullptr || svStatus == nullptr) {
        LBSLOGE(GNSS, "SvStatusCallback, sv_info is null!");
        return;
    }
    if (svInfo->satellitesNum <= 0) {
        LBSLOGD(GNSS, "SvStatusCallback, satellites_num <= 0!");
        return;
    }
    LBSLOGI(GNSS, "id  type   cn0");

    svStatus->SetSatellitesNumber(svInfo->satellitesNum);
    for (unsigned int i = 0; i < svInfo->satellitesNum; i++) {
        LBSLOGI(GNSS,
            "%{public}d    %{public}d  %{public}f",
            svInfo->satellitesList[i].satelliteId, svInfo->satellitesList[i].constellationType,
            svInfo->satellitesList[i].cn0);
        svStatus->SetAltitude(svInfo->satellitesList[i].elevation);
        svStatus->SetAzimuth(svInfo->satellitesList[i].azimuth);
        svStatus->SetCarrierFrequencie(svInfo->satellitesList[i].carrierFrequencie);
        svStatus->SetCarrierToNoiseDensity(svInfo->satellitesList[i].cn0);
        svStatus->SetSatelliteId(svInfo->satellitesList[i].satelliteId);
    }
    DelayedSingleton<GnssAbility>::GetInstance().get()->ReportSv(svStatus);
}

void GnssAbility::NmeaCallback(int64_t timestamp, const char* nmea, int length)
{
    if (nmea == nullptr) {
        LBSLOGE(GNSS, "StatusCallback: param is nullptr");
        return;
    }
    std::string nmeaStr = nmea;
    DelayedSingleton<GnssAbility>::GetInstance().get()->ReportNmea(nmeaStr);
}

bool GnssAbility::NativeInit()
{
    LBSLOGI(GNSS, "NativeInit");
    handle = dlopen(VENDOR_GNSS_ADAPTER_SO_PATH, RTLD_LAZY);
    if (!handle) {
        LBSLOGE(GNSS, "dlopen failed : %{public}s", dlerror());
        return false;
    }
    dlerror();
    GnssVendorDevice* gnssDevice = static_cast<GnssVendorDevice*>(dlsym(handle, "GnssInterface"));
    if (gnssDevice == nullptr) {
        LBSLOGE(GNSS, "dlsym failed : %{public}s", dlerror());
        return false;
    }
    g_gpsInterface = const_cast<GnssVendorInterface*>(gnssDevice->get_gnss_interface());
    if (g_gpsInterface == nullptr) {
        LBSLOGE(GNSS, "get_gnss_interface failed.");
        return false;
    }
    int ret = g_gpsInterface->enable_gnss(&g_callbacks);
    if (ret != 0) {
        LBSLOGE(GNSS, "Error, failed to init\n");
        dlclose(handle);
        return false;
    }
    nativeInitFlag_ = true;
    LBSLOGI(GNSS, "Successfully enable_gnss!");
    return true;
}

void GnssAbility::NativeClear()
{
    if (g_gpsInterface == nullptr || !nativeInitFlag_) {
        LBSLOGD(GNSS, "Error, g_gpsInterface is null!");
        return;
    }
    g_gpsInterface->disable_gnss();
    LBSLOGD(GNSS, "disable_gnss succ.");
}

void GnssAbility::NativeStart()
{
    unsigned int sleepTime = 2 * 1000 * 1000;
    if (!nativeInitFlag_) {
        if (!NativeInit()) {
            LBSLOGE(GNSS, "init failed.");
            return;
        }
        LBSLOGD(GNSS, "init succ.");
        usleep(sleepTime); // sleep for 2 second.
    }
    if (g_gpsInterface == nullptr) {
        LBSLOGD(GNSS, "Error, g_gpsInterface is null!");
        return;
    }
    int ret = g_gpsInterface->start_gnss(1);
    if (ret != 0) {
        LBSLOGD(GNSS, "Error, failed to start!");
        return;
    }
    LBSLOGD(GNSS, "Start navigation successfully, waiting for location/nmea...");
}

void GnssAbility::NativeStop()
{
    if (g_gpsInterface == nullptr || !nativeInitFlag_) {
        LBSLOGD(GNSS, "Error, g_gpsInterface is null!");
        return;
    }
    int ret = g_gpsInterface->stop_gnss(1);
    if (ret != 0) {
        LBSLOGD(GNSS, "Error, failed to stop!");
        return;
    }
    LBSLOGD(GNSS, "Stop navigation successfully.");
}

void GnssAbility::SaDumpInfo(std::string& result)
{
    result += "Gnss Location enable status: true";
    result += "\n";
}

int32_t GnssAbility::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> vecArgs;
    std::transform(args.begin(), args.end(), std::back_inserter(vecArgs), [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    LocationDumper dumper;
    std::string result;
    dumper.GnssDump(SaDumpInfo, vecArgs, result);
    if (!SaveStringToFd(fd, result)) {
        LBSLOGE(GEO_CONVERT, "Gnss save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}
} // namespace Location
} // namespace OHOS