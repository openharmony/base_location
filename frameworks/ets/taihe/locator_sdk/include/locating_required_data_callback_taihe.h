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

#ifndef LOCATING_REQUIRED_DATA_CALLBACK_TAIHE_H
#define LOCATING_REQUIRED_DATA_CALLBACK_TAIHE_H

#include "i_locating_required_data_callback.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include "message_option.h"
#include "common_utils.h"
#include "ohos.geoLocationManager.proj.hpp"
#include "ohos.geoLocationManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Location {
class LocatingRequiredDataCallbackTaihe : public IRemoteStub<ILocatingRequiredDataCallback> {
public:
    LocatingRequiredDataCallbackTaihe();
    virtual ~LocatingRequiredDataCallbackTaihe();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnLocatingDataChange(const std::vector<std::shared_ptr<LocatingRequiredData>>& data) override;
    void ClearSingleResult();
    void SetSingleResult(
        std::vector<std::shared_ptr<LocatingRequiredData>> singleResult);
    bool IsSingleLocationRequest();
    inline std::vector<std::shared_ptr<LocatingRequiredData>> GetSingleResult()
    {
        return singleResult_;
    }
    void CountDown();
    void Wait(int time);
    int GetCount();
    void SetCount(int count);
    void InitLatch();
    inline int GetFixNumber() const
    {
        return fixNumber_;
    }

    inline void SetFixNumber(const int fixNumber)
    {
        fixNumber_ = fixNumber;
    }
    std::vector<std::shared_ptr<LocatingRequiredData>> singleResult_;
    ::taihe::optional<::taihe::callback<
        void(::taihe::array_view<::ohos::geoLocationManager::LocatingRequiredData>)>> callback_;
    int fixNumber_;
    bool remoteDied_;
    std::mutex mutex_;
    std::mutex singleResultMutex_;
    CountDownLatch* latch_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATING_REQUIRED_DATA_CALLBACK_TAIHE_H
