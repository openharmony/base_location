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

#ifndef LOCATING_REQUIRED_DATA_CALLBACK_NAPI_H
#define LOCATING_REQUIRED_DATA_CALLBACK_NAPI_H

#include "i_locating_required_data_callback.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include "message_option.h"
#include "common_utils.h"
#include "napi/native_api.h"
#include "uv.h"

namespace OHOS {
namespace Location {
bool FindRequiredDataCallback(napi_ref cb);
void DeleteRequiredDataCallback(napi_ref cb);
class LocatingRequiredDataCallbackNapi : public IRemoteStub<ILocatingRequiredDataCallback> {
public:
    LocatingRequiredDataCallbackNapi();
    virtual ~LocatingRequiredDataCallbackNapi();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool IsRemoteDied();
    bool Send(const std::vector<std::shared_ptr<LocatingRequiredData>>& data);
    void OnLocatingDataChange(const std::vector<std::shared_ptr<LocatingRequiredData>>& data) override;
    void DeleteHandler();
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);
    bool IsSingleLocationRequest();
    void CountDown();
    void Wait(int time);
    int GetCount();
    void SetCount(int count);
    void InitLatch();
    void ClearSingleResult();
    void SetSingleResult(
        std::vector<std::shared_ptr<LocatingRequiredData>> singleResult);
    napi_ref GetHandleCb();
    void SetHandleCb(const napi_ref& handlerCb);
    napi_env GetEnv();
    void SetEnv(const napi_env& env);
    
    template <typename T>
    bool InitContext(T* context)
    {
        if (context == nullptr) {
            LBSLOGE(LOCATING_DATA_CALLBACK, "context == nullptr.");
            return false;
        }
        context->env = env_;
        context->callback[SUCCESS_CALLBACK] = handlerCb_;
        return true;
    }

    inline bool GetRemoteDied() const
    {
        return remoteDied_;
    }

    inline void SetRemoteDied(const bool remoteDied)
    {
        remoteDied_ = remoteDied;
    }

    inline std::vector<std::shared_ptr<LocatingRequiredData>> GetSingleResult()
    {
        std::unique_lock<std::mutex> guard(singleResultMutex_);
        return singleResult_;
    }

    inline int GetFixNumber() const
    {
        return fixNumber_;
    }

    inline void SetFixNumber(const int fixNumber)
    {
        fixNumber_ = fixNumber;
    }
private:
    int fixNumber_;
    napi_env env_;
    napi_ref handlerCb_;
    bool remoteDied_;
    std::mutex mutex_;
    std::mutex singleResultMutex_;
    CountDownLatch* latch_;
    std::vector<std::shared_ptr<LocatingRequiredData>> singleResult_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATING_REQUIRED_DATA_CALLBACK_NAPI_H
