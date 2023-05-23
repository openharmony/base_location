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

#ifndef LOCATION_DATA_RDB_HELPER_H
#define LOCATION_DATA_RDB_HELPER_H
#include <memory>
#include <utility>
#include <singleton.h>

#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "iremote_object.h"
#include "result_set.h"
#include "uri.h"

#include "constant_definition.h"

namespace OHOS {
namespace Location {
class LocationDataRdbHelper : DelayedSingleton<LocationDataRdbHelper> {
public:
    LocationDataRdbHelper();
    ~LocationDataRdbHelper();
    LocationErrCode RegisterDataObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    LocationErrCode UnregisterDataObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    LocationErrCode GetValue(Uri &uri, const std::string &column, int32_t &value);
    LocationErrCode SetValue(Uri &uri, const std::string &column, int &value);
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper();
private:
    void Initialize();

    sptr<IRemoteObject> remoteObj_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_DATA_RDB_HELPER_H
