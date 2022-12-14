/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef MOCK_GNSS_STATUS_CALLBACK_HOST_H
#define MOCK_GNSS_STATUS_CALLBACK_HOST_H

#include "gmock/gmock.h"

#include "message_option.h"
#include "message_parcel.h"

#include "gnss_status_callback_host.h"
#include "satellite_status.h"

namespace OHOS {
namespace Location {
class MockGnssStatusCallbackHost : public GnssStatusCallbackHost {
public:
    MockGnssStatusCallbackHost() {}
    ~MockGnssStatusCallbackHost() {}
    
    MOCK_METHOD(int, OnRemoteRequest, (uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option));
    MOCK_METHOD(void, OnStatusChange, (const std::unique_ptr<SatelliteStatus>& statusInfo));
};
} // namespace Location
} // namespace OHOS
#endif