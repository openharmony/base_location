/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef LOCATING_REQUIRED_DATA_CALLBACK_PROXY_H
#define LOCATING_REQUIRED_DATA_CALLBACK_PROXY_H

#include <vector>

#include "iremote_proxy.h"
#include "iremote_object.h"

#include "i_locating_required_data_callback.h"

namespace OHOS {
namespace Location {
class LocatingRequiredDataCallbackProxy : public IRemoteProxy<ILocatingRequiredDataCallback> {
public:
    explicit LocatingRequiredDataCallbackProxy(const sptr<IRemoteObject> &impl);
    ~LocatingRequiredDataCallbackProxy() = default;
    void OnLocatingDataChange(const std::vector<std::shared_ptr<LocatingRequiredData>>& data) override;
private:
    static inline BrokerDelegator<LocatingRequiredDataCallbackProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATING_REQUIRED_DATA_CALLBACK_PROXY_H
