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

#ifndef MOCK_NETWORK_ABILITY_STUB_H
#define MOCK_NETWORK_ABILITY_STUB_H

#include "gmock/gmock.h"

#include "message_option.h"
#include "message_parcel.h"
#include "iremote_object.h"
#include "iremote_stub.h"

#include "constant_definition.h"
#include "gnss_ability_skeleton.h"
#include "location.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class MockGnssAbilityStub : public GnssAbilityStub {
public:
    MockGnssAbilityStub() {}
    ~MockGnssAbilityStub() {}
    MOCK_METHOD(void, SendMessage, (uint32_t code, MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD(void, RefrashRequirements, ());
    MOCK_METHOD(void, RegisterGnssStatusCallback, (const sptr<IRemoteObject>& callback, pid_t uid));
    MOCK_METHOD(void, UnregisterGnssStatusCallback, (const sptr<IRemoteObject>& callback));
    MOCK_METHOD(void, RegisterNmeaMessageCallback, (const sptr<IRemoteObject>& callback, pid_t uid));
    MOCK_METHOD(void, UnregisterNmeaMessageCallback, (const sptr<IRemoteObject>& callback));
    MOCK_METHOD(void, RegisterCachedCallback, (const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback));
    MOCK_METHOD(void, UnregisterCachedCallback, (const sptr<IRemoteObject>& callback));
    MOCK_METHOD(int, GetCachedGnssLocationsSize, ());
    MOCK_METHOD(int, FlushCachedGnssLocations, ());
    MOCK_METHOD(void, SendCommand, (std::unique_ptr<LocationCommand>& commands));
    MOCK_METHOD(void, AddFence, (std::unique_ptr<GeofenceRequest>& request));
    MOCK_METHOD(void, RemoveFence, (std::unique_ptr<GeofenceRequest>& request));
    MOCK_METHOD(void, SendLocationRequest, (WorkRecord &workrecord));
    MOCK_METHOD(void, SetEnable, (bool state));
    MOCK_METHOD(bool, EnableMock, ());
    MOCK_METHOD(bool, DisableMock, ());
    MOCK_METHOD(bool, SetMocked, (const int timeInterval,
        const std::vector<std::shared_ptr<Location>> &location));
};
} // namespace Location
} // namespace OHOS
#endif