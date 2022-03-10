/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_GEOFENCE_STATE_H
#define OHOS_GEOFENCE_STATE_H

#include "constant_definition.h"
#include "want_agent_helper.h"

namespace OHOS {
namespace Location {
class GeoFenceState {
public:
    GeoFenceState();
    explicit GeoFenceState(const GeoFence fence, const AbilityRuntime::WantAgent::WantAgent wantAgent);
    virtual ~GeoFenceState() = default;
private:
    int mState;
    GeoFence mFence;
    AbilityRuntime::WantAgent::WantAgent mWantAgent;
};
} // Location
} // OHOS
#endif // OHOS_GEOFENCE_STATE_H