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

#ifdef FEATURE_NETWORK_SUPPORT
#ifndef NETWORK_CALLBACK_HOST_H
#define NETWORK_CALLBACK_HOST_H

#include "i_locator_callback.h"
#include "iremote_stub.h"
#include "location.h"
#include "message_parcel.h"
#include "message_option.h"

namespace OHOS {
namespace Location {
class NetworkCallbackHost : public IRemoteStub<ILocatorCallback> {
public:
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
};
} // namespace Location
} // namespace OHOS
#endif // NETWORK_CALLBACK_HOST_H
#endif // FEATURE_NETWORK_SUPPORT
