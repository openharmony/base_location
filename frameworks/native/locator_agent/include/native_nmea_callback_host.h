/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NATIVE_NMEA_CALLBACK_HOST_H
#define NATIVE_NMEA_CALLBACK_HOST_H

#include <string>

#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"

#include "constant_definition.h"
#include "i_nmea_message_callback.h"
#include "string_ex.h"

namespace OHOS {
namespace Location {
typedef void (* NmeaUpdate)(int64_t timestamp, const std::string msg);

typedef struct {
    NmeaUpdate nmeaUpdate;
} GnssNmeaCallbackIfaces;

class NativeNmeaCallbackHost : public IRemoteStub<INmeaMessageCallback> {
public:
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnMessageChange(int64_t timestamp, const std::string msg) override;

    inline GnssNmeaCallbackIfaces GetCallback() const
    {
        return nmeaCallback_;
    }

    inline void SetCallback(const GnssNmeaCallbackIfaces& nmeaCallback)
    {
        nmeaCallback_ = nmeaCallback;
    }

private:
    GnssNmeaCallbackIfaces nmeaCallback_;
};
} // namespace Location
} // namespace OHOS
#endif // NATIVE_NMEA_CALLBACK_HOST_H
