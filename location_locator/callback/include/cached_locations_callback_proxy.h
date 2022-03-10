/*
 * Copyright (C) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_CACHED_LOCATIONS_CALLBACK_PROXY_H
#define OHOS_CACHED_LOCATIONS_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "i_cached_locations_callback.h"

namespace OHOS {
namespace Location {
class CachedLocationsCallbackProxy : public IRemoteProxy<ICachedLocationsCallback> {
public:
    explicit CachedLocationsCallbackProxy(const sptr<IRemoteObject> &impl);
    ~CachedLocationsCallbackProxy() = default;
    void OnCacheLocationsReport(const std::vector<std::unique_ptr<Location>>& locations) override;
private:
    static inline BrokerDelegator<CachedLocationsCallbackProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_CACHED_LOCATIONS_CALLBACK_PROXY_H