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

#ifndef OHOS_LOCATION_SWITCH_CALLBACK_H
#define OHOS_LOCATION_SWITCH_CALLBACK_H

#include "iremote_broker.h"
#include "ipc_types.h"

namespace OHOS {
namespace Location {
class ISwitchCallback : public IRemoteBroker {
public:
    enum {
        RECEIVE_SWITCH_STATE_EVENT = 1,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"location.ISwitchCallback");
    /*
     * state changed report to kits
     */
    virtual void OnSwitchChange(const int state) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // OHOS_LOCATION_SWITCH_CALLBACK_H