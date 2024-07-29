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

#ifndef LOCATION_DATA_MANAGER_H
#define LOCATION_DATA_MANAGER_H

#include <map>
#include <atomic>
#include <singleton.h>
#include "iremote_object.h"
#include "constant_definition.h"
#include "i_switch_callback.h"
#include "common_utils.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Location {
class DataShareSystemAbilityListener : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};
class LocationDataManager {
public:
    LocationDataManager();
    ~LocationDataManager();
    LocationErrCode ReportSwitchState(bool isEnabled);
    LocationErrCode RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid);
    LocationErrCode UnregisterSwitchCallback(const sptr<IRemoteObject>& callback);
    void SetCachedSwitchState(int32_t state);
    bool IsSwitchStateReg();
    void ResetIsObserverReg();
    void RegisterDatashareObserver();
    static LocationDataManager* GetInstance();

private:
    std::mutex mutex_;
    std::mutex switchStateMutex_;
    std::vector<sptr<ISwitchCallback>> switchCallbacks_;
    int32_t cachedSwitchState_ = DISABLED;
    bool isStateCached_ = false;
    std::atomic_bool isObserverReg_ = false;
    sptr<ISystemAbilityStatusChange> saStatusListener_ =
        sptr<DataShareSystemAbilityListener>(new DataShareSystemAbilityListener());
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_DATA_MANAGER_H
