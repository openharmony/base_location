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

#ifndef LOCATOR_IMPL_TEST_H
#define LOCATOR_IMPL_TEST_H

#include <gtest/gtest.h>

#include "i_locator_callback.h"

#define private public
#define protected public
#include "locator_impl.h"
#undef protected
#undef private
#include "fence_impl.h"

#ifdef FEATURE_GEOCODE_SUPPORT
#include "geo_coding_mock_info.h"
#endif
namespace OHOS {
namespace Location {
class LocatorImplTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    void MockNativePermission();
    void LoadSystemAbility();
#ifdef FEATURE_GEOCODE_SUPPORT
    std::vector<std::shared_ptr<GeocodingMockInfo>> SetGeocodingMockInfo();
#endif

    sptr<ILocatorCallback> callbackStub_;
    std::shared_ptr<LocatorImpl> locatorImpl_;
    std::shared_ptr<FenceImpl> fenceImpl_;
    uint64_t tokenId_;
};

class MockCallbackResumeManager : public ICallbackResumeManager {
public:
    MockCallbackResumeManager() = default;
    ~MockCallbackResumeManager() = default;
    void ResumeCallback() override
    {
        return;
    };
};
} // namespace Location
} // namespace OHOS
#endif // LOCATOR_IMPL_TEST_H