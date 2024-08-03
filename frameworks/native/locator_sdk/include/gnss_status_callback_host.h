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

#ifndef GNSS_STATUS_CALLBACK_HOST_H
#define GNSS_STATUS_CALLBACK_HOST_H

#include "iremote_stub.h"
#include "i_gnss_status_callback.h"
#include "satellite_status.h"

namespace OHOS {
namespace Location {
class GnssStatusCallbackHost : public IRemoteStub<IGnssStatusCallback> {
public:
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnStatusChange(const std::unique_ptr<SatelliteStatus>& statusInfo) override;
};
} // namespace Location
} // namespace OHOS
#endif // GNSS_STATUS_CALLBACK_HOST_H
