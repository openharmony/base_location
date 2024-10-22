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

namespace OHOS {
namespace Location {
std::mutex SelfRequestManager::requestListMutex_;
std::mutex SelfRequestManager::locatorMutex_;
SelfRequestManager* SelfRequestManager::GetInstance()
{
    static SelfRequestManager data;
    return &data;
}

SelfRequestManager::SelfRequestManager()
{
    InitArgsFromProp();
    selfRequestManagerHandler_ = std::make_shared<SelfRequestManagerHandler>(AppExecFwk::EventRunner::Create(true,
        AppExecFwk::ThreadMode::FFRT));

    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    requestConfig->SetTimeInterval(timeInterval_);
    callback_ = sptr<mLocatorCallback>(new (std::nothrow) SelfRequestManager::mLocatorCallback());
    if (callback_ == nullptr) {
        return;
    }
    request_ = std::make_shared<Request>();
    if (request_ == nullptr) {
        return;
    }
    request_->SetUid(SYSTEM_UID);
    request_->SetPid(getpid());
    request_->SetPackageName(PROC_NAME);
    request_->SetRequestConfig(*requestConfig);
    request_->SetLocatorCallBack(callback_);
    request_->SetUuid(PROC_NAME);
    request_->SetTokenId(IPCSkeleton::GetCallingTokenID());
    request_->SetTokenIdEx(IPCSkeleton::GetCallingFullTokenID());
    proxySwtich_ = (LocationDataRdbManager::QuerySwitchState() == ENABLED);
}

SelfRequestManager::~SelfRequestManager()
{
}

void SelfRequestManager::StartLocatorThread()
{
    auto requestManager = RequestManager::GetInstance();
    std::unique_lock<std::mutex> lock(locatorMutex_, std::defer_lock);
    lock.lock();
    if (isLocating_ || !proxySwtich_) {
        LBSLOGD(LOCATOR_BACKGROUND_PROXY, "cancel locating");
        lock.unlock();
        return;
    }
    isLocating_ = true;
    lock.unlock();
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "SelfRequestManager start locating");
    requestManager->HandleStartLocating(request_);
}

void SelfRequestManager::StopLocatorThread()
{
    auto locatorAbility = LocatorAbility::GetInstance();
    std::unique_lock<std::mutex> lock(locatorMutex_, std::defer_lock);
    lock.lock();
    if (!isLocating_) {
        lock.unlock();
        return;
    }
    isLocating_ = false;
    lock.unlock();
    locatorAbility->StopLocating(callback_);
    LBSLOGI(LOCATOR_BACKGROUND_PROXY, "SelfRequestManager stop locating");
}

void SelfRequestManager::StopLocator()
{
    selfRequestManagerHandler_->SendHighPriorityEvent(EVENT_STOPLOCATING, 0, 0);
}

void SelfRequestManager::StartLocator()
{
    selfRequestManagerHandler_->SendHighPriorityEvent(EVENT_STARTLOCATING, 0, 0);
    selfRequestManagerHandler_->SendHighPriorityEvent(EVENT_STOPLOCATING, 0, DEFAULT_TIMEOUT_5S);
}

void SelfRequestManager::mLocatorCallback::OnLocationReport(const std::unique_ptr<Location>& location)
{
    LBSLOGD(LOCATOR_BACKGROUND_PROXY, "locator background OnLocationReport");
    SelfRequestManager::GetInstance()->StopLocator();
}

void SelfRequestManager::mLocatorCallback::OnLocatingStatusChange(const int status)
{
}

void SelfRequestManager::mLocatorCallback::OnErrorReport(const int errorCode)
{
}

LocatorBackgroundHandler::LocatorBackgroundHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner) : EventHandler(runner) {}

LocatorBackgroundHandler::~LocatorBackgroundHandler() {}

void LocatorBackgroundHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    auto SelfRequestManager = SelfRequestManager::GetInstance();
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case EVENT_STARTLOCATING: {
            SelfRequestManager->StartLocatorThread();
            break;
        }
        case EVENT_STOPLOCATING: {
            SelfRequestManager->StopLocatorThread();
            break;
        }
        default:
            break;
    }
}
} // namespace OHOS
} // namespace Location
