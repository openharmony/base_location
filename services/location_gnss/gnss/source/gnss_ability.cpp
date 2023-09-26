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

#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability.h"

#include <file_ex.h>
#include <thread>

#include "event_runner.h"
#include "idevmgr_hdi.h"
#include "ipc_skeleton.h"
#include "iproxy_broker.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
#include "agnss_event_callback.h"
#endif
#include "common_hisysevent.h"
#include "common_utils.h"
#include "gnss_event_callback.h"
#include "i_cached_locations_callback.h"
#include "location_dumper.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locationhub_ipc_interface_code.h"
#include "location_log_event_ids.h"

namespace OHOS {
namespace Location {
namespace {
constexpr int32_t GET_HDI_SERVICE_COUNT = 30;
constexpr uint32_t WAIT_MS = 200;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
constexpr int AGNSS_SERVER_PORT = 7275;
const std::string AGNSS_SERVER_ADDR = "supl.platform.hicloud.com";
#endif
const uint32_t EVENT_REPORT_LOCATION = 0x0100;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
constexpr const char *AGNSS_SERVICE_NAME = "agnss_interface_service";
#endif
constexpr const char *GNSS_SERVICE_NAME = "gnss_interface_service";
const std::string UNLOAD_GNSS_TASK = "gnss_sa_unload";
const uint32_t RETRY_INTERVAL_OF_UNLOAD_SA = 4 * 60 * EVENT_INTERVAL_UNITE;
}

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<GnssAbility>::GetInstance().get());

GnssAbility::GnssAbility() : SystemAbility(LOCATION_GNSS_SA_ID, true)
{
    gnssStatusCallback_ = std::make_unique<std::map<pid_t, sptr<IGnssStatusCallback>>>();
    nmeaCallback_ = std::make_unique<std::map<pid_t, sptr<INmeaMessageCallback>>>();
    gnssInterface_ = nullptr;
    gnssCallback_ = nullptr;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    agnssCallback_ = nullptr;
    agnssInterface_ = nullptr;
#endif
    gnssWorkingStatus_ = GNSS_STATUS_NONE;
    SetAbility(GNSS_ABILITY);
    gnssHandler_ = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    isHdiConnected_ = false;
    LBSLOGI(GNSS, "ability constructed.");
}

GnssAbility::~GnssAbility()
{
    gnssCallback_ = nullptr;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    agnssCallback_ = nullptr;
#endif
    if (isHdiConnected_) {
        DisableGnss();
        RemoveHdi();
        isHdiConnected_ = false;
    }
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

LocationErrCode GnssAbility::SendLocationRequest(WorkRecord &workrecord)
{
    LocationRequest(workrecord);
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::SetEnable(bool state)
{
    LBSLOGI(GNSS, "SetEnable: %{public}d", state);
    state ? StartGnss() : StopGnss();
    return ERRCODE_SUCCESS;
}

void GnssAbility::UnloadGnssSystemAbility()
{
    if (gnssHandler_ == nullptr) {
        LBSLOGE(GNSS, "%{public}s gnssHandler is nullptr", __func__);
        return;
    }
    gnssHandler_->RemoveTask(UNLOAD_GNSS_TASK);
    if (CheckIfGnssConnecting()) {
        return;
    }
    auto task = [this]() {
        auto instance = DelayedSingleton<LocationSaLoadManager>::GetInstance();
        if (instance == nullptr) {
            LBSLOGE(GNSS, "%{public}s instance is nullptr", __func__);
            return;
        }
        instance->UnloadLocationSa(LOCATION_GNSS_SA_ID);
    };
    if (gnssHandler_ != nullptr) {
        gnssHandler_->PostTask(task, UNLOAD_GNSS_TASK, RETRY_INTERVAL_OF_UNLOAD_SA);
    }
}

bool GnssAbility::CheckIfGnssConnecting()
{
    std::unique_lock<std::mutex> gnssLock(gnssMutex_);
    std::unique_lock<std::mutex> nmeaLock(nmeaMutex_);
    return IsMockEnabled() || !gnssStatusCallback_->empty()
        || !nmeaCallback_->empty() || isHdiConnected_ || IsMockProcessing();
}

LocationErrCode GnssAbility::RefrashRequirements()
{
    HandleRefrashRequirements();
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid gnssStatus callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) GnssStatusCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    sptr<IGnssStatusCallback> gnssStatusCallback = iface_cast<IGnssStatusCallback>(callback);
    if (gnssStatusCallback == nullptr) {
        LBSLOGE(GNSS, "cast switch callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<std::mutex> lock(gnssMutex_);
    gnssStatusCallback_->erase(uid);
    gnssStatusCallback_->insert(std::make_pair(uid, gnssStatusCallback));
    LBSLOGD(GNSS, "after uid:%{public}d register, gnssStatusCallback size:%{public}s",
        uid, std::to_string(gnssStatusCallback_->size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "unregister an invalid gnssStatus callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IGnssStatusCallback> gnssStatusCallback = iface_cast<IGnssStatusCallback>(callback);
    if (gnssStatusCallback == nullptr) {
        LBSLOGE(GNSS, "cast gnssStatus callback fail!");
        return ERRCODE_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(gnssMutex_);
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
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid nmea callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) NmeaCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    sptr<INmeaMessageCallback> nmeaCallback = iface_cast<INmeaMessageCallback>(callback);
    if (nmeaCallback == nullptr) {
        LBSLOGE(GNSS, "cast nmea callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    std::unique_lock<std::mutex> lock(nmeaMutex_);
    nmeaCallback_->erase(uid);
    nmeaCallback_->insert(std::make_pair(uid, nmeaCallback));
    LBSLOGD(GNSS, "after uid:%{public}d register, nmeaCallback size:%{public}s",
        uid, std::to_string(nmeaCallback_->size()).c_str());
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "unregister an invalid nmea callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<INmeaMessageCallback> nmeaCallback = iface_cast<INmeaMessageCallback>(callback);
    if (nmeaCallback == nullptr) {
        LBSLOGE(GNSS, "cast nmea callback fail!");
        return ERRCODE_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(nmeaMutex_);
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
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::RegisterCachedCallback(const std::unique_ptr<CachedGnssLocationsRequest>& request,
    const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid cached location callback");
        return ERRCODE_INVALID_PARAM;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) CachedLocationCallbackDeathRecipient());
    callback->AddDeathRecipient(death);
    sptr<ICachedLocationsCallback> cachedCallback = iface_cast<ICachedLocationsCallback>(callback);
    if (cachedCallback == nullptr) {
        LBSLOGE(GNSS, "cast cached location callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    LBSLOGD(GNSS, "request:%{public}d %{public}d",
        request->reportingPeriodSec, request->wakeUpCacheQueueFull ? 1 : 0);
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode GnssAbility::UnregisterCachedCallback(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(GNSS, "register an invalid cached location callback");
        return ERRCODE_INVALID_PARAM;
    }

    sptr<ICachedLocationsCallback> cachedCallback = iface_cast<ICachedLocationsCallback>(callback);
    if (cachedCallback == nullptr) {
        LBSLOGE(GNSS, "cast cached location callback fail!");
        return ERRCODE_INVALID_PARAM;
    }
    return ERRCODE_NOT_SUPPORTED;
}

void GnssAbility::RequestRecord(WorkRecord &workRecord, bool isAdded)
{
    LBSLOGI(GNSS, "enter RequestRecord");
    if (isAdded) {
        if (!isHdiConnected_) {
            ConnectHdi();
            EnableGnss();
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
            SetAgnssCallback();
            SetAgnssServer();
#endif
            isHdiConnected_ = true;
        }
        StartGnss();
    } else {
        // GNSS will stop only if all requests have stopped
        if (isHdiConnected_ && GetRequestNum() == 0) {
            StopGnss();
            RemoveHdi();
            isHdiConnected_ = false;
        }
    }
    std::string state = isAdded ? "start" : "stop";
    WriteGnssStateEvent(state, workRecord.GetPid(0), workRecord.GetUid(0));
}

void GnssAbility::ReConnectHdi()
{
    LBSLOGI(GNSS, "%{public}s called", __func__);
    if (!isHdiConnected_) {
        LBSLOGI(GNSS, "%{public}s: HDI should be disconnected", __func__);
        return;
    }
    ConnectHdi();
    EnableGnss();
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    SetAgnssCallback();
    SetAgnssServer();
#endif
    StartGnss();
}

LocationErrCode GnssAbility::GetCachedGnssLocationsSize(int& size)
{
    size = -1;
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode GnssAbility::FlushCachedGnssLocations()
{
    LBSLOGE(GNSS, "%{public}s not support", __func__);
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode GnssAbility::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::AddFence(std::unique_ptr<GeofenceRequest>& request)
{
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode GnssAbility::RemoveFence(std::unique_ptr<GeofenceRequest>& request)
{
    return ERRCODE_NOT_SUPPORTED;
}

void GnssAbility::ReportGnssSessionStatus(int status)
{
}

void GnssAbility::ReportNmea(int64_t timestamp, const std::string &nmea)
{
    std::unique_lock<std::mutex> lock(nmeaMutex_);
    for (auto iter = nmeaCallback_->begin(); iter != nmeaCallback_->end(); iter++) {
        sptr<INmeaMessageCallback> nmeaCallback = (iter->second);
        nmeaCallback->OnMessageChange(timestamp, nmea);
    }
}

void GnssAbility::ReportSv(const std::unique_ptr<SatelliteStatus> &sv)
{
    std::unique_lock<std::mutex> lock(gnssMutex_);
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
    if (CommonUtils::QuerySwitchState() == DISABLED) {
        LBSLOGE(GNSS, "QuerySwitchState is DISABLED");
        return;
    }
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
    WriteLocationInnerEvent(START_GNSS, {});
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
    WriteLocationInnerEvent(STOP_GNSS, {});
    int ret = gnssInterface_->StopGnss(GNSS_START_TYPE_NORMAL);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_STATUS_SESSION_END;
    }
}

bool GnssAbility::ConnectHdi()
{
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    if (devmgr->LoadDevice(GNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Load gnss service failed!");
        return false;
    }
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    if (devmgr->LoadDevice(AGNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Load agnss service failed!");
        return false;
    }
#endif
    int32_t retry = 0;
    while (retry < GET_HDI_SERVICE_COUNT) {
        std::unique_lock<std::mutex> lock(hdiMutex_, std::defer_lock);
        lock.lock();
        gnssInterface_ = IGnssInterface::Get();
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
        agnssInterface_ = IAGnssInterface::Get();
        if (gnssInterface_ != nullptr && agnssInterface_ != nullptr) {
            agnssCallback_ = new (std::nothrow) AGnssEventCallback();
#else
        if (gnssInterface_ != nullptr) {
#endif
            LBSLOGI(GNSS, "connect v1_0 hdi success.");
            gnssCallback_ = new (std::nothrow) GnssEventCallback();
            RegisterLocationHdiDeathRecipient();
            lock.unlock();
            return true;
        }
        lock.unlock();
        retry++;
        LBSLOGE(GNSS, "connect hdi service failed, retry : %{public}d", retry);
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
    }
    LBSLOGE(GNSS, "connect v1_0 hdi failed.");
    return false;
}

bool GnssAbility::RemoveHdi()
{
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    if (devmgr->UnloadDevice(GNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Load gnss service failed!");
        return false;
    }
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    if (devmgr->UnloadDevice(AGNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Load agnss service failed!");
        return false;
    }
#endif
    return true;
}

#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
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
#endif

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
        LBSLOGE(GNSS, "Gnss save string to fd failed!");
        return ERR_OK;
    }
    return ERR_OK;
}

LocationErrCode GnssAbility::EnableMock()
{
    if (!EnableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::DisableMock()
{
    if (!DisableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::SetMocked(const int timeInterval,
    const std::vector<std::shared_ptr<Location>> &location)
{
    if (!SetMockedLocations(timeInterval, location)) {
        return ERRCODE_NOT_SUPPORTED;
    }
    return ERRCODE_SUCCESS;
}

bool GnssAbility::IsMockEnabled()
{
    return IsLocationMocked();
}

bool GnssAbility::IsMockProcessing()
{
    auto loc = GetLocationMock();
    return !loc.empty();
}

void GnssAbility::ProcessReportLocationMock()
{
    std::vector<std::shared_ptr<Location>> mockLocationArray = GetLocationMock();
    if (mockLocationIndex_ < mockLocationArray.size()) {
        ReportMockedLocation(mockLocationArray[mockLocationIndex_++]);
        if (gnssHandler_ != nullptr) {
            gnssHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION,
                0, GetTimeIntervalMock() * EVENT_INTERVAL_UNITE);
        }
    } else {
        ClearLocationMock();
        mockLocationIndex_ = 0;
    }
}

void GnssAbility::SendReportMockLocationEvent()
{
    if (gnssHandler_ != nullptr) {
        gnssHandler_->SendHighPriorityEvent(EVENT_REPORT_LOCATION, 0, 0);
    }
}

int32_t GnssAbility::ReportMockedLocation(const std::shared_ptr<Location> location)
{
    if ((IsLocationMocked() && !location->GetIsFromMock()) ||
        (!IsLocationMocked() && location->GetIsFromMock())) {
        LBSLOGE(GNSS, "location mock is enabled, do not report gnss location!");
        return ERR_OK;
    }
    ReportLocationInfo(GNSS_ABILITY, location);
#ifdef FEATURE_PASSIVE_SUPPORT
    ReportLocationInfo(PASSIVE_ABILITY, location);
#endif
    return ERR_OK;
}

void GnssAbility::SendMessage(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    if (gnssHandler_ == nullptr) {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return;
    }
    switch (code) {
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST): {
            std::unique_ptr<WorkRecord> workrecord = WorkRecord::Unmarshalling(data);
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
                Get(code, workrecord);
            if (gnssHandler_->SendEvent(event)) {
                reply.WriteInt32(ERRCODE_SUCCESS);
            } else {
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            }
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS): {
            if (!IsMockEnabled()) {
                reply.WriteInt32(ERRCODE_NOT_SUPPORTED);
                break;
            }
            int timeInterval = data.ReadInt32();
            int locationSize = data.ReadInt32();
            timeInterval = timeInterval < 0 ? 1 : timeInterval;
            locationSize = locationSize > INPUT_ARRAY_LEN_MAX ? INPUT_ARRAY_LEN_MAX :
                locationSize;
            std::shared_ptr<std::vector<std::shared_ptr<Location>>> vcLoc =
                std::make_shared<std::vector<std::shared_ptr<Location>>>();
            for (int i = 0; i < locationSize; i++) {
                vcLoc->push_back(Location::UnmarshallingShared(data));
            }
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, vcLoc, timeInterval);
            if (gnssHandler_->SendEvent(event)) {
                reply.WriteInt32(ERRCODE_SUCCESS);
            } else {
                reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
            }
            break;
        }
        default:
            break;
    }
}

void GnssAbility::RegisterLocationHdiDeathRecipient()
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "%{public}s: gnssInterface_ is nullptr", __func__);
        return;
    }
    sptr<IRemoteObject> obj = OHOS::HDI::hdi_objcast<IGnssInterface>(gnssInterface_);
    if (obj == nullptr) {
        LBSLOGE(GNSS, "%{public}s: hdi obj is nullptr", __func__);
        return;
    }
    sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) LocationHdiDeathRecipient());
    obj->AddDeathRecipient(death);
}

GnssHandler::GnssHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

GnssHandler::~GnssHandler() {}

void GnssHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGI(GNSS, "ProcessEvent event:%{public}d", eventId);
    switch (eventId) {
        case EVENT_REPORT_LOCATION: {
            gnssAbility->ProcessReportLocationMock();
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST): {
            std::unique_ptr<WorkRecord> workrecord = event->GetUniqueObject<WorkRecord>();
            if (workrecord != nullptr) {
                gnssAbility->LocationRequest(*workrecord);
            }
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS): {
            int timeInterval = event->GetParam();
            auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
            if (vcLoc != nullptr) {
                std::vector<std::shared_ptr<Location>> mockLocations;
                for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
                    mockLocations.push_back(*it);
                }
                gnssAbility->SetMocked(timeInterval, mockLocations);
            }
            break;
        }
        default:
            break;
    }
    gnssAbility->UnloadGnssSystemAbility();
}

LocationHdiDeathRecipient::LocationHdiDeathRecipient()
{
}

LocationHdiDeathRecipient::~LocationHdiDeathRecipient()
{
}

void LocationHdiDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility != nullptr) {
        LBSLOGI(LOCATOR, "hdi reconnecting");
        // wait for device unloaded
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
        gnssAbility->ReConnectHdi();
        LBSLOGI(LOCATOR, "hdi connected finish");
    }
}
} // namespace Location
} // namespace OHOS
#endif
