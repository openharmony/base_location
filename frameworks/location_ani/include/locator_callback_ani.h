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

#ifndef LOCATOR_CALLBACK_ANI_H
#define LOCATOR_CALLBACK_ANI_H

#include "iremote_stub.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "i_locator_callback.h"
#include "location.h"

namespace OHOS {
namespace Location {
class LocatorCallbackAni : public IRemoteStub<ILocatorCallback> {
public:
    LocatorCallbackAni();
    virtual ~LocatorCallbackAni();
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    bool SendErrorCode(const int& errorCode);

    void OnLocationReport(const std::unique_ptr<Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    void DeleteAllCallbacks();
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

    inline int GetFixNumber() const
    {
        return fixNumber_;
    }

    inline void SetFixNumber(const int fixNumber)
    {
        fixNumber_ = fixNumber;
    }

    inline void SetLocationPriority(const int locationPriority)
    {
        locationPriority_ = locationPriority;
    }

    inline int GetLocationPriority()
    {
        return locationPriority_;
    }

    inline std::shared_ptr<Location> GetSingleLocation()
    {
        std::unique_lock<std::mutex> guard(mutex_);
        return singleLocation_;
    }
    bool NeedSetSingleLocation(const std::unique_ptr<Location>& location);
    bool IfReportAccuracyLocation();
    void SetSingleLocation(const std::unique_ptr<Location>& location);

private:
    int fixNumber_;
    std::mutex mutex_;
    CountDownLatch* latch_;
    std::shared_ptr<Location> singleLocation_;
    int locationPriority_;
    bool inHdArea_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_CALLBACK_ANI_H
