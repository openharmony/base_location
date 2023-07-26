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

#ifndef LOCATION_I_LOCATING_REQUIRED_DATA_CALLBACK_H
#define LOCATION_I_LOCATING_REQUIRED_DATA_CALLBACK_H

#include "iremote_broker.h"
#include "locating_required_data.h"

namespace OHOS {
namespace Location {
class ILocatingRequiredDataCallback : public IRemoteBroker {
public:
    enum {
        RECEIVE_INFO_EVENT = 1,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"location.ILocatingRequiredDataCallback");
    /*
     * locating data report to kits
     */
    virtual void OnLocatingDataChange(const std::vector<std::shared_ptr<LocatingRequiredData>>& data) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_I_LOCATING_REQUIRED_DATA_CALLBACK_H
