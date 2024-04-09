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
#include "location_config_manager.h"
#include "location_dumper.h"
#include "location_log.h"
#include "location_sa_load_manager.h"
#include "locationhub_ipc_interface_code.h"
#include "location_log_event_ids.h"

#include "hook_utils.h"

namespace OHOS {
namespace Location {
namespace {
constexpr uint32_t WAIT_MS = 200;
const uint32_t EVENT_REPORT_MOCK_LOCATION = 0x0100;
const uint32_t RECONNECT_HDI = 0x0103;
const uint32_t INIT_HDI = 0x0104;
const uint32_t EVENT_INTERVAL_UNITE = 1000;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
const uint32_t SET_SUBSCRIBER_SET_ID = 0x0101;
const uint32_t SET_AGNSS_REF_INFO = 0x0102;
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
    gnssInterface_ = nullptr;
    gnssCallback_ = nullptr;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    agnssCallback_ = nullptr;
    agnssInterface_ = nullptr;
#endif
    gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
    SetAbility(GNSS_ABILITY);
    gnssHandler_ = std::make_shared<GnssHandler>(AppExecFwk::EventRunner::Create(true));
    if (gnssHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(INIT_HDI, 0);
        gnssHandler_->SendEvent(event);
    }
    LBSLOGI(GNSS, "ability constructed.");
}

GnssAbility::~GnssAbility()
{
}

bool GnssAbility::CheckIfHdiConnected()
{
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    return gnssInterface_ != nullptr && agnssInterface_ != nullptr;
#else
    return gnssInterface_ != nullptr;
#endif
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
    if (state) {
        ConnectHdi();
    } else {
        DisConnectHdi();
    }
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
        if (CheckIfHdiConnected()) {
            RemoveHdi();
        }
        CommonUtils::UnInitLocationSa(LOCATION_GNSS_SA_ID);
    };
    if (gnssHandler_ != nullptr) {
        gnssHandler_->PostTask(task, UNLOAD_GNSS_TASK, RETRY_INTERVAL_OF_UNLOAD_SA);
    }
}

bool GnssAbility::CheckIfGnssConnecting()
{
    return IsMockEnabled() || GetRequestNum() != 0 || IsMockProcessing();
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
    gnssStatusCallback_.push_back(gnssStatusCallback);
    LBSLOGD(GNSS, "after uid:%{public}d register, gnssStatusCallback size:%{public}s",
        uid, std::to_string(gnssStatusCallback_.size()).c_str());
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
    size_t i = 0;
    for (; i < gnssStatusCallback_.size(); i++) {
        sptr<IRemoteObject> remoteObject = gnssStatusCallback_[i]->AsObject();
        if (remoteObject == callback) {
            break;
        }
    }
    if (i >= gnssStatusCallback_.size()) {
        LBSLOGD(GNSS, "gnssStatus callback is not in vector");
        return ERRCODE_SUCCESS;
    }
    if (gnssStatusCallback_.size() > 0) {
        gnssStatusCallback_.erase(gnssStatusCallback_.begin() + i);
    }
    LBSLOGD(GNSS, "after unregister, gnssStatus callback size:%{public}s",
        std::to_string(gnssStatusCallback_.size()).c_str());
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
    nmeaCallback_.push_back(nmeaCallback);
    LBSLOGD(GNSS, "after uid:%{public}d register, nmeaCallback size:%{public}s",
        uid, std::to_string(nmeaCallback_.size()).c_str());
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
    size_t i = 0;
    for (; i < nmeaCallback_.size(); i++) {
        sptr<IRemoteObject> remoteObject = nmeaCallback_[i]->AsObject();
        if (remoteObject == callback) {
            break;
        }
    }
    if (i >= nmeaCallback_.size()) {
        LBSLOGD(GNSS, "nmea callback is not in vector");
        return ERRCODE_SUCCESS;
    }
    if (nmeaCallback_.size() > 0) {
        nmeaCallback_.erase(nmeaCallback_.begin() + i);
    }
    LBSLOGD(GNSS, "after unregister, nmea callback size:%{public}s",
        std::to_string(nmeaCallback_.size()).c_str());
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
    LBSLOGD(GNSS, "enter RequestRecord");
    if (isAdded) {
        ConnectHdi();
    } else {
        // GNSS will stop only if all requests have stopped
        if (GetRequestNum() == 0) {
            DisConnectHdi();
        }
    }
}

void GnssAbility::ReConnectHdi()
{
    if (gnssHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(RECONNECT_HDI, 0);
        gnssHandler_->SendEvent(event);
    }
}

void GnssAbility::ReConnectHdiImpl()
{
    LBSLOGD(GNSS, "%{public}s called", __func__);
    ConnectHdi();
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

bool GnssAbility::GetCommandFlags(std::unique_ptr<LocationCommand>& commands, GnssAuxiliaryDataType& flags)
{
    std::string cmd = commands->command;
    if (cmd == "delete_auxiliary_data_ephemeris") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_EPHEMERIS;
    } else if (cmd == "delete_auxiliary_data_almanac") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALMANAC;
    } else if (cmd == "delete_auxiliary_data_position") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_POSITION;
    } else if (cmd == "delete_auxiliary_data_time") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_TIME;
    } else if (cmd == "delete_auxiliary_data_iono") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_IONO;
    } else if (cmd == "delete_auxiliary_data_utc") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_UTC;
    } else if (cmd == "delete_auxiliary_data_health") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_HEALTH;
    } else if (cmd == "delete_auxiliary_data_svdir") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVDIR;
    } else if (cmd == "delete_auxiliary_data_svsteer") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SVSTEER;
    } else if (cmd == "delete_auxiliary_data_sadata") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_SADATA;
    } else if (cmd == "delete_auxiliary_data_rti") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_RTI;
    } else if (cmd == "delete_auxiliary_data_celldb_info") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_CELLDB_INFO;
    } else if (cmd == "delete_auxiliary_data_all") {
        flags = GnssAuxiliaryDataType::GNSS_AUXILIARY_DATA_ALL;
    } else {
        LBSLOGE(GNSS, "unknow command %{public}s", cmd.c_str());
        return false;
    }
    return true;
}

