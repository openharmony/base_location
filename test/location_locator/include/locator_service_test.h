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

#ifndef LOCATOR_SERVICE_TEST_H
#define LOCATOR_SERVICE_TEST_H

#include <gtest/gtest.h>

#ifdef FEATURE_GEOCODE_SUPPORT
#include "geocoding_mock_info.h"
#endif
#include "i_locator_callback.h"
#define private public
#define protected public
#include "locator_ability.h"
#undef protected
#undef private
#include "locator_background_proxy.h"
#include "locator_proxy.h"
#include "request.h"

namespace OHOS {
namespace Location {
class LocatorServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    void SetStartUpConfirmed(bool isAuthorized);
    void ChangedLocationMode(bool isEnable);
    void MockNativePermission();
    void LoadSystemAbility();
#ifdef FEATURE_GEOCODE_SUPPORT
    std::vector<std::shared_ptr<GeocodingMockInfo>> SetGeocodingMockInfo();
#endif

    sptr<ILocatorCallback> callbackStub_;
    LocatorBackgroundProxy* backgroundProxy_;
    std::shared_ptr<Request> request_;
    uint64_t tokenId_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_SERVICE_TEST_H
