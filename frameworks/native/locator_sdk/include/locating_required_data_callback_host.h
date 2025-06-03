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

#ifndef LOCATING_REQUIRED_DATA_CALLBACK_HOST_H
#define LOCATING_REQUIRED_DATA_CALLBACK_HOST_H

#include "i_locating_required_data_callback.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include "message_option.h"

namespace OHOS {
namespace Location {
class LocatingRequiredDataCallbackHost : public IRemoteStub<ILocatingRequiredDataCallback> {
public:
    LocatingRequiredDataCallbackHost();
    virtual ~LocatingRequiredDataCallbackHost();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool IsRemoteDied();
    void OnLocatingDataChange(const std::vector<std::shared_ptr<LocatingRequiredData>>& data) override;
    bool IsSingleLocationRequest();
    void ClearSingleResult();
    void SetSingleResult(
        std::vector<std::shared_ptr<LocatingRequiredData>> singleResult);
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
    bool remoteDied_;
    std::mutex mutex_;
    std::mutex singleResultMutex_;
    std::vector<std::shared_ptr<LocatingRequiredData>> singleResult_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATING_REQUIRED_DATA_CALLBACK_HOST_H