LocationErrCode GnssAbility::SendCommand(std::unique_ptr<LocationCommand>& commands)
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    GnssAuxiliaryDataType flags;
    bool result = GetCommandFlags(commands, flags);
    LBSLOGE(GNSS, "GetCommandFlags,flags = %{public}d", flags);
    if (result) {
        gnssInterface_->DeleteAuxiliaryData(flags);
    }
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::SetPositionMode()
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    GnssConfigPara para;
    int suplMode = LocationConfigManager::GetInstance().GetSuplMode();
    if (suplMode == MODE_STANDALONE) {
        para.gnssBasic.gnssMode = GnssWorkingMode::GNSS_WORKING_MODE_STANDALONE;
    } else if (suplMode == MODE_MS_BASED) {
        para.gnssBasic.gnssMode = GnssWorkingMode::GNSS_WORKING_MODE_MS_BASED;
    } else if (suplMode == MODE_MS_ASSISTED) {
        para.gnssBasic.gnssMode = GnssWorkingMode::GNSS_WORKING_MODE_MS_ASSISTED;
    } else {
        LBSLOGE(GNSS, "unknow mode");
        return ERRCODE_SUCCESS;
    }
    int ret = gnssInterface_->SetGnssConfigPara(para);
    if (ret != ERRCODE_SUCCESS) {
        LBSLOGE(GNSS, "SetGnssConfigPara failed , ret =%{public}d", ret);
    }
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
    for (auto nmeaCallback : nmeaCallback_) {
        nmeaCallback->OnMessageChange(timestamp, nmea);
    }
}

void GnssAbility::ReportSv(const std::unique_ptr<SatelliteStatus> &sv)
{
    std::unique_lock<std::mutex> lock(gnssMutex_);
    for (auto gnssStatusCallback : gnssStatusCallback_) {
        gnssStatusCallback->OnStatusChange(sv);
    }
}

bool GnssAbility::EnableGnss()
{
    if (CommonUtils::QuerySwitchState() == DISABLED) {
        LBSLOGE(GNSS, "QuerySwitchState is DISABLED");
        return false;
    }
    if (gnssInterface_ == nullptr || gnssCallback_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ or gnssCallback_ is nullptr");
        return false;
    }
    if (IsGnssEnabled()) {
        LBSLOGE(GNSS, "gnss has been enabled");
        return false;
    }
    int32_t ret = gnssInterface_->EnableGnss(gnssCallback_);
    LBSLOGD(GNSS, "Successfully enable_gnss!, %{public}d", ret);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_ON;
    } else {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret),
            "hdiName", "EnableGnss", "hdiType", "gnss"});
    }
    return true;
}

bool GnssAbility::DisableGnss()
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return false;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return true;
    }
    int ret = gnssInterface_->DisableGnss();
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_ENGINE_OFF;
    } else {
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret),
            "hdiName", "DisableGnss", "hdiType", "gnss"});
    }
    return true;
}

