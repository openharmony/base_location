/*
* Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "fusion_fence_ability.h"
#include "location_log.h"
#include "common_utils.h"
#include "i_fusion_fence_callback.h"
#include "event_runner.h"
#ifdef FEATURE_GNSS_SUPPORT
#include "gnss_ability.h"
#endif

#ifdef LOCATION_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace Location {
const int TIMEOUT_WATCHDOG = 60; // s
const int MAX_GNSS_GEOFENCE_REQUEST_NUM = 10;
const int MAX_GNSS_GEOFENCE_REQUEST_NUM_FOR_ONE_APP = 5;

FusionFenceAbility::FusionFenceAbility()
    : totalGnssFenceCount_(0)
{
    fusionFenceEventCallback_ = new (std::nothrow) FusionFenceEventCallback();
    if (fusionFenceEventCallback_ == nullptr) {
        LBSLOGE(FUSION_FENCE, "FusionFenceAbility: new fusionFenceEventCallback_ failed");
    }
    fusionFenceHandler_ = std::make_shared<FusionFenceHandler>(
        AppExecFwk::EventRunner::Create(true, AppExecFwk::ThreadMode::FFRT));
}

FusionFenceAbility::~FusionFenceAbility()
{
    fusionFenceEventCallback_ = nullptr;
    fusionFenceHandler_.reset();
}

FusionFenceAbility* FusionFenceAbility::GetInstance()
{
    static FusionFenceAbility data;
    return &data;
}

bool FusionFenceAbility::ExecuteFusionFenceProcess(std::shared_ptr<FusionFenceRequest>& request, int code)
{
    FusionFenceStruct fusionFenceStruct;
    fusionFenceStruct.request = request;
    fusionFenceStruct.requestCode = code;
    fusionFenceStruct.retCode = true;
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    fusionFenceStruct.callback = fusionFenceEventCallback_;
    lock.unlock();
    LocationErrCode errCode = HookUtils::ExecuteHook(
        LocationProcessStage::FUSION_FENCE_REQUEST_PROCESS, (void *)&fusionFenceStruct, nullptr);
    if (errCode != ERRCODE_SUCCESS) {
        return false;
    }
    return fusionFenceStruct.retCode;
}

bool FusionFenceAbility::HasDuplicateAddFusionFenceRequest(const std::shared_ptr<FusionFenceRequest>& request)
{
    if (request == nullptr) {
        return false;
    }
    std::string identifier = request->GetIdentifier();
    std::string bundleName = request->GetBundleName();
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    for (const auto& iter : fusionFenceRequestList_) {
        if (iter->GetBundleName() == bundleName && iter->GetIdentifier() == identifier) {
            LBSLOGE(FUSION_FENCE, "FusionFence with identifier %{public}s already exists for bundle %{public}s",
                identifier.c_str(), bundleName.c_str());
            return true;
        }
    }
    return false;
}

bool FusionFenceAbility::IsFusionFenceExists(const std::string& identifier)
{
    if (identifier.empty()) {
        return false;
    }
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    for (const auto& iter : fusionFenceRequestList_) {
        if (iter->GetIdentifier() == identifier) {
            return true;
        }
    }
    return false;
}

LocationErrCode FusionFenceAbility::AddFusionFence(std::shared_ptr<FusionFenceRequest>& request)
{
    if (request == nullptr) {
        LBSLOGE(FUSION_FENCE, "AddFusionFence request is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    if (!IsFusionFenceSupported()) {
        LBSLOGI(FUSION_FENCE, "Is Not Support FenceSupported");
        return ERRCODE_NOT_SUPPORTED;
    }
    std::string identifier = request->GetIdentifier();
    std::string bundleName = request->GetBundleName();
    if (HasDuplicateAddFusionFenceRequest(request)) {
        return ERRCODE_BEACONFENCE_DUPLICATE_INFORMATION;
    }
    LocationErrCode errCode = CheckFenceLimit(request);
    if (errCode != ERRCODE_SUCCESS) {
        return errCode;
    }
    auto callback = request->GetTransitionCallback();
    if (callback != nullptr) {
        sptr<IRemoteObject::DeathRecipient> death(new (std::nothrow) FusionFenceCallbackDeathRecipient());
        callback->AddDeathRecipient(death);
    }
    if (!ExecuteFusionFenceProcess(request, static_cast<int>(GnssInterfaceCode::ADD_FUSION_FENCE))) {
        LBSLOGI(FUSION_FENCE, "AddFusionFence fail, identifier: %{public}s", identifier.c_str());
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    {
        std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
        fusionFenceRequestList_.push_back(request);
        AddFenceCount(request);
    }
    LBSLOGI(FUSION_FENCE, "AddFusionFence success, identifier: %{public}s, size: %{public}zu",
        identifier.c_str(), fusionFenceRequestList_.size());
    return ERRCODE_SUCCESS;
}

LocationErrCode FusionFenceAbility::CheckFenceLimit(std::shared_ptr<FusionFenceRequest>& request)
{
    std::string bundleName = request->GetBundleName();
    int32_t fenceType = request->GetFenceType();
    size_t gnssFenceCount = request->GetGnssFences().size();
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    if (fenceType & FUSION_FENCE_GNSS) {
        size_t totalGnssCount = totalGnssFenceCount_;
#ifdef FEATURE_GNSS_SUPPORT
        totalGnssCount += GnssAbility::GetInstance()->GetGnssGeofenceRequestMapSize();
#endif
        if (totalGnssCount + gnssFenceCount > MAX_GNSS_GEOFENCE_REQUEST_NUM) {
            LBSLOGE(FUSION_FENCE, "Exceeded the limit of the gnss fence request");
            return ERRCODE_GEOFENCE_EXCEED_MAXIMUM;
        }
        auto it = gnssFenceCountMap_.find(bundleName);
        int appGnssCount = (it != gnssFenceCountMap_.end()) ? it->second : 0;
#ifdef FEATURE_GNSS_SUPPORT
        appGnssCount += GnssAbility::GetInstance()->GetGnssGeofenceCountForOneAppOnly(bundleName);
#endif
        if (appGnssCount + gnssFenceCount > MAX_GNSS_GEOFENCE_REQUEST_NUM_FOR_ONE_APP) {
            LBSLOGE(FUSION_FENCE, "Exceeded the limit of the gnss fence request for one app");
            return ERRCODE_GEOFENCE_EXCEED_MAXIMUM;
        }
    }
    return ERRCODE_SUCCESS;
}

void FusionFenceAbility::AddFenceCount(std::shared_ptr<FusionFenceRequest>& request)
{
    std::string bundleName = request->GetBundleName();
    int32_t fenceType = request->GetFenceType();
    size_t gnssCount = request->GetGnssFences().size();
    if (fenceType & FUSION_FENCE_GNSS) {
        gnssFenceCountMap_[bundleName] += gnssCount;
        totalGnssFenceCount_ += gnssCount;
    }
}

void FusionFenceAbility::RemoveFenceCount(std::shared_ptr<FusionFenceRequest>& request)
{
    std::string bundleName = request->GetBundleName();
    int32_t fenceType = request->GetFenceType();
    size_t gnssCount = request->GetGnssFences().size();
    if (fenceType & FUSION_FENCE_GNSS) {
        auto it = gnssFenceCountMap_.find(bundleName);
        if (it != gnssFenceCountMap_.end()) {
            if (it->second >= gnssCount) {
                it->second -= gnssCount;
            } else {
                it->second = 0;
            }
        }
        totalGnssFenceCount_ -= gnssCount;
    }
}

LocationErrCode FusionFenceAbility::RemoveFusionFence(std::shared_ptr<FusionFenceRequest>& request)
{
    LBSLOGI(FUSION_FENCE, "RemoveFusionFence");
    LocationErrCode errCode = ERRCODE_SUCCESS;
 
    if (request == nullptr) {
        LBSLOGE(FUSION_FENCE, "RemoveFusionFence request is nullptr");
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    std::string identifier = request->GetIdentifier();
 
    if (!IsFusionFenceSupported()) {
        LBSLOGI(FUSION_FENCE, "Is Not Support Geofence");
        return ERRCODE_NOT_SUPPORTED;
    }
 
    std::shared_ptr<FusionFenceRequest> requestToRemove = GetFusionFenceRequest(identifier);
    if (requestToRemove == nullptr) {
        return ERRCODE_GEOFENCE_INCORRECT_ID;
    }
 
    if (!ExecuteFusionFenceProcess(
        requestToRemove, static_cast<int>(GnssInterfaceCode::REMOVE_FUSION_FENCE))) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
 
    {
        std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
        for (auto iter = fusionFenceRequestList_.begin(); iter != fusionFenceRequestList_.end(); iter++) {
            if ((*iter)->GetIdentifier() == identifier) {
                RemoveFenceCount(*iter);
                fusionFenceRequestList_.erase(iter);
                break;
            }
        }
    }
    LBSLOGI(FUSION_FENCE, "RemoveFusionFence success, identifier: %{public}s, size: %{public}zu",
        identifier.c_str(), fusionFenceRequestList_.size());
    return ERRCODE_SUCCESS;
}

size_t FusionFenceAbility::GetGnssFenceCount()
{
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    return totalGnssFenceCount_;
}

int FusionFenceAbility::GetGnssFenceCountForOneApp(const std::string& bundleName)
{
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    auto it = gnssFenceCountMap_.find(bundleName);
    return (it != gnssFenceCountMap_.end()) ? it->second : 0;
}

bool FusionFenceAbility::IsFusionFenceSupported()
{
    std::shared_ptr<FusionFenceRequest> request;
    return ExecuteFusionFenceProcess(request, static_cast<int>(GnssInterfaceCode::IS_FUSION_FENCE_SUPPORTED));
}

void FusionFenceAbility::ReportFusionFenceEvent(const FusionFenceTransition& transition)
{
    LBSLOGI(FUSION_FENCE, "%{public}s called, identifier: %{public}s, scene: %{public}d",
        __func__, transition.identifier.c_str(), static_cast<int>(transition.scene));
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    sptr<IRemoteObject> foundCallback = nullptr;
    for (const auto& iter : fusionFenceRequestList_) {
        if (iter->GetIdentifier() == transition.identifier && iter->GetScene() == transition.scene) {
            foundCallback = iter->GetTransitionCallback();
            break;
        }
    }
    if (foundCallback == nullptr) {
        LBSLOGE(FUSION_FENCE, "FusionFenceRequest not found for identifier: %{public}s", transition.identifier.c_str());
        return;
    }
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option(MessageOption::TF_SYNC);
    if (!dataParcel.WriteInterfaceToken(IFusionFenceCallback::GetDescriptor())) {
        LBSLOGE(FUSION_FENCE, "WriteInterfaceToken failed");
        return;
    }
    if (!transition.Marshalling(dataParcel)) {
        LBSLOGE(FUSION_FENCE, "transition Marshalling failed");
        return;
    }
    int error = foundCallback->SendRequest(
        static_cast<uint32_t>(FusionFenceInterfaceCode::ON_TRANSITION_STATUS_CHANGE),
        dataParcel, replyParcel, option);
    if (error != ERR_OK) {
        LBSLOGE(FUSION_FENCE, "SendRequest failed, error: %{public}d", error);
        return;
    }
}

void FusionFenceAbility::ReportOperateResult(std::shared_ptr<FusionFenceRequest> request,
    GnssGeofenceOperateType type, LocationErrCode errorCode)
{
    if (request == nullptr) {
        LBSLOGE(FUSION_FENCE, "ReportOperateResult request is nullptr");
        return;
    }
    std::string fenceId = request->GetIdentifier();
    sptr<IRemoteObject> callback = request->GetTransitionCallback();
    if (callback == nullptr) {
        LBSLOGE(FUSION_FENCE, "callback is nullptr, fenceId: %{public}s", fenceId.c_str());
        return;
    }
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option(MessageOption::TF_SYNC);
    if (!dataParcel.WriteInterfaceToken(IFusionFenceCallback::GetDescriptor())) {
        LBSLOGE(FUSION_FENCE, "WriteInterfaceToken failed");
        return;
    }
    dataParcel.WriteString(fenceId);
    dataParcel.WriteInt32(static_cast<int>(type));
    dataParcel.WriteInt32(static_cast<int>(errorCode));
    int error = callback->SendRequest(
        static_cast<uint32_t>(FusionFenceInterfaceCode::REPORT_OPERATE_RESULT),
        dataParcel, replyParcel, option);
    if (error != ERR_OK) {
        LBSLOGE(FUSION_FENCE, "SendRequest failed, error: %{public}d", error);
        return;
    }
}

std::shared_ptr<FusionFenceRequest> FusionFenceAbility::GetFusionFenceRequest(const std::string& identifier)
{
    std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
    for (const auto& iter : fusionFenceRequestList_) {
        if (iter->GetIdentifier() == identifier) {
            return iter;
        }
    }
    return nullptr;
}

void FusionFenceAbility::RemoveFusionFenceByCallbackWhenAppDie(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(FUSION_FENCE, "RemoveFusionFenceByCallbackWhenAppDie callback is nullptr");
        return;
    }
    std::vector<std::string> identifierList;
    {
        std::unique_lock<ffrt::mutex> lock(fusionFenceMutex_);
        for (auto iter = fusionFenceRequestList_.begin(); iter != fusionFenceRequestList_.end(); iter++) {
            auto requestCallback = (*iter)->GetTransitionCallback();
            if (requestCallback != nullptr && requestCallback == callback) {
                identifierList.push_back((*iter)->GetIdentifier());
            }
        }
    }
    if (!identifierList.empty() && fusionFenceHandler_ != nullptr) {
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FUSION_FENCE),
            std::make_shared<std::vector<std::string>>(identifierList));
        fusionFenceHandler_->SendEvent(event);
    }
}

FusionFenceCallbackDeathRecipient::FusionFenceCallbackDeathRecipient()
{
}

FusionFenceCallbackDeathRecipient::~FusionFenceCallbackDeathRecipient()
{
}

void FusionFenceCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LBSLOGI(FUSION_FENCE, "fusion fence callback OnRemoteDied");
    FusionFenceAbility::GetInstance()->RemoveFusionFenceByCallbackWhenAppDie(remote.promote());
}

FusionFenceHandler::FusionFenceHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : EventHandler(runner)
{
    InitFusionFenceEventProcessMap();
}

FusionFenceHandler::~FusionFenceHandler()
{
}

void FusionFenceHandler::InitFusionFenceEventProcessMap()
{
    if (fusionFenceEventProcessMap_.size() != 0) {
        return;
    }
    fusionFenceEventProcessMap_[static_cast<uint32_t>(GnssInterfaceCode::REMOVE_FUSION_FENCE)] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { HandleRemoveFusionFence(event); };
}

void FusionFenceHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto fusionFenceAbility = FusionFenceAbility::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(FUSION_FENCE, "ProcessEvent event:%{public}d", eventId);
    auto handleFunc = fusionFenceEventProcessMap_.find(eventId);
    if (handleFunc != fusionFenceEventProcessMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
#ifdef LOCATION_HICOLLIE_ENABLE
        int tid = gettid();
        std::string moduleName = "FusionFenceHandler";
        XCollieCallback callbackFunc = [moduleName, eventId, tid](void *) {
            LBSLOGE(
                FUSION_FENCE,"TimeoutCallback tid:%{public}d moduleName:%{public}s excute eventId:%{public}u timeout.",
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

void FusionFenceHandler::HandleRemoveFusionFence(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto identifierListPtr = event->GetSharedObject<std::vector<std::string>>();
    if (identifierListPtr == nullptr || identifierListPtr->empty()) {
        return;
    }
    auto fusionFenceAbility = FusionFenceAbility::GetInstance();
    if (fusionFenceAbility == nullptr) {
        return;
    }
    for (const auto& identifier : *identifierListPtr) {
        auto request = std::make_shared<FusionFenceRequest>();
        request->SetIdentifier(identifier);
        fusionFenceAbility->RemoveFusionFence(request);
    }
}

} // namespace Location
} // namespace OHOS