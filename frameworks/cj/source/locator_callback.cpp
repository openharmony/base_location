/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "location_log.h"
#include "cj_lambda.h"
#include "locator_callback.h"

namespace OHOS {
namespace GeoLocationManager {
LocatorCallback::LocatorCallback()
{
    fixNumber_ = 0;
    inHdArea_ = true;
    singleLocation_ = nullptr;
    callbackValid_ = false;
    locationPriority_ = 0;
    InitLatch();
}

LocatorCallback::LocatorCallback(int64_t callbackId)
{
    fixNumber_ = 0;
    inHdArea_ = true;
    singleLocation_ = nullptr;
    callbackValid_ = false;
    locationPriority_ = 0;
    InitLatch();
    this->callbackId_ = callbackId;
    auto cFunc = reinterpret_cast<void(*)(CJLocation location)>(callbackId);
    callback_ = [ lambda = CJLambda::Create(cFunc)](const std::unique_ptr<Location::Location>& location) ->
        void { lambda(NativeLocationToCJLocation(*location)); };
}

void LocatorCallback::InitLatch()
{
    latch_ = new Location::CountDownLatch();
    if (latch_ == nullptr) {
        LBSLOGE(Location::LOCATOR_CALLBACK, "latch_ is nullptr.");
        return;
    }
    latch_->SetCount(1);
}

LocatorCallback::~LocatorCallback()
{
    delete latch_;
}

int LocatorCallback::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(Location::LOCATOR_CALLBACK, "invalid token.");
        return -1;
    }

    switch (code) {
        case Location::ILocatorCallback::RECEIVE_LOCATION_INFO_EVENT: {
            std::unique_ptr<Location::Location> location = Location::Location::Unmarshalling(data);
            LocatorCallback::OnLocationReport(location);
            if (location->GetLocationSourceType() == Location::LocationSourceType::NETWORK_TYPE &&
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
        case Location::ILocatorCallback::RECEIVE_LOCATION_STATUS_EVENT: {
            int status = data.ReadInt32();
            OnLocatingStatusChange(status);
            break;
        }
        case Location::ILocatorCallback::RECEIVE_ERROR_INFO_EVENT: {
            int errorCode = data.ReadInt32();
            LBSLOGI(Location::LOCATOR_STANDARD, "CallbackSutb receive ERROR_EVENT. errorCode:%{public}d", errorCode);
            if (errorCode == Location::LOCATING_FAILED_INTERNET_ACCESS_FAILURE) {
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

void LocatorCallback::OnLocationReport(const std::unique_ptr<Location::Location>& location)
{
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_ != nullptr) {
        callback_(location);
    }
}

void LocatorCallback::OnLocatingStatusChange(const int status)
{
}

void LocatorCallback::OnErrorReport(const int errorCode)
{
}

bool LocatorCallback::IsSingleLocationRequest()
{
    return (fixNumber_ == 1);
}

void LocatorCallback::CountDown()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->CountDown();
    }
}

void LocatorCallback::Wait(int time)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        latch_->Wait(time);
    }
}

int LocatorCallback::GetCount()
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->GetCount();
    }
    return 0;
}

void LocatorCallback::SetCount(int count)
{
    if (IsSingleLocationRequest() && latch_ != nullptr) {
        return latch_->SetCount(count);
    }
}

bool LocatorCallback::NeedSetSingleLocation(const std::unique_ptr<Location::Location>& location)
{
    if (locationPriority_ == Location::LOCATION_PRIORITY_ACCURACY &&
        singleLocation_ != nullptr &&
        location->GetLocationSourceType() == Location::LocationSourceType::NETWORK_TYPE) {
        return false;
    } else {
        return true;
    }
}

bool LocatorCallback::IfReportAccuracyLocation()
{
    if (locationPriority_ == Location::LOCATION_PRIORITY_ACCURACY &&
        (((singleLocation_->GetLocationSourceType() == Location::LocationSourceType::GNSS_TYPE ||
        singleLocation_->GetLocationSourceType() == Location::LocationSourceType::RTK_TYPE) && inHdArea_) ||
        singleLocation_->GetLocationSourceType() == Location::LocationSourceType::NETWORK_TYPE)) {
        return false;
    } else {
        return true;
    }
}

void LocatorCallback::SetSingleLocation(const std::unique_ptr<Location::Location>& location)
{
    std::unique_lock<std::mutex> guard(mutex_);
    singleLocation_ = std::make_shared<Location::Location>(*location);
}
}
}
