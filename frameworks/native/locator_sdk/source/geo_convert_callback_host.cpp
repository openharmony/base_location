/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "geo_convert_callback_host.h"

#include <sys/time.h>
#include "common_utils.h"
#include "location_log.h"
#include "parameters.h"

namespace OHOS {
namespace Location {
static const int MAX_RESULT = 10;
const int GEOCODE_TIME_OUT = 3;
int GeoConvertCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LBSLOGD(GEO_CONVERT, "GeoConvertCallbackHost::OnRemoteRequest, code=%{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LBSLOGE(GEO_CONVERT, "invalid token.");
        return -1;
    }
    switch (code) {
        case RECEIVE_GEOCODE_INFO_EVENT: {
            int errCode = data.ReadInt32();
            if (errCode != 0) {
                LBSLOGE(GEO_CONVERT, "something wrong, errCode = %{public}d", errCode);
                break;
            }
            int cnt = data.ReadInt32();
            if (cnt > MAX_RESULT) {
                cnt = MAX_RESULT;
            }
            std::list<std::shared_ptr<GeoAddress>> result;
            for (int i = 0; i < cnt; i++) {
                result.push_back(GeoAddress::Unmarshalling(data));
            }
            OnResults(result);
            break;
        }
        case ERROR_INFO_EVENT: {
            int errCode = data.ReadInt32();
            OnErrorReport(errCode);
            break;
        }
        default: {
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
        }
    }
    return 0;
}

void GeoConvertCallbackHost::OnResults(std::list<std::shared_ptr<GeoAddress>> &results)
{
    LBSLOGD(GEO_CONVERT, "GeoConvertCallbackHost::OnResults");
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    result_ = results;
    ready_ = true;
    condition_.notify_all();
}

void GeoConvertCallbackHost::OnErrorReport(const int errorCode)
{
    LBSLOGD(GEO_CONVERT, "GeoConvertCallbackHost::OnErrorReport, errCode = %{public}d", errorCode);
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    ready_ = true;
    condition_.notify_all();
}

std::list<std::shared_ptr<GeoAddress>> GeoConvertCallbackHost::GetResult()
{
    LBSLOGD(GEO_CONVERT, "GeoConvertCallbackHost::GetResult");
    std::list<std::shared_ptr<GeoAddress>> result;
    std::unique_lock<std::mutex> uniqueLock(mutex_);
    int time = system::GetIntParameter("const.location.geocode_timeout", GEOCODE_TIME_OUT);
    auto waitStatus = condition_.wait_for(
        uniqueLock, std::chrono::seconds(time), [this]() { return ready_; });
    ready_ = false;
    if (!waitStatus) {
        LBSLOGE(GEO_CONVERT, "GetResult() timeout!");
        return result;
    } else {
        return result_;
    }
}
} // namespace Location
} // namespace OHOS