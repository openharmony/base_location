/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "poi_info_manager.h"

#include "location_log.h"
#include "common_utils.h"
#include "location_config_manager.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "report_manager.h"
#include "permission_manager.h"
#include "proxy_freeze_manager.h"
#include "locator_ability.h"
#include "locator_background_proxy.h"
#ifdef MOVEMENT_CLIENT_ENABLE
#include "locator_msdp_monitor_manager.h"
#endif
#ifdef LOCATION_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace Location {

const char* DISCONNECT_POI_SERVICE = "DisConnectPoiService";
const int DISCONNECT_POI_SERVICE_DELAY_TIME = 10 * 1000; // 10s
const uint32_t EVENT_REQUEST_POIINFO = 0x0100;
const uint32_t EVENT_RESET_SERVICE_PROXY = 0x0200;
static constexpr int STILL_POI_EXPIRED_TIME = 30 * 60 * 1000; // Unit ms
static constexpr int POI_EXPIRED_TIME = 40 * 1000; // Unit ms
static constexpr int MAX_UTC_TIME_SIZE = 16;
static constexpr int MAX_POI_ARRAY_SIZE = 20;
static constexpr int REQUEST_POI_INFO = 5;
const int TIMEOUT_WATCHDOG = 60; // s

class AbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {
        if (resultCode != ERR_OK) {
            return;
        }
        PoiInfoManager::GetInstance()->NotifyConnected(remoteObject);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int) override
    {
        PoiInfoManager::GetInstance()->NotifyDisConnected();
    }
};

PoiInfoManager* PoiInfoManager::GetInstance()
{
    static PoiInfoManager data;
    return &data;
}

