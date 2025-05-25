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

#ifndef LOCATION_ERROR_CALLBACK_NAPI_H
#define LOCATION_ERROR_CALLBACK_NAPI_H

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator_callback.h"
#include "iremote_stub.h"
#include "location.h"
#include "ohos.geoLocationManager.proj.hpp"
#include "ohos.geoLocationManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Location {
class LocationErrorCallbackTaihe : public IRemoteStub<ILocatorCallback> {
public:
    LocationErrorCallbackTaihe();
    virtual ~LocationErrorCallbackTaihe();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    ::taihe::optional<::taihe::callback<void(::ohos::geoLocationManager::LocationError)>> callback_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_ERROR_CALLBACK_NAPI_H
