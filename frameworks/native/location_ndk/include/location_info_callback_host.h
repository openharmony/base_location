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

#ifndef LOCATION_INFO_CALLBACK_HOST_H
#define LOCATION_INFO_CALLBACK_HOST_H

#include "iremote_stub.h"

#include "oh_location_type.h"
#include "i_locator_callback.h"
#include "location.h"

namespace OHOS {
namespace Location {
class LocationInfoCallbackHost : public IRemoteStub<OHOS::Location::ILocatorCallback> {
public:
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;

    inline void SetCallback(const Location_InfoCallback& locationInfoCallback)
    {
        locationInfoCallback_ = locationInfoCallback;
    }

    inline Location_InfoCallback GetCallback()
    {
        return locationInfoCallback_;
    }

private:
    Location_InfoCallback locationInfoCallback_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_INFO_CALLBACK_HOST_H
