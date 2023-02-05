/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifdef FEATURE_PASSIVE_SUPPORT
#ifndef PASSIVE_ABILITY_PROXY_H
#define PASSIVE_ABILITY_PROXY_H

#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "constant_definition.h"
#include "location.h"
#include "passive_ability_skeleton.h"
#include "work_record.h"

namespace OHOS {
namespace Location {
class PassiveAbilityProxy : public IRemoteProxy<IPassiveAbility> {
public:
    explicit PassiveAbilityProxy(const sptr<IRemoteObject> &impl);
    ~PassiveAbilityProxy() = default;
    LocationErrCode SendLocationRequest(WorkRecord &workrecord) override;
    LocationErrCode SetEnable(bool state) override;
    LocationErrCode EnableMock() override;
    LocationErrCode DisableMock() override;
    LocationErrCode SetMocked(const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) override;
private:
    static inline BrokerDelegator<PassiveAbilityProxy> delegator_;
};
} // namespace Location
} // namespace OHOS
#endif // PASSIVE_ABILITY_PROXY_H
#endif // FEATURE_PASSIVE_SUPPORT
