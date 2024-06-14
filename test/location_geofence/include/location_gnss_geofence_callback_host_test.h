/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef LOCATION_GNSS_GEOFENCE_CALLBACK_HOST_TEST_H
#define LOCATION_GNSS_GEOFENCE_CALLBACK_HOST_TEST_H

#include <gtest/gtest.h>

namespace OHOS {
namespace Location {
class LocationGnssGeofenceCallbackHostTest : public testing::Test {
public:
    void SetUp();
    void TearDown();

    const int DEFAULT_NUM_VALUE = 1;
    const int DEFAULT_CALLBACK_WAIT_TIME = 100;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_GNSS_GEOFENCE_CALLBACK_HOST_TEST_H