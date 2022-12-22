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

#ifndef REQUEST_MANAGER_TEST_H
#define REQUEST_MANAGER_TEST_H

#include <gtest/gtest.h>

#include "i_locator_callback.h"
#include "request.h"
#include "request_manager.h"

namespace OHOS {
namespace Location {
class RequestManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    void MockNativePermission();
    void FillRequestField(std::shared_ptr<Request>& request);
    void VerifyRequestField(std::shared_ptr<Request>& request);

    std::shared_ptr<RequestManager> requestManager_;
    std::shared_ptr<Request> request_;
    sptr<ILocatorCallback> callback_;
    uint64_t tokenId_;
};
} // namespace Location
} // namespace OHOS
#endif // REQUEST_MANAGER_TEST_H