bool GnssAbility::IsGnssEnabled()
{
    return (gnssWorkingStatus_ != GNSS_WORKING_STATUS_ENGINE_OFF &&
        gnssWorkingStatus_ != GNSS_WORKING_STATUS_NONE);
}

void GnssAbility::RestGnssWorkStatus()
{
    std::unique_lock<std::mutex> uniqueLock(statusMutex_);
    gnssWorkingStatus_ = GNSS_WORKING_STATUS_NONE;
}

bool GnssAbility::StartGnss()
{
    if (CommonUtils::QuerySwitchState() == DISABLED) {
        LBSLOGE(GNSS, "QuerySwitchState is DISABLED");
        return false;
    }
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return false;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return false;
    }
    if (gnssWorkingStatus_ == GNSS_WORKING_STATUS_SESSION_BEGIN) {
        LBSLOGD(GNSS, "GNSS navigation started");
        return true;
    }
    if (GetRequestNum() == 0) {
        return true;
    }
    SetPositionMode();
    int ret = gnssInterface_->StartGnss(GNSS_START_TYPE_NORMAL);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_BEGIN;
        WriteLocationInnerEvent(START_GNSS, {});
    } else {
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret), "hdiName", "StartGnss", "hdiType", "gnss"});
    }
    LocationErrCode errCode =
        HookUtils::ExecuteHook(LocationProcessStage::START_GNSS_PROCESS, nullptr, nullptr);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(GNSS, "%{public}s ExecuteHook failed err = %{public}d", __func__, (int)errCode);
    }
    return ret == 0;
}

bool GnssAbility::StopGnss()
{
    if (gnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "gnssInterface_ is nullptr");
        return false;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return true;
    }
    
    int ret = gnssInterface_->StopGnss(GNSS_START_TYPE_NORMAL);
    if (ret == 0) {
        gnssWorkingStatus_ = GNSS_WORKING_STATUS_SESSION_END;
        WriteLocationInnerEvent(STOP_GNSS, {});
    } else {
        WriteLocationInnerEvent(HDI_EVENT, {"errCode", std::to_string(ret), "hdiName", "StopGnss", "hdiType", "gnss"});
    }
    LocationErrCode errCode =
        HookUtils::ExecuteHook(LocationProcessStage::STOP_GNSS_PROCESS, nullptr, nullptr);
    if (errCode != ERRCODE_SUCCESS) {
        LBSLOGE(GNSS, "%{public}s ExecuteHook failed err = %{public}d", __func__, (int)errCode);
    }
    return ret == 0;
}

bool GnssAbility::AddHdi()
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
        LBSLOGD(GNSS, "connect v2_0 hdi success.");
        gnssCallback_ = new (std::nothrow) GnssEventCallback();
        RegisterLocationHdiDeathRecipient();
        lock.unlock();
        return true;
    }
    lock.unlock();
    LBSLOGE(GNSS, "connect v2_0 hdi failed.");
    return false;
}