PoiInfoManager::PoiInfoManager()
{
#ifndef TDD_CASES_ENABLED
    poiInfoHandler_ =
        std::make_shared<PoiInfoHandler>(AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
#endif
}

PoiInfoManager::~PoiInfoManager() {}

void PoiInfoManager::PreRequestPoiInfo(const sptr<IRemoteObject>& cb, AppIdentity identity)
{
    std::unique_ptr<PoiInfoRequest> request = std::make_unique<PoiInfoRequest>();
    request->callback = cb;
    request->identity = identity;
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::
        Get(EVENT_REQUEST_POIINFO, request);
    if (poiInfoHandler_ != nullptr) {
        poiInfoHandler_->SendEvent(event);
    }
}

void PoiInfoManager::RequestPoiInfo(sptr<IRemoteObject>& cb, AppIdentity identity)
{
    if (!IsConnect()) {
        std::string serviceName;
        bool result = LocationConfigManager::GetInstance()->GetNlpServiceName(serviceName);
        if (!result || serviceName.empty()) {
            LBSLOGE(POI, "get service name failed!");
            return;
        }
        if (!CommonUtils::CheckAppInstalled(serviceName)) { // app is not installed
            LBSLOGE(POI, "poi service is not available.");
            return;
        } else if (!ConnectPoiService()) {
            LBSLOGE(POI, "poi service is not ready.");
            return;
        }
    }
    std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
    if (poiServiceProxy_ == nullptr) {
        LBSLOGE(POI, "poiProxy is nullptr.");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    sptr<PoiInfoCallback> callback = new PoiInfoCallback();
    callback->cb_ = cb;
    callback->identity_ = identity;
    data.WriteInterfaceToken(poiServiceProxy_->GetInterfaceDescriptor());
    data.WriteString16(Str8ToStr16(CommonUtils::GenerateUuid())); // transid
    data.WriteString16(Str8ToStr16(identity.GetBundleName()));
    data.WriteRemoteObject(callback->AsObject());
    int error = poiServiceProxy_->SendRequest(REQUEST_POI_INFO, data, reply, option);
    LBSLOGI(POI, "%{public}s SendRequest to poi service. errorCode = %{public}d",
        identity.GetBundleName().c_str(), error);
    PreDisconnectAbilityConnect(); // delay disconnect poi service
}

bool PoiInfoManager::ConnectPoiService()
{
    if (!IsConnect()) {
        AAFwk::Want connectionWant;
        std::string serviceName;
        int32_t ret = ERR_OK;
        bool result = LocationConfigManager::GetInstance()->GetNlpServiceName(serviceName);
        if (!result || serviceName.empty()) {
            LBSLOGE(POI, "get service name failed!");
            return false;
        }
        std::string abilityName = "PoiServiceExtensionAbility";
        connectionWant.SetElementName(serviceName, abilityName);
        {
            std::unique_lock<ffrt::mutex> lock(connMutex_);
            conn_ = sptr<AAFwk::IAbilityConnection>(new AbilityConnection());
            if (conn_ == nullptr) {
                LBSLOGE(POI, "get connection failed!");
                return false;
            }
            ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(connectionWant, conn_, -1);
        }
        if (ret != ERR_OK) {
            LBSLOGE(POI, "connect poi service failed, ret = %{public}d", ret);
            return false;
        }
        std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
        auto waitStatus = connectCondition_.wait_for(
            uniqueLock, std::chrono::seconds(CONNECT_TIME_OUT), [this]() { return poiServiceProxy_ != nullptr; });
        if (!waitStatus) {
            LBSLOGE(POI, "connect poi service timeout");
            return false;
        }
    }
    RegisterPoiServiceDeathRecipient();
    return true;
}

void PoiInfoManager::PreDisconnectAbilityConnect()
{
    if (poiInfoHandler_ == nullptr) {
        LBSLOGE(POI, "poiInfoHandler_ is nullptr");
        return;
    }
    auto task = [this]() {
        //time out, no new request, disconnect poi service
        PoiInfoManager::GetInstance()->DisconnectAbilityConnect();
    };
    poiInfoHandler_->RemoveTask(DISCONNECT_POI_SERVICE);
    poiInfoHandler_->PostTask(task, DISCONNECT_POI_SERVICE, DISCONNECT_POI_SERVICE_DELAY_TIME);
}

void PoiInfoManager::DisconnectAbilityConnect()
{
    {
        std::unique_lock<ffrt::mutex> uniqueLock(connMutex_);
        if (conn_ == nullptr) {
            return;
        }
    }
    LBSLOGI(POI, "Poi Service disconnect");
    UnregisterPoiServiceDeathRecipient();
    {
        std::unique_lock<ffrt::mutex> uniqueLock(connMutex_);
        AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(conn_);
        conn_ = nullptr;
    }
    {
        std::unique_lock<ffrt::mutex> poiServiceLock(poiServiceMutex_);
        poiServiceProxy_ = nullptr;
    }
}

bool PoiInfoManager::IsConnect()
{
    std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
    return poiServiceProxy_ != nullptr;
}

void PoiInfoManager::RegisterPoiServiceDeathRecipient()
{
    std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
    if (poiServiceProxy_ == nullptr) {
        LBSLOGE(POI, "%{public}s poiServiceProxy_ is nullptr", __func__);
        return;
    }
    if (poiServiceRecipient_ == nullptr) {
        poiServiceRecipient_ = sptr<PoiServiceDeathRecipient>(new PoiServiceDeathRecipient());
    }
    poiServiceProxy_->AddDeathRecipient(poiServiceRecipient_);
    LBSLOGI(POI, "%{public}s success", __func__);
}

void PoiInfoManager::UnregisterPoiServiceDeathRecipient()
{
    std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
    LBSLOGI(POI, "UnregisterPoiServiceDeathRecipient enter");
    if (poiServiceProxy_ == nullptr) {
        LBSLOGE(POI, "%{public}s poiServiceProxy_ is nullptr", __func__);
        return;
    }
    if (poiServiceRecipient_ != nullptr) {
        poiServiceProxy_->RemoveDeathRecipient(poiServiceRecipient_);
        poiServiceRecipient_ = nullptr;
    }
}

void PoiInfoManager::NotifyConnected(const sptr<IRemoteObject>& remoteObject)
{
    std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
    poiServiceProxy_ = remoteObject;
    connectCondition_.notify_all();
}

void PoiInfoManager::NotifyDisConnected()
{
    std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
    connectCondition_.notify_all();
}

bool PoiInfoManager::PreResetServiceProxy()
{
    if (poiInfoHandler_ == nullptr) {
        LBSLOGE(POI, "poiInfoHandler_ is nullptr");
        return false;
    }
    poiInfoHandler_->SendHighPriorityEvent(EVENT_RESET_SERVICE_PROXY, 0, 0);
    return true;
}

void PoiInfoManager::ResetServiceProxy()
{
    LBSLOGI(POI, "ResetServiceProxy");
    std::unique_lock<ffrt::mutex> uniqueLock(poiServiceMutex_);
    poiServiceProxy_ = nullptr;
}

PoiServiceDeathRecipient::PoiServiceDeathRecipient()
{}

PoiServiceDeathRecipient::~PoiServiceDeathRecipient()
{}

void PoiServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    PoiInfoManager::GetInstance()->PreResetServiceProxy();
}

PoiInfoHandler::PoiInfoHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitPoiInfoEventProcessMap();
}

