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

#ifndef OHOS_LOCATION_GEOCONVERT_UNITTEST
#define OHOS_LOCATION_GEOCONVERT_UNITTEST

#include <gtest/gtest.h>
#include "geo_convert_skeleton.h"
#include "geo_convert_proxy.h"

class GeoConvertServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    bool IsDeviceSupport();
    bool Available();
    OHOS::sptr<OHOS::Location::GeoConvertProxy> proxy_;
    bool available_;
};
#endif