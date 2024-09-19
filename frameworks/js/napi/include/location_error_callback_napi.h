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

#ifndef LOCATION_ERROR_CALLBACK_NAPI_H
#define LOCATION_ERROR_CALLBACK_NAPI_H
#include "iremote_stub.h"
#include "napi/native_api.h"
#include "uv.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator_callback.h"
#include "location.h"

namespace OHOS {
namespace Location {
class LocationErrorCallbackNapi : public IRemoteStub<ILocatorCallback> {
public:
    LocationErrorCallbackNapi();
    virtual ~LocationErrorCallbackNapi();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    napi_value PackResult(bool switchState);
    bool Send(int switchState);
    void DeleteHandler();
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);

    template <typename T>
    bool InitContext(T* context)
    {
        if (context == nullptr) {
            LBSLOGE(LOCATION_ERR_CALLBACK, "context == nullptr.");
            return false;
        }
        uint32_t refCount = INVALID_REF_COUNT;
        napi_reference_ref(env_, handlerCb_, &refCount);
        context->env = env_;
        context->callback[SUCCESS_CALLBACK] = handlerCb_;
        return true;
    }

    inline napi_env GetEnv() const
    {
        return env_;
    }

    inline void SetEnv(const napi_env& env)
    {
        env_ = env;
    }

    inline napi_ref GetHandleCb() const
    {
        return handlerCb_;
    }

    inline void SetHandleCb(const napi_ref& handlerCb)
    {
        handlerCb_ = handlerCb;
    }

private:
    napi_env env_;
    napi_ref handlerCb_;
    std::mutex mutex_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_ERROR_CALLBACK_NAPI_H
