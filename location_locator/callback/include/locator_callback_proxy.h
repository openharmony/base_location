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

#ifndef OHOS_LOCATION_LOCATOR_CALLBACK_PROXY_H
#define OHOS_LOCATION_LOCATOR_CALLBACK_PROXY_H

#include "i_locator_callback.h"

#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Location {
class LocatorCallbackProxy : public IRemoteProxy<ILocatorCallback> {
public:
    explicit LocatorCallbackProxy(const sptr<IRemoteObject> &impl);
    ~LocatorCallbackProxy() = default;
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
private:
    static inline BrokerDelegator<LocatorCallbackProxy> delegator_;
};

class LocatorCallbackStub : public IRemoteStub<ILocatorCallback> {
public:
    int32_t OnRemoteRequest(uint32_t code,
        MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
private:
    static pid_t lastCallingPid;
    static pid_t lastCallingUid;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_LOCATOR_CALLBACK_PROXY_H