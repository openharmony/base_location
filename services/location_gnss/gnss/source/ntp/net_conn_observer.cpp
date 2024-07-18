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

#ifdef FEATURE_GNSS_SUPPORT
#ifdef NET_MANAGER_ENABLE
#include "net_conn_observer.h"

#include "location_log.h"
#include "ntp_time_helper.h"

using namespace OHOS::NetManagerStandard;

namespace OHOS {
namespace Location {
int32_t NetConnObserver::NetAvailable(sptr<NetHandle> &netHandle)
{
    return ERR_OK;
}

int32_t NetConnObserver::NetCapabilitiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetAllCapabilities> &netAllCap)
{
    LBSLOGI(GNSS, "Observe network capabilities change");
    if (netAllCap == nullptr) {
        LBSLOGE(GNSS, "Observe network netAllCap is null");
        return -1;
    }
    if (netAllCap->netCaps_.count(NetCap::NET_CAPABILITY_INTERNET)) {
#ifdef TIME_SERVICE_ENABLE
        auto npTimeHelper = NtpTimeHelper::GetInstance();
        if (npTimeHelper != nullptr) {
            npTimeHelper->RetrieveAndInjectNtpTime();
        }
#endif
    }
    return ERR_OK;
}

int32_t NetConnObserver::NetConnectionPropertiesChange(sptr<NetHandle> &netHandle,
    const sptr<NetLinkInfo> &info)
{
    return ERR_OK;
}

int32_t NetConnObserver::NetLost(sptr<NetHandle> &netHandle)
{
    return ERR_OK;
}

int32_t NetConnObserver::NetUnavailable()
{
    return ERR_OK;
}

int32_t NetConnObserver::NetBlockStatusChange(sptr<NetHandle> &netHandle, bool blocked)
{
    return ERR_OK;
}
} // namespace Location
} // namespace OHOS

#endif
#endif