bool GnssAbility::RemoveHdi()
{
    if (!CheckIfHdiConnected()) {
        LBSLOGE(GNSS, "hdi is not connected.");
        return false;
    }
    auto devmgr = HDI::DeviceManager::V1_0::IDeviceManager::Get();
    if (devmgr == nullptr) {
        LBSLOGE(GNSS, "fail to get devmgr.");
        return false;
    }
    if (devmgr->UnloadDevice(GNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Load gnss service failed!");
        return false;
    }
    gnssCallback_ = nullptr;
    gnssInterface_ = nullptr;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    if (devmgr->UnloadDevice(AGNSS_SERVICE_NAME) != 0) {
        LBSLOGE(GNSS, "Load agnss service failed!");
        return false;
    }
    agnssCallback_ = nullptr;
    agnssInterface_ = nullptr;
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
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    std::string addrName;
    bool result = LocationConfigManager::GetInstance().GetAgnssServerAddr(addrName);
    if (!result || addrName.empty()) {
        LBSLOGE(GNSS, "get agnss server address failed!");
        return;
    }
    int port = LocationConfigManager::GetInstance().GetAgnssServerPort();
    AGnssServerInfo info;
    info.type = AGNSS_TYPE_SUPL;
    info.server = addrName;
    info.port = port;
    agnssInterface_->SetAgnssServer(info);
}

void GnssAbility::SetAgnssCallback()
{
    LBSLOGD(GNSS, "enter SetAgnssCallback");
    if (agnssInterface_ == nullptr || agnssCallback_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface_ or agnssCallback_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    agnssInterface_->SetAgnssCallback(agnssCallback_);
}

void GnssAbility::SetSetId(const SubscriberSetId& id)
{
    std::unique_ptr<SubscriberSetId> subscribeSetId = std::make_unique<SubscriberSetId>();
    subscribeSetId->type = id.type;
    subscribeSetId->id = id.id;
    if (gnssHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(SET_SUBSCRIBER_SET_ID, subscribeSetId);
        gnssHandler_->SendEvent(event);
    }
}

void GnssAbility::SetSetIdImpl(const SubscriberSetId& id)
{
    if (agnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    agnssInterface_->SetSubscriberSetId(id);
}

void GnssAbility::SetRefInfo(const AGnssRefInfo& refInfo)
{
    std::unique_ptr<AgnssRefInfoMessage> agnssRefInfoMessage = std::make_unique<AgnssRefInfoMessage>();
    if (gnssHandler_ != nullptr) {
        agnssRefInfoMessage->SetAgnssRefInfo(refInfo);
        AppExecFwk::InnerEvent::Pointer event =
            AppExecFwk::InnerEvent::Get(SET_AGNSS_REF_INFO, agnssRefInfoMessage);
        gnssHandler_->SendEvent(event);
    }
}

void GnssAbility::SetRefInfoImpl(const AGnssRefInfo &refInfo)
{
    if (agnssInterface_ == nullptr) {
        LBSLOGE(GNSS, "agnssInterface_ is nullptr");
        return;
    }
    if (!IsGnssEnabled()) {
        LBSLOGE(GNSS, "%{public}s gnss has been disabled", __func__);
        return;
    }
    agnssInterface_->SetAgnssRefInfo(refInfo);
}

AGnssRefInfo AgnssRefInfoMessage::GetAgnssRefInfo()
{
    return agnssRefInfo_;
}

void AgnssRefInfoMessage::SetAgnssRefInfo(const AGnssRefInfo &refInfo)
{
    agnssRefInfo_ = refInfo;
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
    MockLocationStruct mockLocationStruct;
    mockLocationStruct.enableMock = true;
    HookUtils::ExecuteHook(LocationProcessStage::MOCK_LOCATION_PROCESS, (void *)&mockLocationStruct, nullptr);
    return ERRCODE_SUCCESS;
}

LocationErrCode GnssAbility::DisableMock()
{
    if (!DisableLocationMock()) {
        return ERRCODE_NOT_SUPPORTED;
    }
    MockLocationStruct mockLocationStruct;
    mockLocationStruct.enableMock = false;
    HookUtils::ExecuteHook(LocationProcessStage::MOCK_LOCATION_PROCESS, (void *)&mockLocationStruct, nullptr);
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
            gnssHandler_->SendHighPriorityEvent(EVENT_REPORT_MOCK_LOCATION,
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
        gnssHandler_->SendHighPriorityEvent(EVENT_REPORT_MOCK_LOCATION, 0, 0);
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
            SendEvent(event, reply);
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
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS): {
            std::unique_ptr<LocationCommand> locationCommand = std::make_unique<LocationCommand>();
            locationCommand->scenario = data.ReadInt32();
            locationCommand->command = Str16ToStr8(data.ReadString16());
            AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(code, locationCommand);
            SendEvent(event, reply);
            break;
        }
        case static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE): {
            AppExecFwk::InnerEvent::Pointer event =
                AppExecFwk::InnerEvent::Get(code, static_cast<int>(data.ReadBool()));
            SendEvent(event, reply);
            break;
        }
        default:
            break;
    }
}

void GnssAbility::SendEvent(AppExecFwk::InnerEvent::Pointer& event, MessageParcel &reply)
{
    if (gnssHandler_ == nullptr) {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
        return;
    }
    if (gnssHandler_->SendEvent(event)) {
        reply.WriteInt32(ERRCODE_SUCCESS);
    } else {
        reply.WriteInt32(ERRCODE_SERVICE_UNAVAILABLE);
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

void GnssAbility::ConnectHdi()
{
    bool ret = true;
    auto startTime = CommonUtils::GetCurrentTimeStamp();
    ret = AddHdi();
    ret = EnableGnss();
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    SetAgnssCallback();
    SetAgnssServer();
#endif
    ret = StartGnss();
    auto endTime = CommonUtils::GetCurrentTimeStamp();
    WriteLocationInnerEvent(HDI_EVENT, {"ret", std::to_string(ret), "type", "ConnectHdi",
            "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
    }
}

void GnssAbility::DisConnectHdi()
{
    bool ret = true;
    auto startTime = CommonUtils::GetCurrentTimeStamp();
    ret = StopGnss();
    ret = EnableGnss();
    ret = RemoveHdi();
    auto endTime = CommonUtils::GetCurrentTimeStamp();
    WriteLocationInnerEvent(HDI_EVENT, {"ret", std::to_string(ret), "type", "DisConnectHdi",
            "startTime", std::to_string(startTime), "endTime", std::to_string(endTime)});
    }
}

GnssHandler::GnssHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitGnssEventProcessMap();
}

void GnssHandler::InitGnssEventProcessMap()
{
    if (gnssEventProcessMap_.size() != 0) {
        return;
    }
    gnssEventProcessMap_[EVENT_REPORT_MOCK_LOCATION] = &GnssHandler::HandleReportMockLocation;
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_LOCATION_REQUEST)] =
        &GnssHandler::HandleSendLocationRequest;
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_MOCKED_LOCATIONS)] =
        &GnssHandler::HandleSetMockedLocations;
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SEND_COMMANDS)] =
        &GnssHandler::HandleSendCommands;
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
    gnssEventProcessMap_[SET_SUBSCRIBER_SET_ID] = &GnssHandler::HandleSetSubscriberSetId;
    gnssEventProcessMap_[SET_AGNSS_REF_INFO] = &GnssHandler::HandleSetAgnssRefInfo;
