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
#include <singleton.h>
#include "iremote_object.h"
#include "constant_definition.h"
#include "i_switch_callback.h"
#include "common_utils.h"
#include "app_identity.h"
#include "common_event_subscriber.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Location {

typedef struct {
    AppIdentity appIdentity;
    int lastState;
} AppSwitchState;

class LocationDataManager {
public:
    LocationDataManager();
    ~LocationDataManager();
    LocationErrCode ReportSwitchState(bool isEnabled);
    LocationErrCode RegisterSwitchCallback(const sptr<IRemoteObject>& callback, AppIdentity& appIdentity);
    LocationErrCode UnregisterSwitchCallback(const sptr<IRemoteObject>& callback);
    bool IsSwitchObserverReg();
    void SetIsSwitchObserverReg(bool isSwitchObserverReg);
    bool IsFirstReport();
    void SetIsFirstReport(bool isFirstReport);
    void RegisterLocationSwitchObserver();
    static LocationDataManager* GetInstance();

private:
    std::mutex mutex_;
    std::mutex isSwitchObserverRegMutex_;
    bool isSwitchObserverReg_ = false;
    std::map<sptr<IRemoteObject>, AppSwitchState > switchCallbackMap_;

    std::mutex isFirstReportMutex_;
    bool isFirstReport_ = true;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_DATA_MANAGER_H
