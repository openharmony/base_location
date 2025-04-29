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

#ifndef GEO_CONVERT_SERVICE_TEST_H
#define GEO_CONVERT_SERVICE_TEST_H
#ifdef FEATURE_GEOCODE_SUPPORT

#include <gtest/gtest.h>

#include "common_utils.h"
#include "geo_convert_proxy.h"
#include "geo_convert_service.h"
#include "geo_convert_request.h"

namespace OHOS {
namespace Location {
class GeoConvertServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    bool Available();
    void MockNativePermission();
    
    std::shared_ptr<GeoConvertProxy> proxy_;
    sptr<GeoConvertService> service_;
    GeoConvertRequest GeoConvertRequest_;
    bool available_;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GEOCODE_SUPPORT
#endif // GEO_CONVERT_SERVICE_TEST_H
