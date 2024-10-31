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
#include "location_switch_callback.h"

namespace OHOS {
namespace GeoLocationManager {
LocationSwitchCallback::LocationSwitchCallback()
{
    remoteDied_ = false;
}

LocationSwitchCallback::LocationSwitchCallback(int64_t callbackId)
{
    remoteDied_ = false;
    this->callbackId_ = callbackId;
    auto cFunc = reinterpret_cast<void(*)(int32_t switchState)>(callbackId);
    callback_ = [ lambda = CJLambda::Create(cFunc)](int switchState) ->
        void { lambda(static_cast<bool>(switchState)); };
}

LocationSwitchCallback::~LocationSwitchCallback()
{
}

int LocationSwitchCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(Location::SWITCH_CALLBACK, "LocatorCallback::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(Location::SWITCH_CALLBACK, "invalid token.");
        return -1;
    }
    if (remoteDied_) {
        LBSLOGD(Location::SWITCH_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }

    switch (code) {
        case Location::ISwitchCallback::RECEIVE_SWITCH_STATE_EVENT: {
            OnSwitchChange(data.ReadInt32());
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool LocationSwitchCallback::IsRemoteDied()
{
    return remoteDied_;
}

void LocationSwitchCallback::OnSwitchChange(int switchState)
{
    LBSLOGD(Location::SWITCH_CALLBACK, "LocatorCallback::OnSwitchChange");
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_ != nullptr) {
        callback_(switchState);
    }
}
}
}