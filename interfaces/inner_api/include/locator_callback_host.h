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

#ifndef LOCATOR_CALLBACK_HOST_H
#define LOCATOR_CALLBACK_HOST_H

#include <shared_mutex>
#include "common_utils.h"
#include "i_locator_callback.h"
#include "iremote_stub.h"
#include "napi/native_api.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
class LocatorCallbackHost : public IRemoteStub<ILocatorCallback> {
public:
    LocatorCallbackHost();
    virtual ~LocatorCallbackHost();
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void DoSendWork(uv_loop_s *&loop, uv_work_t *&work);
    bool Send(std::unique_ptr<Location>& location);
    void DoSendErrorCode(uv_loop_s *&loop, uv_work_t *&work);
    bool SendErrorCode(const int& errorCode);

    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    void DeleteHandler();
    void DeleteSuccessHandler();
    void DeleteFailHandler();
    void DeleteCompleteHandler();
    void InitLatch();
    bool IsSystemGeoLocationApi();
    bool IsSingleLocationRequest();
    void CountDown();
    void Wait(int time);
    int GetCount();
    void SetCount(int count);

    napi_env m_env;
    napi_ref m_handlerCb;
    napi_ref m_successHandlerCb;
    napi_ref m_failHandlerCb;
    napi_ref m_completeHandlerCb;
    int m_fixNumber;
    napi_deferred m_deferred;
    std::shared_mutex m_mutex;
    CountDownLatch* m_latch;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_CALLBACK_HOST_H
