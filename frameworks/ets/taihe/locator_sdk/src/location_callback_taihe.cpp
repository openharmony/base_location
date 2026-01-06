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

#include "location_callback_taihe.h"

#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
#include "message_option.h"
#include "message_parcel.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator_callback.h"
#include "location.h"
#include "location_log.h"
#include "util.h"

namespace OHOS {
namespace Location {
LocatorCallbackTaihe::LocatorCallbackTaihe()
{
    fixNumber_ = 0;
    inHdArea_ = true;
    singleLocation_ = nullptr;
    locationPriority_ = 0;
    InitLatch();
}

void LocatorCallbackTaihe::InitLatch()
{
    latch_ = new CountDownLatch();
    latch_->SetCount(1);
}

LocatorCallbackTaihe::~LocatorCallbackTaihe()
{
    delete latch_;
    LBSLOGW(LOCATOR_CALLBACK, "~LocatorCallbackTaihe()");
}

int LocatorCallbackTaihe::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case RECEIVE_LOCATION_INFO_EVENT: {
            std::unique_ptr<Location> location = Location::UnmarshallingMakeUnique(data);
            OnLocationReport(location);
            if (location->GetLocationSourceType() == LocationSourceType::NETWORK_TYPE &&
                location->GetAdditionsMap()["inHdArea"] != "") {
                inHdArea_ = (location->GetAdditionsMap()["inHdArea"] == "true");
            }
            if (NeedSetSingleLocation(location)) {
                SetSingleLocation(location);
            }
            if (IfReportAccuracyLocation()) {
                CountDown();
            }
            break;
        }
        case RECEIVE_LOCATION_STATUS_EVENT: {
            int status = data.ReadInt32();
            OnLocatingStatusChange(status);
            break;
        }
        case RECEIVE_ERROR_INFO_EVENT: {
            int errorCode = data.ReadInt32();
            LBSLOGI(LOCATOR_STANDARD, "CallbackSutb receive ERROR_EVENT. errorCode:%{public}d", errorCode);
            if (errorCode == LOCATING_FAILED_INTERNET_ACCESS_FAILURE) {
                inHdArea_ = false;
                if (GetSingleLocation() != nullptr) {
                    CountDown();
                }
            } else {
                OnErrorReport(errorCode);
            }
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void LocatorCallbackTaihe::OnLocationReport(const std::unique_ptr<Location>& location)
{
    std::unique_ptr<Location> locationReport = std::make_unique<Location>(*location);
    ::ohos::geoLocationManager::Location locationTaihe;
    Util::LocationToTaihe(locationTaihe, locationReport);
    if (callback_) {
        (*callback_)(locationTaihe);
    }
}

void LocatorCallbackTaihe::OnLocatingStatusChange(const int status)
{
}

void LocatorCallbackTaihe::OnErrorReport(const int errorCode)
{
}

void LocatorCallbackTaihe::DeleteAllCallbacks()
{
}

bool LocatorCallbackTaihe::IsSingleLocationRequest()
{
    return (fixNumber_ == 1);
}

void LocatorCallbackTaihe::CountDown()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->CountDown();
    }
}

void LocatorCallbackTaihe::Wait(int time)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->Wait(time);
    }
}

int LocatorCallbackTaihe::GetCount()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->GetCount();
    }
    return 0;
}

void LocatorCallbackTaihe::SetCount(int count)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->SetCount(count);
    }
}

bool LocatorCallbackTaihe::NeedSetSingleLocation(const std::unique_ptr<Location>& location)
{
    if (locationPriority_ == LOCATION_PRIORITY_ACCURACY &&
        singleLocation_ != nullptr &&
        location->GetLocationSourceType() == LocationSourceType::NETWORK_TYPE) {
        return false;
    } else {
        return true;
    }
}

bool LocatorCallbackTaihe::IfReportAccuracyLocation()
{
    if (locationPriority_ == LOCATION_PRIORITY_ACCURACY &&
        (((singleLocation_->GetLocationSourceType() == LocationSourceType::GNSS_TYPE ||
        singleLocation_->GetLocationSourceType() == LocationSourceType::RTK_TYPE) && inHdArea_) ||
        singleLocation_->GetLocationSourceType() == LocationSourceType::NETWORK_TYPE)) {
        return false;
    } else {
        return true;
    }
}

void LocatorCallbackTaihe::SetSingleLocation(const std::unique_ptr<Location>& location)
{
    std::unique_lock<std::mutex> guard(mutex_);
    singleLocation_ = std::make_shared<Location>(*location);
}
} // namespace Location
} // namespace OHOS
