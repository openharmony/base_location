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

#ifndef MOCK_PASSIVE_ABILITY_STUB_H
#define MOCK_PASSIVE_ABILITY_STUB_H
#ifdef FEATURE_PASSIVE_SUPPORT

#include "gmock/gmock.h"

#include "message_option.h"
#include "message_parcel.h"
#include "iremote_object.h"
#include "iremote_stub.h"

#include "constant_definition.h"
#include "passive_ability_skeleton.h"
#include "location.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class MockPassiveAbilityStub : public PassiveAbilityStub {
public:
    MockPassiveAbilityStub() {}
    ~MockPassiveAbilityStub() {}
    MOCK_METHOD(void, SendMessage, (uint32_t code, MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD(LocationErrCode, SendLocationRequest, (WorkRecord &workrecord));
    MOCK_METHOD(LocationErrCode, SetEnable, (bool state));
    MOCK_METHOD(LocationErrCode, EnableMock, ());
    MOCK_METHOD(LocationErrCode, DisableMock, ());
    MOCK_METHOD(LocationErrCode, SetMocked, (const int timeInterval,
        const std::vector<std::shared_ptr<Location>> &location));
    MOCK_METHOD(bool, CancelIdleState, ());
    MOCK_METHOD(void, UnloadPassiveSystemAbility, ());
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_PASSIVE_SUPPORT
#endif // MOCK_PASSIVE_ABILITY_STUB_H
