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
#include "locator_background_proxy.h"
#include "locator_callback_proxy.h"
#include "locator_skeleton.h"

class LocatorServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    bool StartAndStopForLocating(OHOS::MessageParcel& data);
    void SetStartUpConfirmed(bool isAuthorized);
    void ChangedLocationMode(bool isEnable);
    OHOS::sptr<OHOS::Location::LocatorProxy> proxy_;
    OHOS::sptr<OHOS::Location::ILocatorCallback> callbackStub_;
    std::shared_ptr<OHOS::Location::LocatorBackgroundProxy> backgroundProxy_;
    std::shared_ptr<OHOS::Location::Request> request_;
};
#endif // LOCATOR_SERVICE_TEST_H
