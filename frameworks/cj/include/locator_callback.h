/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef LOCATOR_CALLBACK_H
#define LOCATOR_CALLBACK_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "i_locator_callback.h"
#include "locator_callback_napi.h"
#include "location.h"
#include "geolocationmanager_utils.h"

namespace OHOS {
namespace GeoLocationManager {
class LocatorCallback : public IRemoteStub<Location::ILocatorCallback> {
public:
    LocatorCallback();
    LocatorCallback(int64_t callbackId);
    virtual ~LocatorCallback();
    virtual int OnRemoteRequest(uint32_t code,
        MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

    void OnLocationReport(const std::unique_ptr<Location::Location>& location) override;
    void OnLocatingStatusChange(const int status) override;
    void OnErrorReport(const int errorCode) override;
    void InitLatch();
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

    inline std::shared_ptr<Location::Location> GetSingleLocation()
    {
        std::unique_lock<std::mutex> guard(mutex_);
        return singleLocation_;
    }

    int64_t GetCallBackId()
    {
        return callbackId_;
    };

    bool NeedSetSingleLocation(const std::unique_ptr<Location::Location>& location);
    bool IfReportAccuracyLocation();
    void SetSingleLocation(const std::unique_ptr<Location::Location>& location);

private:
    int64_t callbackId_ = -1;
    std::function<void(const std::unique_ptr<Location::Location>& location)> callback_ = nullptr;
    int fixNumber_;
    std::mutex mutex_;
    Location::CountDownLatch* latch_;
    std::shared_ptr<Location::Location> singleLocation_;
    int locationPriority_;
    bool inHdArea_;
    bool callbackValid_;
};
}
}

#endif // LOCATOR_CALLBACK_H