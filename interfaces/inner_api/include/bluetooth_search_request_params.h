/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SEARCH_REQUEST_PARAMS_H
#define BLUETOOTH_SEARCH_REQUEST_PARAMS_H

#include <parcel.h>
#include <string>
#include "string_ex.h"

namespace OHOS {
namespace Location {
class BluetoothSearchRequestParams : public Parcelable {
public:
    BluetoothSearchRequestParams();

    ~BluetoothSearchRequestParams() override;


    int32_t rssiThreshold_;
    std::vector<std::string> deviceArray_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_BLUETOOTH_SCAN_RESULT_H