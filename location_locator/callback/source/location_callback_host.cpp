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

#include "location_callback_host.h"

#include <string>
#include "if_system_ability_manager.h"
#include "ipc_file_descriptor.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "common_utils.h"

namespace OHOS {
namespace Location {
LocationCallbackStub::LocationCallbackStub(std::string abilityName)
{
    abilityName_ = abilityName;
    isInitForProxy_ = false;
    label_ = CommonUtils::GetLabel(abilityName);
}

void LocationCallbackStub::init(std::string abilityName)
{
    localDeviceId_ = CommonUtils::InitDeviceId();
    GetRemoteLocatorProxy(localDeviceId_);
}

std::string LocationCallbackStub::GetAbilityName()
{
    return abilityName_;
}

int LocationCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(label_, "invalid token.");
        return -1;
    }
    int ret = EXCEPTION;
    pid_t lastCallingPid = IPCSkeleton::GetCallingPid();
    pid_t lastCallinguid = IPCSkeleton::GetCallingUid();
    LBSLOGI(label_, "OnReceived cmd = %{public}d, flags= %{public}d, pid= %{public}d, uid= %{public}d",
        code, option.GetFlags(), lastCallingPid, lastCallinguid);

    switch (code) {
        case RECEIVE_LOCATION_CHANGE_EVENT: {
            std::unique_ptr<Location> location = Location::UnmarshallingLocation(data);
            OnLocationUpdate(location);
            break;
        }
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}

void LocationCallbackStub::GetRemoteLocatorProxy(std::string deviceId)
{
    if (!isInitForProxy_) {
        proxyLocator_ = std::make_unique<LocatorProxy>(
            CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID, localDeviceId_));
        isInitForProxy_ = true;
        return;
    }
    if (deviceId.compare(localDeviceId_) == 0 && proxyLocator_ != nullptr) {
        return;
    }
    proxyLocator_ = std::make_unique<LocatorProxy>(CommonUtils::GetRemoteObject(LOCATION_LOCATOR_SA_ID, deviceId));
}

void LocationCallbackStub::OnLocationUpdate(const std::unique_ptr<Location>& location)
{
    LBSLOGI(label_, "LocationCallbackStub::OnLocationUpdate");
    init(abilityName_);
    if (proxyLocator_ != nullptr) {
        proxyLocator_->ReportLocation(location, abilityName_);
    }
}

void LocationCallbackStub::OnStatusUpdate(unsigned int gnssSessionStatus)
{
    LBSLOGI(label_, "LocationCallbackStub::OnStatusUpdate");
    init(abilityName_);
    if (proxyLocator_ != nullptr) {
        proxyLocator_->ReportGnssSessionStatus(gnssSessionStatus);
    }
}

void LocationCallbackStub::OnSvStatusUpdate(const std::unique_ptr<SatelliteStatus> &sv)
{
    LBSLOGI(label_, "LocationCallbackStub::OnSvStatusUpdate");
    init(abilityName_);
    if (proxyLocator_ != nullptr) {
        proxyLocator_->ReportSv(sv);
    }
}

void LocationCallbackStub::OnNmeaUpdate(int64_t timestamp, const std::string &nmea)
{
    LBSLOGI(label_, "LocationCallbackStub::OnNmeaUpdate");
    init(abilityName_);
    if (proxyLocator_ != nullptr) {
        proxyLocator_->ReportNmea(nmea);
    }
}
} // namespace Location
} // namespace OHOS