#endif
    gnssEventProcessMap_[RECONNECT_HDI] = &GnssHandler::HandleReconnectHdi;
    gnssEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::SET_ENABLE)] = &GnssHandler::HandleSetEnable;
    gnssEventProcessMap_[INIT_HDI] = &GnssHandler::HandleInitHdi;
}

GnssHandler::~GnssHandler() {}

void GnssHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(GNSS, "ProcessEvent event:%{public}d", eventId);
    auto handleFunc = gnssEventProcessMap_.find(eventId);
    if (handleFunc != gnssEventProcessMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
        (this->*memberFunc)(event);
    }
    gnssAbility->UnloadGnssSystemAbility();
}

void GnssHandler::HandleReportMockLocation(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    gnssAbility->ProcessReportLocationMock();
}

void GnssHandler::HandleSendLocationRequest(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    std::unique_ptr<WorkRecord> workrecord = event->GetUniqueObject<WorkRecord>();
    if (workrecord != nullptr) {
        gnssAbility->LocationRequest(*workrecord);
    }
}

void GnssHandler::HandleSetMockedLocations(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    int timeInterval = event->GetParam();
    auto vcLoc = event->GetSharedObject<std::vector<std::shared_ptr<Location>>>();
    if (vcLoc != nullptr) {
        std::vector<std::shared_ptr<Location>> mockLocations;
        for (auto it = vcLoc->begin(); it != vcLoc->end(); ++it) {
            mockLocations.push_back(*it);
        }
        gnssAbility->SetMocked(timeInterval, mockLocations);
    }
}

void GnssHandler::HandleSendCommands(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    std::unique_ptr<LocationCommand> locationCommand = event->GetUniqueObject<LocationCommand>();
    if (locationCommand != nullptr) {
        gnssAbility->SendCommand(locationCommand);
    }
}
#ifdef HDF_DRIVERS_INTERFACE_AGNSS_ENABLE
void GnssHandler::HandleSetSubscriberSetId(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    std::unique_ptr<SubscriberSetId> subscriberSetId = event->GetUniqueObject<SubscriberSetId>();
    if (subscriberSetId != nullptr) {
        gnssAbility->SetSetIdImpl(*subscriberSetId);
    }
}

void GnssHandler::HandleSetAgnssRefInfo(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    std::unique_ptr<AgnssRefInfoMessage> agnssRefInfoMessage = event->GetUniqueObject<AgnssRefInfoMessage>();
    if (agnssRefInfoMessage != nullptr) {
        AGnssRefInfo refInfo = agnssRefInfoMessage->GetAgnssRefInfo();
        gnssAbility->SetRefInfoImpl(refInfo);
    }
}
#endif

void GnssHandler::HandleReconnectHdi(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    gnssAbility->ReConnectHdiImpl();
}

void GnssHandler::HandleSetEnable(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    int state = event->GetParam();
    gnssAbility->SetEnable(state != 0);
}

void GnssHandler::HandleInitHdi(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto gnssAbility = DelayedSingleton<GnssAbility>::GetInstance();
    if (gnssAbility == nullptr) {
        LBSLOGE(GNSS, "ProcessEvent: gnss ability is nullptr");
        return;
    }
    gnssAbility->AddHdi();
    gnssAbility->EnableGnss();
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
        gnssAbility->RestGnssWorkStatus();
        gnssAbility->ReConnectHdi();
        LBSLOGI(LOCATOR, "hdi connected finish");
    }
}
} // namespace Location
} // namespace OHOS
#endif
