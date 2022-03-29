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

#ifndef OHOS_LOCATOR_CALLBACK_HOST_H
#define OHOS_LOCATOR_CALLBACK_HOST_H
#include <shared_mutex>
#include "i_locator_callback.h"
#include "iremote_stub.h"
#include "napi/native_api.h"
#include "location_util.h"

namespace OHOS {
namespace Location {
class LocatorCallbackHost : public IRemoteStub<ILocatorCallback> {
public:
    LocatorCallbackHost();
    virtual ~LocatorCallbackHost();
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool Send(std::unique_ptr<Location>& location);
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    void DeleteHandler();

    pid_t m_lastCallingPid;
    pid_t m_lastCallingUid;
    napi_env m_env;
    napi_ref m_handlerCb;
    int m_fixNumber;
    napi_deferred m_deferred;
    std::shared_mutex m_mutex;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATOR_CALLBACK_HOST_H
