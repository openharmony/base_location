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

#ifndef OHOS_LOCATION_SA_LOAD_MANAGER_H
#define OHOS_LOCATION_SA_LOAD_MANAGER_H

#include "iremote_object.h"

#include "constant_definition.h"
#include "single_instance.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace Location {
class LocationSaLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

class LocationSaLoadManager {
    DECLARE_SINGLE_INSTANCE(LocationSaLoadManager);
public:
    LocationErrCode LoadLocationSa(int32_t systemAbilityId);
    LocationErrCode UnloadLocationSa(int32_t systemAbilityId);
};
}  // namespace Location
}  // namespace OHOS
#endif // OHOS_LOCATION_SA_LOAD_MANAGER_H