PoiInfoHandler::~PoiInfoHandler()
{}

void PoiInfoHandler::InitPoiInfoEventProcessMap()
{
    if (poiEventProcessMap_.size() != 0) {
        return;
    }
    poiEventProcessMap_[static_cast<uint32_t>(EVENT_REQUEST_POIINFO)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleRequestPoiInfo(event); };
    poiEventProcessMap_[static_cast<uint32_t>(EVENT_RESET_SERVICE_PROXY)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleResetServiceProxy(event); };
}

void PoiInfoHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    auto handleFunc = poiEventProcessMap_.find(eventId);
    if (handleFunc != poiEventProcessMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
#ifdef LOCATION_HICOLLIE_ENABLE
        int tid = gettid();
        std::string moduleName = "PoiInfoHandler";
        XCollieCallback callbackFunc = [moduleName, eventId, tid](void *) {
            LBSLOGE(POI, "TimeoutCallback tid:%{public}d moduleName:%{public}s excute eventId:%{public}u timeout.",
                tid, moduleName.c_str(), eventId);
        };
        std::string dfxInfo = moduleName + "_" + std::to_string(eventId) + "_" + std::to_string(tid);
        int timerId = HiviewDFX::XCollie::GetInstance().SetTimer(dfxInfo, TIMEOUT_WATCHDOG, callbackFunc, nullptr,
            HiviewDFX::XCOLLIE_FLAG_LOG|HiviewDFX::XCOLLIE_FLAG_RECOVERY);
        memberFunc(event);
        HiviewDFX::XCollie::GetInstance().CancelTimer(timerId);
#else
        memberFunc(event);
#endif
    }
}

void PoiInfoHandler::HandleRequestPoiInfo(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto request = event->GetUniqueObject<PoiInfoRequest>();
    if (request == nullptr) {
        LBSLOGE(POI, "nullptr PoiInfoRequest");
        return;
    }
    auto callback = request->callback;
    auto identity = request->identity;
    if (callback == nullptr) {
        LBSLOGE(POI, "nullptr IPoiInfoCallback");
        return;
    }
    PoiInfoManager::GetInstance()->RequestPoiInfo(callback, identity);
}

void PoiInfoHandler::HandleResetServiceProxy(const AppExecFwk::InnerEvent::Pointer& event)
{
    PoiInfoManager::GetInstance()->ResetServiceProxy();
}

PoiInfoCallback::PoiInfoCallback()
{}

PoiInfoCallback::~PoiInfoCallback()
{}

int PoiInfoCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(POI, "invalid token.");
        return -1;
    }
    AppIdentity identity = identity_;
    sptr<IRemoteObject> cb = cb_;
    if (cb == nullptr) {
        LBSLOGE(POI, "null callback, uid: %{public}s, bundleName: %{public}s",
            std::to_string(identity.GetUid()).c_str(), identity.GetBundleName().c_str());
        return -1;
    }
    if (!ReportPoiPermissionCheck(identity)) {
        MessageParcel dataParcel;
        MessageParcel replyParcel;
        MessageOption optionParcel;
        dataParcel.WriteInterfaceToken(GetDescriptor());
        dataParcel.WriteString16(Str8ToStr16(std::to_string(LocationErrCode::ERRCODE_PERMISSION_DENIED)));
        int32_t errCode = cb->SendRequest(ERROR_INFO_EVENT, dataParcel, replyParcel, optionParcel);
        return -1;
    }
    switch (code) {
        case RECEIVE_POI_INFO_EVENT: {
            int32_t errCode = cb->SendRequest(RECEIVE_POI_INFO_EVENT, data, reply, option);
            break;
        }
        case ERROR_INFO_EVENT: {
            int32_t errCode = cb->SendRequest(ERROR_INFO_EVENT, data, reply, option);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool PoiInfoCallback::ReportPoiPermissionCheck(AppIdentity identity)
{
    if (!CommonUtils::IsAppBelongCurrentAccount(identity)) {
        LBSLOGE(POI, "app not belong to currentuser, uid: %{public}s, bundleName: %{public}s",
            std::to_string(identity.GetUid()).c_str(), identity.GetBundleName().c_str());
        return false;
    }
    if (ReportManager::GetInstance()->IsAppBackground(identity.GetBundleName(), identity.GetTokenId(),
        identity.GetTokenIdEx(), identity.GetUid(), identity.GetPid()) &&
        !PermissionManager::CheckBackgroundPermission(identity.GetTokenId(), identity.GetFirstTokenId())) {
        LBSLOGE(POI, "app is in background, uid: %{public}s, bundleName: %{public}s",
            std::to_string(identity.GetUid()).c_str(), identity.GetBundleName().c_str());
        return false;
    }
    if (ProxyFreezeManager::GetInstance()->IsProxyPid(identity.GetPid())) {
        LBSLOGE(POI, "app is in freeze state, uid: %{public}s, bundleName: %{public}s",
            std::to_string(identity.GetUid()).c_str(), identity.GetBundleName().c_str());
        return false;
    }
    if (!PermissionManager::CheckLocationPermission(identity.GetTokenId(), identity.GetFirstTokenId()) ||
        !PermissionManager::CheckApproximatelyPermission(identity.GetTokenId(), identity.GetFirstTokenId())) {
        LBSLOGE(POI, "app do not have PreciseLocationPermission, uid: %{public}s, bundleName: %{public}s",
            std::to_string(identity.GetUid()).c_str(), identity.GetBundleName().c_str());
        return false;
    }
    return true;
}

void PoiInfoCallback::OnPoiInfoChange(std::shared_ptr<PoiInfo> &results)
{}

void PoiInfoCallback::OnErrorReport(const std::string errorCode)
{}

void PoiInfoManager::UpdateCachedPoiInfo(const std::unique_ptr<Location>& location)
{
    std::map<std::string, std::string> additionMap = location->GetAdditionsMap();
    if (additionMap["poiInfos"] != "") {
        std::string poiInfos = std::string("poiInfos:") + additionMap["poiInfos"];
        uint64_t poiInfoTime = GetPoiInfoTime(poiInfos);
        SetLatestPoiInfo(poiInfos);
        SetLatestPoiInfoTime(poiInfoTime);
    }
}

void PoiInfoManager::ClearPoiInfos(const std::unique_ptr<Location>& finalLocation)
{
    std::vector<std::string> additions;
    for (auto& str : finalLocation->GetAdditions()) {
        if (str.find("poiInfos") != std::string::npos) {
            continue;
        }
        additions.push_back(str);
    }
    finalLocation->SetAdditions(additions, false);
    finalLocation->SetAdditionSize(finalLocation->GetAdditions().size());
    return;
}

void PoiInfoManager::UpdateLocationPoiInfo(const std::unique_ptr<Location>& finalLocation)
{
    std::map<std::string, std::string> additionMap = finalLocation->GetAdditionsMap();
    std::string poiInfos = additionMap["poiInfos"];
    uint64_t poiInfoTime = GetPoiInfoTime(poiInfos);
    if (poiInfos != "") {
        if (IsPoiInfoValid(poiInfos, poiInfoTime)) {
            PoiInfo poiInfo = ParsePoiInfoFromStr(poiInfos);
            finalLocation->SetPoiInfo(poiInfo);
            return;
        } else {
            LBSLOGI(REPORT_MANAGER, "PoiInfos Expied");
            ClearPoiInfos(finalLocation);
        }
    }
    std::string latestPoiInfos = GetLatestPoiInfo();
    uint64_t latestPoiInfoTime = GetLatestPoiInfoTime();
    if (!IsPoiInfoValid(latestPoiInfos, latestPoiInfoTime)) {
        std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
        latestPoiInfoStruct_ = {0};
        return;
    }
    AddCachedPoiInfoToLocation(finalLocation);
}

void PoiInfoManager::AddCachedPoiInfoToLocation(const std::unique_ptr<Location>& finalLocation)
{
    LBSLOGI(REPORT_MANAGER, "Add Latest PoiInfos");
    std::string poiInfos = GetLatestPoiInfo();
    std::vector<std::string> addition = finalLocation->GetAdditions();
    addition.push_back(poiInfos);
    finalLocation->SetAdditions(addition, false);
    finalLocation->SetAdditionSize(finalLocation->GetAdditions().size());
    std::string poiKey = "poiInfos:";
    std::size_t pos = poiInfos.find(poiKey);
    if (pos != std::string::npos) {
        std::string result = poiInfos.substr(pos + poiKey.size());
        PoiInfo poiInfo = ParsePoiInfoFromStr(result);
        finalLocation->SetPoiInfo(poiInfo);
    } else {
        LBSLOGE(REPORT_MANAGER, "Not Found poiInfos");
    }
}

bool PoiInfoManager::IsPoiInfoValid(std::string poiInfos, uint64_t poiInfoTime)
{
    uint64_t curTimeStamp = CommonUtils::GetCurrentTimeMilSec();
    uint64_t enterStillTime = 0;
    bool isInStillState = false;
#ifdef MOVEMENT_CLIENT_ENABLE
    isInStillState = LocatorMsdpMonitorManager::GetInstance()->GetStillMovementState();
    enterStillTime = LocatorMsdpMonitorManager::GetInstance()->GetEnterStillTime();
#endif
    if (isInStillState && poiInfoTime > enterStillTime && curTimeStamp - poiInfoTime < STILL_POI_EXPIRED_TIME) {
        return true;
    }
    if (curTimeStamp - poiInfoTime < POI_EXPIRED_TIME) {
        return true;
    }
    return false;
}

std::string PoiInfoManager::GetLatestPoiInfo()
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    return latestPoiInfoStruct_.latestPoiInfos;
}

void PoiInfoManager::SetLatestPoiInfo(std::string poiInfo)
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    latestPoiInfoStruct_.latestPoiInfos = poiInfo;
}

