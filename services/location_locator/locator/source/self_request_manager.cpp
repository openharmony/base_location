 /*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "self_request_manager.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "location_config_manager.h"
#include "location_log.h"
#include "locator_ability.h"
#include "request_manager.h"
#include "permission_manager.h"
#include "location_data_rdb_manager.h"
#include "ipc_skeleton.h"
#ifdef LOCATION_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace Location {
std::mutex SelfRequestManager::locatorMutex_;
const uint32_t EVENT_STARTLOCATING = 0x0100;
const uint32_t EVENT_STOPLOCATING = 0x0200;
const int TIMEOUT_WATCHDOG = 60; // s
SelfRequestManager* SelfRequestManager::GetInstance()
{
    static SelfRequestManager data;
    return &data;
}

SelfRequestManager::SelfRequestManager()
{
    selfRequestManagerHandler_ = std::make_shared<SelfRequestManagerHandler>(AppExecFwk::EventRunner::Create(true,
        AppExecFwk::ThreadMode::FFRT));
    callback_ = sptr<mLocatorCallback>(new (std::nothrow) SelfRequestManager::mLocatorCallback());
    if (callback_ == nullptr) {
        return;
    }
    request_ = std::make_shared<Request>();
}

SelfRequestManager::~SelfRequestManager()
{
}

void SelfRequestManager::ProcessStartSelfRequestEvent(const std::shared_ptr<Request>& request)
{
    int uid = IPCSkeleton::GetCallingUid();
    int userId = CommonUtils::GetUserIdByUid(uid);
    if (isLocating_ || !(LocationDataRdbManager::QuerySwitchStateForUser(userId) == ENABLED)
        || request_ == nullptr || request == nullptr || request->GetRequestConfig() == nullptr) {
        LBSLOGD(LOCATOR, "cancel locating");
        return;
    }
    isLocating_ = true;
    request_->SetUid(IPCSkeleton::GetCallingUid());
    request_->SetPid(IPCSkeleton::GetCallingPid());
    std::string packageName = request->GetPackageName();
    if (packageName == "") {
        packageName = std::to_string(request->GetUid());
    }
    request_->SetPackageName(packageName);
    request_->SetRequestConfig(*request->GetRequestConfig());
    request_->SetUuid(CommonUtils::GenerateUuid());
    request_->SetTokenId(IPCSkeleton::GetCallingTokenID());
    request_->SetTokenIdEx(IPCSkeleton::GetCallingFullTokenID());
    request_->SetLocatorCallBack(callback_);
    request_->ClearAllCategoryCounts();
    LBSLOGI(LOCATOR, "SelfRequestManager start locating");
    LocatorAbility::GetInstance()->HandleStartLocating(request_, callback_);
}

void SelfRequestManager::ProcessStopSelfRequestEvent()
{
    if (!isLocating_) {
        return;
    }
    isLocating_ = false;
    LocatorAbility::GetInstance()->StopLocating(callback_);
    LBSLOGI(LOCATOR, "SelfRequestManager stop locating");
}

void SelfRequestManager::StopSelfRequest()
{
    if (selfRequestManagerHandler_ != nullptr) {
        selfRequestManagerHandler_->SendHighPriorityEvent(EVENT_STOPLOCATING, 0, 0);
    }
}

void SelfRequestManager::StartSelfRequest(const std::shared_ptr<Request>& request)
{
    if (selfRequestManagerHandler_ != nullptr) {
        selfRequestManagerHandler_->SendHighPriorityEvent(EVENT_STARTLOCATING, request, 0);
        selfRequestManagerHandler_->SendHighPriorityEvent(EVENT_STOPLOCATING, 0, DEFAULT_TIMEOUT_5S);
    }
}

void SelfRequestManager::mLocatorCallback::OnLocationReport(const std::unique_ptr<Location>& location)
{
    LBSLOGD(LOCATOR, "locator background OnLocationReport");
    SelfRequestManager::GetInstance()->StopSelfRequest();
}

void SelfRequestManager::mLocatorCallback::OnLocatingStatusChange(const int status)
{
}

void SelfRequestManager::mLocatorCallback::OnErrorReport(const int errorCode)
{
}

SelfRequestManagerHandler::SelfRequestManagerHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner)
{
    InitSelfRequestManagerHandlerEventMap();
}

SelfRequestManagerHandler::~SelfRequestManagerHandler() {}

void SelfRequestManagerHandler::InitSelfRequestManagerHandlerEventMap()
{
    if (selfRequestManagerHandlerEventMap_.size() != 0) {
        return;
    }
    selfRequestManagerHandlerEventMap_[EVENT_STARTLOCATING] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { StartLocatingEvent(event); };
    selfRequestManagerHandlerEventMap_[EVENT_STOPLOCATING] =
        [this](const AppExecFwk::InnerEvent::Pointer& event) { StopLocatingEvent(event); };
}

void SelfRequestManagerHandler::StartLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto SelfRequestManager = SelfRequestManager::GetInstance();
    std::shared_ptr<Request> request = event->GetSharedObject<Request>();
    SelfRequestManager->ProcessStartSelfRequestEvent(request);
}

void SelfRequestManagerHandler::StopLocatingEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto SelfRequestManager = SelfRequestManager::GetInstance();
    SelfRequestManager->ProcessStopSelfRequestEvent();
}

void SelfRequestManagerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    LBSLOGD(LOCATOR, "SelfRequestManagerHandler processEvent event:%{public}d, timestamp = %{public}s",
        eventId, std::to_string(CommonUtils::GetCurrentTimeStamp()).c_str());
    auto handleFunc = selfRequestManagerHandlerEventMap_.find(eventId);
    if (handleFunc != selfRequestManagerHandlerEventMap_.end() && handleFunc->second != nullptr) {
        auto memberFunc = handleFunc->second;
#ifdef LOCATION_HICOLLIE_ENABLE
        int tid = gettid();
        std::string moduleName = "SelfRequestManagerHandler";
        XCollieCallback callbackFunc = [moduleName, eventId, tid](void *) {
            LBSLOGE(LOCATOR, "TimeoutCallback tid:%{public}d moduleName:%{public}s excute eventId:%{public}u timeout.",
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
} // namespace OHOS
} // namespace Location
