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
#include "nmea_message_callback.h"

namespace OHOS {
namespace GeoLocationManager {
NmeaMessageCallback::NmeaMessageCallback()
{
    remoteDied_ = false;
}

NmeaMessageCallback::NmeaMessageCallback(int64_t callbackId)
{
    remoteDied_ = false;
    this->callbackId_ = callbackId;
    auto cFunc = reinterpret_cast<void(*)(char* str)>(callbackId);
    callback_ = [ lambda = CJLambda::Create(cFunc)](const std::string msg) ->
        void { lambda(MallocCString(msg)); };
}

NmeaMessageCallback::~NmeaMessageCallback()
{
}

int NmeaMessageCallback::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(Location::NMEA_MESSAGE_CALLBACK, "NmeaMessageCallback::OnRemoteRequest!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(Location::NMEA_MESSAGE_CALLBACK, "invalid token.");
        return -1;
    }
    if (remoteDied_) {
        LBSLOGD(Location::NMEA_MESSAGE_CALLBACK, "Failed to `%{public}s`,Remote service is died!", __func__);
        return -1;
    }

    switch (code) {
        case Location::INmeaMessageCallback::RECEIVE_NMEA_MESSAGE_EVENT: {
            int64_t timestamp = data.ReadInt64();
            std::string msg = Str16ToStr8(data.ReadString16());
            OnMessageChange(timestamp, msg);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

bool NmeaMessageCallback::IsRemoteDied()
{
    return remoteDied_;
}

void NmeaMessageCallback::OnMessageChange(int64_t timestamp, const std::string msg)
{
    LBSLOGD(Location::NMEA_MESSAGE_CALLBACK, "NmeaMessageCallback::OnMessageChange");
    std::unique_lock<std::mutex> guard(mutex_);
    if (callback_ != nullptr) {
        callback_(msg);
    }
}
}
}