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

#ifndef MOCK_GEO_CONVERT_CALLBACK_HOST_H
#define MOCK_GEO_CONVERT_CALLBACK_HOST_H
#ifdef FEATURE_GEOCODE_SUPPORT

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "system_ability.h"

#include "common_utils.h"
#include "geo_convert_callback_host.h"

namespace OHOS {
namespace Location {
class MockGeoConvertCallbackHost : public GeoConvertCallbackHost {
public:
    MockGeoConvertCallbackHost() {}
    ~MockGeoConvertCallbackHost() {}
    MOCK_METHOD(void, OnResults, (std::list<std::shared_ptr<GeoAddress>> &results));
    MOCK_METHOD(void, OnErrorReport, (const int errorCode));
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GEOCODE_SUPPORT
#endif // MOCK_GEO_CONVERT_CALLBACK_HOST_H