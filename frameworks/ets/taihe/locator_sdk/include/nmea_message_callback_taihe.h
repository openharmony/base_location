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

#ifndef NMEA_MESSAGE_CALLBACK_TAIHE_H
#define NMEA_MESSAGE_CALLBACK_TAIHE_H

#include <string>

#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"

#include "i_nmea_message_callback.h"

#include "ohos.geoLocationManager.proj.hpp"
#include "ohos.geoLocationManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Location {
class NmeaMessageCallbackTaihe : public IRemoteStub<INmeaMessageCallback> {
public:
    NmeaMessageCallbackTaihe();
    virtual ~NmeaMessageCallbackTaihe();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnMessageChange(int64_t timestamp, const std::string msg) override;
    ::taihe::optional<::taihe::callback<void(::taihe::string_view)>> callback_;
};
} // namespace Location
} // namespace OHOS
#endif // NMEA_MESSAGE_CALLBACK_TAIHE_H
