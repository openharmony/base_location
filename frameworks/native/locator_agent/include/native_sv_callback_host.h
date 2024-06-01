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

#ifndef NATIVE_SV_CALLBACK_HOST_H
#define NATIVE_SV_CALLBACK_HOST_H

#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"

#include "i_gnss_status_callback.h"
#include "satellite_status.h"
#include "constant_definition.h"

namespace OHOS {
namespace Location {
typedef void (* SvStatusUpdate)(const std::unique_ptr<SatelliteStatus>& statusInfo);

typedef struct {
    SvStatusUpdate svStatusUpdate;
} SvStatusCallbackIfaces;

class NativeSvCallbackHost : public IRemoteStub<IGnssStatusCallback> {
public:
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnStatusChange(const std::unique_ptr<SatelliteStatus>& statusInfo) override;

    inline SvStatusCallbackIfaces GetCallback() const
    {
        return svCallback_;
    }

    inline void SetCallback(const SvStatusCallbackIfaces& svCallback)
    {
        svCallback_ = svCallback;
    }

private:
    SvStatusCallbackIfaces svCallback_;
};
} // namespace Location
} // namespace OHOS
#endif // NATIVE_SV_CALLBACK_HOST_H
