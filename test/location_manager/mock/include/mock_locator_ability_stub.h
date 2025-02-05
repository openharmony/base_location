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

#ifndef MOCK_LOCATOR_ABILITY_STUB_H
#define MOCK_LOCATOR_ABILITY_STUB_H

#include "gmock/gmock.h"

#include "message_option.h"
#include "message_parcel.h"
#include "iremote_object.h"
#include "iremote_stub.h"

#include "locator_skeleton.h"

namespace OHOS {
namespace Location {
class MockLocatorAbilityStub : public LocatorAbilityStub {
public:
    MockLocatorAbilityStub() {}
    ~MockLocatorAbilityStub() {}
    MOCK_METHOD(void, RemoveUnloadTask, (uint32_t code));
    MOCK_METHOD(void, PostUnloadTask, (uint32_t code));
    MOCK_METHOD(bool, CancelIdleState, (uint32_t code));
};
} // namespace Location
} // namespace OHOS
#endif // MOCK_LOCATOR_ABILITY_STUB_H
