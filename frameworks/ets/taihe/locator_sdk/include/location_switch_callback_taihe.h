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

#ifndef LOCATION_SWITCH_CALLBACK_TAIHE_H
#define LOCATION_SWITCH_CALLBACK_TAIHE_H

#include "i_switch_callback.h"

#include "iremote_stub.h"

#include "ohos.geoLocationManager.proj.hpp"
#include "ohos.geoLocationManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Location {
class LocationSwitchCallbackTaihe : public IRemoteStub<ISwitchCallback> {
public:
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnSwitchChange(int switchState) override;
    ::taihe::optional<::taihe::callback<void(bool)>> callback_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_SWITCH_CALLBACK_TAIHE_H
