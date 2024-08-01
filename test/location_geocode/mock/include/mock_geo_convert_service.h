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

#ifndef MOCK_GEO_CONVERT_SERVICE_H
#define MOCK_GEO_CONVERT_SERVICE_H
#ifdef FEATURE_GEOCODE_SUPPORT

#include <mutex>
#include <singleton.h>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "if_system_ability_manager.h"
#include "iremote_object.h"
#include "message_parcel.h"
#include "message_option.h"
#include "system_ability.h"

#include "common_utils.h"
#include "constant_definition.h"
#include "geo_coding_mock_info.h"
#include "geo_convert_skeleton.h"
#include "geo_convert_service.h"

namespace OHOS {
namespace Location {
class MockGeoConvertService : public GeoConvertService {
public:
    MockGeoConvertService() {}
    ~MockGeoConvertService() {}
    MOCK_METHOD(bool, Init, ());
    MOCK_METHOD(bool, IsConnect, ());
    MOCK_METHOD(bool, GetService, ());
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GEOCODE_SUPPORT
#endif // MOCK_GEO_CONVERT_SERVICE_H