uint64_t PoiInfoManager::GetLatestPoiInfoTime()
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    return latestPoiInfoStruct_.poiInfosTime;
}

void PoiInfoManager::SetLatestPoiInfoTime(uint64_t poiInfoTime)
{
    std::unique_lock<std::mutex> lock(latestPoiInfoMutex_);
    latestPoiInfoStruct_.poiInfosTime = poiInfoTime;
}

uint64_t PoiInfoManager::GetPoiInfoTime(const std::string& poiInfos)
{
    std::string key = "\"time\":";
    size_t pos = poiInfos.find(key);
    if (pos != std::string::npos) {
        pos += key.length();
        std::string number;
        for (; pos < poiInfos.length() && isdigit(poiInfos[pos]); ++pos) {
            number += poiInfos[pos];
        }
        if (CommonUtils::IsValidForStoull(number, MAX_UTC_TIME_SIZE)) {
            return std::stoull(number);
        }
    }
    return 0;
}

Poi PoiInfoManager::ParsePoiInfo(cJSON* poiJson)
{
    Poi poi;
    cJSON *item = cJSON_GetObjectItem(poiJson, "id");
    poi.id = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "confidence");
    poi.confidence = (item && cJSON_IsNumber(item)) ? item->valuedouble : 0.0;
    item = cJSON_GetObjectItem(poiJson, "name");
    poi.name = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "lat");
    poi.latitude = (item && cJSON_IsNumber(item)) ? item->valuedouble : 0.0;
    item = cJSON_GetObjectItem(poiJson, "lon");
    poi.longitude = (item && cJSON_IsNumber(item)) ? item->valuedouble : 0.0;
    item = cJSON_GetObjectItem(poiJson, "administrativeArea");
    poi.administrativeArea = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "subAdministrativeArea");
    poi.subAdministrativeArea = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "locality");
    poi.locality = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "subLocality");
    poi.subLocality = (item && cJSON_IsString(item)) ? item->valuestring : "";
    item = cJSON_GetObjectItem(poiJson, "address");
    poi.address = (item && cJSON_IsString(item)) ? item->valuestring : "";
    return poi;
}

PoiInfo PoiInfoManager::ParsePoiInfoFromStr(const std::string& jsonString)
{
    PoiInfo poiInfo;
    cJSON* cJsonObj = cJSON_Parse(jsonString.c_str());
    if (!cJsonObj) {
        LBSLOGI(REPORT_MANAGER, "Poi cJson Parse Failed");
        return poiInfo;
    }
    cJSON* item = cJSON_GetObjectItem(cJsonObj, "time");
    poiInfo.timestamp = (item && cJSON_IsNumber(item)) ? static_cast<uint64_t>(item->valuedouble) : 0;
    cJSON* poisArray = cJSON_GetObjectItem(cJsonObj, "pois");
    if (poisArray && cJSON_IsArray(poisArray)) {
        int arraySize = cJSON_GetArraySize(poisArray);
        if (arraySize > MAX_POI_ARRAY_SIZE) {
            arraySize = MAX_POI_ARRAY_SIZE;
        }
        for (int i = 0; i < arraySize; ++i) {
            cJSON* poiJson = cJSON_GetArrayItem(poisArray, i);
            if (poiJson) {
                poiInfo.poiArray.push_back(ParsePoiInfo(poiJson));
            }
        }
    }
    cJSON_Delete(cJsonObj);
    return poiInfo;
}

} // namespace OHOS
} // namespace Location