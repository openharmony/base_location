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

#ifndef NATIVE_LOCATION_CALLBACK_HOST_H
#define NATIVE_LOCATION_CALLBACK_HOST_H

#include "iremote_stub.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator_callback.h"
#include "location.h"

namespace OHOS {
namespace Location {
typedef void (* LocationUpdate)(const std::unique_ptr<Location>& location);

typedef struct {
    LocationUpdate locationUpdate;
} LocationCallbackIfaces;

class NativeLocationCallbackHost : public IRemoteStub<ILocatorCallback> {
public:
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    void OnNetWorkErrorReport(const int errorCode) override;

    inline void SetCallback(const LocationCallbackIfaces& locationCallback)
    {
        locationCallback_ = locationCallback;
    }

    inline LocationCallbackIfaces GetCallback()
    {
        return locationCallback_;
    }

private:
    LocationCallbackIfaces locationCallback_;
};
} // namespace Location
} // namespace OHOS
#endif // NATIVE_LOCATION_CALLBACK_HOST_H
