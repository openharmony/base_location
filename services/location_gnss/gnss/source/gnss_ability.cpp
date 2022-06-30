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
#include <thread>
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "common_utils.h"
#include "location_log.h"
#include "location_dumper.h"
#include "agnss_event_callback.h"
#include "gnss_event_callback.h"
#include "gnss_status_callback_proxy.h"
#include "nmea_message_callback_proxy.h"

namespace OHOS {
namespace Location {
namespace {
constexpr int32_t GET_HDI_SERVICE_COUNT = 30;
constexpr uint32_t WAIT_MS = 200;
constexpr int AGNSS_SERVER_PORT = 7275;
const std::string AGNSS_SERVER_ADDR = "supl.platform.hicloud.com";
}

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<GnssAbility>::GetInstance().get());

GnssAbility::GnssAbility() : SystemAbility(LOCATION_GNSS_SA_ID, true)
{
    gnssStatusCallback_ = std::make_unique<std::map<pid_t, sptr<IGnssStatusCallback>>>();
    nmeaCallback_ = std::make_unique<std::map<pid_t, sptr<INmeaMessageCallback>>>();
    gnssInterface_ = nullptr;
    gnssCallback_ = nullptr;
    agnssCallback_ = nullptr;
    agnssInterface_ = nullptr;
    gnssWorkingStatus_ = GNSS_STATUS_NONE;
    SetAbility(GNSS_ABILITY);
    ConnectHdi();
    EnableGnss();
    SetAgnssCallback();
    SetAgnssServer();
    LBSLOGI(GNSS, "ability constructed.");
}

GnssAbility::~GnssAbility()
{
    if (gnssCallback_) {
        delete gnssCallback_;
    }
    if (agnssCallback_) {
        delete agnssCallback_;
    }
    DisableGnss();
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
        StartGnss();
    } else {
        StopGnss();
    }
    std::string state = isAdded ? "start" : "stop";
    WriteGnssStateEvent(state);
}

int GnssAbility::GetCachedGnssLocationsSize()
{
    int size = -1;
    return size;
}

int GnssAbility::FlushCachedGnssLocations()
{
    LBSLOGE(GNSS, "CachedGnssLocations fuction not support");
    return REPLY_CODE_UNSUPPORT;
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

bool GnssAbility::EnableGnss()
{
    if (gnssInterface_ == nullptr || gnssCallback_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ or gnssCallback_ is nullptr");
        return false;
    }
    int32_t ret = gnssInterface_->EnableGnss(gnssCallback_);
    LBSLOGI(GNSS, "Successfully enable_gnss!, %{public}d", ret);
    gnssWorkingStatus_ = (ret == 0) ? GNSS_STATUS_ENGINE_ON : GNSS_STATUS_NONE;
    unsigned int sleepTime = 2 * 1000 * 1000;
    usleep(sleepTime); /* sleep for 2 second. */
    return true;
}

void GnssAbility::DisableGnss()
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been disabled");
        return;
    }
    int ret = gnssInterface_->DisableGnss();
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_STATUS_ENGINE_OFF;
    }
}

bool GnssAbility::IsGnssEnabled()
{
    return (gnssWorkingStatus_ != GNSS_STATUS_ENGINE_OFF &&
        gnssWorkingStatus_ != GNSS_STATUS_NONE);
}

void GnssAbility::StartGnss()
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been disabled");
        return;
    }
    if (gnssWorkingStatus_ == GNSS_STATUS_SESSION_BEGIN) {
        LBSLOGD(GNSS, "GNSS navigation started");
        return;
    }
    if (GetRequestNum() == 0) {
        return;
    }
    int ret = gnssInterface_->StartGnss(GNSS_START_TYPE_NORMAL);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_STATUS_SESSION_BEGIN;
    }
}

void GnssAbility::StopGnss()
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been disabled");
        return;
    }
    if (GetRequestNum() != 0) {
        return;
    }
    int ret = gnssInterface_->StopGnss(GNSS_START_TYPE_NORMAL);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_STATUS_SESSION_END;
    }
}

bool GnssAbility::ConnectHdi()
{
    int32_t retry = 0;
    while (retry < GET_HDI_SERVICE_COUNT) {
        gnssInterface_ = IGnssInterface::Get();
        agnssInterface_ = IAGnssInterface::Get();
        if (gnssInterface_ != nullptr && agnssInterface_ != nullptr) {
            LBSLOGI(GNSS, "connect v1_0 hdi success.");
            gnssCallback_ = new (std::nothrow) GnssEventCallback();
            agnssCallback_ = new (std::nothrow) AGnssEventCallback();
            return true;
        }
        retry++;
        LBSLOGE(GNSS, "connect hdi service failed, retry : %{public}d", retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }
    LBSLOGE(GNSS, "connect v1_0 hdi failed.");
    return false;
}

void GnssAbility::SetAgnssServer()
{
    if (agnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been disabled");
        return;
    }
    AGnssServerInfo info;
    info.type = AGNSS_TYPE_SUPL;
    info.server = AGNSS_SERVER_ADDR;
    info.port = AGNSS_SERVER_PORT;
    agnssInterface_->SetAgnssServer(info);
}

void GnssAbility::SetAgnssCallback()
{
    LBSLOGI(GNSS, "enter SetAgnssCallback");
    if (agnssInterface_ == nullptr || agnssCallback_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface_ or agnssCallback_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been disabled");
        return;
    }
    agnssInterface_->SetAgnssCallback(agnssCallback_);
}

void GnssAbility::SetSetId(const SubscriberSetId& id)
{
    if (agnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been disabled");
        return;
    }
    agnssInterface_->SetSubscriberSetId(id);
}

void GnssAbility::SetRefInfo(const AGnssRefInfo& refInfo)
{
    if (agnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been disabled");
        return;
    }
    agnssInterface_->SetAgnssRefInfo(refInfo);
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
