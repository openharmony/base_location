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

#ifndef NETMANAGER_BASE_NET_CONN_CALLBACK_OBSERVER_H
#define NETMANAGER_BASE_NET_CONN_CALLBACK_OBSERVER_H
#ifdef FEATURE_GNSS_SUPPORT
#ifdef NET_MANAGER_ENABLE

#include "net_all_capabilities.h"
#include "net_conn_callback_stub.h"

namespace OHOS {
namespace Location {
class NetConnObserver : public NetManagerStandard::NetConnCallbackStub {
public:
    int32_t NetAvailable(sptr<NetManagerStandard::NetHandle> &netHandle) override;

    int32_t NetCapabilitiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
        const sptr<NetManagerStandard::NetAllCapabilities> &netAllCap) override;

    int32_t NetConnectionPropertiesChange(sptr<NetManagerStandard::NetHandle> &netHandle,
        const sptr<NetManagerStandard::NetLinkInfo> &info) override;

    int32_t NetLost(sptr<NetManagerStandard::NetHandle> &netHandle) override;

    int32_t NetUnavailable() override;

    int32_t NetBlockStatusChange(sptr<NetManagerStandard::NetHandle> &netHandle, bool blocked) override;
};
} // namespace Location
} // namespace OHOS

#endif
#endif // NET_MANAGER_ENABLE
#endif // NETMANAGER_BASE_NET_CONN_CALLBACK_OBSERVER_H
