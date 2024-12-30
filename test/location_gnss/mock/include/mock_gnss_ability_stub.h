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

#ifndef MOCK_GNSS_ABILITY_STUB_H
#define MOCK_GNSS_ABILITY_STUB_H
#ifdef FEATURE_GNSS_SUPPORT

#include "gmock/gmock.h"

#include "message_option.h"
#include "message_parcel.h"
#include "iremote_object.h"
#include "iremote_stub.h"

#include "constant_definition.h"
#define private public
#include "gnss_ability_skeleton.h"
#undef private
#include "location.h"
#include "work_record.h"
#include "geofence_request.h"

namespace OHOS {
namespace Location {
class MockGnssAbilityStub : public GnssAbilityStub {
public:
    MockGnssAbilityStub() {}
    ~MockGnssAbilityStub() {}
    MOCK_METHOD(void, SendMessage, (uint32_t code, MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD(LocationErrCode, RefrashRequirements, ());
    MOCK_METHOD(LocationErrCode, RegisterGnssStatusCallback, (const sptr<IRemoteObject>& callback,
        AppIdentity &identity));
    MOCK_METHOD(LocationErrCode, UnregisterGnssStatusCallback, (const sptr<IRemoteObject>& callback));
    MOCK_METHOD(LocationErrCode, RegisterNmeaMessageCallback, (const sptr<IRemoteObject>& callback,
        AppIdentity &identity));
    MOCK_METHOD(LocationErrCode, UnregisterNmeaMessageCallback, (const sptr<IRemoteObject>& callback));
    MOCK_METHOD(LocationErrCode, RegisterCachedCallback, (const std::unique_ptr<CachedGnssLocationsRequest>& request,
        const sptr<IRemoteObject>& callback));
    MOCK_METHOD(LocationErrCode, UnregisterCachedCallback, (const sptr<IRemoteObject>& callback));
    MOCK_METHOD(LocationErrCode, GetCachedGnssLocationsSize, (int &size));
    MOCK_METHOD(LocationErrCode, FlushCachedGnssLocations, ());
    MOCK_METHOD(LocationErrCode, SendCommand, (std::unique_ptr<LocationCommand>& commands));
    MOCK_METHOD(LocationErrCode, AddFence, (std::shared_ptr<GeofenceRequest>& request));
    MOCK_METHOD(LocationErrCode, RemoveFence, (std::shared_ptr<GeofenceRequest>& request));
    MOCK_METHOD(LocationErrCode, SendLocationRequest, (WorkRecord &workrecord));
    MOCK_METHOD(LocationErrCode, SetEnable, (bool state));
    MOCK_METHOD(LocationErrCode, EnableMock, ());
    MOCK_METHOD(LocationErrCode, DisableMock, ());
    MOCK_METHOD(LocationErrCode, SetMocked, (const int timeInterval,
        const std::vector<std::shared_ptr<Location>> &location));
    MOCK_METHOD(bool, CancelIdleState, ());
    MOCK_METHOD(void, UnloadGnssSystemAbility, ());
    MOCK_METHOD(LocationErrCode, AddGnssGeofence, (std::shared_ptr<GeofenceRequest>& request));
    MOCK_METHOD(LocationErrCode, RemoveGnssGeofence, (std::shared_ptr<GeofenceRequest>& request));
    MOCK_METHOD(LocationErrCode, QuerySupportCoordinateSystemType,
        (std::vector<CoordinateSystemType>& coordinateSystemTypes));
    MOCK_METHOD(LocationErrCode, SendNetworkLocation, (const std::unique_ptr<Location>& location));
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GNSS_SUPPORT
#endif // MOCK_GNSS_ABILITY_STUB_H
